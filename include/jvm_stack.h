#ifndef JVM_STACK_H
#define JVM_STACK_H

#include "frame.h"
#include <vector>

/**
 * @brief Pilha de frames da JVM — gerencia as chamadas de metodo.
 *
 * Cada invocacao de metodo empurra um Frame; cada retorno o remove.
 * max_depth limita a profundidade recursiva (equivalente a StackOverflow).
 */
struct JvmStack {
    std::vector<Frame *> frames;    /**< frames ativos; topo = back() */
    int                  max_depth; /**< profundidade maxima permitida */
};

/**
 * @brief Aloca e inicializa uma JvmStack vazia.
 *
 * @return Ponteiro para a JvmStack criada.
 */
JvmStack *jvm_stack_create(void);

/**
 * @brief Libera todos os frames e a propria pilha.
 *
 * @param stack  Pilha a destruir; pode ser NULL.
 */
void jvm_stack_destroy(JvmStack *stack);

/**
 * @brief Empurra um frame na pilha.
 *
 * Verifica overflow (stack->frames.size() >= max_depth).
 * Em overflow, imprime erro e encerra o programa.
 *
 * @param stack  Pilha de execucao.
 * @param frame  Frame a empurrar.
 */
void jvm_stack_push(JvmStack *stack, Frame *frame);

/**
 * @brief Remove e destroi o frame do topo da pilha.
 *
 * @param stack  Pilha de execucao.
 */
void jvm_stack_pop(JvmStack *stack);

/**
 * @brief Retorna o frame do topo sem remover.
 *
 * @param stack  Pilha de execucao.
 * @return Frame corrente ou NULL se vazia.
 */
Frame *jvm_stack_current(const JvmStack *stack);

/**
 * @brief Verifica se a pilha esta vazia.
 *
 * @param stack  Pilha de execucao.
 * @return true se nao ha frames ativos.
 */
bool jvm_stack_is_empty(const JvmStack *stack);

#endif /* JVM_STACK_H */
