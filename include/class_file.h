#ifndef CLASS_FILE_H
#define CLASS_FILE_H

/**
 * @file class_file.h
 * @brief Estruturas de dados que representam o formato binario de um arquivo .class.
 *
 * Segue a especificacao JVMS (Java Virtual Machine Specification) capitulo 4.
 * Todos os valores numericos sao armazenados em big-endian conforme o formato.
 */

#include "types.h"

/* ------------------------------------------------------------------ */
/* Tags do Constant Pool (JVMS Tabela 4.4-A)                           */
/* ------------------------------------------------------------------ */
#define CP_UTF8                1
#define CP_INTEGER             3
#define CP_FLOAT               4
#define CP_LONG                5
#define CP_DOUBLE              6
#define CP_CLASS               7
#define CP_STRING              8
#define CP_FIELDREF            9
#define CP_METHODREF           10
#define CP_INTERFACE_METHODREF 11
#define CP_NAME_AND_TYPE       12

/* ------------------------------------------------------------------ */
/* Access flags de classe (JVMS 4.1)                                   */
/* ------------------------------------------------------------------ */
#define ACC_PUBLIC     0x0001
#define ACC_FINAL      0x0010
#define ACC_SUPER      0x0020
#define ACC_INTERFACE  0x0200
#define ACC_ABSTRACT   0x0400
#define ACC_SYNTHETIC  0x1000
#define ACC_ANNOTATION 0x2000
#define ACC_ENUM       0x4000

/* ------------------------------------------------------------------ */
/* Access flags de campo e metodo (JVMS 4.5, 4.6)                      */
/* ------------------------------------------------------------------ */
#define ACC_PRIVATE      0x0002
#define ACC_PROTECTED    0x0004
#define ACC_STATIC       0x0008
#define ACC_VOLATILE     0x0040
#define ACC_TRANSIENT    0x0080
#define ACC_NATIVE       0x0100
#define ACC_SYNCHRONIZED 0x0020
#define ACC_STRICT       0x0800
#define ACC_BRIDGE       0x0040
#define ACC_VARARGS      0x0080

/**
 * @brief Uniao com os dados de cada entrada do constant pool.
 *
 * Cada variante corresponde a um tag CP_*.
 * Utf8: bytes alocados dinamicamente (deve ser liberado em free_class_file).
 * Long e Double ocupam duas entradas; a segunda fica com tag 0 (vazio).
 */
typedef union {
    /** CP_UTF8: string de comprimento fixo sem terminador nulo */
    struct {
        u2   length;
        u1  *bytes; /**< use length para limitar; tem '\0' adicional para printf */
    } utf8;

    /** CP_INTEGER: 4 bytes que formam um int32 big-endian */
    struct { u4 bytes; } integer_val;

    /** CP_FLOAT: 4 bytes IEEE 754 big-endian */
    struct { u4 bytes; } float_val;

    /** CP_LONG: high_bytes:low_bytes formam int64 big-endian */
    struct { u4 high_bytes; u4 low_bytes; } long_val;

    /** CP_DOUBLE: high_bytes:low_bytes formam double IEEE 754 big-endian */
    struct { u4 high_bytes; u4 low_bytes; } double_val;

    /** CP_CLASS: name_index aponta para CP_UTF8 com o nome da classe */
    struct { u2 name_index; } class_info;

    /** CP_STRING: string_index aponta para CP_UTF8 */
    struct { u2 string_index; } string_info;

    /** CP_FIELDREF, CP_METHODREF, CP_INTERFACE_METHODREF */
    struct {
        u2 class_index;          /**< indice CP_CLASS */
        u2 name_and_type_index;  /**< indice CP_NAME_AND_TYPE */
    } ref;

    /** CP_NAME_AND_TYPE */
    struct {
        u2 name_index;       /**< indice CP_UTF8 com o nome */
        u2 descriptor_index; /**< indice CP_UTF8 com o descriptor */
    } name_and_type;
} CpData;

/**
 * @brief Entrada do constant pool.
 *
 * tag == 0 indica slot vazio (parte de uma entrada Long/Double anterior).
 */
typedef struct {
    u1     tag;  /**< um dos CP_* definidos acima, ou 0 para slot vazio */
    CpData data;
} cp_info;

/**
 * @brief Atributo generico (nome + bytes brutos).
 *
 * Atributos nao reconhecidos sao armazenados aqui e ignorados pelo leitor,
 * garantindo que arquivos com atributos futuros continuem sendo lidos.
 */
typedef struct {
    u2  name_index; /**< indice UTF8 no CP com o nome do atributo */
    u4  length;     /**< tamanho em bytes do campo info */
    u1 *info;       /**< bytes brutos; NULL se length == 0 */
} attribute_info;

/**
 * @brief Entrada da tabela de excecoes do Code_attribute.
 *
 * Define um intervalo [start_pc, end_pc) protegido por um handler.
 */
typedef struct {
    u2 start_pc;   /**< inicio do bloco try (offset inclusivo) */
    u2 end_pc;     /**< fim do bloco try (offset exclusivo) */
    u2 handler_pc; /**< inicio do bloco catch/finally */
    u2 catch_type; /**< indice CP_CLASS ou 0 para finally (qualquer excecao) */
} exception_entry;

/**
 * @brief Atributo Code de um metodo (JVMS 4.7.3).
 *
 * Contem os bytecodes, limites de pilha/variaveis locais,
 * tabela de excecoes e sub-atributos (LineNumberTable, etc.).
 */
typedef struct {
    u2              max_stack;               /**< tamanho maximo da pilha de operandos */
    u2              max_locals;              /**< numero de variaveis locais */
    u4              code_length;             /**< numero de bytes de bytecode */
    u1             *code;                    /**< array de bytecodes */
    u2              exception_table_length;  /**< numero de entradas na tabela */
    exception_entry *exception_table;        /**< NULL se vazia */
    u2              attributes_count;        /**< numero de sub-atributos */
    attribute_info *sub_attributes;          /**< sub-atributos (LineNumberTable, etc.) */
} Code_attribute;

/**
 * @brief Informacoes de um campo (field) da classe (JVMS 4.5).
 */
typedef struct {
    u2              access_flags;
    u2              name_index;        /**< indice CP_UTF8 com o nome */
    u2              descriptor_index;  /**< indice CP_UTF8 com o descriptor */
    u2              attributes_count;
    attribute_info *attributes;        /**< pode incluir ConstantValue */
} field_info;

/**
 * @brief Informacoes de um metodo da classe (JVMS 4.6).
 *
 * code_attr aponta para o atributo Code parseado (NULL para nativos/abstratos).
 */
typedef struct {
    u2              access_flags;
    u2              name_index;        /**< indice CP_UTF8 com o nome */
    u2              descriptor_index;  /**< indice CP_UTF8 com o descriptor */
    u2              attributes_count;
    attribute_info *attributes;        /**< pode incluir Exceptions */
    Code_attribute *code_attr;         /**< NULL se nativo ou abstrato */
} method_info;

/**
 * @brief Representacao completa de um arquivo .class (JVMS 4.1).
 *
 * Todos os arrays sao alocados com new[] e devem ser liberados
 * via free_class_file() para evitar vazamentos de memoria.
 */
typedef struct {
    u4          magic;                /**< deve ser 0xCAFEBABE */
    u2          minor_version;
    u2          major_version;
    u2          constant_pool_count;  /**< numero de entradas + 1 (indice 0 nao usado) */
    cp_info    *constant_pool;        /**< indices validos: 1..constant_pool_count-1 */
    u2          access_flags;
    u2          this_class;           /**< indice CP_CLASS desta classe */
    u2          super_class;          /**< indice CP_CLASS da superclasse, ou 0 */
    u2          interfaces_count;
    u2         *interfaces;           /**< indices CP_CLASS das interfaces implementadas */
    u2          fields_count;
    field_info *fields;
    u2          methods_count;
    method_info*methods;
    u2          attributes_count;
    attribute_info *attributes;       /**< pode incluir SourceFile */
} ClassFile;

#endif /* CLASS_FILE_H */
