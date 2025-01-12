@echo off
REM call_vcvars_platform [platform:32,64]
REM   finds and calls the first available vcvars%platform%.bat script and returns
rem   ARGUMENTS: use optional arg 1 to specify the platform

if "%SEARCH_YEARS%"=="" set SEARCH_YEARS=2019 2022
if "%SEARCH_EDITIONS%"=="" set SEARCH_EDITIONS=buildtools enterprise professional

@REM PLATFORM (parameter 1) may be specified
set PLATFORM=64
if NOT "%~1"=="" set PLATFORM=%~1

echo Searching for vcvars batch file for %PLATFORM% bit platform

@REM try combos of year and edition
@REM delayed expansion required to check error level
setlocal enabledelayedexpansion
for %%a IN (%SEARCH_YEARS%) DO (
    for %%b IN (%SEARCH_EDITIONS%) DO (
        call set_msvc_path.cmd VCVARS_PATH2 %%a %%b %PLATFORM%
        if !ERRORLEVEL!==0 (
            endlocal
            call :call_vcvars_script %%a %%b %PLATFORM%
            goto :end_happy
        )
    )
)

REM arrived here? none were found
goto :end_failure

@REM call_vcvars_script [year] [edition]
@REM   calls the script, returns error level of call
:call_vcvars_script
    call set_msvc_path.cmd VCVARS_PATH2 %1 %2 %PLATFORM%
    echo calling "%VCVARS_PATH2%"
    call "%VCVARS_PATH2%"
    exit /b %ERRORLEVEL%

:end_failure
    exit /b 1

:end_happy
    exit /b 0