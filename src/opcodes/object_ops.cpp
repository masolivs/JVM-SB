#include "interpreter.h"
#include "constant_pool.h"
#include "frame.h"
#include <cstdio>
#include <cstdlib>

/**
 * @brief new — aloca um novo objeto no heap e empurra sua referencia na pilha.
 *
 * @details
 *   1. Le o indice de 2 bytes do constant pool e avanca pc += 2.
 *   2. Resolve o nome da classe a ser instanciada via resolve_class_name().
 *   3. Carrega a classe via auto_load_class() (usa cache ou disco).
 *      Se a classe nao for encontrada, empurra 0 (null) e retorna.
 *   4. Aloca o objeto no heap via heap_alloc_object(), que cria um JObject
 *      com todos os campos zerados e retorna o indice na heap table.
 *   5. Empurra o indice (referencia) no topo da operand stack.
 *
 * @note O objeto alocado ainda nao foi inicializado; o bytecode Java emite
 *       um invokespecial para `<init>` logo apos o new.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_new(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    std::string cls = resolve_class_name(frame->klass->cf, idx);
    ClassEntry *ce  = auto_load_class(jvm, cls);
    if (!ce) {
        fprintf(stderr, "new: nao foi possivel carregar classe: %s\n", cls.c_str());
        frame_push(frame, 0);
        return;
    }

    int32_t ref = heap_alloc_object(jvm, ce);
    frame_push(frame, ref);
}

/**
 * @brief checkcast — verifica se um objeto e compativel com um tipo dado.
 *
 * @details Na JVM completa, checkcast lanca ClassCastException se o objeto
 * nao for instancia do tipo referenciado. Nesta implementacao, a verificacao
 * de tipo e omitida (simplificacao aceitavel para o escopo do projeto):
 * os 2 bytes do indice sao consumidos e a referencia e mantida no topo
 * da operand stack sem modificacao.
 *
 * @param jvm    Nao utilizado.
 * @param frame  Frame corrente; pc e avancado 2 bytes.
 */
void op_checkcast(JVM *jvm, Frame *frame) {
    /* Lemos o indice mas nao verificamos — simplificacao aceitavel */
    frame->pc += 2;
    (void)jvm;
    /* Deixa a referencia no topo sem modificar */
}

/**
 * @brief instanceof — verifica se um objeto e instancia de um tipo e empurra 0 ou 1.
 *
 * @details
 *   1. Le o indice de 2 bytes e resolve o nome da classe alvo.
 *   2. Retira a referencia do objeto da operand stack.
 *   3. Se ref == 0 (null), empurra 0 (null nao e instancia de nenhum tipo).
 *   4. Obtem o JObject* via heap_get_object(). Se invalido, empurra 0.
 *   5. Percorre a cadeia de heranca de obj->klass via o ponteiro super:
 *      se alguma classe na cadeia tiver o mesmo nome que target_cls,
 *      empurra 1 (e instancia). Se esgotar a cadeia sem encontrar, empurra 0.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame corrente.
 */
void op_instanceof(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    int32_t ref = frame_pop(frame);
    if (ref == 0) { frame_push(frame, 0); return; }

    std::string target_cls = resolve_class_name(frame->klass->cf, idx);
    JObject *obj = heap_get_object(jvm, ref);
    if (!obj || !obj->klass) { frame_push(frame, 0); return; }

    /* Verifica se o tipo do objeto e o mesmo ou subclasse */
    ClassEntry *ce = obj->klass;
    while (ce) {
        if (ce->name == target_cls) { frame_push(frame, 1); return; }
        ce = ce->super;
    }
    frame_push(frame, 0);
}