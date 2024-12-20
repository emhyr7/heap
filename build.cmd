
@echo off
cd /d "%~dp0"
setlocal

if not exist build mkdir build
cd build

set CFLAGS=-O0 -g -Wall -Wextra -Wno-char-subscripts -Wno-bitwise-op-parentheses

clang %CFLAGS% -o heap.exe ..\code\*.c || exit /b 1
rem lib /nologo /out:heap.lib *.o || exit /b 1
