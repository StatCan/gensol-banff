@echo off
echo Arrow build script for windows

@REM setup build tools
call ..\..\build\windows\setup_build_tools.cmd

echo.
if NOT EXIST build (
    call build.cmd
)
pushd build
echo entered build directory: %CD%

echo.
echo build solution
msbuild -m -t:build -p:configuration=debug nanoarrow.sln

popd