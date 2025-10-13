#include "vtkUniformRemeshFilter.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataObject.h>
#include <vtkFieldData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <unordered_map>
#include <vector>
#include <Eigen/Core>

#include <igl/avg_edge_length.h>

#include "remesher/remesh_botsch.h"

vtkStandardNewMacro(vtkUniformRemeshFilter);

vtkUniformRemeshFilter::vtkUniformRemeshFilter()
{
  this->TargetEdgeLength = 0.0;
  this->NumberOfIterations = 10;
  this->ProjectToInputSurface = false;

  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

void vtkUniformRemeshFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "TargetEdgeLength: " << this->TargetEdgeLength << "\n";
  os << indent << "NumberOfIterations: " << this->NumberOfIterations << "\n";
  os << indent << "ProjectToInputSurface: " << (this->ProjectToInputSurface ? "On" : "Off") << "\n";
}

int vtkUniformRemeshFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port != 0)
    {
    return 0;
    }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

int vtkUniformRemeshFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  if (port != 0)
    {
    return 0;
    }
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

int vtkUniformRemeshFilter::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  (void)request;
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);

  if (!input || !output)
    {
    vtkErrorMacro("Invalid input or output data objects.");
    return 0;
    }

  const vtkIdType numberOfPoints = input->GetNumberOfPoints();
  vtkCellArray* inputPolys = input->GetPolys();
  if (numberOfPoints == 0 || !inputPolys || inputPolys->GetNumberOfCells() == 0)
    {
    output->ShallowCopy(input);
    return 1;
    }

  const vtkIdType numberOfCells = inputPolys->GetNumberOfCells();
  Eigen::MatrixXd vertices(numberOfPoints, 3);
  Eigen::MatrixXi faces(numberOfCells, 3);

  for (vtkIdType pid = 0; pid < numberOfPoints; ++pid)
    {
    double point[3];
    input->GetPoint(pid, point);
    vertices(pid, 0) = point[0];
    vertices(pid, 1) = point[1];
    vertices(pid, 2) = point[2];
    }

  vtkIdType cellPointCount = 0;
  const vtkIdType* cellPoints = nullptr;
  vtkIdType cellId = 0;
  inputPolys->InitTraversal();
  while (inputPolys->GetNextCell(cellPointCount, cellPoints))
    {
    if (cellPointCount != 3)
      {
      vtkErrorMacro("vtkUniformRemeshFilter only supports triangle meshes.");
      return 0;
      }
    faces(cellId, 0) = static_cast<int>(cellPoints[0]);
    faces(cellId, 1) = static_cast<int>(cellPoints[1]);
    faces(cellId, 2) = static_cast<int>(cellPoints[2]);
    ++cellId;
    }

  if (cellId != numberOfCells)
    {
    vtkErrorMacro("Failed to traverse all input cells.");
    return 0;
    }

  double targetEdge = this->TargetEdgeLength;
  if (targetEdge <= 0.0)
    {
    targetEdge = igl::avg_edge_length(vertices, faces);
    }

  std::vector<char> boundaryVertexMask(static_cast<std::size_t>(numberOfPoints), 0);
  std::unordered_map<unsigned long long, int> edgeUseCount;
  edgeUseCount.reserve(static_cast<std::size_t>(numberOfCells * 3));

  const auto encodeEdge = [](int a, int b) -> unsigned long long
    {
    if (a > b)
      {
      std::swap(a, b);
      }
    return (static_cast<unsigned long long>(a) << 32) | static_cast<unsigned int>(b);
    };

  for (Eigen::Index faceId = 0; faceId < faces.rows(); ++faceId)
    {
    const int a = faces(faceId, 0);
    const int b = faces(faceId, 1);
    const int c = faces(faceId, 2);
    ++edgeUseCount[encodeEdge(a, b)];
    ++edgeUseCount[encodeEdge(b, c)];
    ++edgeUseCount[encodeEdge(c, a)];
    }

  for (Eigen::Index faceId = 0; faceId < faces.rows(); ++faceId)
    {
    const int tri[3] = {
      faces(faceId, 0),
      faces(faceId, 1),
      faces(faceId, 2) };
    for (int edgeIndex = 0; edgeIndex < 3; ++edgeIndex)
      {
      const int u = tri[edgeIndex];
      const int v = tri[(edgeIndex + 1) % 3];
      if (edgeUseCount[encodeEdge(u, v)] == 1)
        {
        boundaryVertexMask[static_cast<std::size_t>(u)] = 1;
        boundaryVertexMask[static_cast<std::size_t>(v)] = 1;
        }
      }
    }

  int boundaryCount = 0;
  for (char flag : boundaryVertexMask)
    {
    boundaryCount += flag ? 1 : 0;
    }

  Eigen::VectorXi boundaryVertices;
  if (boundaryCount > 0)
    {
    boundaryVertices.resize(boundaryCount);
    int insertIndex = 0;
    for (vtkIdType vertexId = 0; vertexId < numberOfPoints; ++vertexId)
      {
      if (boundaryVertexMask[static_cast<std::size_t>(vertexId)])
        {
        boundaryVertices(insertIndex++) = static_cast<int>(vertexId);
        }
      }
    }

  try
    {
    if (boundaryCount > 0)
      {
      remesh_botsch(vertices, faces, targetEdge, this->NumberOfIterations, boundaryVertices, this->ProjectToInputSurface);
      }
    else
      {
      remesh_botsch(vertices, faces, targetEdge, this->NumberOfIterations, this->ProjectToInputSurface);
      }
    }
  catch (const std::bad_alloc& allocationError)
    {
    vtkErrorMacro("Remeshing failed due to memory allocation error: " << allocationError.what());
    return 0;
    }
  catch (const std::exception& standardError)
    {
    vtkErrorMacro("Remeshing failed: " << standardError.what());
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Remeshing failed due to an unknown error.");
    return 0;
    }

  vtkNew<vtkPoints> remeshedPoints;
  remeshedPoints->SetNumberOfPoints(static_cast<vtkIdType>(vertices.rows()));
  for (Eigen::Index vid = 0; vid < vertices.rows(); ++vid)
    {
    remeshedPoints->SetPoint(static_cast<vtkIdType>(vid), vertices(vid, 0), vertices(vid, 1), vertices(vid, 2));
    }

  vtkNew<vtkCellArray> remeshedPolys;
  const Eigen::Index triangleCount = faces.rows();
  for (Eigen::Index tid = 0; tid < triangleCount; ++tid)
    {
    vtkIdType triangle[3] = {
      static_cast<vtkIdType>(faces(tid, 0)),
      static_cast<vtkIdType>(faces(tid, 1)),
      static_cast<vtkIdType>(faces(tid, 2)) };
    remeshedPolys->InsertNextCell(3, triangle);
    }

  output->Initialize();
  output->SetPoints(remeshedPoints);
  output->SetPolys(remeshedPolys);
  output->GetPointData()->Initialize();
  output->GetCellData()->Initialize();
  if (input->GetFieldData())
    {
    output->GetFieldData()->ShallowCopy(input->GetFieldData());
    }

  return 1;
}
