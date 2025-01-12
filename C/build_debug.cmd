@echo off
echo in-house C build script: libraries and procedures

@REM setup build tools
call .\build\windows\setup_build_tools.cmd

echo.
echo building debug_en
msbuild -m -t:build -p:configuration=debug_en;platform=x64 Banff.sln