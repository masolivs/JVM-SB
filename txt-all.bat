@echo off
if not exist output mkdir output
for %%f in (tests\class\*.class) do (
    build\jvm.exe -d %%f > output\%%~nf.txt && echo Gerado: output\%%~nf.txt
)