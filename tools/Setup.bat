@echo off
setlocal enabledelayedexpansion

:: ----------------------------------------------------------
:: Usage:
::   Install-All.bat path\to\vcpkg.exe  [--copy] [--noclone]
:: ----------------------------------------------------------

if "%~1"=="" (
    echo [ERROR] Missing argument: path to vcpkg.exe
    echo Usage: Install-All.bat path\to\vcpkg.exe [--copy] [--noclone]
    exit /b 1
)

:: Required argument
set VCPKG_PATH=%~1

:: Collect optional args
set OPTIONAL_ARGS=

shift
:parseArgs
if "%~1"=="" goto argsDone

if "%~1"=="--copy" (
    set OPTIONAL_ARGS=!OPTIONAL_ARGS! --copy
) else if "%~1"=="--noclone" (
    set OPTIONAL_ARGS=!OPTIONAL_ARGS! --noclone
) else (
    echo [WARNING] Unknown argument "%~1" ignored.
)

shift
goto parseArgs

:argsDone

echo ----------------------------------------------
echo Running installers with:
echo   vcpkg: %VCPKG_PATH%
echo   options: %OPTIONAL_ARGS%
echo ----------------------------------------------

:: Pass VCPKG_PATH into environment for Node scripts
set VCPKG_ROOT=%VCPKG_PATH%

echo.
node Install-onnxruntime.js %OPTIONAL_ARGS%
if errorlevel 1 exit /b 1

echo.
node Install-Qt.js %OPTIONAL_ARGS%
if errorlevel 1 exit /b 1

echo.
node Install-opencv.js %OPTIONAL_ARGS%
if errorlevel 1 exit /b 1

echo.
echo All installations complete.
exit /b 0
