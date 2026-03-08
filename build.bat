@echo off
setlocal EnableExtensions
for /f %%A in ('echo prompt $E ^| cmd') do set "ESC=%%A"

set "MODE=%~1"
if "%MODE%"=="" set "MODE=release"

set "TARGET=%~2"
if "%TARGET%"=="" set "TARGET=neoluma"

set "CFG_PRESET=release-ninja"
set "BUILD_PRESET=release"
set "CONFIG=Release"

if /I "%MODE%"=="debug" (
  set "CFG_PRESET=debug-ninja"
  set "BUILD_PRESET=debug"
  set "CONFIG=Debug"
)

echo %ESC%[38;2;232;75;133m[INFO]%ESC%[0m Configure preset: %CFG_PRESET%
echo %ESC%[38;2;232;75;133m[INFO]%ESC%[0m Build preset: %BUILD_PRESET%
echo %ESC%[38;2;232;75;133m[INFO]%ESC%[0m Target: %TARGET%

echo %ESC%[38;2;232;75;133m[INFO]%ESC%[0m Configuring...
cmake --preset "%CFG_PRESET%"
if errorlevel 1 goto :cfg_fail

if /I "%TARGET%"=="all" goto :do_all
if /I "%TARGET%"=="installer" goto :do_installer
if /I "%TARGET%"=="payload" goto :do_payload
if /I "%TARGET%"=="neoluma" goto :do_neoluma

echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m Unknown target: %TARGET%
echo    valid targets: neoluma, payload, installer, all
exit /b 1

:do_neoluma
echo %ESC%[38;2;232;75;133m[INFO]%ESC%[0m Building neoluma...
cmake --build --preset "%BUILD_PRESET%" --target install
if errorlevel 1 goto :build_fail
goto :after_build

:do_payload
echo %ESC%[38;2;232;75;133m[INFO]%ESC%[0m Building payload...
cmake --build --preset "%BUILD_PRESET%" --target payload
if errorlevel 1 goto :build_fail
goto :done

:do_installer
call :do_payload
if errorlevel 1 exit /b 1
goto :tauri

:do_all
call :do_payload
if errorlevel 1 exit /b 1
:tauri
echo %ESC%[38;2;232;75;133m[INFO]%ESC%[0m Building Tauri installer (single EXE)...

pushd src\Installer
call npm run tauri build
if errorlevel 1 (
  popd
  echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m Tauri build failed.
  exit /b 1
)

set "TAURI_EXE=src-tauri\target\release\neoluma-installer.exe"
if not exist "%TAURI_EXE%" (
  popd
  echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m Installer exe not found: %TAURI_EXE%
  exit /b 1
)

popd

REM ==== copy installer to main executables folder ====
set "DEST_DIR=.build\.executables\%CONFIG%"
if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"

copy /Y "src\Installer\src-tauri\target\release\neoluma-installer.exe" "%DEST_DIR%\neoluma-installer.exe" >nul

if errorlevel 1 (
  echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m Failed to copy installer exe.
  exit /b 1
)

echo %ESC%[38;2;117;255;135m[SUCCESS]%ESC%[0m Installer copied to %DEST_DIR%\neoluma-installer.exe
goto :done

:after_build
set "EXE=.build\.executables\%CONFIG%\neoluma.exe"
set "RUNTIME=.build\.runtime\%CONFIG%\"
if exist "%EXE%" (
  echo %ESC%[38;2;117;255;135m[SUCCESS]%ESC%[0m Built: %EXE%
) else (
  echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m Can't find built exe at: %EXE%
  exit /b 1
)
if exist "%RUNTIME%" (
  echo %ESC%[38;2;117;255;135m[SUCCESS]%ESC%[0m Runtime built at: %RUNTIME%
) else (
  echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m Can't find built runtime at: %RUNTIME%
  exit /b 1
)
goto :done

:cfg_fail
echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m CMake configure failed.
exit /b 1

:build_fail
echo %ESC%[38;2;255;80;80m[ERROR]%ESC%[0m Build failed.
exit /b 1

:done
echo %ESC%[38;2;117;255;135m[DONE]%ESC%[0m
exit /b 0