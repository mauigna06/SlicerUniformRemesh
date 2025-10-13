#include "UniformRemeshCLP.h"

#include "vtkUniformRemeshFilter.h"

#include <iostream>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelStorageNode.h>
#include <vtkNew.h>
#include <vtkPolyData.h>

namespace
{

int RemeshSurface(const std::string& inputModel,
                  const std::string& outputModel,
                  double targetEdgeLength,
                  int numberOfIterations,
                  bool projectToInputSurface)
{
  vtkNew<vtkMRMLModelStorageNode> inputStorageNode;
  inputStorageNode->SetFileName(inputModel.c_str());
  vtkNew<vtkMRMLModelNode> inputModelNode;
  if (!inputStorageNode->ReadData(inputModelNode))
    {
    std::cerr << "Failed to read input model: " << inputModel << std::endl;
    return EXIT_FAILURE;
    }

  vtkPolyData* inputPolyData = inputModelNode->GetPolyData();
  if (!inputPolyData)
    {
    std::cerr << "Input model does not contain surface geometry." << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkUniformRemeshFilter> remeshFilter;
  remeshFilter->SetInputData(inputPolyData);
  remeshFilter->SetTargetEdgeLength(targetEdgeLength);
  remeshFilter->SetNumberOfIterations(numberOfIterations);
  remeshFilter->SetProjectToInputSurface(projectToInputSurface);
  remeshFilter->Update();

  vtkPolyData* remeshedOutput = remeshFilter->GetOutput();
  if (!remeshedOutput)
    {
    std::cerr << "Remeshing did not produce an output surface." << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkPolyData> remeshedSurface;
  remeshedSurface->DeepCopy(remeshedOutput);

  vtkNew<vtkMRMLModelNode> outputModelNode;
  outputModelNode->SetAndObservePolyData(remeshedSurface);

  vtkNew<vtkMRMLModelStorageNode> outputStorageNode;
  outputStorageNode->SetFileName(outputModel.c_str());
  if (!outputStorageNode->WriteData(outputModelNode))
    {
    std::cerr << "Failed to write output model: " << outputModel << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

} // end anonymous namespace

int main(int argc, char* argv[])
{
  PARSE_ARGS;

  return RemeshSurface(inputModel,
                       outputModel,
                       targetEdgeLength,
                       numberOfIterations,
                       projectToInputSurface);
}
