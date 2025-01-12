@echo off
REM setup_build_tools 
REM  performs setup of build tools for windows C code

echo Setting up build tools for Windows C code

@REM change to directory of this script, will `popd` upon return
pushd %~dp0

call call_vcvars_platform.cmd
set error_code=%ERRORLEVEL%

popd

if NOT "%ERROR_CODE%"=="0" (
    echo.
    echo ERROR: setup failed
    echo.
)

exit /b %error_code%