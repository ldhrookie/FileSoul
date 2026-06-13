@echo off
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0run_filesoul.ps1" %*
exit /b %ERRORLEVEL%

