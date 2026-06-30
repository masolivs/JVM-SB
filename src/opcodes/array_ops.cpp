#include "interpreter.h"
#include "constant_pool.h"
#include "frame.h"
#include <cstring>

/**
 * @brief Macro: verifica bounds do array e lanca ArrayIndexOutOfBoundsException.
 *
 * @details Se index < 0 ou index >= arr->arraylength, chama jvm_throw() com
 * "java/lang/ArrayIndexOutOfBoundsException" e retorna da funcao chamadora.
 * Deve ser usada apos CHECK_NULL, pois assume que arr e nao-nulo.
 *
 * @param arr    Ponteiro para o JArray ja validado como nao-nulo.
 * @param index  Indice a verificar.
 */
#define CHECK_BOUNDS(arr, index) \
    do { \
        if ((index) < 0 || (index) >= (arr)->arraylength) { \
            jvm_throw(jvm, "java/lang/ArrayIndexOutOfBoundsException"); \
            return; \
        } \
    } while(0)

/**
 * @brief Macro: verifica ponteiro nulo e lanca NullPointerException.
 *
 * @details Se ptr for NULL, chama jvm_throw() com
 * "java/lang/NullPointerException" e retorna da funcao chamadora.
 *
 * @param ptr  Ponteiro a verificar (geralmente resultado de heap_get_array).
 */
#define CHECK_NULL(ptr) \
    do { \
        if (!(ptr)) { \
            jvm_throw(jvm, "java/lang/NullPointerException"); \
            return; \
        } \
    } while(0)

/**
 * @brief newarray — aloca array de tipo primitivo e empurra referencia na pilha.
 *
 * @details
 *   1. Le o byte atype (tipo do elemento conforme JVM §6.5) e avanca pc += 1.
 *   2. Retira o tamanho (count) da operand stack. Se count < 0, lanca
 *      NegativeArraySizeException via jvm_throw().
 *   3. Aloca o array no heap via heap_alloc_array() com o tipo e tamanho dados.
 *   4. Empurra o indice (referencia) na operand stack.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_newarray(JVM *jvm, Frame *frame) {
    u1 atype = frame->method->code_attr->code[frame->pc++];
    int32_t count = frame_pop(frame);
    if (count < 0) { jvm_throw(jvm, "java/lang/NegativeArraySizeException"); return; }
    int32_t ref = heap_alloc_array(jvm, (ArrayType)atype, count);
    frame_push(frame, ref);
}

/**
 * @brief anewarray — aloca array de referencias e empurra referencia na pilha.
 *
 * @details Analogo a newarray, mas para arrays de referencias (objetos).
 * O indice de 2 bytes que identifica a classe do elemento e lido (pc += 2)
 * mas ignorado: todos os arrays de referencias usam o tipo T_REF.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_anewarray(JVM *jvm, Frame *frame) {
    frame->pc += 2;
    int32_t count = frame_pop(frame);
    if (count < 0) { jvm_throw(jvm, "java/lang/NegativeArraySizeException"); return; }
    int32_t ref = heap_alloc_array(jvm, T_REF, count);
    frame_push(frame, ref);
}

/**
 * @brief arraylength — empurra o comprimento de um array na operand stack.
 *
 * @details Retira a referencia do array, obtem o JArray* via heap_get_array(),
 * verifica null (CHECK_NULL) e empurra arr->arraylength.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_arraylength(JVM *jvm, Frame *frame) {
    int32_t ref = frame_pop(frame);
    JArray *arr = heap_get_array(jvm, ref);
    CHECK_NULL(arr);
    frame_push(frame, arr->arraylength);
}

/* ------------------------------------------------------------------ */
/* Carga de elemento de array (xaload)                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Implementacao comum para todos os opcodes xaload de 1 slot.
 *
 * @details Algoritmo:
 *   1. Retira o indice (topo) e a referencia do array (segundo slot).
 *   2. Obtem o JArray* via heap_get_array(); verifica null e bounds.
 *   3. Le o valor via array_get() e empurra na operand stack.
 *
 * Utilizado por op_iaload, op_faload, op_aaload, op_baload, op_caload
 * e op_saload, que diferem apenas no tipo semantico do elemento mas
 * compartilham o mesmo layout de 1 slot na pilha.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
static void aload_generic(JVM *jvm, Frame *frame) {
    int32_t index = frame_pop(frame);
    int32_t ref   = frame_pop(frame);
    JArray *arr = heap_get_array(jvm, ref);
    CHECK_NULL(arr);
    CHECK_BOUNDS(arr, index);
    frame_push(frame, array_get(arr, index));
}

/** @brief iaload — carrega int de array; delega a aload_generic.       */
void op_iaload(JVM *jvm, Frame *frame) { aload_generic(jvm, frame); }
/** @brief faload — carrega float de array; delega a aload_generic.     */
void op_faload(JVM *jvm, Frame *frame) { aload_generic(jvm, frame); }
/** @brief aaload — carrega referencia de array; delega a aload_generic. */
void op_aaload(JVM *jvm, Frame *frame) { aload_generic(jvm, frame); }
/** @brief baload — carrega byte/boolean de array; delega a aload_generic. */
void op_baload(JVM *jvm, Frame *frame) { aload_generic(jvm, frame); }
/** @brief caload — carrega char de array; delega a aload_generic.      */
void op_caload(JVM *jvm, Frame *frame) { aload_generic(jvm, frame); }
/** @brief saload — carrega short de array; delega a aload_generic.     */
void op_saload(JVM *jvm, Frame *frame) { aload_generic(jvm, frame); }

/**
 * @brief laload — carrega long (8 bytes) de array e empurra como 2 slots.
 *
 * @details Como long ocupa 8 bytes no buffer de elementos, faz leitura
 * manual via memcpy para evitar problemas de alinhamento:
 *   - Le 4 bytes como high word e 4 bytes como low word.
 *   - Empurha high word primeiro, depois low word (convencao JVM para 2 slots).
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_laload(JVM *jvm, Frame *frame) {
    int32_t index = frame_pop(frame);
    int32_t ref   = frame_pop(frame);
    JArray *arr = heap_get_array(jvm, ref);
    CHECK_NULL(arr);
    CHECK_BOUNDS(arr, index);
    char *base = (char*)arr->elements + index * 8;
    int32_t hi, lo;
    memcpy(&hi, base,   4);
    memcpy(&lo, base+4, 4);
    frame_push(frame, hi);
    frame_push(frame, lo);
}

/**
 * @brief daload — carrega double (8 bytes) de array e empurha como 2 slots.
 *
 * @details Le 8 bytes via memcpy para um double e usa frame_push_double()
 * para empacotar os 2 slots na operand stack no formato correto da JVM.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_daload(JVM *jvm, Frame *frame) {
    int32_t index = frame_pop(frame);
    int32_t ref   = frame_pop(frame);
    JArray *arr = heap_get_array(jvm, ref);
    CHECK_NULL(arr);
    CHECK_BOUNDS(arr, index);
    char *base = (char*)arr->elements + index * 8;
    double v;
    memcpy(&v, base, 8);
    frame_push_double(frame, v);
}

/* ------------------------------------------------------------------ */
/* Armazenamento em array (xastore)                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Implementacao comum para todos os opcodes xastore de 1 slot.
 *
 * @details Algoritmo (ordem de pop conforme spec JVM):
 *   1. Retira o valor a armazenar (topo).
 *   2. Retira o indice (segundo slot).
 *   3. Retira a referencia do array (terceiro slot).
 *   4. Obtem o JArray*, verifica null e bounds.
 *   5. Armazena via array_set().
 *
 * Utilizado por op_iastore, op_fastore, op_aastore, op_bastore,
 * op_castore e op_sastore.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
static void astore_generic(JVM *jvm, Frame *frame) {
    int32_t value = frame_pop(frame);
    int32_t index = frame_pop(frame);
    int32_t ref   = frame_pop(frame);
    JArray *arr = heap_get_array(jvm, ref);
    CHECK_NULL(arr);
    CHECK_BOUNDS(arr, index);
    array_set(arr, index, value);
}

/** @brief iastore — armazena int em array; delega a astore_generic.         */
void op_iastore(JVM *jvm, Frame *frame) { astore_generic(jvm, frame); }
/** @brief fastore — armazena float em array; delega a astore_generic.       */
void op_fastore(JVM *jvm, Frame *frame) { astore_generic(jvm, frame); }
/** @brief aastore — armazena referencia em array; delega a astore_generic.  */
void op_aastore(JVM *jvm, Frame *frame) { astore_generic(jvm, frame); }
/** @brief bastore — armazena byte/boolean em array; delega a astore_generic. */
void op_bastore(JVM *jvm, Frame *frame) { astore_generic(jvm, frame); }
/** @brief castore — armazena char em array; delega a astore_generic.        */
void op_castore(JVM *jvm, Frame *frame) { astore_generic(jvm, frame); }
/** @brief sastore — armazena short em array; delega a astore_generic.       */
void op_sastore(JVM *jvm, Frame *frame) { astore_generic(jvm, frame); }

/**
 * @brief lastore — armazena long (2 slots) em posicao de array.
 *
 * @details A ordem de pop e: low word (topo), high word, indice, referencia.
 * Escreve os 8 bytes com memcpy para evitar problemas de alinhamento:
 * high word nos primeiros 4 bytes, low word nos 4 bytes seguintes.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_lastore(JVM *jvm, Frame *frame) {
    int32_t lo    = frame_pop(frame);
    int32_t hi    = frame_pop(frame);
    int32_t index = frame_pop(frame);
    int32_t ref   = frame_pop(frame);
    JArray *arr = heap_get_array(jvm, ref);
    CHECK_NULL(arr);
    CHECK_BOUNDS(arr, index);
    char *base = (char*)arr->elements + index * 8;
    memcpy(base,   &hi, 4);
    memcpy(base+4, &lo, 4);
}

/**
 * @brief dastore — armazena double (2 slots) em posicao de array.
 *
 * @details Identico a lastore em estrutura: retira low e high da pilha,
 * monta um buffer de 8 bytes {hi, lo} e copia para a posicao correta
 * no buffer de elementos do array via memcpy.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_dastore(JVM *jvm, Frame *frame) {
    int32_t lo    = frame_pop(frame);
    int32_t hi    = frame_pop(frame);
    int32_t index = frame_pop(frame);
    int32_t ref   = frame_pop(frame);
    JArray *arr = heap_get_array(jvm, ref);
    CHECK_NULL(arr);
    CHECK_BOUNDS(arr, index);
    char *base = (char*)arr->elements + index * 8;
    int32_t buf[2] = {hi, lo};
    memcpy(base, buf, 8);
}

/* ------------------------------------------------------------------ */
/* multianewarray — array multidimensional                              */
/* ------------------------------------------------------------------ */

/**
 * @brief Mapeia o caractere de tipo base de um descriptor de array para ArrayType.
 *
 * @details Usado para decidir o tipo de elemento da dimensao mais interna
 * de um array criado por multianewarray. 'L' (referencia a objeto) e
 * qualquer caractere desconhecido caem em T_REF.
 *
 * @param c  Caractere do descriptor: I, J, F, D, B, C, S, Z ou L.
 * @return ArrayType correspondente.
 */
static ArrayType base_type_from_char(char c) {
    switch (c) {
        case 'I': return T_INT;
        case 'J': return T_LONG;
        case 'F': return T_FLOAT;
        case 'D': return T_DOUBLE;
        case 'B': return T_BYTE;
        case 'C': return T_CHAR;
        case 'S': return T_SHORT;
        case 'Z': return T_BOOLEAN;
        case 'L': default: return T_REF;
    }
}

/**
 * @brief Constroi recursivamente um array multidimensional "retangular".
 *
 * @details Caso base (ndims == 1): aloca um array folha do tipo leaf_type
 * com counts[0] elementos (heap_alloc_array). Caso recursivo: aloca um
 * array de referencias (T_REF) com counts[0] elementos e, para cada
 * posicao, constroi recursivamente um sub-array com counts+1/ndims-1 e
 * guarda a referencia via array_set.
 *
 * Se counts[0] < 0, lanca NegativeArraySizeException e retorna 0 (a
 * referencia parcialmente construida ate aqui fica orfa, mas o
 * interpretador encerra a execucao normal pelo mecanismo de excecao).
 *
 * @param jvm        JVM em execucao (heap e tratamento de excecao).
 * @param counts     Array com o tamanho de cada dimensao restante;
 *                    counts[0] e a dimensao atual.
 * @param ndims      Numero de dimensoes restantes (>= 1).
 * @param leaf_type  Tipo de elemento da dimensao mais interna (ndims == 1).
 * @return Referencia (indice na heap) do array construido, ou 0 em erro.
 *
 * @see op_multianewarray()
 */
static int32_t build_multiarray(JVM *jvm, const int32_t *counts, int ndims, ArrayType leaf_type) {
    int32_t len = counts[0];
    if (len < 0) {
        jvm_throw(jvm, "java/lang/NegativeArraySizeException");
        return 0;
    }
    if (ndims == 1) {
        return heap_alloc_array(jvm, leaf_type, len);
    }
    int32_t ref = heap_alloc_array(jvm, T_REF, len);
    JArray *arr = heap_get_array(jvm, ref);
    for (int32_t i = 0; i < len; i++) {
        int32_t sub = build_multiarray(jvm, counts + 1, ndims - 1, leaf_type);
        if (jvm->exception_pending) return 0;
        array_set(arr, i, sub);
    }
    return ref;
}

/**
 * @brief multianewarray — aloca um array multidimensional e empurra sua referencia.
 *
 * @details Formato da instrucao: indice de 2 bytes no CP (classe do array,
 * ex: descriptor "[[I" ou "[[Ljava/lang/String;") seguido de 1 byte com o
 * numero de dimensoes a inicializar.
 *
 *   1. Le o indice CP_CLASS e o byte de dimensoes; avanca pc += 3.
 *   2. Resolve o nome da classe via resolve_class_name() — para tipos
 *      array, o proprio nome JA E o descriptor (ex: "[[I"), entao nao
 *      e preciso nenhuma conversao adicional.
 *   3. Retira da operand stack `dimensions` valores int32 (os tamanhos de
 *      cada dimensao). A ordem de empilhamento no bytecode-fonte e
 *      count1..countN (count1 mais profundo, countN no topo), entao o
 *      primeiro pop devolve countN; os valores sao armazenados em counts[]
 *      de forma que counts[0] = count1 ... counts[dimensions-1] = countN.
 *   4. Conta os '[' iniciais do descriptor (profundidade total do tipo) e
 *      le o caractere seguinte para descobrir o tipo base (I, J, F, D, B,
 *      C, S, Z ou L). Se `dimensions` for menor que a profundidade total
 *      do descriptor, a dimensao mais interna criada fica com elementos
 *      T_REF (sub-arrays nao inicializados, i.e. null) — mesmo
 *      comportamento da JVM real para multianewarray parcial.
 *   5. Constroi a estrutura recursivamente via build_multiarray() e
 *      empurra a referencia resultante na operand stack.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 *
 * @warning Nao verifica se `dimensions` excede o numero de '[' do
 * descriptor (bytecode malformado nesse caso e comportamento indefinido,
 * assim como em outros pontos deste interpretador que confiam no
 * verificador da JVM real, que aqui nao existe).
 *
 * @see build_multiarray()
 * @see op_anewarray(), op_newarray() — equivalentes de 1 dimensao.
 */
void op_multianewarray(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0] << 8) | c[1]);
    u1 dimensions = c[2];
    frame->pc += 3;

    const ClassFile *cf = frame->klass->cf;
    std::string descriptor = resolve_class_name(cf, idx);

    /* Retira os tamanhos da pilha: topo = countN, fundo = count1 */
    int32_t *counts = new int32_t[dimensions];
    for (int i = dimensions - 1; i >= 0; i--) {
        counts[i] = frame_pop(frame);
    }

    /* Conta a profundidade total do tipo e descobre o tipo base */
    size_t depth = 0;
    while (depth < descriptor.size() && descriptor[depth] == '[') depth++;
    char base_char = (depth < descriptor.size()) ? descriptor[depth] : 'L';
    ArrayType leaf_type = ((size_t)dimensions < depth) ? T_REF : base_type_from_char(base_char);

    int32_t ref = build_multiarray(jvm, counts, dimensions, leaf_type);
    delete[] counts;

    if (jvm->exception_pending) return;
    frame_push(frame, ref);
}