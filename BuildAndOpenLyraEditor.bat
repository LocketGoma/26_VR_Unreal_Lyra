@echo off
setlocal DisableDelayedExpansion
title Lyra Editor - Manual Build
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\Build-LyraEditor.ps1" -StartDirectory "%~dp0." %*
set "BuildResult=%ERRORLEVEL%"
echo.
if not "%BuildResult%"=="0" echo Build or engine detection failed. Exit code: %BuildResult%
pause
exit /b %BuildResult%
