@echo off
if not exist output mkdir output
for %%f in (exemplos\*.class) do (
    build\leitor.exe %%f > output\%%~nf.txt && echo Gerado: output\%%~nf.txt
)