@echo off
echo Arrow build script for windows

@REM setup build tools
call ..\..\build\windows\setup_build_tools.cmd

echo.
md build
pushd build
echo created and entered build directory: %CD%

echo. 
echo generate solution
cmake ..\..\_submodules\arrow-nanoarrow

echo.
echo build solution
msbuild -m -t:build -p:configuration=Release nanoarrow.sln

popd