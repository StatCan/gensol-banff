@echo off
echo in-house C build script: libraries and procedures

@REM setup build tools
call .\build\windows\setup_build_tools.cmd

echo.
echo cleaning release_en
msbuild -m -t:clean -p:configuration=release_en;platform=x64 Banff.sln

echo.
echo cleaning release_fr
msbuild -m -t:clean -p:configuration=release_fr;platform=x64 Banff.sln

echo.
echo cleaning debug_en
msbuild -m -t:clean -p:configuration=debug_en;platform=x64 Banff.sln