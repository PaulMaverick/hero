@echo off

if not exist ..\build mkdir ..\build
pushd ..\build

set Compiler=g++

%Compiler% ^
    W:\handmade\hero\win32_handmade.cpp ^
    -g ^
    -o handmade.exe ^
    -luser32 ^
    -lgdi32 
popd