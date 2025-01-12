@echo off
echo in-house C build script: libraries and procedures

@REM setup build tools
call .\build\windows\setup_build_tools.cmd

echo.
echo building release_en
msbuild -m -t:build -p:configuration=release_en;platform=x64 Banff.sln

echo.
echo building release_fr
msbuild -m -t:build -p:configuration=release_fr;platform=x64 Banff.sln