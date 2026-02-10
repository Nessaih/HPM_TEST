@echo off
setlocal enabledelayedexpansion

:: Main logic
call :CheckPython
if %PYTHON_OK% equ 1 (
    echo Python environment check passed. Using Python script to update version...
    call :RunPythonUpdate
) else (
    echo Python environment not available. Using PowerShell to update file timestamp...
    call :TouchVersionFile
)

exit /b 0

:: ==============================================
:: Function definitions
:: ==============================================

:CheckPython
:: Check Python availability and set PYTHON_OK flag (1=available, 0=unavailable)
set PYTHON_OK=0

:: Check if Python is installed
where python >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] Python not installed
    goto :EOF
)
@REM echo [INFO] Python found
:: Get Python version
for /f "tokens=2 delims= " %%a in ('python --version 2^>^&1') do set ver=%%a

:: Extract major and minor version numbers
for /f "tokens=1,2 delims=." %%a in ("%ver%") do (
    set major=%%a
    set minor=%%b
)
@REM echo [INFO] Python version: %major%.%minor%

:: Check if version >= 3.6
if %major% lss 3 (
    echo [WARNING] Python 3.6 or higher required ^(current: %ver%^)
    goto :EOF
)
@REM echo [INFO] Python major: %major%

if %major% equ 3 if %minor% lss 6 (
    echo [WARNING] Python 3.6 or higher required ^(current: %ver%^)
    goto :EOF
)

@REM echo [INFO] Python minor: %minor%

:: All checks passed
set PYTHON_OK=1
goto :EOF

:RunPythonUpdate
:: Update version using Python script
set "scpt=%~dp0UpdateVersion.py"
set "verc=%~dp0..\src\version\version.c"

:: Verify file existence
if not exist "%scpt%" (
    echo [ERROR] Python script not found: %scpt%
    exit /b 1
)

if not exist "%verc%" (
    echo [ERROR] Version file not found: %verc%
    exit /b 1
)

:: Execute Python script
python "%scpt%" "%verc%"
goto :EOF

:TouchVersionFile
:: Update file timestamp using PowerShell (simulate touch command)
set "verc=%~dp0..\src\version\version.c"

if not exist "%verc%" (
    echo [ERROR] Version file not found: %verc%
    exit /b 1
)

powershell -Command "(Get-Item '%verc%').LastWriteTime = Get-Date"
echo File timestamp updated: %verc%
goto :EOF
