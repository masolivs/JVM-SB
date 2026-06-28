#!/bin/bash
for f in tests/class/*.class; do
    if ./build/jvm.exe "$f" > /dev/null 2>&1; then
        echo "PASS $f"
    else
        echo "FAIL $f"
    fi
done