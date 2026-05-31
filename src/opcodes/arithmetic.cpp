#include "interpreter.h"
#include "frame.h"
#include <cstdio>
#include <cstdlib>

/* ------------------------------------------------------------------ */
/* Operacoes aritmeticas inteiras (int, 32 bits)                        */
/*                                                                      */
/* Padrao comum: retira b (topo) e a (segundo slot), aplica a op b e   */
/* empurha o resultado. Todos os opcodes desta secao operam sobre       */
/* int32_t e usam frame_pop / frame_push.                               */
/* ------------------------------------------------------------------ */

/** @brief iadd — empurha a + b (int). */
void op_iadd(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); frame_push(frame,a+b); }
/** @brief isub — empurha a - b (int). */
void op_isub(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); frame_push(frame,a-b); }
/** @brief imul — empurha a * b (int). */
void op_imul(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); frame_push(frame,a*b); }

/**
 * @brief idiv — empurha a / b (int), com verificacao de divisao por zero.
 *
 * @details Lanca java/lang/ArithmeticException via jvm_throw() se b == 0,
 * seguindo a especificacao JVM §6.5 (idiv). A divisao trunca em direcao a zero.
 */
void op_idiv(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t b=frame_pop(frame), a=frame_pop(frame);
    if (b==0){ jvm_throw(jvm, "java/lang/ArithmeticException"); return; }
    frame_push(frame, a/b);
}

/**
 * @brief irem — empurha a % b (int), com verificacao de divisao por zero.
 *
 * @details Lanca java/lang/ArithmeticException se b == 0. O sinal do
 * resultado segue o sinal do dividendo (comportamento de C++ e da JVM).
 */
void op_irem(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t b=frame_pop(frame), a=frame_pop(frame);
    if (b==0){ jvm_throw(jvm, "java/lang/ArithmeticException"); return; }
    frame_push(frame, a%b);
}

/** @brief ineg — nega o inteiro no topo da pilha (empurha -a). */
void op_ineg(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, -frame_pop(frame)); }

/* ------------------------------------------------------------------ */
/* Operacoes aritmeticas long (64 bits)                                 */
/*                                                                      */
/* Identicas as de int, mas operam sobre int64_t usando                 */
/* frame_pop_long / frame_push_long (2 slots na pilha por valor).       */
/* ldiv e lrem lancam ArithmeticException se b == 0.                    */
/* ------------------------------------------------------------------ */

/** @brief ladd — empurha a + b (long, 2 slots). */
void op_ladd(JVM *jvm, Frame *frame) { (void)jvm; int64_t b=frame_pop_long(frame),a=frame_pop_long(frame); frame_push_long(frame,a+b); }
/** @brief lsub — empurha a - b (long). */
void op_lsub(JVM *jvm, Frame *frame) { (void)jvm; int64_t b=frame_pop_long(frame),a=frame_pop_long(frame); frame_push_long(frame,a-b); }
/** @brief lmul — empurha a * b (long). */
void op_lmul(JVM *jvm, Frame *frame) { (void)jvm; int64_t b=frame_pop_long(frame),a=frame_pop_long(frame); frame_push_long(frame,a*b); }

/**
 * @brief ldiv — empurha a / b (long), com verificacao de divisao por zero.
 */
void op_ldiv(JVM *jvm, Frame *frame) {
    (void)jvm;
    int64_t b=frame_pop_long(frame), a=frame_pop_long(frame);
    if (b==0){ jvm_throw(jvm, "java/lang/ArithmeticException"); return; }
    frame_push_long(frame, a/b);
}

/**
 * @brief lrem — empurha a % b (long), com verificacao de divisao por zero.
 */
void op_lrem(JVM *jvm, Frame *frame) {
    (void)jvm;
    int64_t b=frame_pop_long(frame), a=frame_pop_long(frame);
    if (b==0){ jvm_throw(jvm, "java/lang/ArithmeticException"); return; }
    frame_push_long(frame, a%b);
}

/** @brief lneg — nega o long no topo da pilha (empurha -a, 2 slots). */
void op_lneg(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame, -frame_pop_long(frame)); }

/* ------------------------------------------------------------------ */
/* Operacoes aritmeticas float (32 bits)                                */
/*                                                                      */
/* Usam frame_pop_float / frame_push_float. A divisao float nao lanca  */
/* excecao em divisao por zero (resulta em Infinity ou NaN, conforme    */
/* IEEE 754). frem calcula o resto por truncamento inteiro de a/b.      */
/* ------------------------------------------------------------------ */

/** @brief fadd — empurha a + b (float). */
void op_fadd(JVM *jvm, Frame *frame) { (void)jvm; float b=frame_pop_float(frame),a=frame_pop_float(frame); frame_push_float(frame,a+b); }
/** @brief fsub — empurha a - b (float). */
void op_fsub(JVM *jvm, Frame *frame) { (void)jvm; float b=frame_pop_float(frame),a=frame_pop_float(frame); frame_push_float(frame,a-b); }
/** @brief fmul — empurha a * b (float). */
void op_fmul(JVM *jvm, Frame *frame) { (void)jvm; float b=frame_pop_float(frame),a=frame_pop_float(frame); frame_push_float(frame,a*b); }
/** @brief fdiv — empurha a / b (float); divisao por zero retorna Infinity/NaN. */
void op_fdiv(JVM *jvm, Frame *frame) { (void)jvm; float b=frame_pop_float(frame),a=frame_pop_float(frame); frame_push_float(frame,a/b); }

/**
 * @brief frem — empurha o resto de a / b (float).
 *
 * @details Calcula o resto por truncamento inteiro: a - trunc(a/b) * b.
 * Difere de fmod() da libm; segue a semantica da JVM §6.5 (frem).
 */
void op_frem(JVM *jvm, Frame *frame) {
    (void)jvm;
    float b=frame_pop_float(frame), a=frame_pop_float(frame);
    frame_push_float(frame, a - (float)((int32_t)(a/b))*b);
}

/** @brief fneg — nega o float no topo da pilha. */
void op_fneg(JVM *jvm, Frame *frame) { (void)jvm; frame_push_float(frame, -frame_pop_float(frame)); }

/* ------------------------------------------------------------------ */
/* Operacoes aritmeticas double (64 bits)                               */
/*                                                                      */
/* Identicas as de float, mas usam frame_pop_double / frame_push_double */
/* (2 slots por valor). drem usa o mesmo calculo de truncamento.        */
/* ------------------------------------------------------------------ */

/** @brief dadd — empurha a + b (double, 2 slots). */
void op_dadd(JVM *jvm, Frame *frame) { (void)jvm; double b=frame_pop_double(frame),a=frame_pop_double(frame); frame_push_double(frame,a+b); }
/** @brief dsub — empurha a - b (double). */
void op_dsub(JVM *jvm, Frame *frame) { (void)jvm; double b=frame_pop_double(frame),a=frame_pop_double(frame); frame_push_double(frame,a-b); }
/** @brief dmul — empurha a * b (double). */
void op_dmul(JVM *jvm, Frame *frame) { (void)jvm; double b=frame_pop_double(frame),a=frame_pop_double(frame); frame_push_double(frame,a*b); }
/** @brief ddiv — empurha a / b (double); divisao por zero retorna Infinity/NaN. */
void op_ddiv(JVM *jvm, Frame *frame) { (void)jvm; double b=frame_pop_double(frame),a=frame_pop_double(frame); frame_push_double(frame,a/b); }

/**
 * @brief drem — empurha o resto de a / b (double).
 *
 * @details Mesma semantica de frem: a - trunc(a/b) * b, usando int64_t
 * para o truncamento, conforme especificacao JVM §6.5 (drem).
 */
void op_drem(JVM *jvm, Frame *frame) {
    (void)jvm;
    double b=frame_pop_double(frame), a=frame_pop_double(frame);
    frame_push_double(frame, a - (double)((int64_t)(a/b))*b);
}

/** @brief dneg — nega o double no topo da pilha (2 slots). */
void op_dneg(JVM *jvm, Frame *frame) { (void)jvm; frame_push_double(frame, -frame_pop_double(frame)); }

/* ------------------------------------------------------------------ */
/* iinc — incremento direto de variavel local                           */
/* ------------------------------------------------------------------ */

/**
 * @brief iinc — adiciona uma constante com sinal a uma variavel local int.
 *
 * @details Le dois operandos do bytecode: index (1 byte, sem sinal) e
 * const (1 byte, com sinal int8_t). Soma const ao valor atual de
 * local_vars[index] sem passar pela operand stack. Avanca pc += 2.
 *
 * @param jvm    Nao utilizado.
 * @param frame  Frame corrente; local_vars e lida e escrita diretamente.
 */
void op_iinc(JVM *jvm, Frame *frame) {
    (void)jvm;
    u1 index = frame->method->code_attr->code[frame->pc++];
    int8_t  cst = (int8_t)frame->method->code_attr->code[frame->pc++];
    frame_set_local(frame, index, frame_get_local(frame, index) + cst);
}

/* ------------------------------------------------------------------ */
/* Deslocamentos e operacoes bit a bit — int (32 bits)                  */
/*                                                                      */
/* ishl/ishr/iushr: mascara o deslocamento com 0x1F (5 bits) conforme  */
/* a JVM, pois deslocamentos >= 32 em int32_t sao indefinidos em C++.  */
/* ishr e aritmetico (propaga sinal); iushr e logico (zero-fill).       */
/* ------------------------------------------------------------------ */

/** @brief ishl  — deslocamento aritmetico a esquerda: a << (s & 0x1F). */
void op_ishl (JVM *jvm, Frame *frame) { (void)jvm; int32_t s=frame_pop(frame)&0x1F, v=frame_pop(frame); frame_push(frame,v<<s); }
/** @brief ishr  — deslocamento aritmetico a direita (com sinal): a >> (s & 0x1F). */
void op_ishr (JVM *jvm, Frame *frame) { (void)jvm; int32_t s=frame_pop(frame)&0x1F, v=frame_pop(frame); frame_push(frame,v>>s); }
/** @brief iushr — deslocamento logico a direita (sem sinal): (uint32)a >> (s & 0x1F). */
void op_iushr(JVM *jvm, Frame *frame) { (void)jvm; int32_t s=frame_pop(frame)&0x1F; uint32_t v=(uint32_t)frame_pop(frame); frame_push(frame,(int32_t)(v>>s)); }

/* ------------------------------------------------------------------ */
/* Deslocamentos e operacoes bit a bit — long (64 bits)                 */
/*                                                                      */
/* O valor deslocado (v) e long (2 slots); o deslocamento (s) e        */
/* int (1 slot, mascara 0x3F = 6 bits).                                 */
/* ------------------------------------------------------------------ */

/** @brief lshl  — deslocamento aritmetico a esquerda (long): v << (s & 0x3F). */
void op_lshl (JVM *jvm, Frame *frame) { (void)jvm; int32_t s=frame_pop(frame)&0x3F; int64_t v=frame_pop_long(frame); frame_push_long(frame,v<<s); }
/** @brief lshr  — deslocamento aritmetico a direita (long, com sinal). */
void op_lshr (JVM *jvm, Frame *frame) { (void)jvm; int32_t s=frame_pop(frame)&0x3F; int64_t v=frame_pop_long(frame); frame_push_long(frame,v>>s); }
/** @brief lushr — deslocamento logico a direita (long, sem sinal). */
void op_lushr(JVM *jvm, Frame *frame) { (void)jvm; int32_t s=frame_pop(frame)&0x3F; uint64_t v=(uint64_t)frame_pop_long(frame); frame_push_long(frame,(int64_t)(v>>s)); }

/* ---- logica bit a bit int ---- */
/** @brief iand — empurha a & b (int). */
void op_iand (JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); frame_push(frame,a&b); }
/** @brief ior  — empurha a | b (int). */
void op_ior  (JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); frame_push(frame,a|b); }
/** @brief ixor — empurha a ^ b (int). */
void op_ixor (JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); frame_push(frame,a^b); }

/* ---- logica bit a bit long ---- */
/** @brief land — empurha a & b (long, 2 slots). */
void op_land (JVM *jvm, Frame *frame) { (void)jvm; int64_t b=frame_pop_long(frame),a=frame_pop_long(frame); frame_push_long(frame,a&b); }
/** @brief lor  — empurha a | b (long). */
void op_lor  (JVM *jvm, Frame *frame) { (void)jvm; int64_t b=frame_pop_long(frame),a=frame_pop_long(frame); frame_push_long(frame,a|b); }
/** @brief lxor — empurha a ^ b (long). */
void op_lxor (JVM *jvm, Frame *frame) { (void)jvm; int64_t b=frame_pop_long(frame),a=frame_pop_long(frame); frame_push_long(frame,a^b); }

/* ------------------------------------------------------------------ */
/* Comparacoes long / float / double                                    */
/*                                                                      */
/* Todas empurham um int32_t com o resultado: 1 se a > b,              */
/* 0 se a == b, -1 se a < b (ou NaN, conforme a variante cmpl/cmpg).   */
/* ------------------------------------------------------------------ */

/**
 * @brief lcmp — compara dois longs e empurha 1, 0 ou -1.
 *
 * @details Retira b (topo, 2 slots) e a (2 slots). Empurha 1 se a > b,
 * -1 se a < b, 0 se iguais. Nao ha variantes l/g pois long nao tem NaN.
 */
void op_lcmp(JVM *jvm, Frame *frame) {
    (void)jvm;
    int64_t b=frame_pop_long(frame), a=frame_pop_long(frame);
    frame_push(frame, a>b ? 1 : (a<b ? -1 : 0));
}

/**
 * @brief fcmpl — compara dois floats; NaN ou a < b empurha -1.
 *
 * @details Usado pelo compilador quando o desvio subsequente e iflt/ifle.
 * NaN produz -1 (tratamento "less-than" para NaN).
 */
void op_fcmpl(JVM *jvm, Frame *frame) {
    (void)jvm;
    float b=frame_pop_float(frame), a=frame_pop_float(frame);
    if (a>b) frame_push(frame,1);
    else if (a==b) frame_push(frame,0);
    else frame_push(frame,-1); /* NaN ou a<b */
}

/**
 * @brief fcmpg — compara dois floats; NaN ou a > b empurha 1.
 *
 * @details Usado quando o desvio subsequente e ifgt/ifge.
 * NaN produz 1 (tratamento "greater-than" para NaN).
 */
void op_fcmpg(JVM *jvm, Frame *frame) {
    (void)jvm;
    float b=frame_pop_float(frame), a=frame_pop_float(frame);
    if (a<b) frame_push(frame,-1);
    else if (a==b) frame_push(frame,0);
    else frame_push(frame,1); /* NaN ou a>b */
}

/**
 * @brief dcmpl — compara dois doubles; NaN ou a < b empurha -1.
 *
 * @details Semantica identica a fcmpl, mas para double (2 slots cada).
 */
void op_dcmpl(JVM *jvm, Frame *frame) {
    (void)jvm;
    double b=frame_pop_double(frame), a=frame_pop_double(frame);
    if (a>b) frame_push(frame,1);
    else if (a==b) frame_push(frame,0);
    else frame_push(frame,-1);
}

/**
 * @brief dcmpg — compara dois doubles; NaN ou a > b empurha 1.
 *
 * @details Semantica identica a fcmpg, mas para double (2 slots cada).
 */
void op_dcmpg(JVM *jvm, Frame *frame) {
    (void)jvm;
    double b=frame_pop_double(frame), a=frame_pop_double(frame);
    if (a<b) frame_push(frame,-1);
    else if (a==b) frame_push(frame,0);
    else frame_push(frame,1);
}