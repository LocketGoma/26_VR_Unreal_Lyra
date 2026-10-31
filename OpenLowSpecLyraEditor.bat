@echo off
setlocal DisableDelayedExpansion
title Lyra Editor - Low Spec Startup
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\Start-LowSpecEditor.ps1" -StartDirectory "%~dp0." %*
set "LaunchResult=%ERRORLEVEL%"
if "%LaunchResult%"=="0" exit /b 0
echo.
echo Low-spec startup failed. Exit code: %LaunchResult%
pause
exit /b %LaunchResult%
