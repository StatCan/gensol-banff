# Jansson Submodule

This folder contains build scripts for the Jansson open source library.  It will also house the build of this library.  Source code is located in the submodule [`C/libraries/_submodules/jansson/`](/C/libraries/_submodules/jansson/) which links to the GitHub repository [`akheron/jansson`](https://github.com/akheron/jansson).  

This library must be build prior to building `Banff.sln`.  

## Building Jansson

Release builds of this library can be produced using included scripts.  Debug builds (Windows only) can be produced manually.  Build outputs will be written to the folder [`./builds/`](./builds/).  

### Windows

> Prerequisite: see [development_environment.md](/docs/developer/development_environment.md) for required software.  

Produce a release build by running [`build.cmd`](./build.cmd).  

To produce debug builds (Windows only), run [`build.cmd`](./build.cmd), navigate to the [`build`](./build/) folder, open [`jansson.sln`](./build/jansson.sln) using the Visual Studio GUI , select the `Debug` configuration and build the `ALL_BUILD` project.  

### Linux

> Prerequisite: see [development_environment.md](/docs/developer/development_environment.md) for required software.  

Produce a release build by running `make` in this directory.  
