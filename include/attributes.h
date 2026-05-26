#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "class_file.h"

/**
 * @brief Parseia o atributo Code de um metodo.
 *
 * Percorre o array de atributos genericos, encontra o de nome "Code"
 * e extrai max_stack, max_locals, bytecodes e tabela de excecoes.
 *
 * @param attrs  Array de atributos genericos do metodo.
 * @param count  Numero de atributos.
 * @param cp     Constant pool da classe (para resolver o nome "Code").
 * @return Ponteiro para Code_attribute alocado com new, ou NULL se ausente.
 */
Code_attribute *parse_code_attribute(const attribute_info *attrs,
                                     u2                    count,
                                     const cp_info        *cp);

/**
 * @brief Parseia o atributo LineNumberTable (opcional, para debug).
 *
 * Nao usada durante a execucao; util para mensagens de erro.
 * Retorna numero de entradas via *out_count e aloca array via new[].
 *
 * @param attr       Atributo generico contendo o LineNumberTable.
 * @param out_count  Recebe o numero de entradas.
 * @return Array de pares {start_pc, line_number}, ou NULL.
 */
u2 *parse_linenumber_table(const attribute_info *attr, u2 *out_count);

/**
 * @brief Parseia o atributo Exceptions de um metodo.
 *
 * @param attr       Atributo generico Exceptions.
 * @param out_count  Recebe o numero de classes de excecao listadas.
 * @return Array de indices CP_CLASS, ou NULL.
 */
u2 *parse_exceptions_attribute(const attribute_info *attr, u2 *out_count);

#endif /* ATTRIBUTES_H */
