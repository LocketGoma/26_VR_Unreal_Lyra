@echo off
setlocal DisableDelayedExpansion
title Lyra - Restore Original Settings
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\Start-LowSpecEditor.ps1" -StartDirectory "%~dp0." -Restore %*
set "RestoreResult=%ERRORLEVEL%"
echo.
if not "%RestoreResult%"=="0" echo Restore failed. Exit code: %RestoreResult%
pause
exit /b %RestoreResult%
