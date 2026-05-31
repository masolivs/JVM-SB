#include "interpreter.h"
#include "frame.h"
#include <cstdint>

/**
 * @brief Macro auxiliar para instrucoes de desvio condicional com offset de 2 bytes.
 *
 * @details Le dois bytes a partir de frame->pc formando um int16_t com sinal
 * (offset relativo). Se a condicao for verdadeira, o novo PC e calculado como:
 *   frame->pc = (frame->pc - 1) + offset
 * O subtracao de 1 compensa o fato de o PC ja ter sido incrementado pelo loop
 * de interpretacao antes de chamar o handler (o offset e relativo ao byte do
 * opcode, nao ao byte seguinte).
 * Se a condicao for falsa, avanca 2 bytes para pular os operandos.
 *
 * @param cond  Expressao booleana que determina se o desvio ocorre.
 */
#define BRANCH2(cond) \
    do { \
        u1 *c = frame->method->code_attr->code + frame->pc; \
        int16_t off = (int16_t)((c[0]<<8)|c[1]); \
        if (cond) frame->pc = (uint32_t)((int32_t)frame->pc - 1 + off); \
        else frame->pc += 2; \
    } while(0)

/* ------------------------------------------------------------------ */
/* Desvios incondicionais                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief goto — desvio incondicional com offset de 2 bytes com sinal.
 *
 * @details Le um int16_t a partir do PC e calcula o novo PC como
 * (pc_do_opcode + offset), onde pc_do_opcode = frame->pc - 1.
 *
 * @param jvm    Nao utilizado.
 * @param frame  Frame corrente; pc e atualizado para o destino do desvio.
 */
void op_goto  (JVM *jvm, Frame *frame) { (void)jvm; u1*c=frame->method->code_attr->code+frame->pc; int16_t off=(int16_t)((c[0]<<8)|c[1]); frame->pc=(uint32_t)((int32_t)frame->pc-1+off); }

/**
 * @brief goto_w — desvio incondicional com offset de 4 bytes com sinal.
 *
 * @details Identico a goto, mas le um int32_t em vez de int16_t, permitindo
 * saltos para qualquer posicao em metodos grandes (> 32 KB).
 *
 * @param jvm    Nao utilizado.
 * @param frame  Frame corrente.
 */
void op_goto_w(JVM *jvm, Frame *frame) { (void)jvm; u1*c=frame->method->code_attr->code+frame->pc; int32_t off=(int32_t)(((uint32_t)c[0]<<24)|((uint32_t)c[1]<<16)|((uint32_t)c[2]<<8)|c[3]); frame->pc=(uint32_t)((int32_t)frame->pc-1+off); }

/* ------------------------------------------------------------------ */
/* Desvios condicionais — comparacao com zero                           */
/* Todas usam a macro BRANCH2; a diferenca e apenas a condicao testada. */
/* ------------------------------------------------------------------ */

/** @brief ifeq — desvia se o inteiro no topo da pilha for igual a zero.    */
void op_ifeq(JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v==0); }
/** @brief ifne — desvia se o inteiro no topo for diferente de zero.         */
void op_ifne(JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v!=0); }
/** @brief iflt — desvia se o inteiro no topo for menor que zero.            */
void op_iflt(JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v<0);  }
/** @brief ifge — desvia se o inteiro no topo for maior ou igual a zero.     */
void op_ifge(JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v>=0); }
/** @brief ifgt — desvia se o inteiro no topo for maior que zero.            */
void op_ifgt(JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v>0);  }
/** @brief ifle — desvia se o inteiro no topo for menor ou igual a zero.     */
void op_ifle(JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v<=0); }

/* ------------------------------------------------------------------ */
/* Desvios condicionais — comparacao entre dois inteiros (if_icmp*)    */
/* Retira dois valores (b = topo, a = segundo) e compara a op b.       */
/* ------------------------------------------------------------------ */

/** @brief if_icmpeq — desvia se a == b (dois ints). */
void op_if_icmpeq(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a==b); }
/** @brief if_icmpne — desvia se a != b. */
void op_if_icmpne(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a!=b); }
/** @brief if_icmplt — desvia se a < b.  */
void op_if_icmplt(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a<b);  }
/** @brief if_icmpge — desvia se a >= b. */
void op_if_icmpge(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a>=b); }
/** @brief if_icmpgt — desvia se a > b.  */
void op_if_icmpgt(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a>b);  }
/** @brief if_icmple — desvia se a <= b. */
void op_if_icmple(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a<=b); }

/* ------------------------------------------------------------------ */
/* Desvios condicionais — comparacao entre referencias (if_acmp*)      */
/* Referencias sao armazenadas como int32_t (indice no heap).           */
/* ------------------------------------------------------------------ */

/** @brief if_acmpeq — desvia se as duas referencias apontam para o mesmo objeto. */
void op_if_acmpeq(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a==b); }
/** @brief if_acmpne — desvia se as referencias sao diferentes.                    */
void op_if_acmpne(JVM *jvm, Frame *frame) { (void)jvm; int32_t b=frame_pop(frame),a=frame_pop(frame); BRANCH2(a!=b); }

/** @brief ifnull    — desvia se a referencia no topo e null (ref == 0). */
void op_ifnull   (JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v==0); }
/** @brief ifnonnull — desvia se a referencia no topo nao e null.        */
void op_ifnonnull(JVM *jvm, Frame *frame) { (void)jvm; int32_t v=frame_pop(frame); BRANCH2(v!=0); }

/* ------------------------------------------------------------------ */
/* Switch                                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief tableswitch — desvio indexado por tabela densa de offsets.
 *
 * @details Usado pelo compilador para switches com chaves contíguas (ex:
 * case 1, 2, 3, 4). O algoritmo:
 *   1. Registra pc0 = pc - 1 (posicao do opcode) para calcular offsets
 *      absolutos a partir de um ponto de referencia fixo.
 *   2. Alinha pc para o proximo multiplo de 4 (requisito da JVM: os
 *      operandos do tableswitch devem estar alinhados a 4 bytes).
 *   3. Le tres int32_t big-endian: default_offset, low e high.
 *   4. Retira a chave (key) da operand stack.
 *   5. Se key < low ou key > high, aplica o offset default.
 *      Caso contrario, usa key - low como indice na tabela de offsets
 *      e le o offset correspondente (cada entrada tem 4 bytes).
 *   6. Atualiza frame->pc = pc0 + offset selecionado.
 *
 * @param jvm    Nao utilizado diretamente (presente por assinatura uniforme).
 * @param frame  Frame corrente; pc e atualizado para o destino do desvio.
 */
void op_tableswitch(JVM *jvm, Frame *frame) {
    (void)jvm;
    uint32_t pc0 = frame->pc - 1; /* pc do opcode */
    /* pula bytes de alinhamento */
    while (frame->pc % 4 != 0) frame->pc++;

    u1 *c = frame->method->code_attr->code;
    int32_t def  = (int32_t)(((uint32_t)c[frame->pc]<<24)|((uint32_t)c[frame->pc+1]<<16)|((uint32_t)c[frame->pc+2]<<8)|c[frame->pc+3]); frame->pc+=4;
    int32_t low  = (int32_t)(((uint32_t)c[frame->pc]<<24)|((uint32_t)c[frame->pc+1]<<16)|((uint32_t)c[frame->pc+2]<<8)|c[frame->pc+3]); frame->pc+=4;
    int32_t high = (int32_t)(((uint32_t)c[frame->pc]<<24)|((uint32_t)c[frame->pc+1]<<16)|((uint32_t)c[frame->pc+2]<<8)|c[frame->pc+3]); frame->pc+=4;

    int32_t key = frame_pop(frame);
    if (key < low || key > high) {
        frame->pc = (uint32_t)((int32_t)pc0 + def);
    } else {
        int32_t idx = key - low;
        int32_t off = (int32_t)(((uint32_t)c[frame->pc+idx*4]<<24)|((uint32_t)c[frame->pc+idx*4+1]<<16)|((uint32_t)c[frame->pc+idx*4+2]<<8)|c[frame->pc+idx*4+3]);
        frame->pc = (uint32_t)((int32_t)pc0 + off);
    }
}

/**
 * @brief lookupswitch — desvio por busca linear em tabela de pares (chave, offset).
 *
 * @details Usado para switches com chaves esparsas (ex: case 1, 100, 9999).
 * O algoritmo:
 *   1. Registra pc0 e alinha o PC para multiplo de 4 (mesmo que tableswitch).
 *   2. Le default_offset e npairs (numero de entradas na tabela).
 *   3. Retira a chave da operand stack.
 *   4. Percorre linearmente os npairs pares {match, offset}: se key == match,
 *      usa o offset correspondente e interrompe a busca.
 *   5. Se nenhum par corresponder, usa o offset default.
 *   6. Atualiza frame->pc = pc0 + offset selecionado.
 *
 * @param jvm    Nao utilizado diretamente.
 * @param frame  Frame corrente; pc e atualizado para o destino do desvio.
 */
void op_lookupswitch(JVM *jvm, Frame *frame) {
    (void)jvm;
    uint32_t pc0 = frame->pc - 1;
    while (frame->pc % 4 != 0) frame->pc++;

    u1 *c = frame->method->code_attr->code;
    int32_t def     = (int32_t)(((uint32_t)c[frame->pc]<<24)|((uint32_t)c[frame->pc+1]<<16)|((uint32_t)c[frame->pc+2]<<8)|c[frame->pc+3]); frame->pc+=4;
    int32_t npairs  = (int32_t)(((uint32_t)c[frame->pc]<<24)|((uint32_t)c[frame->pc+1]<<16)|((uint32_t)c[frame->pc+2]<<8)|c[frame->pc+3]); frame->pc+=4;

    int32_t key = frame_pop(frame);
    int32_t jump = def;
    for (int32_t i = 0; i < npairs; i++) {
        int32_t match = (int32_t)(((uint32_t)c[frame->pc]<<24)|((uint32_t)c[frame->pc+1]<<16)|((uint32_t)c[frame->pc+2]<<8)|c[frame->pc+3]); frame->pc+=4;
        int32_t off   = (int32_t)(((uint32_t)c[frame->pc]<<24)|((uint32_t)c[frame->pc+1]<<16)|((uint32_t)c[frame->pc+2]<<8)|c[frame->pc+3]); frame->pc+=4;
        if (key == match) { jump = off; break; }
    }
    frame->pc = (uint32_t)((int32_t)pc0 + jump);
}

/* ------------------------------------------------------------------ */
/* Retornos                                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Implementacao central de todos os opcodes de retorno.
 *
 * @details Remove o frame atual da pilha e, se houver um frame chamador,
 * transfere o valor de retorno para a operand stack desse frame:
 *   - nvals == 0: retorno void (op_return); nenhum valor e transferido.
 *   - nvals == 1: retorno de 1 slot (int, float, referencia); retira v1
 *     do frame corrente e empurra em caller.
 *   - nvals == 2: retorno de 2 slots (long, double); retira v1 (low word)
 *     e v2 (high word) e os empurra em caller na ordem correta (high primeiro,
 *     depois low), preservando a convencao de 2 slots da JVM.
 *
 * A chamada a jvm_stack_pop() destrói o frame corrente antes de acessar
 * o frame chamador, pois pop() remove o frame da pilha sem desalocá-lo
 * imediatamente.
 *
 * @param jvm    JVM em execucao; usada para acessar a pilha de frames.
 * @param nvals  Numero de slots a transferir (0, 1 ou 2).
 */
static void do_return(JVM *jvm, int nvals) {
    int32_t v1=0, v2=0;
    if (nvals >= 1) v1 = frame_pop(jvm_stack_current(jvm->stack));
    if (nvals >= 2) v2 = frame_pop(jvm_stack_current(jvm->stack));

    jvm_stack_pop(jvm->stack); /* destroi frame corrente */

    if (!jvm_stack_is_empty(jvm->stack) && nvals > 0) {
        Frame *caller = jvm_stack_current(jvm->stack);
        if (nvals == 2) frame_push(caller, v2); /* high word primeiro */
        frame_push(caller, v1);
    }
}

/** @brief return  — retorno void; remove o frame sem transferir valor.              */
void op_return (JVM *jvm, Frame *frame) { (void)frame; do_return(jvm, 0); }
/** @brief ireturn — retorna um inteiro de 32 bits para o frame chamador.            */
void op_ireturn(JVM *jvm, Frame *frame) { (void)frame; do_return(jvm, 1); }
/** @brief freturn — retorna um float (1 slot) para o frame chamador.                */
void op_freturn(JVM *jvm, Frame *frame) { (void)frame; do_return(jvm, 1); }
/** @brief areturn — retorna uma referencia (1 slot) para o frame chamador.          */
void op_areturn(JVM *jvm, Frame *frame) { (void)frame; do_return(jvm, 1); }
/** @brief lreturn — retorna um long (2 slots: high + low) para o frame chamador.    */
void op_lreturn(JVM *jvm, Frame *frame) { (void)frame; do_return(jvm, 2); }
/** @brief dreturn — retorna um double (2 slots: high + low) para o frame chamador.  */
void op_dreturn(JVM *jvm, Frame *frame) { (void)frame; do_return(jvm, 2); }