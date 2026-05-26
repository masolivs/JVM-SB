/**
 * @file constant_pool.cpp
 * @brief Implementacao das funcoes de resolucao de entradas do constant pool.
 *
 * Cada funcao "resolve" navega recursivamente pelas referencias do CP
 * para produzir uma string legivel (estilo jclasslib/javap).
 *
 * Exemplos de saida:
 *   resolve_methodref  ->  "java/lang/Object.<init>:()V"
 *   resolve_fieldref   ->  "java/lang/System.out:Ljava/io/PrintStream;"
 *   resolve_nameandtype->  "println:(Ljava/lang/String;)V"
 */

#include "constant_pool.h"
#include <cstring>
#include <cstdio>

/**
 * @brief Retorna a string UTF8 do CP no indice dado.
 *
 * O buffer interno ja e terminado em '\0' pelo leitor (class_reader.cpp),
 * portanto a conversao para std::string e segura.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice 1-based no constant pool.
 * @return String C++ com o conteudo UTF8, ou "<invalid-utf8>" se o tag for errado.
 */
std::string resolve_utf8(const ClassFile *cf, u2 index) {
    const cp_info &entry = cf->constant_pool[index];
    if (entry.tag != CP_UTF8) return "<invalid-utf8>";
    return std::string(
        reinterpret_cast<const char *>(entry.data.utf8.bytes),
        entry.data.utf8.length);
}

/**
 * @brief Retorna o nome binario da classe referenciada por um CP_CLASS.
 *
 * O nome usa '/' como separador de pacote (ex: "java/lang/Object").
 * Arrays aparecem com sua descricao completa (ex: "[Ljava/lang/String;").
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice de uma entrada CP_CLASS.
 * @return Nome da classe, ou "<invalid-class>" se o tag for errado.
 */
std::string resolve_class_name(const ClassFile *cf, u2 index) {
    const cp_info &entry = cf->constant_pool[index];
    if (entry.tag != CP_CLASS) return "<invalid-class>";
    return resolve_utf8(cf, entry.data.class_info.name_index);
}

/**
 * @brief Retorna o conteudo de uma entrada CP_STRING.
 *
 * CP_STRING e apenas um indice para um CP_UTF8; esta funcao resolve
 * essa indireccao e devolve o valor da string literal.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice de uma entrada CP_STRING.
 * @return Conteudo da string literal, ou "<invalid-string>" se o tag for errado.
 */
std::string resolve_string(const ClassFile *cf, u2 index) {
    const cp_info &entry = cf->constant_pool[index];
    if (entry.tag != CP_STRING) return "<invalid-string>";
    return resolve_utf8(cf, entry.data.string_info.string_index);
}

/**
 * @brief Retorna "nome:descriptor" de um CP_NAME_AND_TYPE.
 *
 * Exemplos:
 *   "<init>:()V"
 *   "println:(Ljava/lang/String;)V"
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice de uma entrada CP_NAME_AND_TYPE.
 * @return String formatada, ou "<invalid-nat>" se o tag for errado.
 */
std::string resolve_nameandtype(const ClassFile *cf, u2 index) {
    const cp_info &entry = cf->constant_pool[index];
    if (entry.tag != CP_NAME_AND_TYPE) return "<invalid-nat>";

    std::string name = resolve_utf8(cf, entry.data.name_and_type.name_index);
    std::string desc = resolve_utf8(cf, entry.data.name_and_type.descriptor_index);
    return name + ":" + desc;
}

/**
 * @brief Retorna a descricao de um CP_METHODREF ou CP_INTERFACE_METHODREF.
 *
 * Formato: "Classe.metodo:descriptor"
 * Exemplo: "java/lang/Object.<init>:()V"
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice CP_METHODREF ou CP_INTERFACE_METHODREF.
 * @return String formatada, ou "<invalid-methodref>" se o tag for errado.
 */
std::string resolve_methodref(const ClassFile *cf, u2 index) {
    const cp_info &entry = cf->constant_pool[index];
    if (entry.tag != CP_METHODREF && entry.tag != CP_INTERFACE_METHODREF)
        return "<invalid-methodref>";

    std::string cls = resolve_class_name(cf, entry.data.ref.class_index);
    std::string nat = resolve_nameandtype(cf, entry.data.ref.name_and_type_index);
    return cls + "." + nat;
}

/**
 * @brief Retorna a descricao de um CP_FIELDREF.
 *
 * Formato: "Classe.campo:tipo"
 * Exemplo: "java/lang/System.out:Ljava/io/PrintStream;"
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice CP_FIELDREF.
 * @return String formatada, ou "<invalid-fieldref>" se o tag for errado.
 */
std::string resolve_fieldref(const ClassFile *cf, u2 index) {
    const cp_info &entry = cf->constant_pool[index];
    if (entry.tag != CP_FIELDREF) return "<invalid-fieldref>";

    std::string cls = resolve_class_name(cf, entry.data.ref.class_index);
    std::string nat = resolve_nameandtype(cf, entry.data.ref.name_and_type_index);
    return cls + "." + nat;
}

/**
 * @brief Formata o valor de uma entrada do CP como string legivel.
 *
 * Usado por ldc/ldc_w para exibir o operando independente do tipo.
 * Tipos suportados: CP_STRING, CP_INTEGER, CP_FLOAT, CP_CLASS.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice de qualquer entrada do CP.
 * @return String descrevendo o valor, ex: "\"Hello\"", "42", "3.14f", "class Foo".
 */
std::string resolve_cp_value(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    char buf[64];

    switch (e.tag) {
        case CP_STRING:
            return "\"" + resolve_string(cf, index) + "\"";
        case CP_INTEGER: {
            int32_t v;
            memcpy(&v, &e.data.integer_val.bytes, 4);
            snprintf(buf, sizeof(buf), "int %d", v);
            return buf;
        }
        case CP_FLOAT: {
            float v;
            memcpy(&v, &e.data.float_val.bytes, 4);
            snprintf(buf, sizeof(buf), "float %f", v);
            return buf;
        }
        case CP_CLASS:
            return "class " + resolve_class_name(cf, index);
        case CP_LONG: {
            int64_t v = ((int64_t)e.data.long_val.high_bytes << 32) |
                         e.data.long_val.low_bytes;
            snprintf(buf, sizeof(buf), "long %lld", (long long)v);
            return buf;
        }
        case CP_DOUBLE: {
            uint64_t bits = ((uint64_t)e.data.double_val.high_bytes << 32) |
                             e.data.double_val.low_bytes;
            double v;
            memcpy(&v, &bits, 8);
            snprintf(buf, sizeof(buf), "double %f", v);
            return buf;
        }
        default:
            return "?";
    }
}
