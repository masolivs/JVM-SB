#include "frame.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/**
 * @brief Aloca e inicializa um novo Frame para o metodo dado.
 *
 * @details Configura o frame a partir dos atributos do metodo:
 *  - max_stack e max_locals sao lidos de code_attr (ou zerados se ausentes).
 *  - operand_stack e pre-alocada com reserve(max_stack) para evitar
 *    realocacoes durante a execucao.
 *  - local_vars e inicializada com max_locals posicoes zeradas via assign().
 *  - pc e call_site_pc sao zerados; method e klass recebem os ponteiros dados.
 *
 * @param method  Metodo a executar; pode ser NULL (frame sem codigo).
 * @param klass   Classe dona do metodo; usada para resolver o constant pool.
 * @return Ponteiro para o Frame alocado.
 */
Frame *frame_create(method_info *method, ClassEntry *klass) {
    Frame *f = new Frame();

    if (method && method->code_attr) {
        f->max_stack  = method->code_attr->max_stack;
        f->max_locals = method->code_attr->max_locals;
    } else {
        f->max_stack  = 0;
        f->max_locals = 0;
    }

    f->operand_stack.reserve((size_t)f->max_stack);
    f->local_vars.assign((size_t)f->max_locals, 0);

    f->pc           = 0;
    f->call_site_pc = 0;
    f->method       = method;
    f->klass        = klass;
    return f;
}

/**
 * @brief Libera um Frame alocado por frame_create().
 *
 * @details Deleta a estrutura Frame; os vetores operand_stack e local_vars
 * sao destruidos automaticamente pelo destrutor do std::vector.
 * E seguro chamar com frame == NULL (delete de NULL e no-op em C++).
 *
 * @param frame  Frame a destruir; pode ser NULL.
 */
void frame_destroy(Frame *frame) {
    delete frame;
}

/* ------------------------------------------------------------------ */
/* Operand stack — valores de 1 slot (int32, float, referencia)        */
/* ------------------------------------------------------------------ */

/**
 * @brief Empurha um valor de 32 bits no topo da operand stack.
 *
 * @param frame  Frame corrente.
 * @param value  Valor a empurhar.
 */
void frame_push(Frame *frame, int32_t value) {
    frame->operand_stack.push_back(value);
}

/**
 * @brief Retira e retorna o valor do topo da operand stack.
 *
 * @param frame  Frame corrente.
 * @return Valor retirado do topo.
 */
int32_t frame_pop(Frame *frame) {
    int32_t v = frame->operand_stack.back();
    frame->operand_stack.pop_back();
    return v;
}

/**
 * @brief Retorna o valor do topo sem retira-lo da operand stack.
 *
 * @param frame  Frame corrente.
 * @return Valor do topo.
 */
int32_t frame_peek(const Frame *frame) {
    return frame->operand_stack.back();
}

/* ------------------------------------------------------------------ */
/* Operand stack — long (2 slots)                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Empurha um long (64 bits) como dois slots consecutivos.
 *
 * @details A JVM representa long em 2 slots: high word primeiro, depois
 * low word. O split e feito com deslocamento de bits:
 *  - hi = bits 63..32 (cast para int32_t).
 *  - lo = bits 31..0  (cast para int32_t).
 * Empurha hi e depois lo, de modo que lo fica no topo da pilha.
 *
 * @param frame  Frame corrente.
 * @param value  Valor long a empurhar.
 */
void frame_push_long(Frame *frame, int64_t value) {
    int32_t hi = (int32_t)((uint64_t)value >> 32);
    int32_t lo = (int32_t)((uint64_t)value & 0xFFFFFFFFu);
    frame_push(frame, hi);
    frame_push(frame, lo);
}

/**
 * @brief Retira um long (2 slots) da operand stack e o reconstroi.
 *
 * @details Retira lo (topo) e depois hi (segundo slot). Reconstroi o
 * int64_t com: (uint64_t)(uint32_t)hi << 32 | (uint32_t)lo.
 * O cast para uint32_t antes do shift evita comportamento indefinido
 * em C++ com inteiros negativos.
 *
 * @param frame  Frame corrente.
 * @return Valor long reconstituido.
 */
int64_t frame_pop_long(Frame *frame) {
    int32_t lo = frame_pop(frame);
    int32_t hi = frame_pop(frame);
    return (int64_t)(((uint64_t)(uint32_t)hi << 32) | (uint32_t)lo);
}

/* ------------------------------------------------------------------ */
/* Operand stack — float (1 slot, bits reinterpretados)                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Empurha um float na operand stack como seus bits int32 equivalentes.
 *
 * @details Usa memcpy para reinterpretar os 4 bytes do float como int32_t
 * sem qualquer conversao numerica (type-punning seguro em C++).
 * frame_pop_float() reverte o processo.
 *
 * @param frame  Frame corrente.
 * @param value  Valor float a empurhar.
 */
void frame_push_float(Frame *frame, float value) {
    int32_t bits;
    memcpy(&bits, &value, 4);
    frame_push(frame, bits);
}

/**
 * @brief Retira um float da operand stack reinterpretando seus bits.
 *
 * @details Retira o int32_t do topo e reinterpreta seus 4 bytes como float
 * via memcpy, revertendo o que frame_push_float() fez.
 *
 * @param frame  Frame corrente.
 * @return Valor float reconstituido.
 */
float frame_pop_float(Frame *frame) {
    int32_t bits = frame_pop(frame);
    float v;
    memcpy(&v, &bits, 4);
    return v;
}

/* ------------------------------------------------------------------ */
/* Operand stack — double (2 slots, bits reinterpretados)               */
/* ------------------------------------------------------------------ */

/**
 * @brief Empurha um double na operand stack como 2 slots de bits int32.
 *
 * @details Reinterpreta os 8 bytes do double como uint64_t via memcpy e
 * delega a frame_push_long() para empacotar nos 2 slots (hi, lo).
 *
 * @param frame  Frame corrente.
 * @param value  Valor double a empurhar.
 */
void frame_push_double(Frame *frame, double value) {
    uint64_t bits;
    memcpy(&bits, &value, 8);
    frame_push_long(frame, (int64_t)bits);
}

/**
 * @brief Retira um double da operand stack reinterpretando seus 2 slots.
 *
 * @details Retira 2 slots via frame_pop_long() (reconstroi uint64_t) e
 * reinterpreta os 8 bytes como double via memcpy.
 *
 * @param frame  Frame corrente.
 * @return Valor double reconstituido.
 */
double frame_pop_double(Frame *frame) {
    uint64_t bits = (uint64_t)frame_pop_long(frame);
    double v;
    memcpy(&v, &bits, 8);
    return v;
}

/* ------------------------------------------------------------------ */
/* Variaveis locais — 1 slot                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Le uma variavel local de 1 slot pelo indice.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice em local_vars[].
 * @return Valor armazenado.
 */
int32_t frame_get_local(const Frame *frame, int idx) {
    return frame->local_vars[(size_t)idx];
}

/**
 * @brief Escreve um valor de 1 slot em uma variavel local.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice em local_vars[].
 * @param value  Valor a armazenar.
 */
void frame_set_local(Frame *frame, int idx, int32_t value) {
    frame->local_vars[(size_t)idx] = value;
}

/* ------------------------------------------------------------------ */
/* Variaveis locais — long (2 slots)                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Le um long (2 slots) das variaveis locais.
 *
 * @details Le local_vars[idx] como high word e local_vars[idx+1] como
 * low word e reconstroi o int64_t com o mesmo algoritmo de frame_pop_long.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice do slot high em local_vars[].
 * @return Valor long reconstituido.
 */
int64_t frame_get_local_long(const Frame *frame, int idx) {
    int32_t hi = frame->local_vars[(size_t)idx];
    int32_t lo = frame->local_vars[(size_t)(idx + 1)];
    return (int64_t)(((uint64_t)(uint32_t)hi << 32) | (uint32_t)lo);
}

/**
 * @brief Escreve um long (2 slots) nas variaveis locais.
 *
 * @details Divide o int64_t em high word (bits 63..32) e low word
 * (bits 31..0) e os armazena em local_vars[idx] e local_vars[idx+1].
 *
 * @param frame  Frame corrente.
 * @param idx    Indice do slot high em local_vars[].
 * @param value  Valor long a armazenar.
 */
void frame_set_local_long(Frame *frame, int idx, int64_t value) {
    frame->local_vars[(size_t)idx]     = (int32_t)((uint64_t)value >> 32);
    frame->local_vars[(size_t)(idx+1)] = (int32_t)((uint64_t)value & 0xFFFFFFFFu);
}

/* ------------------------------------------------------------------ */
/* Variaveis locais — double (2 slots, bits reinterpretados)            */
/* ------------------------------------------------------------------ */

/**
 * @brief Le um double (2 slots) das variaveis locais.
 *
 * @details Le os 2 slots como uint64_t via frame_get_local_long() e
 * reinterpreta os 8 bytes como double via memcpy.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice do slot high em local_vars[].
 * @return Valor double reconstituido.
 */
double frame_get_local_double(const Frame *frame, int idx) {
    uint64_t bits = (uint64_t)frame_get_local_long(frame, idx);
    double v;
    memcpy(&v, &bits, 8);
    return v;
}

/**
 * @brief Escreve um double (2 slots) nas variaveis locais.
 *
 * @details Reinterpreta os 8 bytes do double como uint64_t via memcpy e
 * delega a frame_set_local_long() para escrever nos 2 slots.
 *
 * @param frame  Frame corrente.
 * @param idx    Indice do slot high em local_vars[].
 * @param value  Valor double a armazenar.
 */
void frame_set_local_double(Frame *frame, int idx, double value) {
    uint64_t bits;
    memcpy(&bits, &value, 8);
    frame_set_local_long(frame, idx, (int64_t)bits);
}