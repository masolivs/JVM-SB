# Leitor e Exibidor de Arquivos .class — JVM_SB

Ferramenta em C++ que lê arquivos `.class` no formato binário da JVM e exibe
suas estruturas internas de forma similar ao [jclasslib](https://github.com/ingokegel/jclasslib)
e ao `javap -verbose`, sem precisar de JRE instalada.

Desenvolvida como trabalho prático — **Software Básico (CIC0104)**, UnB — 2026/1.

---

## Requisitos

- Compilador C++11: `g++` (GCC ≥ 4.8) ou `clang++`
- `make`
- `cppcheck` (opcional, para análise estática)

---

## Compilar e executar

```bash
make                                  # compila em build/leitor
./build/leitor exemplos/Fibonacci.class
./build/leitor -o Fibonacci.txt exemplos/Fibonacci.class   # salva em arquivo
```

### Saída em .txt para todos os exemplos de uma vez

```bash
make txt-all                          # gera exemplos/*.txt
make txt CLASS=exemplos/Belote.class  # gera somente este
```

### Redirecionamento manual equivalente

```bash
./build/leitor exemplos/HelloWorld.class > HelloWorld.txt
```

---

## Análise de qualidade

### Análise estática (cppcheck)

```bash
make check
# Relatório gerado em build/cppcheck.log
```

### Análise dinâmica (sanitizers)

```bash
make asan   # AddressSanitizer + UBSan — detecta leaks, UB, out-of-bounds
make tsan   # ThreadSanitizer

# Após make asan, execute normalmente:
ASAN_OPTIONS=detect_leaks=1 ./build/leitor exemplos/cafebabe.class
```

---

## O que é exibido

### 1. Cabeçalho
- Magic number (`0xCAFEBABE`)
- Versões minor e major com nome Java (Java 5 a Java 21)
- `SourceFile` — nome do arquivo `.java` original

### 2. Constant Pool
- Índice `#N`, tag, índices internos e **valor resolvido recursivamente**
- Todos os 11 tipos: Utf8, Integer, Float, Long, Double, Class, String,
  Fieldref, Methodref, InterfaceMethodref, NameAndType
- Slots vazios de Long/Double sinalizados

### 3. Informações da classe
- Access flags traduzidos (public, final, super, interface, abstract, enum…)
- `this_class` e `super_class` com índice e nome resolvido
- Interfaces implementadas

### 4. Fields — separados em estáticos e de instância
- Access flags traduzidos + descriptor
- `ConstantValue` exibido inline para campos `static final`

### 5. Métodos
- Access flags + descriptor
- Cláusula `throws` (atributo `Exceptions`)
- Atributo `Code`: `max_stack`, `max_locals`, `code_length`
- **Disassembly** com offset, mnemônico e operandos resolvidos do CP:
  - `ldc` / `ldc_w`: resolve `CP_STRING`, `CP_INTEGER`, `CP_FLOAT`, `CP_CLASS`
  - `ldc2_w`: resolve `CP_LONG` e `CP_DOUBLE`
  - Branches: offset relativo **e** absoluto `(-> N)`
  - `tableswitch`: todos os cases + default com targets absolutos
  - `lookupswitch`: todos os pares key/offset + default
  - Todos os `invoke*`, `getstatic`/`putfield`, `new`, `checkcast` com CP resolvido
- Tabela de exceções (`exception_table`) com tipos
- `LineNumberTable` — linha do `.java` para cada bytecode

---

## Estrutura do projeto

```
JVM-SB/
├── Makefile                  # build, txt, cppcheck, asan, tsan
├── README.md
├── Doxyfile                  # config Doxygen
├── include/
│   ├── class_file.h          # structs do formato .class (ClassFile, cp_info…)
│   ├── class_reader.h        # API do leitor
│   ├── constant_pool.h       # funções de resolução do CP
│   ├── attributes.h          # parse de Code, LineNumberTable, Exceptions,
│   │                         #   SourceFile, ConstantValue
│   ├── opcodes.h             # tabela mnemonic[256]
│   ├── displayer.h           # API do exibidor
│   ├── errors.h              # enum JvmError
│   └── types.h               # tipos primitivos (u1, u2, u4)
├── src/
│   ├── main.cpp              # ponto de entrada (suporta -o)
│   ├── class_reader.cpp      # leitura BIG-ENDIAN + free_class_file
│   ├── constant_pool.cpp     # resolução recursiva de entradas do CP
│   ├── attributes.cpp        # parsing de todos os atributos suportados
│   ├── opcodes.cpp           # tabela de mnemônicos (0x00–0xCA)
│   └── displayer.cpp         # exibição formatada de todas as seções
└── exemplos/                 # arquivos .class para teste
```
---

## Integrantes

| Nome | Matrícula |
|------|-----------|
| Breno Back dos Santos Miranda da Silva | 190063980 |
| Danilo Silveira da Silva | 222014142 |
| Gustavo Vieira de Araújo | 211068440 |
| Julia Paulo Amorim | 241039270 |
| Leticia Gonçalves Bomfim | 241002411 |
| Mariana Soares Oliveira | 231013663 |
