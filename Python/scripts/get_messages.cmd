@echo off
rem # search banff source for messages requiring translation
rem # messages marked by wrapping in `gettext` function `_^(^)`

echo Script Start: %0

rem # environment
set PY_BIN=python
pushd ..
set PKG_ROOT=%cd%
popd

rem # set and check paths
set py_gettext=%PKG_ROOT%\scripts\i18n\pygettext.py
if NOT EXIST %py_gettext% (
    echo ERROR: cannot locate pygettext.py at path %py_gettext%
    goto exit_error
)

rem # cannot end with trailing `\`
set gettext_search_dir=%PKG_ROOT%\src\banff
if NOT EXIST %gettext_search_dir% (
    echo ERROR: cannot locate gettext_search_dir at path %gettext_search_dir%
    goto exit_error
)

set gettext_output_dir=%PKG_ROOT%\src\banff\nls\messages\
if NOT EXIST %gettext_output_dir% (
    echo ERROR: cannot locate gettext_output_dir at path %gettext_output_dir%
    goto exit_error
)

set gettext_output_file=banff.pot

rem # run Python script
@echo on
%PY_BIN% "%py_gettext%" --no-location -o "%gettext_output_dir%%gettext_output_file%" "%gettext_search_dir%"
@echo off
echo script complete
dir %gettext_output_dir%%gettext_output_file%
pause

goto :EOF
rem ##### FUNCTIONS #####
:exit_error
    echo ERROR OCCURRED
    pause
    exit /B -1
