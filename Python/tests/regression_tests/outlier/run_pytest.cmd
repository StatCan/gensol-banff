@echo off
IF %0 == "%~0" set PAUSE_AT_END=true
..\..\run_pytest.cmd %*