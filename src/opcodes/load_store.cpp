#include "interpreter.h"
#include "frame.h"
#include "constant_pool.h"
#include <cstring>
#include <cstdint>

/* ------------------------------------------------------------------ */
/* Constantes — empurham valores literais sem ler operandos             */
/* ------------------------------------------------------------------ */

/** @brief nop — nao faz nada; avanca apenas o PC. */
void op_nop        (JVM *jvm, Frame *frame) { (void)jvm; (void)frame; }
/** @brief aconst_null — empurha a referencia nula (0) na pilha. */
void op_aconst_null(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, 0); }

/**
 * @defgroup iconst Constantes inteiras (iconst_m1 .. iconst_5)
 *
 * Empurham o valor inteiro literal correspondente (-1 a 5) sem ler
 * operandos do bytecode. Equivalem a bipush com valor fixo.
 */
/** @brief iconst_m1 — empurha -1 (int). */
void op_iconst_m1  (JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, -1); }
/** @brief iconst_0  — empurha 0 (int). */
void op_iconst_0   (JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, 0); }
/** @brief iconst_1  — empurha 1 (int). */
void op_iconst_1   (JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, 1); }
/** @brief iconst_2  — empurha 2 (int). */
void op_iconst_2   (JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, 2); }
/** @brief iconst_3  — empurha 3 (int). */
void op_iconst_3   (JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, 3); }
/** @brief iconst_4  — empurha 4 (int). */
void op_iconst_4   (JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, 4); }
/** @brief iconst_5  — empurha 5 (int). */
void op_iconst_5   (JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame, 5); }

/** @brief lconst_0 — empurha 0L (long, 2 slots). */
void op_lconst_0   (JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame, 0LL); }
/** @brief lconst_1 — empurha 1L (long, 2 slots). */
void op_lconst_1   (JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame, 1LL); }

/** @brief fconst_0 — empurha 0.0f (float). */
void op_fconst_0   (JVM *jvm, Frame *frame) { (void)jvm; frame_push_float(frame, 0.0f); }
/** @brief fconst_1 — empurha 1.0f (float). */
void op_fconst_1   (JVM *jvm, Frame *frame) { (void)jvm; frame_push_float(frame, 1.0f); }
/** @brief fconst_2 — empurha 2.0f (float). */
void op_fconst_2   (JVM *jvm, Frame *frame) { (void)jvm; frame_push_float(frame, 2.0f); }

/** @brief dconst_0 — empurha 0.0 (double, 2 slots). */
void op_dconst_0   (JVM *jvm, Frame *frame) { (void)jvm; frame_push_double(frame, 0.0); }
/** @brief dconst_1 — empurha 1.0 (double, 2 slots). */
void op_dconst_1   (JVM *jvm, Frame *frame) { (void)jvm; frame_push_double(frame, 1.0); }

/**
 * @brief bipush — le 1 byte com sinal do bytecode e empurha como int32.
 *
 * @details O byte e interpretado como int8_t (com sinal) e estendido para
 * int32_t antes de ser empurrado. Cobre o intervalo [-128, 127].
 */
void op_bipush(JVM *jvm, Frame *frame) {
    (void)jvm;
    int8_t v = (int8_t)frame->method->code_attr->code[frame->pc++];
    frame_push(frame, (int32_t)v);
}

/**
 * @brief sipush — le 2 bytes com sinal do bytecode e empurha como int32.
 *
 * @details Monta um int16_t big-endian a partir de dois bytes consecutivos
 * e o estende para int32_t. Cobre o intervalo [-32768, 32767].
 */
void op_sipush(JVM *jvm, Frame *frame) {
    (void)jvm;
    u1 *c = frame->method->code_attr->code + frame->pc;
    int16_t v = (int16_t)((c[0]<<8)|c[1]);
    frame->pc += 2;
    frame_push(frame, (int32_t)v);
}

/* ------------------------------------------------------------------ */
/* ldc / ldc_w / ldc2_w — carga de constante do constant pool          */
/* ------------------------------------------------------------------ */

/**
 * @brief Empurha o valor de uma entrada do constant pool na operand stack.
 *
 * @details Despacha conforme o tag da entrada:
 *  - CP_INTEGER: copia 4 bytes via memcpy e empurha como int32.
 *  - CP_FLOAT:   copia 4 bytes via memcpy e empurha como float.
 *  - CP_STRING:  resolve a string, aloca um array de chars (T_CHAR) no heap
 *                com cada caractere em 2 bytes (UTF-16 simplificado) e
 *                empurha a referencia do array.
 *  - CP_CLASS:   empurha 0 (nao implementado por completo).
 *  - Outros:     empurha 0 como fallback.
 *
 * @param jvm    JVM em execucao; usada para alocar arrays no heap.
 * @param frame  Frame corrente.
 * @param idx    Indice no constant pool da classe corrente.
 */
static void ldc_push(JVM *jvm, Frame *frame, u2 idx) {
    const ClassFile *cf = frame->klass->cf;
    const cp_info   &e  = cf->constant_pool[idx];
    switch (e.tag) {
        case CP_INTEGER: {
            /* bytes ja esta em host order apos read_u4; cast direto e seguro */
            int32_t v = (int32_t)e.data.integer_val.bytes;
            frame_push(frame, v); break;
        }
        case CP_FLOAT: {
            /* type-punning via memcpy e o unico jeito portavel (evita UB de aliasing) */
            uint32_t bits = e.data.float_val.bytes;
            float v; memcpy(&v, &bits, sizeof(v));
            frame_push_float(frame, v); break;
        }
        case CP_STRING: {
            /* Strings sao armazenadas como referencias no heap */
            std::string s = resolve_string(cf, idx);
            /* Aloca array de chars no heap como referencia de string */
            int32_t ref = heap_alloc_array(jvm, T_CHAR, (int32_t)s.size());
            JArray *arr = heap_get_array(jvm, ref);
            for (int32_t i = 0; i < (int32_t)s.size(); i++) {
                uint16_t ch = (uint16_t)(unsigned char)s[(size_t)i];
                memcpy((char*)arr->elements + i*2, &ch, 2);
            }
            frame_push(frame, ref); break;
        }
        case CP_CLASS: {
            /* Empurra referencia a ClassEntry (nao implementado por completo) */
            frame_push(frame, 0); break;
        }
        default:
            frame_push(frame, 0); break;
    }
}

/**
 * @brief ldc — carrega constante de 1 byte de indice (valores 1..255).
 *
 * @details Le 1 byte como indice sem sinal e delega a ldc_push().
 * Usado para constantes int, float e String de indice baixo.
 */
void op_ldc  (JVM *jvm, Frame *frame) { u1 idx=frame->method->code_attr->code[frame->pc++]; ldc_push(jvm,frame,(u2)idx); }

/**
 * @brief ldc_w — carrega constante de 2 bytes de indice (forma ampliada de ldc).
 *
 * @details Identico a ldc, mas le um indice de 2 bytes big-endian,
 * permitindo acessar entradas alem do indice 255.
 */
void op_ldc_w(JVM *jvm, Frame *frame) {
    u1 *c=frame->method->code_attr->code+frame->pc; frame->pc+=2;
    ldc_push(jvm, frame, (u2)((c[0]<<8)|c[1]));
}

/**
 * @brief ldc2_w — carrega constante long ou double (2 slots) do constant pool.
 *
 * @details Le um indice de 2 bytes. A entrada deve ser CP_LONG ou CP_DOUBLE
 * (ocupam 2 slots no constant pool conforme a especificacao JVM).
 * Para CP_LONG: monta int64_t a partir de high_bytes e low_bytes.
 * Para CP_DOUBLE: monta uint64_t e reinterpreta os bits como double via memcpy.
 */
void op_ldc2_w(JVM *jvm, Frame *frame) {
    (void)jvm;
    u1 *c=frame->method->code_attr->code+frame->pc; frame->pc+=2;
    u2 idx=(u2)((c[0]<<8)|c[1]);
    const cp_info &e = frame->klass->cf->constant_pool[idx];
    if (e.tag==CP_LONG) {
        int64_t v=((int64_t)e.data.long_val.high_bytes<<32)|e.data.long_val.low_bytes;
        frame_push_long(frame, v);
    } else if (e.tag==CP_DOUBLE) {
        uint64_t bits=((uint64_t)e.data.double_val.high_bytes<<32)|e.data.double_val.low_bytes;
        double v; memcpy(&v,&bits,8);
        frame_push_double(frame, v);
    }
}

/* ------------------------------------------------------------------ */
/* Carga de variavel local (xload / xload_N)                            */
/*                                                                      */
/* xload le 1 byte de indice do bytecode (pc++).                        */
/* xload_N usa indice fixo N sem ler operandos.                         */
/* Tipos de 2 slots (long, double) usam frame_get_local_long.           */
/* float e armazenado como int32 reinterpretado; usa frame_get_local.   */
/* ------------------------------------------------------------------ */

/* ---- iload / iload_N ---- */
/** @brief iload   — empurha local_vars[index] (int); le index do bytecode. */
void op_iload  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_push(frame,frame_get_local(frame,i)); }
/** @brief iload_0 — empurha local_vars[0] (int). */
void op_iload_0(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,0)); }
/** @brief iload_1 — empurha local_vars[1] (int). */
void op_iload_1(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,1)); }
/** @brief iload_2 — empurha local_vars[2] (int). */
void op_iload_2(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,2)); }
/** @brief iload_3 — empurha local_vars[3] (int). */
void op_iload_3(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,3)); }

/* ---- lload / lload_N — long (2 slots) ---- */
/** @brief lload   — empurha local_vars[index] como long (2 slots). */
void op_lload  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_push_long(frame,frame_get_local_long(frame,i)); }
/** @brief lload_0 — empurha local_vars[0..1] como long. */
void op_lload_0(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,0)); }
/** @brief lload_1 — empurha local_vars[1..2] como long. */
void op_lload_1(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,1)); }
/** @brief lload_2 — empurha local_vars[2..3] como long. */
void op_lload_2(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,2)); }
/** @brief lload_3 — empurha local_vars[3..4] como long. */
void op_lload_3(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,3)); }

/* ---- fload / fload_N — float armazenado como int32 reinterpretado ---- */
/** @brief fload   — empurha local_vars[index] (float como int32). */
void op_fload  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_push(frame,frame_get_local(frame,i)); }
/** @brief fload_0 — empurha local_vars[0] (float). */
void op_fload_0(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,0)); }
/** @brief fload_1 — empurha local_vars[1] (float). */
void op_fload_1(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,1)); }
/** @brief fload_2 — empurha local_vars[2] (float). */
void op_fload_2(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,2)); }
/** @brief fload_3 — empurha local_vars[3] (float). */
void op_fload_3(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,3)); }

/* ---- dload / dload_N — double (2 slots, usa get_local_long internamente) ---- */
/** @brief dload   — empurha local_vars[index] como double (2 slots). */
void op_dload  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_push_long(frame,frame_get_local_long(frame,i)); }
/** @brief dload_0 — empurha local_vars[0..1] como double. */
void op_dload_0(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,0)); }
/** @brief dload_1 — empurha local_vars[1..2] como double. */
void op_dload_1(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,1)); }
/** @brief dload_2 — empurha local_vars[2..3] como double. */
void op_dload_2(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,2)); }
/** @brief dload_3 — empurha local_vars[3..4] como double. */
void op_dload_3(JVM *jvm, Frame *frame) { (void)jvm; frame_push_long(frame,frame_get_local_long(frame,3)); }

/* ---- aload / aload_N — referencia (armazenada como int32) ---- */
/** @brief aload   — empurha local_vars[index] (referencia). */
void op_aload  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_push(frame,frame_get_local(frame,i)); }
/** @brief aload_0 — empurha local_vars[0] (referencia; frequentemente 'this'). */
void op_aload_0(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,0)); }
/** @brief aload_1 — empurha local_vars[1] (referencia). */
void op_aload_1(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,1)); }
/** @brief aload_2 — empurha local_vars[2] (referencia). */
void op_aload_2(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,2)); }
/** @brief aload_3 — empurha local_vars[3] (referencia). */
void op_aload_3(JVM *jvm, Frame *frame) { (void)jvm; frame_push(frame,frame_get_local(frame,3)); }

/* ------------------------------------------------------------------ */
/* Armazenamento em variavel local (xstore / xstore_N)                  */
/*                                                                      */
/* Simetrico a carga: retira o valor do topo da operand stack e        */
/* escreve em local_vars[index]. xstore le o indice do bytecode;       */
/* xstore_N usa indice fixo.                                            */
/* ------------------------------------------------------------------ */

/* ---- istore / istore_N ---- */
/** @brief istore   — armazena int em local_vars[index]; le index do bytecode. */
void op_istore  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_set_local(frame,i,frame_pop(frame)); }
/** @brief istore_0 — armazena int em local_vars[0]. */
void op_istore_0(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,0,frame_pop(frame)); }
/** @brief istore_1 — armazena int em local_vars[1]. */
void op_istore_1(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,1,frame_pop(frame)); }
/** @brief istore_2 — armazena int em local_vars[2]. */
void op_istore_2(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,2,frame_pop(frame)); }
/** @brief istore_3 — armazena int em local_vars[3]. */
void op_istore_3(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,3,frame_pop(frame)); }

/* ---- lstore / lstore_N — long (2 slots) ---- */
/** @brief lstore   — armazena long em local_vars[index..index+1]. */
void op_lstore  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_set_local_long(frame,i,frame_pop_long(frame)); }
/** @brief lstore_0 — armazena long em local_vars[0..1]. */
void op_lstore_0(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,0,frame_pop_long(frame)); }
/** @brief lstore_1 — armazena long em local_vars[1..2]. */
void op_lstore_1(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,1,frame_pop_long(frame)); }
/** @brief lstore_2 — armazena long em local_vars[2..3]. */
void op_lstore_2(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,2,frame_pop_long(frame)); }
/** @brief lstore_3 — armazena long em local_vars[3..4]. */
void op_lstore_3(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,3,frame_pop_long(frame)); }

/* ---- fstore / fstore_N — float (armazenado como int32 reinterpretado) ---- */
/** @brief fstore   — armazena float em local_vars[index]. */
void op_fstore  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_set_local(frame,i,frame_pop(frame)); }
/** @brief fstore_0 — armazena float em local_vars[0]. */
void op_fstore_0(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,0,frame_pop(frame)); }
/** @brief fstore_1 — armazena float em local_vars[1]. */
void op_fstore_1(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,1,frame_pop(frame)); }
/** @brief fstore_2 — armazena float em local_vars[2]. */
void op_fstore_2(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,2,frame_pop(frame)); }
/** @brief fstore_3 — armazena float em local_vars[3]. */
void op_fstore_3(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,3,frame_pop(frame)); }

/* ---- dstore / dstore_N — double (2 slots) ---- */
/** @brief dstore   — armazena double em local_vars[index..index+1]. */
void op_dstore  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_set_local_long(frame,i,frame_pop_long(frame)); }
/** @brief dstore_0 — armazena double em local_vars[0..1]. */
void op_dstore_0(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,0,frame_pop_long(frame)); }
/** @brief dstore_1 — armazena double em local_vars[1..2]. */
void op_dstore_1(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,1,frame_pop_long(frame)); }
/** @brief dstore_2 — armazena double em local_vars[2..3]. */
void op_dstore_2(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,2,frame_pop_long(frame)); }
/** @brief dstore_3 — armazena double em local_vars[3..4]. */
void op_dstore_3(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local_long(frame,3,frame_pop_long(frame)); }

/* ---- astore / astore_N — referencia ---- */
/** @brief astore   — armazena referencia em local_vars[index]. */
void op_astore  (JVM *jvm, Frame *frame) { (void)jvm; u1 i=frame->method->code_attr->code[frame->pc++]; frame_set_local(frame,i,frame_pop(frame)); }
/** @brief astore_0 — armazena referencia em local_vars[0]. */
void op_astore_0(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,0,frame_pop(frame)); }
/** @brief astore_1 — armazena referencia em local_vars[1]. */
void op_astore_1(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,1,frame_pop(frame)); }
/** @brief astore_2 — armazena referencia em local_vars[2]. */
void op_astore_2(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,2,frame_pop(frame)); }
/** @brief astore_3 — armazena referencia em local_vars[3]. */
void op_astore_3(JVM *jvm, Frame *frame) { (void)jvm; frame_set_local(frame,3,frame_pop(frame)); }