@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64
cd /d "%~dp0"
if not exist build mkdir build
cd build
cmake .. -G "NMake Makefiles"
cmake --build .
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! Run: build\zombie-survivor.exe
    pause
) else (
    echo Build failed.
    pause
)
