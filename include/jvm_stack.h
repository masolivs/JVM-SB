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
 *
 * @see jvm_stack_destroy()
 *
 * @code
 * JvmStack *stack = jvm_stack_create();
 * jvm_stack_push(stack, frame_create(method, klass));
 * Frame *top = jvm_stack_current(stack);
 * jvm_stack_pop(stack);
 * jvm_stack_destroy(stack);
 * @endcode
 */
JvmStack *jvm_stack_create(void);

/**
 * @brief Libera todos os frames e a propria pilha.
 *
 * @param stack  Pilha a destruir; pode ser NULL.
 *
 * @see jvm_stack_create()
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
 *
 * @warning Em overflow esta funcao encerra o processo (nao lanca
 * StackOverflowError como exececao Java catavel); reflete a limitacao
 * de nao haver um mecanismo de erro fatal mais granular nesta JVM.
 *
 * @see jvm_stack_pop()
 */
void jvm_stack_push(JvmStack *stack, Frame *frame);

/**
 * @brief Remove e destroi o frame do topo da pilha.
 *
 * @param stack  Pilha de execucao.
 *
 * @see jvm_stack_push()
 * @see jvm_stack_current()
 */
void jvm_stack_pop(JvmStack *stack);

/**
 * @brief Retorna o frame do topo sem remover.
 *
 * @param stack  Pilha de execucao.
 * @return Frame corrente ou NULL se vazia.
 *
 * @see jvm_stack_is_empty()
 */
Frame *jvm_stack_current(const JvmStack *stack);

/**
 * @brief Verifica se a pilha esta vazia.
 *
 * @param stack  Pilha de execucao.
 * @return true se nao ha frames ativos.
 *
 * @see jvm_stack_current()
 */
bool jvm_stack_is_empty(const JvmStack *stack);

#endif /* JVM_STACK_H */