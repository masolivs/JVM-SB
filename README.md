# JVM Interpretador — Software Básico (CIC0104) — UnB 2026/1

Implementação de uma Java Virtual Machine em C++11 capaz de **ler, exibir e executar** arquivos `.class` do formato binário da JVM — sem precisar de JRE instalada.

---

## Requisitos

- `g++` com suporte a C++11 (GCC ≥ 4.8)
- `make` (Linux) ou `mingw32-make` (Windows, via [MinGW-w64](https://www.mingw-w64.org/))

---

## Como compilar

### Linux

```bash
make -f Makefile.linux
```

O executável é gerado em `build/jvm`.

### Windows (MinGW-w64)

```bat
make.bat
```

O executável é gerado em `build\jvm.exe`.

---

## Como executar

### Modo interpretador — executa o `main` da classe

```bash
./build/jvm tests/class/HelloWorld.class       # Linux
build\jvm.exe tests\class\HelloWorld.class     # Windows
```

### Modo exibidor — lê e exibe a estrutura do `.class`

```bash
./build/jvm -d tests/class/HelloWorld.class    # Linux
build\jvm.exe -d tests\class\HelloWorld.class  # Windows
```

### Rodar todos os testes

```bash
make -f Makefile.linux test    # Linux
test.bat                       # Windows
```

---

## O que é implementado

### Leitor/Exibidor (modo `-d`)

- Parser BIG-ENDIAN completo do formato `.class`
- Constant Pool com resolução recursiva de todos os 11 tipos
- Disassembly com mnemônicos, operandos resolvidos, `tableswitch`/`lookupswitch`

### Interpretador (modo padrão)

- **Dispatch table O(1)** — `OpcodeHandler dispatch[256]` indexada pelo byte do opcode
- **Pilha de frames** — `JvmStack` com `Frame` (operand stack + variáveis locais + PC)
- **Heap unificado** — `vector<HeapEntry>` para objetos e arrays; referências como `int32_t`
- **Área de métodos** — `unordered_map<string, ClassEntry*>` com carregamento automático
- **Objetos** — `JObject` com campos de instância cobrindo toda a hierarquia de herança
- **Arrays** — `JArray` com campo `arraylength` explícito; bounds check automático
- **Herança e polimorfismo** — `find_method_ex` retorna a classe declarante do método
- **Exceções** — `athrow`, `exception_table`, propagação entre frames

Opcodes implementados por categoria em `src/opcodes/`:
`arithmetic · load_store · stack_ops · control · invoke`
`field_ops · object_ops · array_ops · convert · exceptions`

---

## O que não é implementado

- Garbage Collector — objetos vivem até o fim da execução
- Biblioteca padrão Java (JRE) — sem `java.lang`, `java.util`, `java.io` reais
- `invokedynamic` / `BootstrapMethods`
- Multithreading
- Reflection

---

## Estrutura do projeto

```
Interpretador/
├── Makefile               ← build Windows (mingw32-make)
├── Makefile.linux         ← build Linux (make -f Makefile.linux)
├── make.bat               ← atalho Windows
├── test.bat               ← testes Windows
├── txt-all.bat            ← gera .txt de todos os .class (Windows)
├── Doxyfile               ← config Doxygen
├── README.md
├── include/
│   ├── types.h            ← tipos primitivos (u1, u2, u4)
│   ├── errors.h           ← enum JvmError
│   ├── class_file.h       ← structs do formato .class
│   ├── class_reader.h     ← leitura e liberação de ClassFile
│   ├── constant_pool.h    ← resolução do Constant Pool
│   ├── attributes.h       ← parse de Code, LineNumberTable, Exceptions…
│   ├── opcodes.h          ← enum Opcode + tabela mnemonic[256]
│   ├── displayer.h        ← exibição formatada
│   ├── frame.h            ← Frame (operand stack + local vars + PC)
│   ├── jvm_stack.h        ← pilha de frames
│   ├── object.h           ← JObject (campos de instância)
│   ├── array.h            ← JArray (arraylength + elements)
│   ├── method_area.h      ← MethodArea + ClassEntry
│   └── interpreter.h      ← JVM, dispatch table, heap, interpret loop
├── src/
│   ├── main.cpp           ← ponto de entrada (-d exibidor | padrão interpretador)
│   ├── class_reader.cpp
│   ├── constant_pool.cpp
│   ├── attributes.cpp
│   ├── opcodes.cpp
│   ├── displayer.cpp
│   ├── frame.cpp
│   ├── jvm_stack.cpp
│   ├── object.cpp
│   ├── array.cpp
│   ├── method_area.cpp
│   ├── interpreter.cpp
│   └── opcodes/           ← handlers por categoria de opcode
│       ├── arithmetic.cpp
│       ├── load_store.cpp
│       ├── stack_ops.cpp
│       ├── control.cpp
│       ├── invoke.cpp
│       ├── field_ops.cpp
│       ├── object_ops.cpp
│       ├── array_ops.cpp
│       ├── convert.cpp
│       └── exceptions.cpp
├── tests/
│   ├── class/             ← arquivos .class de teste
│   └── java/              ← arquivos .java de teste
└── exemplos/              ← arquivos .class do professor (leitor/exibidor)
```

---

## Integrantes

| # | Nome | Matrícula | Contribuição principal |
|---|------|-----------|----------------------|
| 1 | Leticia Gonçalves Bomfim | 241002411 | Structs do formato `.class` — `class_file.h` |
| 2 | Julia Paulo Amorim | 241039270 | Parser BIG-ENDIAN — `class_reader.cpp`, validação do magic number |
| 3 | Danilo Silveira da Silva | 222014142 | Displayer completo, `main.cpp`, Makefile e exemplos de teste |
| 4 | Mariana Soares Oliveira | 231013663 | Estrutura inicial, tabela `mnemonic[256]`, parsers de atributos, resolução do `ldc`, unificação do build |
| 5 | Breno Back dos Santos Miranda da Silva | 190063980 | Parsing do `Code_attribute`, exception table e sub-atributos |
| 6 | Gustavo Vieira de Araújo | 211068440 | Resolução recursiva do Constant Pool, interpreter loop, dispatch table, heap, frames, herança, exceções, área de métodos |
