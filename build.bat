@echo off
echo Killing existing TotalSearch process if running...
taskkill /F /IM TotalSearch.exe 2>nul

echo Configuring project...
cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.9.1/msvc2022_64/lib/cmake"

echo Building project...
cmake --build ./build --config Release

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable location: ./build\output\Release\TotalSearch.exe
) else (
    echo Build failed!
    exit /b 1
) 