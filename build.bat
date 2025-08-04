@echo off
setlocal

set EXE_NAME=Neoluma.exe
set BUILD_DIR=.build
set RESULTING_PATH=.executables/Neoluma.exe

echo [INFO] Cleaning and creating build folder...
if exist %BUILD_DIR% (
    rmdir /s /q %BUILD_DIR%
)
mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo [INFO] Configuring with CMake for Visual Studio...
cmake .. -G "Ninja" || (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

echo [INFO] Building with CMake...
cmake --build . || (
    echo [ERROR] Build failed.
    exit /b 1
)

if exist %RESULTING_PATH% (
    echo [SUCCESS] Neoluma built successfully! Launch: ./.build/%RESULTING_PATH%
) else (
    echo [ERROR] Failed to find executable.
    exit /b 1
)

endlocal
