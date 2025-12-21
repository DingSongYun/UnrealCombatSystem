@echo off
setlocal

rem Define Engine Path based on documentation
set "UE_ROOT=D:\Epic Games\UE_5.7"

rem Check for UnrealBuildTool
set "UBT=%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

if not exist "%UBT%" (
    echo [Error] UnrealBuildTool not found at:
    echo "%UBT%"
    echo.
    echo Please check if the engine path in this script matches your installation.
    pause
    exit /b 1
)

rem Get Project Path (Parent directory of Tools)
pushd "%~dp0.."
set "PROJECT_DIR=%CD%"
popd

set "UPROJECT_FILE=%PROJECT_DIR%\UnrealCombatSystem.uproject"

if not exist "%UPROJECT_FILE%" (
    echo [Error] .uproject file not found at:
    echo "%UPROJECT_FILE%"
    pause
    exit /b 1
)

echo =======================================================
echo Generating Visual Studio Project Files...
echo Engine:  %UE_ROOT%
echo Project: %UPROJECT_FILE%
echo =======================================================

"%UBT%" -projectfiles -project="%UPROJECT_FILE%" -game -rocket -progress

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [Failed] Project generation failed. Please check the logs above.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [Success] Solution file generated successfully!
pause
