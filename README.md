# JVM Simplificada — Software Básico · UnB

Implementação de uma Java Virtual Machine simplificada em C++11,
desenvolvida para a disciplina de Software Básico.

## Integrantes

- Breno Back dos Santos Miranda da Silva - 190063980
- Danilo Silveira da Silva - 222014142
- Gustavo Vieira de Araújo - 211068440
- Julia Paulo Amorim - 241039270
- Leticia Gonçalves Bonfim - 241039270
- Mariana Soares Oliveira - 231013663

## Requisitos

- MSYS2 com UCRT64
- g++ 11+
- make
- cppcheck
- doxygen + graphviz

## Como compilar

```bash
make
```

## Como rodar o exibidor (modo -d)

```bash
make display CLASS=tests/class/HelloWorld.class
```

## Como rodar a JVM

```bash
make run CLASS=HelloWorld
```

## Como rodar análise estática

```bash
make check
```

## Como rodar com sanitizers

```bash
make sanitize
./build/jvm tests/class/HelloWorld.class
```

## Como gerar documentação

```bash
make docs
```

Abre `docs/html/index.html` no navegador.

## Estrutura do projeto

```
include/    → headers (.h)
src/        → implementações (.cpp)
src/opcodes → um arquivo por categoria de opcode
tests/java  → arquivos .java de teste
tests/class → arquivos .class compilados
docs/       → documentação gerada pelo Doxygen
```