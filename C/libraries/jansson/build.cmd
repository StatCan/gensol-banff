@echo off
echo Jansson build script for windows

@REM setup build tools
call ..\..\build\windows\setup_build_tools.cmd

echo.
md builds\x64
pushd builds\x64
echo created and entered build directory: %CD%

echo. 
echo generate solution
@REM use VS 2019
cmake -G "Visual Studio 16 2019" -A x64 ..\..\..\_submodules\jansson
@REM for VS 2022, use the following
@REM  cmake -G "Visual Studio 17 2022" -A x64 ..\..\..\_submodules\jansson

echo.
echo build solution
msbuild -m -t:build -p:configuration=Release jansson.sln

popd