#include "vtkUniformRemesh.h"

#include <vtkAlgorithm.h>
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

#include <remesher/remesh_botsch.h>

#include <array>
#include <exception>
#include <limits>
#include <vector>

vtkStandardNewMacro(vtkUniformRemesh);

vtkUniformRemesh::vtkUniformRemesh()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);

  this->NumberOfIterations = 10;
  this->DesiredEdgeLength = 0.1;
  this->ReprojectToInputSurface = true;
}

void vtkUniformRemesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfIterations: " << this->NumberOfIterations << "\n";
  os << indent << "DesiredEdgeLength: " << this->DesiredEdgeLength << "\n";
  os << indent << "ReprojectToInputSurface: "
     << (this->ReprojectToInputSurface ? "On" : "Off") << "\n";
}

int vtkUniformRemesh::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port != 0)
  {
    return 0;
  }

  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

int vtkUniformRemesh::RequestData(vtkInformation* vtkNotUsed(request),
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkPolyData* input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!input || !output)
  {
    vtkErrorMacro("Invalid input or output polydata");
    return 0;
  }

  if (input->GetNumberOfPoints() == 0 || input->GetNumberOfCells() == 0)
  {
    output->ShallowCopy(input);
    return 1;
  }

  if (this->DesiredEdgeLength <= 0.0)
  {
    vtkErrorMacro("Desired edge length must be positive");
    return 0;
  }

  if (this->NumberOfIterations <= 0)
  {
    vtkErrorMacro("Number of iterations must be positive");
    return 0;
  }

  vtkCellArray* polys = input->GetPolys();
  if (!polys || polys->GetNumberOfCells() == 0)
  {
    vtkErrorMacro("Input polydata does not contain any polygons");
    return 0;
  }

  std::vector<std::array<vtkIdType, 3>> triangles;
  triangles.reserve(polys->GetNumberOfCells());

  vtkIdType npts = 0;
  const vtkIdType* pts = nullptr;
  for (polys->InitTraversal(); polys->GetNextCell(npts, pts);)
  {
    if (npts != 3)
    {
      vtkErrorMacro("vtkUniformRemesh expects purely triangular meshes");
      return 0;
    }

    triangles.push_back({ { pts[0], pts[1], pts[2] } });
  }

  const vtkIdType numberOfPoints = input->GetNumberOfPoints();

  Eigen::MatrixXd V(numberOfPoints, 3);
  for (vtkIdType pid = 0; pid < numberOfPoints; ++pid)
  {
    double point[3];
    input->GetPoint(pid, point);
    V(pid, 0) = point[0];
    V(pid, 1) = point[1];
    V(pid, 2) = point[2];
  }

  Eigen::MatrixXi F(static_cast<Eigen::Index>(triangles.size()), 3);
  for (Eigen::Index tid = 0; tid < F.rows(); ++tid)
  {
    const auto& tri = triangles[static_cast<std::size_t>(tid)];
    for (int c = 0; c < 3; ++c)
    {
      if (tri[c] < 0 || tri[c] > std::numeric_limits<int>::max())
      {
        vtkErrorMacro("Triangle index exceeds 32-bit integer range required by gpytoolbox");
        return 0;
      }
      F(tid, c) = static_cast<int>(tri[c]);
    }
  }

  try
  {
    remesh_botsch(V, F, this->DesiredEdgeLength, this->NumberOfIterations,
      this->ReprojectToInputSurface);
  }
  catch (const std::exception& exc)
  {
    vtkErrorMacro(<< "gpytoolbox remesh_botsch failed: " << exc.what());
    return 0;
  }
  catch (...)
  {
    vtkErrorMacro("gpytoolbox remesh_botsch failed with an unknown error");
    return 0;
  }

  vtkNew<vtkPoints> remeshedPoints;
  remeshedPoints->SetNumberOfPoints(V.rows());
  for (Eigen::Index pid = 0; pid < V.rows(); ++pid)
  {
    remeshedPoints->SetPoint(pid, V(pid, 0), V(pid, 1), V(pid, 2));
  }

  vtkNew<vtkCellArray> remeshedPolys;
  for (Eigen::Index tid = 0; tid < F.rows(); ++tid)
  {
    vtkIdType tri[3] = {
      static_cast<vtkIdType>(F(tid, 0)),
      static_cast<vtkIdType>(F(tid, 1)),
      static_cast<vtkIdType>(F(tid, 2)) };
    remeshedPolys->InsertNextCell(3, tri);
  }

  output->Initialize();
  output->SetPoints(remeshedPoints);
  output->SetPolys(remeshedPolys);
  output->GetPointData()->Initialize();
  output->GetCellData()->Initialize();
  output->GetFieldData()->ShallowCopy(input->GetFieldData());

  return 1;
}
