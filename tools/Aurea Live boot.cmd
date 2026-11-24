@echo off
setlocal
rem Writes the Aurea OS image to a USB drive. Everything on the chosen drive
rem is erased. Pass -ListOnly to see the eligible drives without writing.
title Aurea Live boot
if /I "%~1"=="-ListOnly" goto run
net session >nul 2>&1
if not errorlevel 1 goto run
echo Writing a USB drive needs administrator rights. Windows will ask for permission.
rem Hand the path over through the environment: it may contain spaces or parentheses.
set "AUREA_SELF=%~f0"
set "AUREA_ARGS=%*"
if "%~1"=="" (
    powershell.exe -NoProfile -Command "Start-Process -FilePath $env:AUREA_SELF -Verb RunAs"
) else (
    powershell.exe -NoProfile -Command "Start-Process -FilePath $env:AUREA_SELF -ArgumentList $env:AUREA_ARGS -Verb RunAs"
)
exit /b
:run
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0aurea-live-boot.ps1" %*
echo.
pause
