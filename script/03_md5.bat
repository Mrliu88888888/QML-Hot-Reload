@echo off
chcp 65001

cd out\build\bin
powershell -c "Get-ChildItem -Exclude *.md5 | Get-FileHash -Algorithm MD5 | Format-List | Out-File QHotReload.md5"
cd ..\..\..
