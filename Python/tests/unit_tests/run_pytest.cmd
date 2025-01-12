@echo off

rem ## Change log##
rem ## 2023.09.01  - /PASS no longer runs performance tests
rem ## 2023.10.05  - change the default options to reduce clutter in output

set version=2023.10.05
set python_cmd=python
set pytest_options_common=--verbose -r pfEP --tb=short
set pytest_options=--cache-clear %pytest_options_common%
set pytest_options_rerun_fail= %pytest_options_common% --last-failed
set call_pytest_opt=%python_cmd% -m pytest %pytest_options%

if /I "%autoclear%"=="true" cls

call :print_header

rem go to help
if "%1"=="/?" (
    goto :print_usage
)
if /I "%1"=="-h" (
    goto :print_usage
)
if /I "%1"=="/h" (
    goto :print_usage
)
if /I "%1"=="/lf" (
    %python_cmd% -m pytest %pytest_options_rerun_fail%
    goto :the_end
)
if /I "%1"=="/pass" (
    %call_pytest_opt% -m "(m_validated) and not(m_missing_validation or m_missing_feature or m_for_review or m_performance_test)"
    goto :the_end
)
if /I "%1"=="/perf" (
    setlocal
    set BANFF_DEBUG_STATS=true
    if /I "%2"=="" (
        %call_pytest_opt% -m "(m_performance_test)"
    ) else (
        %call_pytest_opt% %2 %3 %4 %5
    )
    endlocal
    goto :the_end
)
if /I "%1"=="/valid" (
    %call_pytest_opt% -m "(m_validated)"
    goto :the_end
)
if /I "%1"=="/fail" (
    %call_pytest_opt% -m "(m_validated) and (m_missing_validation or m_missing_feature)"
    goto :the_end
)
if /I "%1"=="/missing-feature" (
    %call_pytest_opt% -m "m_missing_feature"
    goto :the_end
)
if /I "%1"=="/missing-validation" (
    %call_pytest_opt% -m "m_missing_validation"
    goto :the_end
)
if /I "%1"=="/review" (
    %call_pytest_opt% -m "m_for_review"
    goto :the_end
)
if /I "%1"=="/dev-pass" (
    %call_pytest_opt% -m "(m_in_development) and not(m_missing_validation or m_missing_feature)"
    goto :the_end
)
if /I "%1"=="/dev" (
    %call_pytest_opt% -m "(m_in_development)"
    goto :the_end
)
if /I "%1"=="/dev-fail" (
    %call_pytest_opt% -m "(m_in_development) and (m_missing_validation or m_missing_feature)"
    goto :the_end
)
)
if /I "%1"=="/all-pass" (
    %call_pytest_opt% -m "(m_validated or m_in_development) and not(m_missing_validation or m_missing_feature)"
    goto :the_end
)
if /I "%1"=="/all-fail" (
    %call_pytest_opt% -m "(m_missing_validation or m_missing_feature)"
    goto :the_end
)

rem run pytest
if "%1"=="" (
    echo running pytest on the current directory
    %call_pytest_opt% .
) else (
    echo running pytest on %1
    %call_pytest_opt% %*
)

rem helper labels
goto :the_end
:print_usage
echo.
echo RUN_PYTEST ^[path^] ^[filename^] ^[additional PyTest Options^]
echo RUN_PYTEST ^<selection-flag^>
echo.
echo Description: 
echo     Runs Python's PyTest module, applying default options and easing test selection.
echo     Default Options: %pytest_options%
echo.
echo     Searches the current directory using standard test discovery protocols.
echo     Specify a directory or file.  
echo     Provide any valid PyTest option.  
echo     Use a "Selection Flag" ^(see below^).
echo.
echo Selection Flags:
echo     /VALID                  only run validated tests
echo.
echo     /PASS                   only run valid tests that 
echo                             are expected to pass
echo                             EXCEPT performance tests
echo.
echo     /PERF ^[test spec^]       only run performance tests
echo                             use ^[test spec^] to specify a 
echo                             particular test
echo.
echo     /FAIL                   only run valid tests that 
echo                             are expected to fail
echo.
echo     /LF                     run using  pytest's 
echo                             --last-failed option 
echo.
echo     /MISSING-FEATURE        only run valid tests that 
echo                             identify a missing feature
echo.
echo     /MISSING-VALIDATION     only run valid tests that 
echo                             identify a missing validation
echo.
echo     /REVIEW                 only run that require review
echo.
echo     /DEV                    only run tests in development
echo.
echo     /DEV-PASS               only run tests in development 
echo                             that are expected to pass
echo.
echo     /DEV-FAIL               only run tests in development 
echo                             that are expected to fail
echo.
echo     /ALL-PASS               run all tests that 
echo                             are expected to pass
echo.
echo     /ALL-FAIL               run all tests that 
echo                             are expected to fail
echo.
echo Examples:
echo     RUN_PYTEST /PASS
echo     RUN_PYTEST -k test_DonorA -s
echo     RUN_PYTEST test_donorP01.py
goto :the_end

:print_header
echo run_pytest.sh - version:%version%
echo.
goto :the_end

:run_pytest
%call_pytest_opt%
goto :the_end

:the_end
IF %0 == "%~0" PAUSE
