@echo off


:: get target path
set TARGET_PATH=%1

:: get target name
for %%I in ("%TARGET_PATH%") do set "TARGET_NAME=%%~nxI"

:: get output directory
for %%I in ("%TARGET_PATH%\..\..\..\..") do set "OUTPUT=%%~fI\output"


set OUT=%TARGET_PATH%.out
set ELF=%OUTPUT%\%TARGET_NAME%.elf
set HEX=%OUTPUT%\%TARGET_NAME%.hex
set BIN=%OUTPUT%\%TARGET_NAME%.bin

@REM echo Project target name: %TARGET_NAME%
@REM echo Project output path: %OUTPUT%



:: calculate application checksum、
@REM ielftool --fill="0xFF;__ICFEDIT_region_IROM1_start__-__ICFEDIT_region_IROM1_end__" ^
@REM --checksum="__checksum:4,crc32,0xffffffff;__ICFEDIT_region_IROM1_start__-__ICFEDIT_region_IROM1_end__" ^
@REM --verbose %OUT% %OUT%

:: generate additional output: binary
ielftool --bin --verbose %OUT% %BIN% >nul 2>&1

:: generate additional output: hex
ielftool --ihex --verbose %OUT% %HEX% >nul 2>&1

:: generate additional output: elf
cp %OUT% %ELF% >nul 2>&1

:: generate update package
call :MakeUpdatePackage

@echo on

goto :EOF

:: ==============================================
:: Function definitions
:: ==============================================

:MakeUpdatePackage
:: Generate update package using Python script
set "scpt=%~dp0MakeUpdatePack.py"

:: Verify file existence
if not exist "%scpt%" (
    echo [WARNING] MakeUpdatePack.py script not found: %scpt%
    goto :EOF
)

:: Check Python availability
where python >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] Python not available, skip update package generation
    goto :EOF
)

:: Execute Python script
echo Generating update package...
python "%scpt%"
if %errorlevel% equ 0 (
    echo Update package generation completed successfully
) else (
    echo [WARNING] Update package generation failed
)

goto :EOF
