#ifndef FRAME_H
#define FRAME_H

#include "class_file.h"
#include <vector>
#include <cstdint>

/* Evita include circular: method_area.h inclui class_file.h */
struct ClassEntry;

/**
 * @brief Representa uma chamada de metodo em execucao.
 *
 * Cada invocacao de metodo cria um Frame na pilha de execucao.
 * O Frame contem:
 *  - operand_stack: pilha de operandos onde calculos acontecem (slots 32-bit)
 *  - local_vars:    variaveis locais + parametros (slots 32-bit)
 *  - pc:            indice do proximo bytecode a executar
 *  - method/klass:  metodo e classe correntes
 *
 * Long e double ocupam 2 slots consecutivos (high_word, low_word).
 * Referencias sao armazenadas como int32_t (indice na heap table da JVM).
 */
struct Frame {
    std::vector<int32_t> operand_stack; /**< pilha de operandos */
    int                  max_stack;     /**< capacidade maxima */

    std::vector<int32_t> local_vars;    /**< variaveis locais */
    int                  max_locals;

    uint32_t    pc;           /**< program counter — offset no code[] */
    uint32_t    call_site_pc; /**< PC do invoke no chamador (para propagacao de excecao) */
    method_info *method;      /**< metodo em execucao */
    ClassEntry  *klass;       /**< classe dona do metodo */
};

/**
 * @brief Aloca e inicializa um novo Frame.
 *
 * Reserva local_vars com max_locals posicoes (zeradas) e
 * operand_stack vazia com capacidade max_stack.
 *
 * @param method  Metodo a executar.
 * @param klass   Classe dona do metodo.
 * @return Ponteiro para o Frame alocado.
 */
Frame *frame_create(method_info *method, ClassEntry *klass);

/**
 * @brief Libera um Frame alocado por frame_create.
 *
 * @param frame  Frame a destruir; pode ser NULL.
 */
void frame_destroy(Frame *frame);

/**
 * @brief Empurra um valor inteiro de 32 bits na operand stack.
 *
 * @param frame  Frame corrente.
 * @param value  Valor a empurrar.
 */
void frame_push(Frame *frame, int32_t value);

/**
 * @brief Retira e retorna o valor do topo da operand stack.
 *
 * @param frame  Frame corrente.
 * @return Valor do topo.
 */
int32_t frame_pop(Frame *frame);

/**
 * @brief Retorna o valor do topo sem retirar.
 *
 * @param frame  Frame corrente.
 * @return Valor do topo.
 */
int32_t frame_peek(const Frame *frame);

/**
 * @brief Empurra um long (64 bits) como dois slots: high depois low.
 *
 * @param frame  Frame corrente.
 * @param value  Valor long.
 */
void frame_push_long(Frame *frame, int64_t value);

/**
 * @brief Retira um long (2 slots) da operand stack.
 *
 * @param frame  Frame corrente.
 * @return Valor long.
 */
int64_t frame_pop_long(Frame *frame);

/**
 * @brief Empurra um double (64 bits) como dois slots.
 *
 * @param frame  Frame corrente.
 * @param value  Valor double.
 */
void frame_push_double(Frame *frame, double value);

/**
 * @brief Retira um double (2 slots) da operand stack.
 *
 * @param frame  Frame corrente.
 * @return Valor double.
 */
double frame_pop_double(Frame *frame);

/**
 * @brief Empurra um float na operand stack (reinterpret como int32).
 *
 * @param frame  Frame corrente.
 * @param value  Valor float.
 */
void frame_push_float(Frame *frame, float value);

/**
 * @brief Retira um float da operand stack.
 *
 * @param frame  Frame corrente.
 * @return Valor float.
 */
float frame_pop_float(Frame *frame);

/**
 * @brief Le uma variavel local pelo indice.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice na tabela de variaveis locais.
 * @return Valor da variavel local.
 */
int32_t frame_get_local(const Frame *frame, int idx);

/**
 * @brief Escreve uma variavel local pelo indice.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice na tabela de variaveis locais.
 * @param value  Valor a armazenar.
 */
void frame_set_local(Frame *frame, int idx, int32_t value);

/**
 * @brief Le um long (2 slots) das variaveis locais.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice do slot high.
 * @return Valor long.
 */
int64_t frame_get_local_long(const Frame *frame, int idx);

/**
 * @brief Escreve um long (2 slots) nas variaveis locais.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice do slot high.
 * @param value  Valor long.
 */
void frame_set_local_long(Frame *frame, int idx, int64_t value);

/**
 * @brief Le um double (2 slots) das variaveis locais.
 */
double frame_get_local_double(const Frame *frame, int idx);

/**
 * @brief Escreve um double (2 slots) nas variaveis locais.
 */
void frame_set_local_double(Frame *frame, int idx, double value);

#endif /* FRAME_H */
