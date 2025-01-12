# Development Environment Recommendations

## Procedure Development

The procedures are implemented in *C* and *Python*.  When developing C code, Windows with Microsoft
Visual Studio Professional/Enterprise generally provides the richest set of tools.  
Visual Studio Code provides a rich Python development experience on either Windows or Linux.  

### C code

**Windows**: *develop, build, live debug, CPU profiling*

- *Microsoft Visual Studio 2019 Professional/Enterprise* with the *Desktop development with C++* workload

**Linux**: *building*  
Setting up the build environment on your Unix-like system will differ slightly system to system.  

- `gcc` (GNU Compiler Collection)
- `make` ([*GNU Make*](https://www.gnu.org/software/make/) is used for scripting the build process)
- `git` (obtaining source code)
- `python3` (for basic C build testing)

### Python

**Windows or Linux**: *develop, build, live debug*

- Python `build` package
- Visual Studio Code ("*VS Code*")
  - Python extension (`ms-python.python`)
  - C/C++ Extension Pack (`ms-vscode.cpptools-extension-pack`)
    - facilitates working on the C code, but be careful, for major changes use Visual Studio Professional/Enterprise
