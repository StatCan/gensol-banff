REM build.cmd
REM  The master build script for Windows, like a hacky Makefile with no options
REM  Initially implemented for the Windows CICD pipeline, it is pretty minimal
REM  TODO: error checking

REM build jansson library
pushd C\libraries\jansson
cmd.exe /c build_debug.cmd
popd

REM build arrow library
pushd C\libraries\arrow
cmd.exe /c build_debug.cmd
popd

REM build in-house: libraries and procedures
pushd C
cmd.exe /c build_debug.cmd
popd