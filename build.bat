@echo off
setlocal

for /f %%A in ('echo prompt $E ^| cmd') do set "\\033=%%A"

set EXE_NAME=Neoluma.exe
set BUILD_DIR=.build
set RESULTING_PATH=.executables/Neoluma.exe

echo %\\033%[38;2;232;75;133m[INFO] Cleaning and creating build folder...%\\033%[38;2;117;181;255m
:: if exist %BUILD_DIR% (
::    rmdir /s /q %BUILD_DIR%
:: )
:: mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo %\\033%[38;2;232;75;133m[INFO] Configuring with CMake for Visual Studio...%\\033%[38;2;117;181;255m
cmake .. -G "Ninja" || (
    echo %\\033%[38;2;255;80;80m[ERROR] CMake configuration failed.%\\033%[0m
    exit /b 1
)

echo %\\033%[38;2;232;75;133m[INFO] Building with CMake...%\\033%[38;2;117;181;255m
cmake --build . || (
    echo %\\033%[38;2;255;80;80m[ERROR] Build failed.%\\033%[0m
    exit /b 1
)

if exist %RESULTING_PATH% (
    echo %\\033%[38;2;117;255;135m[SUCCESS] Neoluma built successfully! Launch: ./.build/%RESULTING_PATH% %\\033%[0m
) else (
    echo %\\033%[38;2;255;80;80m[ERROR] Failed to find executable. %\\033%[0m
    exit /b 1
)

endlocal
