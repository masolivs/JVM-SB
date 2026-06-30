#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

/**
 * @file attributes.h
 * @brief Declaracoes das funcoes de parse de atributos do formato .class.
 *
 * Cobre os atributos obrigatorios definidos na especificacao JVMS 8:
 *   - Code          (metodos)
 *   - LineNumberTable (sub-atributo de Code, debug)
 *   - Exceptions    (metodos — clausula throws)
 *   - SourceFile    (classe)
 *   - ConstantValue (campos static final)
 */

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
 *
 * @see parse_linenumber_table() — le um sub-atributo de Code_attribute::sub_attributes.
 * @see Code_attribute
 *
 * @code
 * Code_attribute *ca = parse_code_attribute(method.attributes,
 *                                            method.attributes_count, cf->constant_pool);
 * if (ca) {
 *     // ca->code, ca->code_length prontos para disassembly/execucao
 * }
 * @endcode
 */
Code_attribute *parse_code_attribute(const attribute_info *attrs,
                                     u2                    count,
                                     const cp_info        *cp);

/**
 * @brief Parseia o atributo LineNumberTable (sub-atributo de Code).
 *
 * Busca dentro dos sub-atributos do Code_attribute pelo LineNumberTable.
 * Util para exibir o numero da linha do .java correspondente a cada bytecode.
 *
 * @param attrs      Array de atributos genericos (sub-atributos do Code).
 * @param count      Numero de sub-atributos.
 * @param cp         Constant pool da classe.
 * @param out_count  Recebe o numero de entradas na tabela.
 * @return Array plano de pares {start_pc, line_number} (u2), ou NULL.
 *         O chamador e responsavel por liberar com delete[].
 *
 * @see parse_code_attribute() — produz o Code_attribute cujo
 *      sub_attributes/attributes_count sao passados aqui.
 *
 * @code
 * u2 n = 0;
 * u2 *table = parse_linenumber_table(ca->sub_attributes, ca->attributes_count, cp, &n);
 * for (u2 i = 0; i < n; i++) {
 *     u2 start_pc = table[i * 2], line = table[i * 2 + 1];
 * }
 * delete[] table;
 * @endcode
 */
u2 *parse_linenumber_table(const attribute_info *attrs,
                           u2                    count,
                           const cp_info        *cp,
                           u2                   *out_count);

/**
 * @brief Parseia o atributo Exceptions de um metodo (clausula throws).
 *
 * @param attrs      Array de atributos genericos do metodo.
 * @param count      Numero de atributos.
 * @param cp         Constant pool da classe.
 * @param out_count  Recebe o numero de classes de excecao listadas.
 * @return Array de indices CP_CLASS, ou NULL.
 *         O chamador e responsavel por liberar com delete[].
 *
 * @see resolve_class_name() em constant_pool.h — resolve cada indice
 *      retornado para o nome textual da classe de excecao.
 */
u2 *parse_exceptions_attribute(const attribute_info *attrs,
                               u2                    count,
                               const cp_info        *cp,
                               u2                   *out_count);

/**
 * @brief Parseia o atributo SourceFile de uma classe.
 *
 * O atributo contem um unico u2: indice UTF8 com o nome do arquivo .java.
 *
 * @param attrs  Array de atributos genericos da classe.
 * @param count  Numero de atributos.
 * @param cp     Constant pool da classe.
 * @return Indice UTF8 do nome do arquivo fonte, ou 0 se ausente.
 *
 * @see parse_constantvalue_attribute() — mesmo padrao de retorno (indice ou 0).
 */
u2 parse_sourcefile_attribute(const attribute_info *attrs,
                              u2                    count,
                              const cp_info        *cp);

/**
 * @brief Parseia o atributo ConstantValue de um campo static final.
 *
 * O atributo contem um unico u2: indice do CP com o valor constante
 * (pode ser CP_INTEGER, CP_FLOAT, CP_LONG, CP_DOUBLE ou CP_STRING).
 *
 * @param attrs  Array de atributos genericos do campo.
 * @param count  Numero de atributos.
 * @param cp     Constant pool da classe.
 * @return Indice do CP com o valor, ou 0 se ausente.
 *
 * @see resolve_cp_value() em constant_pool.h — formata o valor apontado
 *      pelo indice retornado.
 * @see parse_sourcefile_attribute() — mesmo padrao de retorno (indice ou 0).
 */
u2 parse_constantvalue_attribute(const attribute_info *attrs,
                                 u2                    count,
                                 const cp_info        *cp);

#endif /* ATTRIBUTES_H */