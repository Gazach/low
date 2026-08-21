@echo off
setlocal

if "%1"=="" (
    echo Usage: build.bat [debug^|release]
    goto end
)

REM Path to Clang — update if your LLVM is installed elsewhere
set CLANG_PATH=C:\Program Files\LLVM\bin

if /I "%1"=="debug" (
    set CMAKE_BUILD_TYPE=Debug
) else if /I "%1"=="release" (
    set CMAKE_BUILD_TYPE=Release
) else (
    echo Invalid build type: %1
    echo Use "debug" or "release"
    goto end
)

set ROOT_DIR=%~dp0
set BUILD_DIR=%ROOT_DIR%build\%1

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo [cmake] Configuring %CMAKE_BUILD_TYPE% build...
cmake -S "%ROOT_DIR%." -B "%BUILD_DIR%" -G "Ninja" ^
  -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% ^
  -DCMAKE_C_COMPILER="%CLANG_PATH%\clang.exe" ^
  -DCMAKE_CXX_COMPILER="%CLANG_PATH%\clang++.exe" ^
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if errorlevel 1 (
    echo [error] CMake configuration failed.
    goto end
)

echo [ninja] Building...
ninja -C "%BUILD_DIR%"
if errorlevel 1 (
    echo [error] Build failed.
    goto end
)

echo [run] Running..
"%BUILD_DIR%\low-server.exe"

:end
endlocal
pause
