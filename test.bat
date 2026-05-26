@echo off
for %%f in (exemplos\*.class) do (
    build\leitor.exe %%f > nul 2>&1 && echo PASS %%f || echo FAIL %%f
)