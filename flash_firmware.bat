@echo off
REM Wrapper care ruleaza flash_firmware.ps1 ocolind ExecutionPolicy.
REM Utilizare:
REM   flash_firmware.bat                         (meniu interactiv)
REM   flash_firmware.bat -File firmware66.bin -Port COM10
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0flash_firmware.ps1" %*
