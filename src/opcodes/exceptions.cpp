#include "interpreter.h"
#include "frame.h"

/**
 * @brief athrow — lanca excecao Java.
 *
 * Delega ao sistema de excecoes do interpret loop via jvm_throw().
 * O loop chama handle_exception() que busca o handler na exception_table
 * e propaga pelos frames caso nao encontre.
 */
void op_athrow(JVM *jvm, Frame *frame) {
    int32_t ref = frame_pop(frame);

    if (ref == 0) {
        /* throw null -> NullPointerException */
        jvm_throw(jvm, "java/lang/NullPointerException", 0);
        return;
    }

    JObject *exc = heap_get_object(jvm, ref);
    std::string exc_name = "java/lang/Throwable";
    if (exc && exc->klass) exc_name = exc->klass->name;

    jvm_throw(jvm, exc_name, ref);
}

/**
 * @brief monitorenter — sem suporte a threads, e um nop.
 */
void op_monitorenter(JVM *jvm, Frame *frame) {
    (void)jvm;
    frame_pop(frame);
}

/**
 * @brief monitorexit — sem suporte a threads, e um nop.
 */
void op_monitorexit(JVM *jvm, Frame *frame) {
    (void)jvm;
    frame_pop(frame);
}
