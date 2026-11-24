@echo off
setlocal
rem Boots the Aurea OS release image in QEMU. Options are passed through to
rem aurea-live-vm.ps1, for example:  "Aurea Live VM.cmd" -MemoryMB 2048
title Aurea Live VM
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0aurea-live-vm.ps1" %*
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" pause
exit /b %RC%
