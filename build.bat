@echo off
cd build || (
    echo [ERROR] Couldn't find the build folder.
    exit /b 1
)

echo Cleaning the build folder...
rmdir /s /q . >nul 2>&1

cd ..
mkdir build
cd build

echo Launching cmake...
cmake .. || (
    echo [ERROR] Error in launching cmake.
    exit /b 1
)

echo Building with make...
make || (
    echo [ERROR] Building error.
    exit /b 1
)

if exist neoluma.exe (
    echo Neoluma compiled successfully! Launch with: ./build/neoluma.exe
) else (
    echo [ERROR] Neoluma failed to compile.
    exit /b 1
)
