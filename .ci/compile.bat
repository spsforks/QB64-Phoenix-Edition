@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

qb64_bootstrap.exe -x -w source\qb64.bas
IF ERRORLEVEL 1 exit /b 1

cd ..\..

del qb64_bootstrap.exe
del /q /s internal\source\*
move internal\temp\* internal\source\

internal\c\c_compiler\bin\mingw32-make.exe OS=win clean
