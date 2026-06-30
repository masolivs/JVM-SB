#include "interpreter.h"
#include "constant_pool.h"
#include "errors.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* forward declarations */
ClassEntry       *auto_load_class(JVM *jvm, const std::string &name);
static void       resolve_super_chain(JVM *jvm, ClassEntry *ce);

/* ------------------------------------------------------------------ */
/* Forward declarations dos handlers (definidos em src/opcodes/)        */
/* ------------------------------------------------------------------ */

/* arithmetic.cpp */
void op_iadd(JVM*, Frame*); void op_isub(JVM*, Frame*); void op_imul(JVM*, Frame*);
void op_idiv(JVM*, Frame*); void op_irem(JVM*, Frame*); void op_ineg(JVM*, Frame*);
void op_ladd(JVM*, Frame*); void op_lsub(JVM*, Frame*); void op_lmul(JVM*, Frame*);
void op_ldiv(JVM*, Frame*); void op_lrem(JVM*, Frame*); void op_lneg(JVM*, Frame*);
void op_fadd(JVM*, Frame*); void op_fsub(JVM*, Frame*); void op_fmul(JVM*, Frame*);
void op_fdiv(JVM*, Frame*); void op_frem(JVM*, Frame*); void op_fneg(JVM*, Frame*);
void op_dadd(JVM*, Frame*); void op_dsub(JVM*, Frame*); void op_dmul(JVM*, Frame*);
void op_ddiv(JVM*, Frame*); void op_drem(JVM*, Frame*); void op_dneg(JVM*, Frame*);
void op_iinc(JVM*, Frame*);
void op_ishl(JVM*, Frame*); void op_ishr(JVM*, Frame*); void op_iushr(JVM*, Frame*);
void op_lshl(JVM*, Frame*); void op_lshr(JVM*, Frame*); void op_lushr(JVM*, Frame*);
void op_iand(JVM*, Frame*); void op_ior(JVM*, Frame*);  void op_ixor(JVM*, Frame*);
void op_land(JVM*, Frame*); void op_lor(JVM*, Frame*);  void op_lxor(JVM*, Frame*);
void op_lcmp(JVM*, Frame*); void op_fcmpl(JVM*, Frame*); void op_fcmpg(JVM*, Frame*);
void op_dcmpl(JVM*, Frame*); void op_dcmpg(JVM*, Frame*);

/* load_store.cpp */
void op_nop(JVM*, Frame*);
void op_aconst_null(JVM*, Frame*);
void op_iconst_m1(JVM*, Frame*); void op_iconst_0(JVM*, Frame*);
void op_iconst_1(JVM*, Frame*);  void op_iconst_2(JVM*, Frame*);
void op_iconst_3(JVM*, Frame*);  void op_iconst_4(JVM*, Frame*);
void op_iconst_5(JVM*, Frame*);
void op_lconst_0(JVM*, Frame*);  void op_lconst_1(JVM*, Frame*);
void op_fconst_0(JVM*, Frame*);  void op_fconst_1(JVM*, Frame*);
void op_fconst_2(JVM*, Frame*);
void op_dconst_0(JVM*, Frame*);  void op_dconst_1(JVM*, Frame*);
void op_bipush(JVM*, Frame*);    void op_sipush(JVM*, Frame*);
void op_ldc(JVM*, Frame*);       void op_ldc_w(JVM*, Frame*);
void op_ldc2_w(JVM*, Frame*);
void op_iload(JVM*, Frame*);     void op_iload_0(JVM*, Frame*);
void op_iload_1(JVM*, Frame*);   void op_iload_2(JVM*, Frame*);
void op_iload_3(JVM*, Frame*);
void op_lload(JVM*, Frame*);     void op_lload_0(JVM*, Frame*);
void op_lload_1(JVM*, Frame*);   void op_lload_2(JVM*, Frame*);
void op_lload_3(JVM*, Frame*);
void op_fload(JVM*, Frame*);     void op_fload_0(JVM*, Frame*);
void op_fload_1(JVM*, Frame*);   void op_fload_2(JVM*, Frame*);
void op_fload_3(JVM*, Frame*);
void op_dload(JVM*, Frame*);     void op_dload_0(JVM*, Frame*);
void op_dload_1(JVM*, Frame*);   void op_dload_2(JVM*, Frame*);
void op_dload_3(JVM*, Frame*);
void op_aload(JVM*, Frame*);     void op_aload_0(JVM*, Frame*);
void op_aload_1(JVM*, Frame*);   void op_aload_2(JVM*, Frame*);
void op_aload_3(JVM*, Frame*);
void op_istore(JVM*, Frame*);    void op_istore_0(JVM*, Frame*);
void op_istore_1(JVM*, Frame*);  void op_istore_2(JVM*, Frame*);
void op_istore_3(JVM*, Frame*);
void op_lstore(JVM*, Frame*);    void op_lstore_0(JVM*, Frame*);
void op_lstore_1(JVM*, Frame*);  void op_lstore_2(JVM*, Frame*);
void op_lstore_3(JVM*, Frame*);
void op_fstore(JVM*, Frame*);    void op_fstore_0(JVM*, Frame*);
void op_fstore_1(JVM*, Frame*);  void op_fstore_2(JVM*, Frame*);
void op_fstore_3(JVM*, Frame*);
void op_dstore(JVM*, Frame*);    void op_dstore_0(JVM*, Frame*);
void op_dstore_1(JVM*, Frame*);  void op_dstore_2(JVM*, Frame*);
void op_dstore_3(JVM*, Frame*);
void op_astore(JVM*, Frame*);    void op_astore_0(JVM*, Frame*);
void op_astore_1(JVM*, Frame*);  void op_astore_2(JVM*, Frame*);
void op_astore_3(JVM*, Frame*);
void op_wide(JVM*, Frame*);

/* stack_ops.cpp */
void op_pop(JVM*, Frame*);    void op_pop2(JVM*, Frame*);
void op_dup(JVM*, Frame*);    void op_dup_x1(JVM*, Frame*);
void op_dup_x2(JVM*, Frame*); void op_dup2(JVM*, Frame*);
void op_dup2_x1(JVM*, Frame*); void op_dup2_x2(JVM*, Frame*);
void op_swap(JVM*, Frame*);

/* control.cpp */
void op_goto(JVM*, Frame*);      void op_goto_w(JVM*, Frame*);
void op_jsr(JVM*, Frame*);       void op_jsr_w(JVM*, Frame*);
void op_ret(JVM*, Frame*);
void op_ifeq(JVM*, Frame*);      void op_ifne(JVM*, Frame*);
void op_iflt(JVM*, Frame*);      void op_ifge(JVM*, Frame*);
void op_ifgt(JVM*, Frame*);      void op_ifle(JVM*, Frame*);
void op_if_icmpeq(JVM*, Frame*); void op_if_icmpne(JVM*, Frame*);
void op_if_icmplt(JVM*, Frame*); void op_if_icmpge(JVM*, Frame*);
void op_if_icmpgt(JVM*, Frame*); void op_if_icmple(JVM*, Frame*);
void op_if_acmpeq(JVM*, Frame*); void op_if_acmpne(JVM*, Frame*);
void op_ifnull(JVM*, Frame*);    void op_ifnonnull(JVM*, Frame*);
void op_tableswitch(JVM*, Frame*); void op_lookupswitch(JVM*, Frame*);
void op_ireturn(JVM*, Frame*);   void op_lreturn(JVM*, Frame*);
void op_freturn(JVM*, Frame*);   void op_dreturn(JVM*, Frame*);
void op_areturn(JVM*, Frame*);   void op_return(JVM*, Frame*);

/* invoke.cpp */
void op_invokestatic(JVM*, Frame*);    void op_invokespecial(JVM*, Frame*);
void op_invokevirtual(JVM*, Frame*);   void op_invokeinterface(JVM*, Frame*);

/* field_ops.cpp */
void op_getstatic(JVM*, Frame*); void op_putstatic(JVM*, Frame*);
void op_getfield(JVM*, Frame*);  void op_putfield(JVM*, Frame*);

/* object_ops.cpp */
void op_new(JVM*, Frame*);
void op_checkcast(JVM*, Frame*);
void op_instanceof(JVM*, Frame*);

/* array_ops.cpp */
void op_newarray(JVM*, Frame*);   void op_anewarray(JVM*, Frame*);
void op_arraylength(JVM*, Frame*);
void op_iaload(JVM*, Frame*);  void op_laload(JVM*, Frame*);
void op_faload(JVM*, Frame*);  void op_daload(JVM*, Frame*);
void op_aaload(JVM*, Frame*);  void op_baload(JVM*, Frame*);
void op_caload(JVM*, Frame*);  void op_saload(JVM*, Frame*);
void op_iastore(JVM*, Frame*); void op_lastore(JVM*, Frame*);
void op_fastore(JVM*, Frame*); void op_dastore(JVM*, Frame*);
void op_aastore(JVM*, Frame*); void op_bastore(JVM*, Frame*);
void op_castore(JVM*, Frame*); void op_sastore(JVM*, Frame*);
void op_multianewarray(JVM*, Frame*);

/* convert.cpp */
void op_i2l(JVM*, Frame*); void op_i2f(JVM*, Frame*); void op_i2d(JVM*, Frame*);
void op_l2i(JVM*, Frame*); void op_l2f(JVM*, Frame*); void op_l2d(JVM*, Frame*);
void op_f2i(JVM*, Frame*); void op_f2l(JVM*, Frame*); void op_f2d(JVM*, Frame*);
void op_d2i(JVM*, Frame*); void op_d2l(JVM*, Frame*); void op_d2f(JVM*, Frame*);
void op_i2b(JVM*, Frame*); void op_i2c(JVM*, Frame*); void op_i2s(JVM*, Frame*);

/* exceptions.cpp */
void op_athrow(JVM*, Frame*);
void op_monitorenter(JVM*, Frame*);
void op_monitorexit(JVM*, Frame*);

/* ------------------------------------------------------------------ */
/* Handler padrao para opcodes nao implementados                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Imprime aviso e encerra quando um opcode nao implementado e executado.
 *
 * @details Recupera o byte do opcode a partir de frame->pc - 1 (o PC ja foi
 * incrementado antes da chamada ao handler) e imprime uma mensagem de erro
 * com o valor hexadecimal do opcode e a posicao no bytecode. Em seguida,
 * encerra o processo com exit(1), sinalizando falha ao sistema operacional.
 *
 * @param jvm   JVM em execucao (nao utilizado, presente para compatibilidade
 *              com o tipo OpcodeHandler).
 * @param frame Frame do metodo em execucao no momento do erro.
 */
static void op_not_implemented(JVM *jvm, Frame *frame) {
    (void)jvm;
    /* pc ja foi incrementado antes de chamar o handler */
    u1 op = frame->method->code_attr->code[frame->pc - 1];
    fprintf(stderr, "Opcode nao implementado: 0x%02X no pc=%u\n",
            op, frame->pc - 1);
    exit(1);
}

/* ------------------------------------------------------------------ */
/* Dispatch table                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Preenche a dispatch table com os handlers de cada opcode.
 *
 * @details O algoritmo opera em duas etapas:
 *   1. Inicializacao defensiva: todos os 256 slots recebem o ponteiro
 *      op_not_implemented, garantindo que qualquer opcode desconhecido
 *      gere um erro explicito em vez de comportamento indefinido.
 *   2. Registro dos opcodes implementados: cada slot e sobrescrito com
 *      o ponteiro para o handler correto, identificado pelo valor
 *      hexadecimal do opcode conforme a especificacao JVM §6.
 *
 * A tabela resultante permite despacho em O(1): o loop de interpretacao
 * indexa diretamente pelo byte lido do bytecode.
 *
 * @param jvm  JVM cuja dispatch table sera preenchida.
 */
void init_dispatch_table(JVM *jvm) {
    /* Todos comecam como nao implementado */
    for (int i = 0; i < 256; i++)
        jvm->dispatch[i] = op_not_implemented;

    jvm->dispatch[0x00] = op_nop;
    jvm->dispatch[0x01] = op_aconst_null;
    jvm->dispatch[0x02] = op_iconst_m1;
    jvm->dispatch[0x03] = op_iconst_0;
    jvm->dispatch[0x04] = op_iconst_1;
    jvm->dispatch[0x05] = op_iconst_2;
    jvm->dispatch[0x06] = op_iconst_3;
    jvm->dispatch[0x07] = op_iconst_4;
    jvm->dispatch[0x08] = op_iconst_5;
    jvm->dispatch[0x09] = op_lconst_0;
    jvm->dispatch[0x0A] = op_lconst_1;
    jvm->dispatch[0x0B] = op_fconst_0;
    jvm->dispatch[0x0C] = op_fconst_1;
    jvm->dispatch[0x0D] = op_fconst_2;
    jvm->dispatch[0x0E] = op_dconst_0;
    jvm->dispatch[0x0F] = op_dconst_1;
    jvm->dispatch[0x10] = op_bipush;
    jvm->dispatch[0x11] = op_sipush;
    jvm->dispatch[0x12] = op_ldc;
    jvm->dispatch[0x13] = op_ldc_w;
    jvm->dispatch[0x14] = op_ldc2_w;
    jvm->dispatch[0x15] = op_iload;
    jvm->dispatch[0x16] = op_lload;
    jvm->dispatch[0x17] = op_fload;
    jvm->dispatch[0x18] = op_dload;
    jvm->dispatch[0x19] = op_aload;
    jvm->dispatch[0x1A] = op_iload_0;
    jvm->dispatch[0x1B] = op_iload_1;
    jvm->dispatch[0x1C] = op_iload_2;
    jvm->dispatch[0x1D] = op_iload_3;
    jvm->dispatch[0x1E] = op_lload_0;
    jvm->dispatch[0x1F] = op_lload_1;
    jvm->dispatch[0x20] = op_lload_2;
    jvm->dispatch[0x21] = op_lload_3;
    jvm->dispatch[0x22] = op_fload_0;
    jvm->dispatch[0x23] = op_fload_1;
    jvm->dispatch[0x24] = op_fload_2;
    jvm->dispatch[0x25] = op_fload_3;
    jvm->dispatch[0x26] = op_dload_0;
    jvm->dispatch[0x27] = op_dload_1;
    jvm->dispatch[0x28] = op_dload_2;
    jvm->dispatch[0x29] = op_dload_3;
    jvm->dispatch[0x2A] = op_aload_0;
    jvm->dispatch[0x2B] = op_aload_1;
    jvm->dispatch[0x2C] = op_aload_2;
    jvm->dispatch[0x2D] = op_aload_3;
    jvm->dispatch[0x2E] = op_iaload;
    jvm->dispatch[0x2F] = op_laload;
    jvm->dispatch[0x30] = op_faload;
    jvm->dispatch[0x31] = op_daload;
    jvm->dispatch[0x32] = op_aaload;
    jvm->dispatch[0x33] = op_baload;
    jvm->dispatch[0x34] = op_caload;
    jvm->dispatch[0x35] = op_saload;
    jvm->dispatch[0x36] = op_istore;
    jvm->dispatch[0x37] = op_lstore;
    jvm->dispatch[0x38] = op_fstore;
    jvm->dispatch[0x39] = op_dstore;
    jvm->dispatch[0x3A] = op_astore;
    jvm->dispatch[0x3B] = op_istore_0;
    jvm->dispatch[0x3C] = op_istore_1;
    jvm->dispatch[0x3D] = op_istore_2;
    jvm->dispatch[0x3E] = op_istore_3;
    jvm->dispatch[0x3F] = op_lstore_0;
    jvm->dispatch[0x40] = op_lstore_1;
    jvm->dispatch[0x41] = op_lstore_2;
    jvm->dispatch[0x42] = op_lstore_3;
    jvm->dispatch[0x43] = op_fstore_0;
    jvm->dispatch[0x44] = op_fstore_1;
    jvm->dispatch[0x45] = op_fstore_2;
    jvm->dispatch[0x46] = op_fstore_3;
    jvm->dispatch[0x47] = op_dstore_0;
    jvm->dispatch[0x48] = op_dstore_1;
    jvm->dispatch[0x49] = op_dstore_2;
    jvm->dispatch[0x4A] = op_dstore_3;
    jvm->dispatch[0x4B] = op_astore_0;
    jvm->dispatch[0x4C] = op_astore_1;
    jvm->dispatch[0x4D] = op_astore_2;
    jvm->dispatch[0x4E] = op_astore_3;
    jvm->dispatch[0x4F] = op_iastore;
    jvm->dispatch[0x50] = op_lastore;
    jvm->dispatch[0x51] = op_fastore;
    jvm->dispatch[0x52] = op_dastore;
    jvm->dispatch[0x53] = op_aastore;
    jvm->dispatch[0x54] = op_bastore;
    jvm->dispatch[0x55] = op_castore;
    jvm->dispatch[0x56] = op_sastore;
    jvm->dispatch[0x57] = op_pop;
    jvm->dispatch[0x58] = op_pop2;
    jvm->dispatch[0x59] = op_dup;
    jvm->dispatch[0x5A] = op_dup_x1;
    jvm->dispatch[0x5B] = op_dup_x2;
    jvm->dispatch[0x5C] = op_dup2;
    jvm->dispatch[0x5D] = op_dup2_x1;
    jvm->dispatch[0x5E] = op_dup2_x2;
    jvm->dispatch[0x5F] = op_swap;
    jvm->dispatch[0x60] = op_iadd;
    jvm->dispatch[0x61] = op_ladd;
    jvm->dispatch[0x62] = op_fadd;
    jvm->dispatch[0x63] = op_dadd;
    jvm->dispatch[0x64] = op_isub;
    jvm->dispatch[0x65] = op_lsub;
    jvm->dispatch[0x66] = op_fsub;
    jvm->dispatch[0x67] = op_dsub;
    jvm->dispatch[0x68] = op_imul;
    jvm->dispatch[0x69] = op_lmul;
    jvm->dispatch[0x6A] = op_fmul;
    jvm->dispatch[0x6B] = op_dmul;
    jvm->dispatch[0x6C] = op_idiv;
    jvm->dispatch[0x6D] = op_ldiv;
    jvm->dispatch[0x6E] = op_fdiv;
    jvm->dispatch[0x6F] = op_ddiv;
    jvm->dispatch[0x70] = op_irem;
    jvm->dispatch[0x71] = op_lrem;
    jvm->dispatch[0x72] = op_frem;
    jvm->dispatch[0x73] = op_drem;
    jvm->dispatch[0x74] = op_ineg;
    jvm->dispatch[0x75] = op_lneg;
    jvm->dispatch[0x76] = op_fneg;
    jvm->dispatch[0x77] = op_dneg;
    jvm->dispatch[0x78] = op_ishl;
    jvm->dispatch[0x79] = op_lshl;
    jvm->dispatch[0x7A] = op_ishr;
    jvm->dispatch[0x7B] = op_lshr;
    jvm->dispatch[0x7C] = op_iushr;
    jvm->dispatch[0x7D] = op_lushr;
    jvm->dispatch[0x7E] = op_iand;
    jvm->dispatch[0x7F] = op_land;
    jvm->dispatch[0x80] = op_ior;
    jvm->dispatch[0x81] = op_lor;
    jvm->dispatch[0x82] = op_ixor;
    jvm->dispatch[0x83] = op_lxor;
    jvm->dispatch[0x84] = op_iinc;
    jvm->dispatch[0x85] = op_i2l;
    jvm->dispatch[0x86] = op_i2f;
    jvm->dispatch[0x87] = op_i2d;
    jvm->dispatch[0x88] = op_l2i;
    jvm->dispatch[0x89] = op_l2f;
    jvm->dispatch[0x8A] = op_l2d;
    jvm->dispatch[0x8B] = op_f2i;
    jvm->dispatch[0x8C] = op_f2l;
    jvm->dispatch[0x8D] = op_f2d;
    jvm->dispatch[0x8E] = op_d2i;
    jvm->dispatch[0x8F] = op_d2l;
    jvm->dispatch[0x90] = op_d2f;
    jvm->dispatch[0x91] = op_i2b;
    jvm->dispatch[0x92] = op_i2c;
    jvm->dispatch[0x93] = op_i2s;
    jvm->dispatch[0x94] = op_lcmp;
    jvm->dispatch[0x95] = op_fcmpl;
    jvm->dispatch[0x96] = op_fcmpg;
    jvm->dispatch[0x97] = op_dcmpl;
    jvm->dispatch[0x98] = op_dcmpg;
    jvm->dispatch[0x99] = op_ifeq;
    jvm->dispatch[0x9A] = op_ifne;
    jvm->dispatch[0x9B] = op_iflt;
    jvm->dispatch[0x9C] = op_ifge;
    jvm->dispatch[0x9D] = op_ifgt;
    jvm->dispatch[0x9E] = op_ifle;
    jvm->dispatch[0x9F] = op_if_icmpeq;
    jvm->dispatch[0xA0] = op_if_icmpne;
    jvm->dispatch[0xA1] = op_if_icmplt;
    jvm->dispatch[0xA2] = op_if_icmpge;
    jvm->dispatch[0xA3] = op_if_icmpgt;
    jvm->dispatch[0xA4] = op_if_icmple;
    jvm->dispatch[0xA5] = op_if_acmpeq;
    jvm->dispatch[0xA6] = op_if_acmpne;
    jvm->dispatch[0xA7] = op_goto;
    jvm->dispatch[0xA8] = op_jsr;
    jvm->dispatch[0xA9] = op_ret;
    jvm->dispatch[0xAA] = op_tableswitch;
    jvm->dispatch[0xAB] = op_lookupswitch;
    jvm->dispatch[0xAC] = op_ireturn;
    jvm->dispatch[0xAD] = op_lreturn;
    jvm->dispatch[0xAE] = op_freturn;
    jvm->dispatch[0xAF] = op_dreturn;
    jvm->dispatch[0xB0] = op_areturn;
    jvm->dispatch[0xB1] = op_return;
    jvm->dispatch[0xB2] = op_getstatic;
    jvm->dispatch[0xB3] = op_putstatic;
    jvm->dispatch[0xB4] = op_getfield;
    jvm->dispatch[0xB5] = op_putfield;
    jvm->dispatch[0xB6] = op_invokevirtual;
    jvm->dispatch[0xB7] = op_invokespecial;
    jvm->dispatch[0xB8] = op_invokestatic;
    jvm->dispatch[0xB9] = op_invokeinterface;
    jvm->dispatch[0xBB] = op_new;
    jvm->dispatch[0xBC] = op_newarray;
    jvm->dispatch[0xBD] = op_anewarray;
    jvm->dispatch[0xBE] = op_arraylength;
    jvm->dispatch[0xBF] = op_athrow;
    jvm->dispatch[0xC0] = op_checkcast;
    jvm->dispatch[0xC1] = op_instanceof;
    jvm->dispatch[0xC2] = op_monitorenter;
    jvm->dispatch[0xC3] = op_monitorexit;
    jvm->dispatch[0xC4] = op_wide;
    jvm->dispatch[0xC5] = op_multianewarray;
    jvm->dispatch[0xC6] = op_ifnull;
    jvm->dispatch[0xC7] = op_ifnonnull;
    jvm->dispatch[0xC8] = op_goto_w;
    jvm->dispatch[0xC9] = op_jsr_w;
}

/* ------------------------------------------------------------------ */
/* Heap helpers                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Aloca um JObject no heap da JVM e retorna o indice (referencia).
 *
 * @details Cria uma nova entrada na heap table com tag HEAP_OBJECT. O objeto
 * e construido via object_new(klass) e armazenado em HeapEntry::ptr. O indice
 * retornado e a posicao do elemento no vetor jvm->heap, que e sempre >= 1
 * (o indice 0 e reservado como sentinel de null desde jvm_create).
 *
 * @param jvm    JVM cujo heap sera expandido.
 * @param klass  Classe do objeto a alocar; define os campos e o layout.
 * @return Indice int32_t na heap table, usado como referencia de objeto.
 */
int32_t heap_alloc_object(JVM *jvm, ClassEntry *klass) {
    HeapEntry e;
    e.tag = HEAP_OBJECT;
    e.ptr = object_new(klass);
    jvm->heap.push_back(e);
    return (int32_t)(jvm->heap.size() - 1);
}

/**
 * @brief Aloca um JArray no heap da JVM e retorna o indice.
 *
 * @details Cria uma nova entrada na heap table com tag HEAP_ARRAY. O array
 * e construido via array_new(type, length), que aloca o buffer de elementos
 * zerados. O indice retornado segue a mesma convencao de heap_alloc_object.
 *
 * @param jvm     JVM cujo heap sera expandido.
 * @param type    Tipo dos elementos do array (ex: AT_INT, AT_REF).
 * @param length  Numero de elementos a alocar.
 * @return Indice int32_t na heap table, usado como referencia de array.
 */
int32_t heap_alloc_array(JVM *jvm, ArrayType type, int32_t length) {
    HeapEntry e;
    e.tag = HEAP_ARRAY;
    e.ptr = array_new(type, length);
    jvm->heap.push_back(e);
    return (int32_t)(jvm->heap.size() - 1);
}

/**
 * @brief Retorna o JObject* associado ao indice do heap.
 *
 * @details Faz cast do ponteiro generico HeapEntry::ptr para JObject*.
 * Retorna NULL se ref <= 0 (referencia nula ou invalida), evitando acesso
 * fora dos limites do vetor.
 *
 * @param jvm  JVM dona do heap.
 * @param ref  Indice na heap table (0 = null).
 * @return JObject* correspondente, ou NULL se ref <= 0.
 */
JObject *heap_get_object(JVM *jvm, int32_t ref) {
    if (ref <= 0) return NULL;
    return static_cast<JObject *>(jvm->heap[(size_t)ref].ptr);
}

/**
 * @brief Retorna o JArray* associado ao indice do heap.
 *
 * @details Analogo a heap_get_object, mas para entradas com tag HEAP_ARRAY.
 * Retorna NULL se ref <= 0.
 *
 * @param jvm  JVM dona do heap.
 * @param ref  Indice na heap table (0 = null).
 * @return JArray* correspondente, ou NULL se ref <= 0.
 */
JArray *heap_get_array(JVM *jvm, int32_t ref) {
    if (ref <= 0) return NULL;
    return static_cast<JArray *>(jvm->heap[(size_t)ref].ptr);
}

/* ------------------------------------------------------------------ */
/* Sistema de excecoes                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Sinaliza uma excecao pendente na JVM.
 *
 * @details Marca jvm->exception_pending como true e registra o nome da
 * classe e a referencia do objeto de excecao. Nao realiza desvio de fluxo
 * imediato: o loop interpret() verifica o flag no inicio de cada ciclo e
 * aciona handle_exception() para buscar o handler adequado no frame atual.
 *
 * Opcodes internos (divisao por zero, acesso nulo, fora dos limites de array)
 * devem chamar jvm_throw() em vez de exit(), permitindo que o bytecode Java
 * capture a excecao com try/catch.
 *
 * @param jvm        JVM em execucao.
 * @param class_name Nome interno da classe da excecao
 *                   (ex: "java/lang/NullPointerException").
 * @param ref        Referencia na heap para o objeto de excecao
 *                   (0 para excecoes sinteticas sem objeto real).
 */
void jvm_throw(JVM *jvm, const std::string &class_name, int32_t ref) {
    jvm->exception_pending = true;
    jvm->exception_class   = class_name;
    jvm->exception_ref     = ref;
}

/**
 * @brief Verifica se a excecao lancada e compativel com o tipo capturado.
 *
 * @details Implementa a regra "e instancia de" da JVM em tres etapas:
 *   1. Correspondencia exata entre thrown e caught.
 *   2. Hierarquia estatica: caught == Throwable ou Exception captura qualquer
 *      excecao; caught == RuntimeException captura as subclasses conhecidas
 *      hardcoded (ArrayIndexOutOfBounds, NullPointer, ClassCast, etc.).
 *   3. Hierarquia dinamica: sobe a cadeia de super classes da excecao lancada
 *      (campo ClassEntry::super) ate encontrar caught ou esgotar a cadeia.
 *
 * @param jvm     JVM em execucao (usada para buscar ClassEntry pelo nome).
 * @param thrown  Nome interno da classe da excecao lancada.
 * @param caught  Nome interno da classe do catch clause.
 * @return true se thrown e capturavel por caught.
 */
static bool exception_matches(JVM *jvm, const std::string &thrown,
                               const std::string &caught) {
    if (caught == thrown)                    return true;
    if (caught == "java/lang/Throwable")     return true;
    if (caught == "java/lang/Exception")     return true;
    if (caught == "java/lang/RuntimeException") {
        /* Excecoes de runtime conhecidas */
        if (thrown == "java/lang/ArrayIndexOutOfBoundsException") return true;
        if (thrown == "java/lang/IndexOutOfBoundsException")      return true;
        if (thrown == "java/lang/NullPointerException")           return true;
        if (thrown == "java/lang/ClassCastException")             return true;
        if (thrown == "java/lang/ArithmeticException")            return true;
        if (thrown == "java/lang/IllegalArgumentException")       return true;
        if (thrown == "java/lang/IllegalStateException")          return true;
        if (thrown == "java/lang/NumberFormatException")          return true;
    }
    /* Sobe a cadeia de heranca da excecao lancada */
    ClassEntry *ce = find_class(jvm->method_area, thrown);
    while (ce && ce->super) {
        if (ce->super->name == caught) return true;
        ce = ce->super;
    }
    return false;
}

/**
 * @brief Tenta encontrar um handler na exception_table do frame.
 *
 * @details Percorre linearmente a exception_table do metodo em execucao.
 * Para cada entrada, verifica:
 *   1. Se o PC que gerou a excecao (exception_pc) esta no intervalo
 *      [start_pc, end_pc) da entrada.
 *   2. Se catch_type == 0 (bloco finally, captura qualquer excecao), ou
 *      se a classe capturada e compativel com a excecao lancada via
 *      exception_matches().
 *
 * Quando um handler e encontrado:
 *   - A operand stack do frame e limpa (comportamento especificado pela JVM).
 *   - A referencia do objeto de excecao e empurrada no topo da stack.
 *   - O PC do frame e redirecionado para handler_pc.
 *   - O flag exception_pending e limpo.
 *
 * @param jvm  JVM com excecao pendente.
 * @param f    Frame onde procurar o handler.
 * @return true se handler encontrado e excecao consumida; false caso contrario.
 */
static bool handle_exception(JVM *jvm, Frame *f) {
    if (!f->method || !f->method->code_attr) return false;
    Code_attribute *code = f->method->code_attr;

    for (u2 i = 0; i < code->exception_table_length; i++) {
        exception_entry &e = code->exception_table[i];
        if (jvm->exception_pc < e.start_pc || jvm->exception_pc >= e.end_pc)
            continue;

        bool matches;
        if (e.catch_type == 0) {
            matches = true; /* finally — captura qualquer excecao */
        } else {
            std::string catch_name = resolve_class_name(f->klass->cf, e.catch_type);
            matches = exception_matches(jvm, jvm->exception_class, catch_name);
        }

        if (matches) {
            /* Handler encontrado: limpa stack, empurra ref e desvia */
            f->operand_stack.clear();
            frame_push(f, jvm->exception_ref);
            f->pc = e.handler_pc;
            jvm->exception_pending = false;
            return true;
        }
    }
    return false;
}

/**
 * @brief Loop principal de interpretacao da JVM.
 *
 * @details Implementa o ciclo fetch-decode-execute sobre a pilha de frames:
 *
 *   1. **Verificacao de excecao**: se exception_pending estiver ativo,
 *      tenta resolver via handle_exception() no frame atual. Se encontrar
 *      um handler, continua a execucao nesse frame. Se nao encontrar,
 *      registra o call_site_pc do frame atual como novo exception_pc
 *      (para que o frame chamador busque no ponto certo) e desempilha
 *      o frame, propagando a excecao para o chamador.
 *
 *   2. **Fetch**: salva o PC atual em exception_pc (usado se o opcode
 *      seguinte lancar uma excecao), le o byte do opcode em code[pc]
 *      e incrementa pc.
 *
 *   3. **Decode + Execute**: indexa diretamente dispatch[op] e chama
 *      o handler, que le seus proprios operandos e avanca pc.
 *
 * O loop termina quando a pilha de frames fica vazia (metodo main retornou)
 * ou quando uma excecao nao capturada chega ao frame raiz. Neste ultimo caso,
 * imprime "Exception in thread main <classe>" em stderr.
 *
 * @param jvm  JVM em execucao com pelo menos um frame na pilha.
 */
void interpret(JVM *jvm) {
    while (!jvm_stack_is_empty(jvm->stack)) {
        Frame *f = jvm_stack_current(jvm->stack);

        /* Excecao propagando — tenta handler neste frame */
        if (jvm->exception_pending) {
            if (handle_exception(jvm, f)) continue;
            /* Sem handler: propaga para o chamador */
            jvm->exception_pc = f->call_site_pc;
            jvm_stack_pop(jvm->stack);
            continue;
        }

        /* Salva PC antes de avancar — usado como throw_pc se excecao ocorrer */
        jvm->exception_pc = f->pc;
        u1 op = f->method->code_attr->code[f->pc++];
        jvm->dispatch[op](jvm, f);
    }

    if (jvm->exception_pending) {
        fprintf(stderr, "Exception in thread \"main\" %s\n",
                jvm->exception_class.c_str());
        jvm->exception_pending = false;
    }
}

/**
 * @brief Cria um frame para o metodo e o empurra na pilha de execucao.
 *
 * @details Delega a alocacao e inicializacao do frame para frame_create(),
 * que configura local_vars (zeradas, tamanho max_locals) e operand_stack
 * (vazia, capacidade max_stack) a partir dos atributos do metodo. Em seguida,
 * empurra o frame na JvmStack. O loop interpret() passara a executar a partir
 * deste novo frame no proximo ciclo.
 *
 * @param jvm    JVM em execucao.
 * @param m      Metodo a invocar; deve conter um Code_attribute valido.
 * @param klass  Classe dona do metodo; usada para resolver o constant pool.
 */
void execute_method(JVM *jvm, method_info *m, ClassEntry *klass) {
    Frame *f = frame_create(m, klass);
    jvm_stack_push(jvm->stack, f);
}

/* ------------------------------------------------------------------ */
/* Ciclo de vida da JVM                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Cria e inicializa uma JVM completa a partir de um arquivo .class.
 *
 * @details O algoritmo segue as etapas abaixo:
 *   1. Aloca a estrutura JVM e inicializa method_area e jvm_stack.
 *   2. Insere o sentinel de null em heap[0] (HeapEntry com tag HEAP_NULL),
 *      garantindo que a referencia 0 nunca aponte para um objeto valido.
 *   3. Zera o estado de excecao (exception_pending, exception_ref, exception_pc).
 *   4. Chama init_dispatch_table() para montar a tabela de despacho O(1).
 *   5. Deriva o nome da classe e o diretorio de busca a partir de class_path:
 *      - Remove a extensao ".class" do caminho.
 *      - Separa o ultimo separador de diretorio ('/' ou '\') para obter
 *        class_dir (usado por auto_load_class) e o nome simples da classe.
 *      - Se nao houver separador, class_dir recebe ".".
 *   6. Carrega a classe principal via load_class() e resolve sua cadeia de
 *      heranca via resolve_super_chain().
 *
 * @param class_path  Caminho para o arquivo .class principal
 *                    (ex: "tests/class/HelloWorld.class").
 * @return Ponteiro para a JVM criada, ou NULL em caso de erro de leitura.
 */
JVM *jvm_create(const char *class_path) {
    JVM *jvm = new JVM();

    jvm->method_area = method_area_create();
    jvm->stack       = jvm_stack_create();

    /* heap[0] = null sentinel */
    HeapEntry null_entry;
    null_entry.tag = HEAP_NULL;
    null_entry.ptr = NULL;
    jvm->heap.push_back(null_entry);

    /* Sem excecao pendente ao iniciar */
    jvm->exception_pending = false;
    jvm->exception_ref     = 0;
    jvm->exception_pc      = 0;

    init_dispatch_table(jvm);

    /* Deriva nome da classe e diretorio a partir do path */
    std::string path(class_path);
    std::string name = path;

    /* Remove extensao .class se presente */
    size_t dot = name.rfind(".class");
    if (dot != std::string::npos) name = name.substr(0, dot);

    /* Extrai diretorio para auto_load_class */
    size_t sep = name.find_last_of("/\\");
    if (sep != std::string::npos) {
        jvm->class_dir = name.substr(0, sep);
        name = name.substr(sep + 1);
    } else {
        jvm->class_dir = ".";
    }

    jvm->main_class = name; /* guarda o nome para jvm_run() localizar a classe correta */

    ClassEntry *ce = load_class(jvm->method_area, name, path);
    if (!ce) {
        fprintf(stderr, "Erro: nao foi possivel carregar '%s'\n", class_path);
        jvm_destroy(jvm);
        return NULL;
    }

    resolve_super_chain(jvm, ce);
    return jvm;
}

/**
 * @brief Resolve recursivamente a cadeia de heranca de uma ClassEntry.
 *
 * @details Para cada ClassEntry carregada, verifica se o campo super_class
 * do ClassFile aponta para uma superclasse (valor != 0). O algoritmo:
 *   1. Retorna imediatamente se ce e NULL, nao tem ClassFile, ou nao tem
 *      super_class definido.
 *   2. Retorna se ce->super ja foi resolvido (evita ciclos e trabalho duplo).
 *   3. Resolve o nome da superclasse via resolve_class_name().
 *   4. Se a superclasse e "java/lang/Object", interrompe a cadeia (Object
 *      nao e carregado do disco nesta implementacao).
 *   5. Caso contrario, carrega a superclasse via auto_load_class() e a
 *      atribui a ce->super, chamando-se recursivamente para resolver a
 *      cadeia completa.
 *
 * @param jvm  JVM em execucao; usada para acessar method_area e class_dir.
 * @param ce   ClassEntry cuja cadeia de heranca sera resolvida.
 */
static void resolve_super_chain(JVM *jvm, ClassEntry *ce) {
    if (!ce || !ce->cf || ce->cf->super_class == 0) return;
    if (ce->super != NULL) return; /* ja resolvido */

    std::string super_name = resolve_class_name(ce->cf, ce->cf->super_class);
    if (super_name == "java/lang/Object") return; /* nao precisamos carregar Object */

    ce->super = auto_load_class(jvm, super_name); /* recursivo */
}

/**
 * @brief Busca ou carrega automaticamente uma classe pelo nome interno.
 *
 * @details O algoritmo segue tres etapas em ordem:
 *   1. Busca na method_area por uma ClassEntry ja carregada com o nome dado.
 *      Se encontrar, retorna imediatamente (cache hit).
 *   2. Se o nome comeca com "java/", "javax/" ou "sun/", cria uma ClassEntry
 *      sintetica sem ClassFile (cf = NULL, initialized = true). Isso evita
 *      tentativas de leitura de disco para classes do JRE que nao existem
 *      no projeto, permitindo que a cadeia de heranca seja representada
 *      sem falhas.
 *   3. Tenta carregar o arquivo "<class_dir>/<nome>.class" via load_class().
 *      Se falhar, tenta "<nome>.class" no diretorio corrente como fallback.
 *      Se carregar com sucesso, resolve a cadeia de heranca da nova ClassEntry.
 *
 * @param jvm   JVM em execucao.
 * @param name  Nome interno da classe (ex: "Jogador", "java/lang/Object").
 * @return ClassEntry* carregada ou sintetica; NULL se o arquivo nao for
 *         encontrado e a classe nao for do JRE.
 */
ClassEntry *auto_load_class(JVM *jvm, const std::string &name) {
    ClassEntry *ce = find_class(jvm->method_area, name);
    if (ce) return ce;

    /* Classes JRE (java/, javax/, sun/) nao estao no disco do projeto.
     * Cria entrada sintetica diretamente sem tentar o disco. */
    if (name.size() > 5 &&
        (name.substr(0,5) == "java/" || name.substr(0,6) == "javax/" ||
         name.substr(0,4) == "sun/")) {
        ce = new ClassEntry();
        ce->name        = name;
        ce->cf          = NULL;
        ce->initialized = true;
        ce->super       = NULL;
        jvm->method_area->classes[name] = ce;
        return ce;
    }

    /* Monta caminho: class_dir/NomeClasse.class */
    std::string path = jvm->class_dir + "/" + name + ".class";
    ce = load_class(jvm->method_area, name, path);
    if (!ce) {
        path = name + ".class";
        ce = load_class(jvm->method_area, name, path);
    }

    if (ce) resolve_super_chain(jvm, ce);
    return ce;
}

/**
 * @brief Executa o metodo main da classe principal carregada na JVM.
 *
 * @details Localiza a classe principal pelo nome registrado em jvm->main_class
 * (salvo em jvm_create), evitando depender da ordem de iteracao do std::map
 * que poderia retornar uma superclasse carregada automaticamente em vez da
 * classe principal (ex: ao rodar Gato, Animal seria retornado por begin()
 * pois o map ordena as chaves alfabeticamente).
 *
 * @param jvm  JVM inicializada por jvm_create() com a classe principal ja
 *             carregada na method_area.
 */
void jvm_run(JVM *jvm) {
    ClassEntry *ce = find_class(jvm->method_area, jvm->main_class);
    if (!ce) {
        fprintf(stderr, "Erro: classe principal '%s' nao encontrada\n",
                jvm->main_class.c_str());
        return;
    }

    method_info *main_m = find_method(ce, "main", "([Ljava/lang/String;)V");
    if (!main_m) {
        fprintf(stderr, "Erro: metodo main nao encontrado em '%s'\n",
                ce->name.c_str());
        return;
    }

    execute_method(jvm, main_m, ce);
    interpret(jvm);
}

/**
 * @brief Libera todos os recursos alocados pela JVM.
 *
 * @details A liberacao segue a ordem inversa de alocacao em jvm_create():
 *   1. Itera sobre heap[1..n-1] (heap[0] e o sentinel null e nao possui
 *      memoria propria a liberar). Para cada entrada, chama object_free()
 *      ou array_free() conforme a tag HEAP_OBJECT ou HEAP_ARRAY.
 *   2. Destroi a JvmStack via jvm_stack_destroy(), liberando todos os
 *      frames remanescentes.
 *   3. Destroi a MethodArea via method_area_destroy(), liberando as
 *      ClassEntries e seus ClassFiles.
 *   4. Deleta a propria estrutura JVM.
 *
 * E seguro chamar com jvm == NULL (retorna imediatamente sem acao).
 *
 * @param jvm  JVM a destruir; pode ser NULL.
 */
void jvm_destroy(JVM *jvm) {
    if (!jvm) return;

    /* Libera heap */
    for (size_t i = 1; i < jvm->heap.size(); i++) {
        if (jvm->heap[i].tag == HEAP_OBJECT)
            object_free(static_cast<JObject *>(jvm->heap[i].ptr));
        else if (jvm->heap[i].tag == HEAP_ARRAY)
            array_free(static_cast<JArray *>(jvm->heap[i].ptr));
    }

    jvm_stack_destroy(jvm->stack);
    method_area_destroy(jvm->method_area);
    delete jvm;
}