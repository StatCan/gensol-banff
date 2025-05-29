# Build Guide

This document provides technical details on various build related activities for C and Python code.  
The Banff system is implemented using both C and Python code, with the latter implementing a wrapper for the former.  
At the highest level, C code is built into shared libraries (files with extension `.dll` on Windows, `.so` on Linux) and
Python code is built into a package which includes the C code builds.  

Python code is platform independent, except for some code related to integration with C builds.  
C builds are platform specific.  We currently support Windows and Linux platforms,
producing a unique Python package build (`.whl` wheel files) for each supported platform.  

Builds should generally be performed by a CI/CD pipeline to ensure build consistency, and benefit from
various security and code quality scans, automated testing, and ability to deploy builds.
Building locally is often necessary during development and debugging activities, particularly for C code.  
The information in this guide should help with understanding and implementing either type of build.  

## Table of Contents

- [Table of Contents](#table-of-contents)
- [Acquiring Source Code](#acquiring-source-code)
- [CI/CD Builds](#cicd-builds)
- [Building Locally](#building-locally)
- [Building C Code](#building-c-code)
  - [Linux C Builds](#linux-c-builds)
    - [Testing C Builds on Linux](#testing-c-builds-on-linux)
  - [Windows C Builds](#windows-c-builds)
    - [Windows Open Source Builds](#windows-open-source-builds)
    - [Windows In House Builds](#windows-in-house-builds)
    - [Testing C Builds on Windows](#testing-c-builds-on-windows)
- [Python Builds](#python-builds)
  - [Linux Python Builds](#linux-python-builds)
  - [Windows Python Builds](#windows-python-builds)
  - [Native Language Support (NLS) in Python](#native-language-support-nls-in-python)
- [Fully Scripted Release Builds](#fully-scripted-release-builds)
  - [Fully Scripted Windows Builds](#fully-scripted-windows-builds)
  - [Fully Scripted Linux Builds](#fully-scripted-linux-builds)
- [Appendix](#appendix)
  - [Troubleshooting Manual Windows C Builds](#troubleshooting-manual-windows-c-builds)
    - [Cleaning on Windows](#cleaning-on-windows)
    - [Property Sheet Changes](#property-sheet-changes)

## Acquiring Source Code

Source code is maintained in a Git repository and incorpoerates code from other Git repositories using *submodules*.  
When working with the repository locally it is essential to acquire and manage submodules.  
CI/CD pipelines often handle submodules correctly by default.  

> **Handling Submodules**  
>
> - when cloning the repository locally, use the `--recurse-submodules` option to automatically acquire each submodule and check out the correct commit
> - when switching to a different branch in the repository, the "correct commit" may change
>   - check by running `git status` in the *superproject*,  submodules will appear as an unstaged change if they are checked out to the wrong commit
>   - the command `git submodule update` can be used to check out the correct commit in each submodule

## CI/CD Builds

...not yet documented here

## Building Locally

Building locally means acquiring a copy of the source code and building it on the developer's machine.  
This facilitates testing and live debugging during development.  

Be aware that installed Python packages can typically be live debugged, so there is no need to build from source
locally if you only need to inspect how an installed package behaves.  
While it is possible to make modifications to an installed Python package, such a package is not under
version control and this should be avoided.  

When making changes to C code, build and debugging facilities are provided by Visual Studio Enterprise/Professional on Windows.  

If only making changes to Python code, C builds will still be required to build and test locally.  
A workaround to building C code would be to acquire the builds from a previous release of the package
and place them in the correct location in your work tree.  

## Building C Code

> **TIP**: build scripts  
> The scripts `build.cmd` and `build_debug.cmd` at the root of this repository can be used to build all C code in release or debug mode, respectively.  

There are 9 Banff procedures written in C, each having its own C code.  
All procedures depend on a set of in-house C libraries, which must be built prior to building the procedures.  
Some in-house libraries depend on open source libraries, which must be built prior to the in-house libraries.  
Therefore, the build order for C code is *open-source libraries* > *in-house libraries* > *procedures*.  

At a high level, there are 2 methods of building the C code: scripted or manually.  The scripted method is supported on both
Linux (using *GNU Make*) and Windows (using batch scripts with Visual Studio Developer Tools).  The manual method is available
only on Windows as a combination of batch scripts and the Visual Studio Professional/Enterprise GUI.  

### Linux C Builds

On Linux, the build process is managed using GNU Make files.  A `Makefile` file is included with each library
and procedure and handles building and cleaning, among other things.  

The Makefile located at [`C/build/unix-like/Makefile`](../../C/build/unix-like/Makefile) manages the whole process of
building by calling the individual Makefiles for open source libraries, in-house libraries, and each procedure in the proper
order.  It is possible to manually run each Makefile individually instead of via this makefile if desired.  

The main makefile provides the following options

- building
  - `make`: build all procedures and their dependencies
  - `make libs_open_source`: build all open source libraries
  - `make libs_internal`: build all in-house libraries
  - `make <proc-name>`: build a specific procedure
    - `make determin`
    - `make donorimp`
    - `make editstat`
    - `make errorloc`
    - `make estimato`
    - `make massimpu`
    - `make outlier`
    - `make prorate`
    - `make verifyed`
- testing
  - `make test`: test all procedures
  - `make <proc-name>_test`: test a specific procedure
    - `make determin_test`
    - ...
- cleaning
  - `make clean`: clean all builds
  - `make clean_procs`: clean all procedure builds
  - `make clean_libs_open_source`: clean all open source library builds
  - `make clean_libs_internal`: clean all in-house library builds
- diagnostics
  - `make diagnose`: print paths used by makefiles to locate source code

All makefiles:

- builds are optimized for release
- no support for debug builds on Linux
- only build one native language at a time

> **Language selection**  
> These Make files only build one language (English or French) at a time.  
> To specify the language, add `STC_LANG=en` (default) or `STC_LANG=fr` after the `make` command.  

In a terminal, `cd C/build/unix-like/`

1. **Clean Builds**: run the command `make clean`  
    - use `make clean STC_LANG=fr` for French builds

2. **Execute Build**: run the command `make`  
    - use `make STC_LANG=fr` for French builds

#### Testing C Builds on Linux

A basic test of each procedure build can be ran using the command `make test` (`make test STC_LANG=fr` to test French builds).  
This only validates that Python can load each shared library and call a simple function, it is a poor validation of the overall
build.  To perform more regerous testing, build the Python package and run automated tests.  

### Windows C Builds

On Windows the build process is managed using Windows batch scripts, optionally in combination with the Visual Studio GUI.  

> **Build Tools**  
> The batch scripts use Visual Studio build tools (part of its "*Desktop development with C++*" workload).  These build tools are
> typically not available in a fresh command prompt, even when they are installed.  
> The script located at [`C/build/windows/setup_build_tools.cmd`](../../C/build/windows/setup_build_tools.cmd) attempts to find
> the tools and make them available for other build scripts.  

#### Windows Open Source Builds

A script, `build.cmd`, is included with each open source library and implements that libraries specific build process.  This typically
require first using `cmake` to build a Visual Studio solution and second, building that solution.  While the script automatically
performs both steps, the latter step is only performed for the *Release* configuration.  

To obtain *Debug* builds you can use `build_debug.cmd` open the generated `.sln` file in Visual Studio and build your preferred configuration from there.  

> Open Source libraries are located at:  
>
> - [`C/libraries/arrow/`](../../C/libraries/arrow/)
> - [`C/libraries/jansson/`](../../C/libraries/jansson/)
>

#### Windows In House Builds

The Visual Studio Solution located at [`C/Banff.sln`](../../C/Banff.sln) manages build configurations for all in-house C libraries and procedures.  
The solution manages dependencies and facilitates live debugging.  

> **Notes about `banff.sln`**  
>
> - language-specific configurations `release_en` and `release_fr` are optmized for release builds
> - `debug_en` facilitates live debugging
>   - there is currently no French debug configuration
> - see [Troubleshooting Manual Windows C Builds](#troubleshooting-manual-windows-c-builds)
>

The batch script located at [`C/build.cmd`](../../C/build.cmd) can be used to build the `release_en` and `release_fr` configurations in `Banff.sln`.  
Generate debug builds using [`C/build_debug.cmd`](../../C/build_debug.cmd).  

- note that this requires open open source libraries to be built ahead of time (call each build script first or see [Fully Scripted Release Builds](#fully-scripted-release-builds))

#### Testing C Builds on Windows

There is no simple test script for C builds as there is on Linux.  The only way is to build the Python wrapper and run
automated tests.  

## Python Builds

Python builds are performed using Python's `build` package by navigating to the `Python` directory
and running the command `python -m build`.  This will generate a *source distribution* ("sdist", `.tar.gz` file) and a
*build distribution* ("bdist", `.whl` file).  

While the Python code is platform-independent, the package build necessarily incorperates platform-specific C builds.
It is possible to include C builds for multiple platforms in a single Python package build, which can be convenient for
development and testing.  
For release builds a separate package specific to each supported platform should be built and [tagged](https://packaging.python.org/en/latest/specifications/platform-compatibility-tags/) and only include that platform's C build.  

### Linux Python Builds

On Linux, the Makefile located at [Python/Makefile](../../Python/Makefile) will build an untagged package via `make` or `make python_package`.  

- `make install` installs the built Python package
- `make uninstall` uninstalls the build Python package

[Tagging](https://packaging.python.org/en/latest/specifications/platform-compatibility-tags/) of Linux builds can be managed
using the `auditwheel` package.  

### Windows Python Builds

On Windows, commands for building, installing, and uninstalling should be run manually.  
[Tagging](https://packaging.python.org/en/latest/specifications/platform-compatibility-tags/) of Windows builds can be managed
using the `wheel` package.  

### Native Language Support (NLS) in Python

The Python package can support multiple languages in a single build using *GNU gettext*.  This process involves compiling and including a binary translation file in Package builds.  At the time of this writing, this binary file is committed to source control, so this step is handled during development, not during builds.  

For more details, see [native_language_support.md](../../Python/src/banff/_common/docs/native_language_support.md).  

## Fully Scripted Release Builds

Linux and Windows both provide a script for running the entire build process including C and Python builds.  

### Fully Scripted Linux Builds

On Linux, [`Makefile`](../../Makefile) can be used to produce release builds for all C and Python source from a fresh repository.  

- build everything
  - `make`: build all C and Python source
    - builds both English and French C code builds
- build C code
  - `make c_procs`: build English and French procedures and their dependencies
    - `make c_procs_en`: build only English procedures and dependencies
    - `make c_procs_fr`: build only French procedures and dependencies
- build Python code
  - `make python_package`: build Python package and dependencies (including C code in both languages)
    - `make python_package_only`: build Python package but disregard C builds
      - C builds included if available, otherwise will not be included in package build
- install/uninstall
  - `make install`: install Python package
    - will fail if Python package isn't already built
  - `make uninstall`: uninstall Python package
- testing
  - `make test`: run automated tests on installed Python package
- clean
  - `make clean`: clean all builds
  - `make clean_c_proc_en`: clean English C procedure and library builds
  - `make clean_c_proc_fr`: clean French C procedure and library builds
  - `make clean_python`: clean Python builds

### Fully Scripted Windows Builds

On Windows, [`build.cmd`](../../build.cmd) can be used to produce release builds of all C source from a fresh repository.  

- for cleaning, do this manually using the Visual Studio GUI
- for testing, do this manually
- does **not** build Python source
  - manually navigate to [`Python`](../../Python/) and run `python -m build`

## Appendix

### Troubleshooting Manual Windows C Builds

#### Cleaning on Windows

Sometimes, especially when switching between the `release_en` and `debug_en` configurations, Visual Studio will not
rebuild procedures.  Cleaning the solution often resolves these issues.  
Whenever you are not getting the expected behaviour, try cleaning.  
In Visual Studio Enterprise, in the *Solution Explorer*, right click and project and select *Clean*.  Builds associated with the active configuration will be deleted.  
For bulk cleaning, use *Build > Batch Build...*, select all projects and click *Clean*.  

#### Property Sheet Changes

Once Visual Studio loads the `Banff.sln` file, changes made to any `.props` files by external programs may not be detected
by Visual Studio until the solution is unloaded and reloaded.  
