# SlicerUniformRemesh

This extension packages the `vtkUniformRemeshFilter` geometry filter and the `UniformRemesh` command-line interface (CLI) for 3D Slicer. It wraps gpytoolbox's uniform remeshing implementation so you can remesh surface models directly inside Slicer, either from the Python console or via the CLI module executable.

## Python Console Usage

Once the extension is built or installed, launch Slicer (or use the extension-specific launcher generated during the build, e.g. `SlicerWithSlicerUniformRemesh`). In the Python console you can instantiate the wrapped filter without modifying `sys.path`:

```python
import vtkUniformRemeshPython as vtkUR

remesh = vtkUR.vtkUniformRemeshFilter()
remesh.SetTargetEdgeLength(0.0)        # 0 uses the average edge length of the input
remesh.SetNumberOfIterations(3)       # 1-1000 iterations
remesh.SetProjectToInputSurface(True)  # optional projection back to the input surface
remesh.SetInputData(modelNode.GetPolyData())
remesh.Update()

outputPolyData = remesh.GetOutput()
```

When working with MRML model nodes, copy the output into a node to visualize it:

```python
remeshedModelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode", "Remeshed")
remeshedModelNode.SetAndObservePolyData(remesh.GetOutput())
```

## CLI Usage

The CLI executable `UniformRemesh` is installed under the extension's `lib/Slicer-*/cli-modules` directory. You can run it from a terminal or via Slicer's GUI (Modules → All Modules → UniformRemesh).

Example command-line invocation:

```bash
./lib/Slicer-5.9/cli-modules/UniformRemesh \
  --inputModel /path/to/input.vtp \
  --outputModel /path/to/output.vtp \
  --targetEdgeLength 1.5 \
  --numberOfIterations 5 \
  --projectToInputSurface
```

Parameter summary:

- `--inputModel` / `--outputModel`: geometry files Slicer can read/write (VTP, VTK, OBJ, STL, etc.).
- `--targetEdgeLength`: desired edge length; use `0` to estimate from the input.
- `--numberOfIterations`: number of remeshing iterations (1-1000; default 3).
- `--projectToInputSurface`: optional flag to project vertices back to the original surface after each iteration (helpful for open meshes).

You can also launch the CLI from Slicer's graphical interface by navigating to the **UniformRemesh** module, selecting the input/output model nodes, and adjusting the parameters.

## Building

This repository provides a Slicer SuperBuild extension. Configure and build it following Slicer's extension build instructions. The build produces:

- `libvtkUniformRemesh.so` – the VTK filter library
- `vtkUniformRemeshPython.so` – the Python-wrapped filter
- `UniformRemesh` – the CLI executable and associated XML description

After building, use the generated `SlicerWithSlicerUniformRemesh` launcher (or install the extension into a Slicer release) to ensure Slicer discovers the wrapped filter automatically.
