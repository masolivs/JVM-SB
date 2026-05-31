@echo off
for %%f in (tests\class\*.class) do (
    build\jvm.exe %%f > nul 2>&1 && echo PASS %%f || echo FAIL %%f
)