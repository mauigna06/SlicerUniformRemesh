#ifndef vtkUniformRemeshFilter_h
#define vtkUniformRemeshFilter_h

#include "vtkUniformRemeshConfigure.h"

#include <vtkPolyDataAlgorithm.h>

class vtkUniformRemesh_EXPORT vtkUniformRemeshFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkUniformRemeshFilter* New();
  vtkTypeMacro(vtkUniformRemeshFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSetMacro(TargetEdgeLength, double);
  vtkGetMacro(TargetEdgeLength, double);

  vtkSetClampMacro(NumberOfIterations, int, 1, 1000);
  vtkGetMacro(NumberOfIterations, int);

  vtkSetMacro(ProjectToInputSurface, bool);
  vtkGetMacro(ProjectToInputSurface, bool);
  vtkBooleanMacro(ProjectToInputSurface, bool);

protected:
  vtkUniformRemeshFilter();
  ~vtkUniformRemeshFilter() override = default;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int FillOutputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkUniformRemeshFilter(const vtkUniformRemeshFilter&) = delete;
  void operator=(const vtkUniformRemeshFilter&) = delete;

  double TargetEdgeLength;
  int NumberOfIterations;
  bool ProjectToInputSurface;
};

#endif
