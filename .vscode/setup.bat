@echo off

:: make sure output directories are created
mkdir out 2> NUL
mkdir out\x86  2> NUL
mkdir out\x86\release 2> NUL
mkdir out\x86\debug 2> NUL
mkdir out\x64  2> NUL
mkdir out\x64\release 2> NUL
mkdir out\x64\debug 2> NUL

:: reset ERRORLEVEL to 0
ver > NUL
