@echo off


:: get target path
set TARGET_PATH=%1

:: get target name
for %%I in ("%TARGET_PATH%") do set "TARGET_NAME=%%~nxI"

:: get output directory
for %%I in ("%TARGET_PATH%\..\..\..\..") do set "OUTPUT=%%~fI\output"


set OUT=%TARGET_PATH%.out
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

@echo on
