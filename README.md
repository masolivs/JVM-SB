# Leitor e Exibidor de Arquivos .class

Ferramenta em C++ que lê arquivos `.class` no formato binário da JVM (Java Virtual
Machine) e exibe suas estruturas internas de forma similar ao visualizador
[jclasslib](https://github.com/ingokegel/jclasslib) — sem precisar de JRE instalada.

Desenvolvida como trabalho prático da disciplina **Software Básico (CIC0104)**,
Universidade de Brasília — 2026/1.

---

## Requisitos

- Compilador C++ com suporte a C++11: `g++` (GCC) ou `clang++`
- `make`

---

## Compilar

```bash
make
```

O binário é gerado em `build/leitor`.

### Flags de compilação utilizadas

```
g++ -std=c++11 -Wall -Wextra -Wpedantic -g
```

### Limpar build

```bash
make clean
```

---

## Usar

```bash
./build/leitor <arquivo.class>
```

### Exemplos

```bash
./build/leitor exemplos/fatorial.class
./build/leitor exemplos/fibonacci.class
./build/leitor exemplos/lookupswitch.class
./build/leitor exemplos/Belote.class
```

---

## O que é exibido

### 1. Cabeçalho
- Magic number (`0xCAFEBABE`)
- Versão minor e major (com nome da versão Java correspondente)

### 2. Constant Pool
- Índice de cada slot (`#1`, `#2`, `#3`…)
- Tipo da constante (Utf8, Integer, Float, Long, Double, Class, String, Fieldref, Methodref, InterfaceMethodref, NameAndType)
- Valor resolvido recursivamente — por exemplo:
  ```
  #1  = Methodref  #6.#17  // java/lang/Object.<init>:()V
  #17 = NameAndType #7.#8  // <init>:()V
  ```

### 3. Informações da classe
- Flags de acesso (`public`, `final`, `abstract`, `interface`…)
- This class e Super class (índice + nome resolvido)
- Lista de interfaces implementadas

### 4. Campos (Fields)
- Nome, descriptor e flags de acesso de cada campo

### 5. Métodos
- Nome, descriptor e flags de acesso
- Atributo Code: `max_stack`, `max_locals`, `code_length`
- Bytecodes com offset, mnemônico e argumento resolvido do CP:
  ```
  0: getstatic      #2  // java/lang/System.out:Ljava/io/PrintStream;
  3: bipush         6
  5: invokestatic   #3  // fatorial.fat:(I)I
  8: invokevirtual  #4  // java/io/PrintStream.println:(I)V
  ```
- `tableswitch` e `lookupswitch` com todos os cases e offsets:
  ```
  1: lookupswitch { // 3
              -100: 36
                 0: 38
               100: 40
           default: 42
       }
  ```
- Tabela de exceções (exception table) quando presente

---

## O que não está implementado

- Atributos extras: `LineNumberTable`, `LocalVariableTable`, `StackMapTable`, `InnerClasses`, `Annotations`
- Execução do bytecode (este projeto é apenas leitor/exibidor)

---

## Estrutura do projeto

```
leitor_exibidor/
├── Makefile
├── README.md
├── include/
│   ├── class_file.h      # structs do formato .class (ClassFile, cp_info, method_info…)
│   ├── class_reader.h    # interface do leitor
│   ├── constant_pool.h   # funções de resolução do CP
│   ├── attributes.h      # parse do Code_attribute e exception_table
│   ├── opcodes.h         # tabela mnemonic[256]
│   ├── displayer.h       # interface do exibidor
│   ├── errors.h          # enum JvmError
│   └── types.h           # tipos primitivos (u1, u2, u4)
├── src/
│   ├── main.cpp          # ponto de entrada
│   ├── class_reader.cpp  # leitura BIG-ENDIAN do binário .class
│   ├── constant_pool.cpp # resolução recursiva de entradas do CP
│   ├── attributes.cpp    # parsing do atributo Code
│   ├── opcodes.cpp       # tabela completa de mnemônicos (0x00–0xCA)
│   └── displayer.cpp     # exibição formatada de todas as seções
└── exemplos/             # arquivos .class para teste
```

---

## Divisão de tarefas

| Pessoa | Arquivos | Contribuição |
|--------|----------|--------------|
| Pessoa_1 | `include/class_file.h`, `types.h`, `errors.h` | Definiu todas as structs do formato .class: `ClassFile`, `cp_info`, `method_info`, `Code_attribute`, `exception_entry`, tipos primitivos e códigos de erro |
| Pessoa_2 | `src/class_reader.cpp`, `include/class_reader.h` | Implementou o leitor binário BIG-ENDIAN: leitura do magic number, versões, flags, interfaces, campos, métodos e atributos direto do arquivo `.class` |
| Pessoa_3 | `src/constant_pool.cpp`, `include/constant_pool.h` | Implementou a resolução do Constant Pool: `resolve_utf8`, `resolve_class_name`, `resolve_methodref`, `resolve_fieldref`, `resolve_nameandtype`, `resolve_string` |
| Pessoa_4 | `src/attributes.cpp`, `include/attributes.h` | Implementou o parsing do atributo `Code` (max_stack, max_locals, bytecodes, exception_table) e demais atributos |
| Pessoa_5 | `src/opcodes.cpp`, `include/opcodes.h`, `include/displayer.h` | Implementou a tabela completa de mnemônicos (`mnemonic[256]`) para todos os 202 opcodes da JVM e definiu a interface do exibidor |
| Pessoa_6 | `src/displayer.cpp`, `src/main.cpp`, `Makefile`, `README.md` | Implementou o exibidor completo (CP, flags, campos, métodos, bytecodes com argumentos resolvidos, tableswitch/lookupswitch), ponto de entrada e documentação |
