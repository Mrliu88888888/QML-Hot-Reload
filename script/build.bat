@echo off
chcp 65001

call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
call:build "Qt5.6.3-msvc2013"

call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
call:build "Qt5.12.5-msvc2015_64"
call:build "Qt5.12.5-msvc2017_64"
call:build "Qt5.15.2-msvc2019_64"
call:build "Qt6.5.3-msvc2019_64"
exit 0

:build
    cmake --preset %1 .
    cd out\build\%1
    cmake --build .
    cd ..\..\..

    call:cp %1
goto:eof

:cp
    cd out\build
    mkdir bin
    copy %1\QHotReload\*.exe bin
    cd ..\..
goto:eof
