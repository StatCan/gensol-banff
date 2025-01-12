# Apache Nanoarrow Submodule

This folder contains build scripts for the Apache Arrow-Nanoarrow open source library.  It will also house the build of this library.  Source code is located in the submodule [`C/libraries/_submodules/arrow-nanoarrow/`](/C/libraries/_submodules/arrow-nanoarrow/) which links to the GitHub repository [`apache/arrow-nanoarrow`](https://github.com/apache/arrow-nanoarrow).  

This library must be build prior to building `Banff.sln`.  

## Building Nanoarrow

Release builds of this library can be produced using included scripts.  Debug builds (Windows only) can be produced manually.  Build outputs will be written to the folder [`./build/`](./build/).  

### Windows

> Prerequisite: see [development_environment.md](/docs/developer/development_environment.md) for required software.  

Produce a release build by running [`build.cmd`](./build.cmd).  

To produce debug builds (Windows only), run [`build.cmd`](./build.cmd), navigate to the [`build`](./build/) folder, open [`nanoarrow.sln`](./build/nanoarrow.sln) using the Visual Studio GUI , select the `Debug` configuration and build the `ALL_BUILD` project.  

### Linux

> Prerequisite: see [development_environment.md](/docs/developer/development_environment.md) for required software.  

Produce a release build by running `make` in this directory.  

## Updating Nanoarrow Version

When nanoarrow releases a new version, the submodule can be advanced to start using that new version.  Updating build scripts may be required.  This results in the C code using a new version.  Be sure to update Python's nanoarrow dependency as well.  Test with the corresponding Python `nanoarrow` package and older versions within our supported range of versions.  Remove any versions that are incompatible with the new C build.  
