#ifndef CLASS_FILE_H
#define CLASS_FILE_H

#include "types.h"

/* Tags do constant pool conforme especificacao JVM 8 */
#define CP_UTF8               1
#define CP_INTEGER            3
#define CP_FLOAT              4
#define CP_LONG               5
#define CP_DOUBLE             6
#define CP_CLASS              7
#define CP_STRING             8
#define CP_FIELDREF           9
#define CP_METHODREF          10
#define CP_INTERFACE_METHODREF 11
#define CP_NAME_AND_TYPE      12

/* Access flags de classe */
#define ACC_PUBLIC     0x0001
#define ACC_FINAL      0x0010
#define ACC_SUPER      0x0020
#define ACC_INTERFACE  0x0200
#define ACC_ABSTRACT   0x0400

/* Access flags de campo e metodo */
#define ACC_PRIVATE    0x0002
#define ACC_PROTECTED  0x0004
#define ACC_STATIC     0x0008
#define ACC_VOLATILE   0x0040
#define ACC_TRANSIENT  0x0080
#define ACC_NATIVE     0x0100
#define ACC_SYNCHRONIZED 0x0020
#define ACC_STRICT     0x0800

/**
 * @brief Uniao com os dados de cada entrada do constant pool.
 *
 * Cada variante corresponde a um tag CP_*.
 * Utf8: bytes alocados dinamicamente (deve ser liberado).
 * Long e Double ocupam duas entradas; a segunda fica com tag 0.
 */
typedef union {
    /** CP_UTF8 */
    struct {
        u2   length;
        u1  *bytes; /**< string sem terminador nulo; use length */
    } utf8;

    /** CP_INTEGER */
    struct { u4 bytes; } integer_val;

    /** CP_FLOAT */
    struct { u4 bytes; } float_val;

    /** CP_LONG — high_bytes:low_bytes formam int64 big-endian */
    struct { u4 high_bytes; u4 low_bytes; } long_val;

    /** CP_DOUBLE */
    struct { u4 high_bytes; u4 low_bytes; } double_val;

    /** CP_CLASS */
    struct { u2 name_index; } class_info;

    /** CP_STRING */
    struct { u2 string_index; } string_info;

    /** CP_FIELDREF, CP_METHODREF, CP_INTERFACE_METHODREF */
    struct {
        u2 class_index;
        u2 name_and_type_index;
    } ref;

    /** CP_NAME_AND_TYPE */
    struct {
        u2 name_index;
        u2 descriptor_index;
    } name_and_type;
} CpData;

/**
 * @brief Entrada do constant pool.
 */
typedef struct {
    u1     tag;  /**< um dos CP_* acima */
    CpData data;
} cp_info;

/**
 * @brief Atributo generico (nome + bytes brutos).
 *
 * Atributos nao reconhecidos sao armazenados aqui e ignorados.
 *
 * @see Code_attribute — versao parseada do atributo "Code".
 * @see attributes.h — funcoes parse_*_attribute() que interpretam
 *      o array info[] conforme o nome do atributo.
 */
typedef struct {
    u2  name_index; /**< indice UTF8 com o nome do atributo */
    u4  length;
    u1 *info;       /**< bytes brutos; NULL se length == 0 */
} attribute_info;

/**
 * @brief Entrada da tabela de excecoes do Code_attribute.
 *
 * @see Code_attribute
 */
typedef struct {
    u2 start_pc;   /**< inicio do bloco try (inclusive) */
    u2 end_pc;     /**< fim do bloco try (exclusive) */
    u2 handler_pc; /**< inicio do handler */
    u2 catch_type; /**< indice CP_CLASS ou 0 para finally */
} exception_entry;

/**
 * @brief Atributo Code de um metodo.
 *
 * Contem os bytecodes e informacoes de execucao.
 *
 * @see exception_entry — entradas da exception_table.
 * @see parse_code_attribute() em attributes.h — preenche esta struct.
 * @see parse_linenumber_table() em attributes.h — le sub_attributes
 *      em busca de LineNumberTable (debug info).
 */
typedef struct {
    u2              max_stack;
    u2              max_locals;
    u4              code_length;
    u1             *code;             /**< array de bytecodes */
    u2              exception_table_length;
    exception_entry *exception_table;
    u2              attributes_count;      /**< numero de sub-atributos de Code (ex: LineNumberTable) */
    attribute_info *sub_attributes; /**< NULL se vazia */
} Code_attribute;

/**
 * @brief Informacoes de um campo (field) da classe.
 *
 * @see method_info — estrutura analoga para metodos.
 * @see ACC_PUBLIC, ACC_PRIVATE, ACC_PROTECTED, ACC_STATIC,
 *      ACC_FINAL, ACC_VOLATILE, ACC_TRANSIENT — flags possiveis em
 *      access_flags (combinaveis via OR bit a bit).
 */
typedef struct {
    u2              access_flags;
    u2              name_index;
    u2              descriptor_index;
    u2              attributes_count;
    attribute_info *attributes;
} field_info;

/**
 * @brief Informacoes de um metodo da classe.
 *
 * code_attr aponta para o atributo Code parseado (NULL para nativos/abstratos).
 *
 * @see field_info — estrutura analoga para campos.
 * @see Code_attribute — bytecode e dados de execucao do metodo.
 * @see parse_code_attribute() em attributes.h — preenche code_attr.
 *
 * @code
 * for (u2 i = 0; i < cf->methods_count; i++) {
 *     method_info &m = cf->methods[i];
 *     std::string name = resolve_utf8(cf, m.name_index);
 *     if (name == "main" && m.code_attr) {
 *         // m.code_attr->code contem o bytecode do metodo
 *     }
 * }
 * @endcode
 */
typedef struct {
    u2              access_flags;
    u2              name_index;
    u2              descriptor_index;
    u2              attributes_count;
    attribute_info *attributes;
    Code_attribute *code_attr; /**< NULL se metodo nativo ou abstrato */
} method_info;

/**
 * @brief Representacao completa de um arquivo .class.
 *
 * Todos os arrays sao alocados com new[] e devem ser liberados
 * via free_class_file().
 *
 * @see field_info, method_info, attribute_info, cp_info
 * @see read_class_file() em class_reader.h — produz um ClassFile a
 *      partir de um arquivo .class no disco.
 *
 * @code
 * ClassFile *cf = NULL;
 * if (read_class_file("HelloWorld.class", &cf) == JVM_OK) {
 *     std::string class_name = resolve_class_name(cf, cf->this_class);
 *     free_class_file(cf);
 * }
 * @endcode
 */
typedef struct {
    u4          magic;               /**< deve ser 0xCAFEBABE */
    u2          minor_version;
    u2          major_version;
    u2          constant_pool_count; /**< numero de entradas + 1 */
    cp_info    *constant_pool;       /**< indices 1..constant_pool_count-1 */
    u2          access_flags;
    u2          this_class;          /**< indice CP_CLASS */
    u2          super_class;         /**< indice CP_CLASS ou 0 */
    u2          interfaces_count;
    u2         *interfaces;          /**< indices CP_CLASS */
    u2          fields_count;
    field_info *fields;
    u2          methods_count;
    method_info*methods;
    u2          attributes_count;
    attribute_info *attributes;
} ClassFile;

#endif /* CLASS_FILE_H */