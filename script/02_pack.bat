@echo off
chcp 65001

call:build "Qt5.6.3-msvc2013" "NSIS"
call:build "Qt5.12.5-msvc2017_64" "NSIS64"
call:build "Qt5.15.2-msvc2019_64" "NSIS64"
exit 0

:build
    cd out\build\%1
    cpack -G %2
    cd ..\..\..

    call:cp %1
goto:eof

:cp
    cd out\build
    mkdir bin
    copy %1\QHotReload*.exe bin
    cd ..\..
goto:eof
