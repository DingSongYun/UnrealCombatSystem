@echo off
setlocal

rem Define paths
set "UE_ROOT=D:\Epic Games\UE_5.7"
set "BUILD_TOOL=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR_EXE=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"

pushd "%~dp0.."
set "PROJECT_DIR=%CD%"
popd

set "UPROJECT_FILE=%PROJECT_DIR%\UnrealCombatSystem.uproject"

echo =======================================================
echo Building UnrealCombatSystemEditor...
echo Project: %UPROJECT_FILE%
echo =======================================================

call "%BUILD_TOOL%" UnrealCombatSystemEditor Win64 Development -Project="%UPROJECT_FILE%" -WaitMutex -FromMsBuild

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [Failed] Build failed with error code %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [Success] Build complete. Launching Editor...
echo =======================================================

start "" "%EDITOR_EXE%" "%UPROJECT_FILE%" -log

exit /b 0