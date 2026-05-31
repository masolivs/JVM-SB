#include "interpreter.h"
#include "constant_pool.h"
#include "frame.h"
#include <cstdio>

/**
 * @brief Localiza o indice de um campo estatico dentro de ClassEntry::static_fields.
 *
 * @details Percorre a tabela de campos do ClassFile da classe, contando apenas
 * os campos marcados com ACC_STATIC. Para cada campo estatico, compara nome e
 * descriptor com os argumentos fornecidos. Retorna o indice relativo (posicao
 * dentre os campos estaticos, nao o indice global no ClassFile).
 *
 * @param ce    Classe onde buscar; pode ser NULL (retorna -1).
 * @param name  Nome do campo buscado.
 * @param desc  Descriptor do campo buscado (ex: "I", "Ljava/lang/String;").
 * @return Indice em static_fields[], ou -1 se nao encontrado.
 */
static int find_static_field_index(ClassEntry *ce, const std::string &name,
                                   const std::string &desc) {
    if (!ce || !ce->cf) return -1;
    int idx = 0;
    for (u2 i = 0; i < ce->cf->fields_count; i++) {
        if (!(ce->cf->fields[i].access_flags & ACC_STATIC)) continue;
        std::string fn = resolve_utf8(ce->cf, ce->cf->fields[i].name_index);
        std::string fd = resolve_utf8(ce->cf, ce->cf->fields[i].descriptor_index);
        if (fn == name && fd == desc) return idx;
        idx++;
    }
    return -1;
}

/**
 * @brief getstatic — le o valor de um campo estatico e empurra na pilha.
 *
 * @details
 *   1. Le o indice de 2 bytes do constant pool e avanca pc += 2.
 *   2. Resolve a classe e o par nome:descriptor do campo.
 *   3. Caso especial: System.out e emulado empurrando a referencia 0
 *      (null simulado); o handler de invokevirtual trata System.out.println.
 *   4. Carrega a classe via auto_load_class() e localiza o campo com
 *      find_static_field_index(). Se nao encontrado, empurra 0.
 *   5. Empurra o valor armazenado em ClassEntry::static_fields[idx].
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_getstatic(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    const ClassFile *cf = frame->klass->cf;
    std::string cls  = resolve_class_name(cf, cf->constant_pool[idx].data.ref.class_index);
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t col = nat.find(':');
    std::string fname = nat.substr(0, col);
    std::string fdesc = nat.substr(col + 1);

    /* System.out: empurra referencia nula (simulado no invokevirtual) */
    if (cls == "java/lang/System" && fname == "out") {
        frame_push(frame, 0); /* ref simulada */
        return;
    }

    ClassEntry *ce = auto_load_class(jvm, cls);
    if (!ce) { fprintf(stderr,"getstatic: classe nao encontrada: %s\n", cls.c_str()); frame_push(frame,0); return; }

    int fi = find_static_field_index(ce, fname, fdesc);
    if (fi < 0) { frame_push(frame, 0); return; }
    frame_push(frame, ce->static_fields[(size_t)fi]);
}

/**
 * @brief putstatic — retira valor da pilha e armazena em campo estatico.
 *
 * @details
 *   1. Le o indice de 2 bytes e avanca pc += 2.
 *   2. Resolve a classe e o par nome:descriptor do campo.
 *   3. Retira o valor do topo da operand stack.
 *   4. Carrega a classe e localiza o campo estatico via
 *      find_static_field_index(). Se encontrado, escreve o valor em
 *      ClassEntry::static_fields[idx].
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_putstatic(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    const ClassFile *cf = frame->klass->cf;
    std::string cls  = resolve_class_name(cf, cf->constant_pool[idx].data.ref.class_index);
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t col = nat.find(':');
    std::string fname = nat.substr(0, col);
    std::string fdesc = nat.substr(col + 1);

    int32_t value = frame_pop(frame);

    ClassEntry *ce = auto_load_class(jvm, cls);
    if (!ce) { return; }

    int fi = find_static_field_index(ce, fname, fdesc);
    if (fi >= 0) ce->static_fields[(size_t)fi] = value;
}

/**
 * @brief getfield — le campo de instancia de um objeto e empurra na pilha.
 *
 * @details
 *   1. Le o indice de 2 bytes e avanca pc += 2.
 *   2. Retira a referencia do objeto (ref) da operand stack.
 *   3. Resolve o objeto via heap_get_object(); se null, encerra com erro
 *      (NullPointerException — pendente de integracao com jvm_throw).
 *   4. Resolve o par nome:descriptor do campo a partir do constant pool.
 *   5. Localiza o campo de instancia via find_field_index(), que busca
 *      na classe do objeto e sobe a hierarquia se necessario.
 *   6. Empurra o valor via object_get_field().
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_getfield(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    int32_t ref = frame_pop(frame);
    JObject *obj = heap_get_object(jvm, ref);
    if (!obj) { fprintf(stderr,"NullPointerException em getfield\n"); exit(1); }

    const ClassFile *cf = frame->klass->cf;
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t col = nat.find(':');
    std::string fname = nat.substr(0, col);
    std::string fdesc = nat.substr(col + 1);

    int fi = find_field_index(obj->klass, fname, fdesc);
    if (fi < 0) { frame_push(frame, 0); return; }
    frame_push(frame, object_get_field(obj, fi));
}

/**
 * @brief putfield — retira valor e referencia da pilha e escreve em campo de instancia.
 *
 * @details
 *   1. Le o indice de 2 bytes e avanca pc += 2.
 *   2. Retira o valor (topo) e a referencia do objeto (segundo slot).
 *      Nota: a ordem de pop e value primeiro, depois ref, pois a instrucao
 *      empilha ref antes de value.
 *   3. Resolve o objeto; aborta com erro se null.
 *   4. Resolve o par nome:descriptor e localiza o campo via find_field_index().
 *   5. Escreve o valor via object_set_field().
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_putfield(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    int32_t value = frame_pop(frame);
    int32_t ref   = frame_pop(frame);

    JObject *obj = heap_get_object(jvm, ref);
    if (!obj) { fprintf(stderr,"NullPointerException em putfield\n"); exit(1); }

    const ClassFile *cf = frame->klass->cf;
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t col = nat.find(':');
    std::string fname = nat.substr(0, col);
    std::string fdesc = nat.substr(col + 1);

    int fi = find_field_index(obj->klass, fname, fdesc);
    if (fi >= 0) object_set_field(obj, fi, value);
}