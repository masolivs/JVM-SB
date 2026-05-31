#!/usr/bin/env bash
set -euo pipefail

JVM=./build/jvm
CLASSDIR=tests/class
OUTDIR=output

if [ ! -x "$JVM" ]; then
    echo "Erro: '$JVM' nao encontrado. Execute 'make -f Makefile.linux' primeiro." >&2
    exit 1
fi

mkdir -p "$OUTDIR"

for f in "$CLASSDIR"/*.class; do
    name=$(basename "$f" .class)
    if "$JVM" -d -o "$OUTDIR/$name.txt" "$f"; then
        echo "Gerado: $OUTDIR/$name.txt"
    else
        echo "ERRO ao processar: $f" >&2
    fi
done

cat "$OUTDIR"/*.txt > "$OUTDIR/all.txt"
echo "Gerado: $OUTDIR/all.txt"