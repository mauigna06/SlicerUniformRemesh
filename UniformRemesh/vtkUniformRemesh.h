#ifndef vtkUniformRemesh_h
#define vtkUniformRemesh_h

#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>

class vtkUniformRemesh : public vtkPolyDataAlgorithm
{
public:
  static vtkUniformRemesh* New();
  vtkTypeMacro(vtkUniformRemesh, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSetClampMacro(NumberOfIterations, int, 1, VTK_INT_MAX);
  vtkGetMacro(NumberOfIterations, int);

  vtkSetClampMacro(DesiredEdgeLength, double, 1e-6, VTK_DOUBLE_MAX);
  vtkGetMacro(DesiredEdgeLength, double);

  vtkSetMacro(ReprojectToInputSurface, bool);
  vtkGetMacro(ReprojectToInputSurface, bool);
  vtkBooleanMacro(ReprojectToInputSurface, bool);

protected:
  vtkUniformRemesh();
  ~vtkUniformRemesh() override = default;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector) override;

private:
  vtkUniformRemesh(const vtkUniformRemesh&) = delete;
  void operator=(const vtkUniformRemesh&) = delete;

  int NumberOfIterations;
  double DesiredEdgeLength;
  bool ReprojectToInputSurface;
};

#endif
