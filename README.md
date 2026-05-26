# Leitor e Exibidor de Arquivos `.class`

Ferramenta de linha de comando em C++ que lê arquivos `.class` no formato binário da JVM e exibe suas estruturas internas de forma similar ao [jclasslib](https://github.com/ingokegel/jclasslib) e ao `javap -verbose` — sem precisar de JRE instalada.

Desenvolvido como trabalho prático de **Software Básico (CIC0104)** — Universidade de Brasília — 2026/1.

---

## Requisitos

- `g++` com suporte a C++11 (GCC ≥ 4.8)
- `make`
- Linux (o projeto foi desenvolvido e testado exclusivamente no Linux)

---

## Como compilar

```bash
make
```

O executável é gerado em `build/leitor`. As flags de compilação utilizadas são:

```
g++ -std=c++11 -Wall -Wextra -Wpedantic -g
```

---

## Como executar

```bash
./build/leitor <arquivo.class>
./build/leitor -o saida.txt <arquivo.class>    # salva saída em arquivo
```

**Exemplos práticos:**

```bash
./build/leitor exemplos/HelloWorld.class
./build/leitor exemplos/fatorial.class
./build/leitor exemplos/lookupswitch.class
./build/leitor exemplos/double_aritmetica.class
```

**Gerar `.txt` para todos os exemplos de uma vez:**

```bash
make txt-all    # saída em output/<nome>.txt
```

**Rodar todos os testes:**

```bash
make test
```

---

## O que o leitor exibe

O leitor percorre o arquivo `.class` binário (big-endian) e imprime cinco seções em sequência.

### 1. Cabeçalho

Valida o `magic number` e exibe as versões e o arquivo `.java` de origem.

```
=== Cabecalho ============================================
  Magic:          0xCAFEBABE
  Minor version:  0
  Major version:  52  (Java 8)
  SourceFile:     HelloWorld.java
```

Se o arquivo não for um `.class` válido (magic ≠ `0xCAFEBABE`), o leitor rejeita com erro. Versões suportadas: Java 1.1 a Java 21 (major 45–65).

---

### 2. Constant Pool

Exibe todas as entradas com índice, tipo e **valor resolvido recursivamente** — da mesma forma que o jclasslib.

```
=== Constant Pool (28 entradas) ==========================
  #1    = Methodref          #2.#3   // java/lang/Object.<init>:()V
  #2    = Class              #4      // java/lang/Object
  #3    = NameAndType        #5.#6   // <init>:()V
  #7    = Fieldref           #8.#9   // java/lang/System.out:Ljava/io/PrintStream;
  #13   = String             #14     // "Hello, World!"
```

A resolução é recursiva: um `Methodref` mostra o nome da classe e o `NameAndType` resolvidos, que por sua vez resolvem suas entradas `Utf8`. Todos os 11 tipos do CP são suportados:

| Tag | Tipo | Exibição |
|-----|------|----------|
| 1 | `Utf8` | string literal |
| 3 | `Integer` | valor decimal |
| 4 | `Float` | valor float |
| 5 | `Long` | valor long |
| 6 | `Double` | valor double |
| 7 | `Class` | nome da classe |
| 8 | `String` | string resolvida |
| 9 | `Fieldref` | `Classe.campo:tipo` |
| 10 | `Methodref` | `Classe.metodo:descriptor` |
| 11 | `InterfaceMethodref` | `Interface.metodo:descriptor` |
| 12 | `NameAndType` | `nome:descriptor` |

Slots duplos de `Long` e `Double` são sinalizados corretamente:

```
  #2    = Double             98.240000
  #3      (slot vazio — parte de Long/Double anterior)
```

---

### 3. Informações da Classe

```
=== Informacoes da Classe ================================
  Access flags:  0x0021  (public super)
  This class:    #21  // HelloWorld
  Super class:   #2   // java/lang/Object
  Interfaces (0):
```

Access flags traduzidos: `public`, `final`, `super`, `interface`, `abstract`, `annotation`, `enum`, `synthetic`.

---

### 4. Fields

Campos estáticos e de instância são exibidos separadamente. Campos `static final` mostram o `ConstantValue` inline.

```
=== Fields (2) ==========================================
  --- Fields estaticos (1) ---
    [0x0019] public static final  MAX  I = 100
  --- Fields de instancia (1) ---
    [0x0002] private               nome  Ljava/lang/String;
```

---

### 5. Métodos — Bytecodes com mnemônicos

Esta é a seção principal. Para cada método são exibidos: access flags, cláusula `throws`, `max_stack`, `max_locals`, disassembly completo e `LineNumberTable`.

**Exemplo — `HelloWorld.main`:**

```
  Method #1: main  ([Ljava/lang/String;)V
    Access flags: 0x0009  (public static)
    Code:
      max_stack=2  max_locals=1  code_length=9
       0: getstatic        #7   // java/lang/System.out:Ljava/io/PrintStream;
       3: ldc              #13  // "Hello, World!"
       5: invokevirtual    #15  // java/io/PrintStream.println:(Ljava/lang/String;)V
       8: return
    LineNumberTable:
      line 3: 0
      line 4: 8
```

**Exemplo — fatorial recursivo (`fatorial.fat`):**

```
  Method #2: fat  (I)I
    Access flags: 0x0009  (public static)
    Code:
      max_stack=3  max_locals=1  code_length=15
       0: iload_0
       1: ifne             5   (-> 6)
       4: iconst_1
       5: ireturn
       6: iload_0
       7: iload_0
       8: iconst_1
       9: isub
      10: invokestatic     #3  // fatorial.fat:(I)I
      13: imul
      14: ireturn
    LineNumberTable:
      line 9: 0
      line 11: 4
      line 15: 6
```

**Exemplo — `lookupswitch` com pares chave/offset:**

```
  Method #0: chooseFar  (I)I
    Code:
       0: iload_0
       1: lookupswitch     { // 3
                   -100: 36
                      0: 38
                    100: 40
                 default: 42
              }
      36: iconst_m1
      37: ireturn
```

#### Resolução de operandos

Instruções que referenciam o Constant Pool exibem o valor resolvido após `//`:

| Instrução | Formato exibido |
|-----------|----------------|
| `ldc`, `ldc_w`, `ldc2_w` | `#N // valor` |
| `getstatic`, `putstatic`, `getfield`, `putfield` | `#N // Classe.campo:tipo` |
| `invokevirtual`, `invokespecial`, `invokestatic` | `#N // Classe.metodo:descriptor` |
| `invokeinterface` | `#N, count // Interface.metodo:descriptor` |
| `new`, `anewarray`, `checkcast`, `instanceof` | `#N // NomeClasse` |
| `ifeq`..`if_acmpne`, `goto`, `jsr` | `offset (-> absoluto)` |
| `tableswitch` | todos os cases com target absoluto |
| `lookupswitch` | todos os pares chave → target absoluto |

---

## Resultados nos exemplos do professor

Testado nos 21 arquivos do `Exemplos.rar`:

- **20/21** lidos e exibidos corretamente
- **`cafebabe.class`** rejeitado com `exit code 1` — comportamento correto:
  o arquivo começa com `0x4D5A` (cabeçalho PE do Windows), não com `0xCAFEBABE`

```bash
$ ./build/leitor exemplos/cafebabe.class
Erro ao ler 'exemplos/cafebabe.class': codigo 1
```

---

## O que não foi implementado

- Atributos opcionais: `InnerClasses`, `EnclosingMethod`, `Signature`, `StackMapTable`, `BootstrapMethods`
- `invokedynamic` — exibe apenas o índice no CP (sem resolver `BootstrapMethods`)
- Execução dos bytecodes — este projeto é exclusivamente leitor e exibidor

---

## Estrutura do projeto

```
JVM-SB/
├── Makefile
├── README.md
├── exemplos/              ← 21 arquivos .class do professor
├── include/
│   ├── types.h            ← tipos primitivos (u1, u2, u4)
│   ├── errors.h           ← enum JvmError
│   ├── class_file.h       ← structs do formato .class (ClassFile, cp_info…)
│   ├── class_reader.h     ← leitura e liberação de ClassFile
│   ├── constant_pool.h    ← funções de resolução do CP
│   ├── attributes.h       ← parse de Code, LineNumberTable, Exceptions…
│   ├── opcodes.h          ← enum Opcode + tabela mnemonic[256]
│   └── displayer.h        ← exibição formatada
└── src/
    ├── main.cpp           ← ponto de entrada (suporta -o)
    ├── class_reader.cpp   ← leitura BIG-ENDIAN + free_class_file
    ├── constant_pool.cpp  ← resolução recursiva do CP
    ├── attributes.cpp     ← parsing de todos os atributos suportados
    ├── opcodes.cpp        ← tabela mnemonic[256] (0x00–0xC9)
    └── displayer.cpp      ← formatação de todas as seções
```

---

## Integrantes

| # | Nome | Matrícula | Contribuição principal |
|---|------|-----------|----------------------|
| 1 | Leticia Gonçalves Bomfim | 241002411 | Parser binário BIG-ENDIAN, cabeçalho, flags, interfaces, fields, atributos |
| 2 | Julia Paulo Amorim | 241039270 | Exibição do Constant Pool com resolução recursiva |
| 3 | Gustavo Vieira de Araújo | 211068440 | Tabela `mnemonic[256]` — mapeamento de todos os opcodes |
| 4 | Breno Back dos Santos Miranda da Silva | 190063980 | Resolução de operandos do CP no disassembly, tableswitch/lookupswitch |
| 5 | Mariana Soares Oliveira | 231013663 | QA — testes nos 21 exemplos, validação de saída, instruções de compilação |
| 6 | Danilo Silveira da Silva | 222014142 | Slides, roteiro do vídeo e edição final |
