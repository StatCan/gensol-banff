@echo off
echo Jansson build script for windows

@REM setup build tools
call ..\..\build\windows\setup_build_tools.cmd

echo.
if NOT EXIST builds (
    call build.cmd
)
pushd builds\x64
echo entered build directory: %CD%

echo.
echo build solution
msbuild -m -t:build -p:configuration=debug jansson.sln

popd