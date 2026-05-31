#include "interpreter.h"
#include "frame.h"

/**
 * @defgroup conversoes Conversoes numericas (§3.11.8 da especificacao JVM)
 *
 * Cada opcode retira um valor do tipo de origem da operand stack,
 * aplica o cast C++ correspondente e empurha o resultado no tipo de destino.
 * Tipos de 2 slots (long, double) usam frame_pop_long/push_long e
 * frame_pop_double/push_double; os demais usam frame_pop/push ou
 * frame_pop_float/push_float.
 *
 * Notas de precisao:
 *  - i2l: extensao de sinal (int32 -> int64).
 *  - l2i: trunca os 32 bits altos (descarta high word).
 *  - i2b: estreita para int8_t e re-estende para int32_t (sign-extend de 8 bits).
 *  - i2c: mascara para uint16_t (zero-extend, sem sinal — char Java e unsigned).
 *  - i2s: estreita para int16_t e re-estende para int32_t (sign-extend de 16 bits).
 *  - Conversoes float/double seguem o arredondamento IEEE 754 padrao.
 */

/* ---- int -> outro tipo ---- */
/** @brief i2l — converte int (1 slot) para long (2 slots), com extensao de sinal. */
void op_i2l(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,(int64_t)frame_pop(frame)); }
/** @brief i2f — converte int para float (pode perder precisao para valores > 2^24). */
void op_i2f(JVM *jvm, Frame *frame) { (void)jvm; frame_push_float(frame,(float)frame_pop(frame)); }
/** @brief i2d — converte int para double (sem perda de precisao para int32). */
void op_i2d(JVM *jvm, Frame *frame) { (void)jvm; frame_push_double(frame,(double)frame_pop(frame)); }

/* ---- long -> outro tipo ---- */
/** @brief l2i — trunca long para int, descartando os 32 bits superiores. */
void op_l2i(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,(int32_t)frame_pop_long(frame)); }
/** @brief l2f — converte long para float (pode perder precisao para valores > 2^24). */
void op_l2f(JVM *jvm, Frame *frame) { (void)jvm; frame_push_float(frame,(float)frame_pop_long(frame)); }
/** @brief l2d — converte long para double (pode perder precisao para valores > 2^53). */
void op_l2d(JVM *jvm, Frame *frame) { (void)jvm; frame_push_double(frame,(double)frame_pop_long(frame)); }

/* ---- float -> outro tipo ---- */
/** @brief f2i — converte float para int por truncamento em direcao a zero. */
void op_f2i(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,(int32_t)frame_pop_float(frame)); }
/** @brief f2l — converte float para long por truncamento em direcao a zero. */
void op_f2l(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,(int64_t)frame_pop_float(frame)); }
/** @brief f2d — converte float para double sem perda (double tem mais precisao). */
void op_f2d(JVM *jvm, Frame *frame) { (void)jvm; frame_push_double(frame,(double)frame_pop_float(frame)); }

/* ---- double -> outro tipo ---- */
/** @brief d2i — converte double para int por truncamento em direcao a zero. */
void op_d2i(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,(int32_t)frame_pop_double(frame)); }
/** @brief d2l — converte double para long por truncamento em direcao a zero. */
void op_d2l(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,(int64_t)frame_pop_double(frame)); }
/** @brief d2f — converte double para float (pode perder precisao; arredonda IEEE 754). */
void op_d2f(JVM *jvm, Frame *frame) { (void)jvm; frame_push_float(frame,(float)frame_pop_double(frame)); }

/* ---- int -> tipos menores (narrowing) ---- */
/** @brief i2b — estreita int para byte: cast para int8_t e re-extende para int32_t. */
void op_i2b(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,(int32_t)(int8_t)frame_pop(frame)); }
/** @brief i2c — estreita int para char: mascara com uint16_t (zero-extend, sem sinal). */
void op_i2c(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,(int32_t)(uint16_t)frame_pop(frame)); }
/** @brief i2s — estreita int para short: cast para int16_t e re-extende para int32_t. */
void op_i2s(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,(int32_t)(int16_t)frame_pop(frame)); }