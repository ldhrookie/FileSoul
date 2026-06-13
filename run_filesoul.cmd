@echo off
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0run_filesoul.ps1" %*
set FILESOUL_EXIT_CODE=%ERRORLEVEL%
if not "%FILESOUL_NO_PAUSE%"=="1" (
    echo.
    pause
)
exit /b %FILESOUL_EXIT_CODE%

