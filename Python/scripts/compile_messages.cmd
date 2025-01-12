@echo off
rem # convert `gettext` `.po` files to `.mo` files for each
rem # language supported

echo Script Start: %0

rem # environment
set PY_BIN=python
pushd ..
set PKG_ROOT=%cd%
popd

rem # set and check paths
set py_msgfmt=%PKG_ROOT%\scripts\i18n\msgfmt.py
if NOT EXIST %py_msgfmt% (
    echo ERROR: cannot locate msgfmt.py at path %py_msgfmt%
    goto exit_error
)

rem # run for each language
call :run_msgfmt en
if NOT ERRORLEVEL 0 goto exit_error
call :run_msgfmt fr
if NOT ERRORLEVEL 0 goto exit_error

pause


goto :EOF
rem ##### FUNCTIONS #####
:run_msgfmt
    rem # run msgfmt.py on a language dir, call like `call :run_msgfmt fr`
    set lang_dir=%~1

    rem # lang-spcific dir
    set msgfmt_lang_dir=%PKG_ROOT%\src\banff\nls\messages\%lang_dir%\LC_MESSAGES\
    if NOT EXIST %msgfmt_lang_dir% (
        echo ERROR: cannot locate msgfmt_lang_dir at path %msgfmt_lang_dir%
        goto exit_error
    )

    rem # input file
    set msgfmt_input_file=%msgfmt_lang_dir%\banff.po
    if NOT EXIST %msgfmt_input_file% (
        echo ERROR: cannot locate msgfmt_input_file at path %msgfmt_input_file%
        goto exit_error
    )

    rem # run Python script
    pushd %msgfmt_lang_dir%
    echo running script on %lang_dir% directory
    %PY_BIN% "%py_msgfmt%" "%msgfmt_input_file%"
    echo script complete
    popd
    exit /B

:exit_error
    echo ERROR OCCURRED
    pause
    exit /B -1
