@echo off
setlocal
set "emulator=%~dp0main.exe"

if not exist "%emulator%" (
    echo [ERROR] Emulator executable not found: "%emulator%" 1>&2
    exit /b 1
)

"%emulator%" %* /debug
exit /b %errorlevel%