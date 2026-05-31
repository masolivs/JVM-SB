#include "jvm_stack.h"
#include <cstdio>
#include <cstdlib>

/**
 * @brief Cria e inicializa uma JvmStack vazia.
 *
 * @details Aloca a estrutura JvmStack e define max_depth como 512, limite
 * que previne estouro de pilha em recursoes infinitas. O vetor de frames
 * comeca vazio; frames sao adicionados via jvm_stack_push().
 *
 * @return Ponteiro para a JvmStack criada.
 */
JvmStack *jvm_stack_create(void) {
    JvmStack *s = new JvmStack();
    s->max_depth = 512;
    return s;
}

/**
 * @brief Destroi uma JvmStack liberando todos os frames remanescentes.
 *
 * @details Percorre o vetor de frames e chama frame_destroy() em cada um
 * antes de deletar a estrutura JvmStack. Isso garante que nenhum frame
 * fique vivo apos o encerramento da JVM (ex: encerramento abrupto por
 * excecao nao capturada ou StackOverflowError).
 * E seguro chamar com stack == NULL.
 *
 * @param stack  Pilha a destruir; pode ser NULL.
 */
void jvm_stack_destroy(JvmStack *stack) {
    if (!stack) return;
    for (size_t i = 0; i < stack->frames.size(); i++)
        frame_destroy(stack->frames[i]);
    delete stack;
}

/**
 * @brief Empurha um frame na pilha de execucao.
 *
 * @details Verifica se o numero atual de frames atingiu max_depth antes
 * de empurhar. Se o limite for atingido, imprime "StackOverflowError" em
 * stderr e encerra o processo com exit(1), simulando o comportamento da
 * JVM real para recursao excessiva.
 * Caso contrario, adiciona o frame ao final do vetor (topo da pilha).
 *
 * @param stack  Pilha de execucao.
 * @param frame  Frame a empurhar; assumido como alocado por frame_create().
 */
void jvm_stack_push(JvmStack *stack, Frame *frame) {
    if ((int)stack->frames.size() >= stack->max_depth) {
        fprintf(stderr, "StackOverflowError: profundidade maxima %d atingida\n",
                stack->max_depth);
        exit(1);
    }
    stack->frames.push_back(frame);
}

/**
 * @brief Remove e destroi o frame do topo da pilha.
 *
 * @details Chama frame_destroy() no frame do topo (ultimo elemento do vetor)
 * e o remove via pop_back(). Se a pilha estiver vazia, retorna sem acao.
 * Apos esta chamada, jvm_stack_current() passara a retornar o frame anterior,
 * que se torna o novo contexto de execucao.
 *
 * @param stack  Pilha de execucao.
 */
void jvm_stack_pop(JvmStack *stack) {
    if (stack->frames.empty()) return;
    frame_destroy(stack->frames.back());
    stack->frames.pop_back();
}

/**
 * @brief Retorna o frame do topo da pilha sem remove-lo.
 *
 * @param stack  Pilha de execucao.
 * @return Frame corrente (topo), ou NULL se a pilha estiver vazia.
 */
Frame *jvm_stack_current(const JvmStack *stack) {
    if (stack->frames.empty()) return NULL;
    return stack->frames.back();
}

/**
 * @brief Verifica se a pilha de execucao esta vazia.
 *
 * @param stack  Pilha de execucao.
 * @return true se nao ha frames na pilha; false caso contrario.
 */
bool jvm_stack_is_empty(const JvmStack *stack) {
    return stack->frames.empty();
}