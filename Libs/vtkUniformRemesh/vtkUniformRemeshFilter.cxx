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

  remesh_botsch(vertices, faces, targetEdge, this->NumberOfIterations, this->ProjectToInputSurface);

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
