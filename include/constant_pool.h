#ifndef CONSTANT_POOL_H
#define CONSTANT_POOL_H

#include "class_file.h"
#include <string>

/**
 * @brief Retorna a string UTF8 do CP no indice dado.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice 1-based no constant pool.
 * @return String C++ com o conteudo UTF8.
 *
 * @see resolve_class_name()
 * @see resolve_string()
 *
 * @code
 * std::string name = resolve_class_name(cf, cf->this_class);
 * // name == "Jogador", por exemplo
 * @endcode
 */
std::string resolve_utf8(const ClassFile *cf, u2 index);

/**
 * @brief Retorna o nome da classe referenciada por um CP_CLASS.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice CP_CLASS.
 * @return Nome da classe, ex: "java/lang/Object".
 *
 * @see resolve_utf8()
 */
std::string resolve_class_name(const ClassFile *cf, u2 index);

/**
 * @brief Retorna o conteudo de uma CP_STRING.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice CP_STRING.
 * @return Conteudo da string literal.
 *
 * @see resolve_utf8()
 */
std::string resolve_string(const ClassFile *cf, u2 index);

/**
 * @brief Retorna a descricao de um CP_METHODREF ou CP_INTERFACE_METHODREF.
 *
 * Formato: "Classe.metodo:descriptor"
 * Ex: "java/lang/Object.<init>:()V"
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice CP_METHODREF ou CP_INTERFACE_METHODREF.
 * @return String formatada.
 *
 * @see resolve_fieldref()
 * @see resolve_nameandtype()
 */
std::string resolve_methodref(const ClassFile *cf, u2 index);

/**
 * @brief Retorna a descricao de um CP_FIELDREF.
 *
 * Formato: "Classe.campo:tipo"
 * Ex: "java/lang/System.out:Ljava/io/PrintStream;"
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice CP_FIELDREF.
 * @return String formatada.
 *
 * @see resolve_methodref()
 * @see resolve_nameandtype()
 */
std::string resolve_fieldref(const ClassFile *cf, u2 index);

/**
 * @brief Retorna "nome:descriptor" de um CP_NAME_AND_TYPE.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice CP_NAME_AND_TYPE.
 * @return String "nome:descriptor".
 *
 * @see resolve_methodref()
 * @see resolve_fieldref()
 */
std::string resolve_nameandtype(const ClassFile *cf, u2 index);

#endif /* CONSTANT_POOL_H */

/**
 * @brief Formata o valor de qualquer entrada do CP como string legivel.
 *
 * Suporta: CP_STRING, CP_INTEGER, CP_FLOAT, CP_CLASS, CP_LONG, CP_DOUBLE.
 * Usado internamente por ldc, ldc_w e ldc2_w para exibir operandos.
 *
 * @param cf     ClassFile carregado.
 * @param index  Indice de qualquer entrada do CP.
 * @return String descrevendo o valor (ex: "\"Hello\"", "int 42", "double 3.14").
 *
 * @see resolve_string()
 * @see resolve_class_name()
 *
 */
std::string resolve_cp_value(const ClassFile *cf, u2 index);