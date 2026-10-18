@echo off
rem TRR Hybrid Controls installer: double-click to install, or pass -Uninstall.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1" %*
echo.
pause
