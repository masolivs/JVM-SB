@echo off
if not exist output mkdir output
for %%f in (tests\class\*.class) do (
    build\jvm.exe -d -o output\%%~nf.txt %%f && echo Gerado: output\%%~nf.txt
)
type output\*.txt > output\all.txt
echo Gerado: output\all.txt