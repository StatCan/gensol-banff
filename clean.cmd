REM clean.cmd
REM  The master clean script for Windows, like a hacky Makefile with no options
REM  Initially implemented for the Windows CICD pipeline, it is pretty minimal
REM  TODO: error checking

REM clean jansson library
pushd C\libraries\jansson
cmd.exe /c clean.cmd
popd

REM clean arrow library
pushd C\libraries\arrow
cmd.exe /c clean.cmd
popd

REM clean in-house: libraries and procedures
pushd C
cmd.exe /c clean.cmd
popd