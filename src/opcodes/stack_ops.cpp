#include "interpreter.h"
#include "frame.h"

void op_pop (JVM *jvm, Frame *frame) { (void)jvm; frame_pop(frame); }
void op_pop2(JVM *jvm, Frame *frame) { (void)jvm; frame_pop(frame); frame_pop(frame); }

void op_dup(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t v = frame_peek(frame);
    frame_push(frame, v);
}
void op_dup_x1(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t v1 = frame_pop(frame);
    int32_t v2 = frame_pop(frame);
    frame_push(frame, v1);
    frame_push(frame, v2);
    frame_push(frame, v1);
}
void op_dup_x2(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t v1 = frame_pop(frame);
    int32_t v2 = frame_pop(frame);
    int32_t v3 = frame_pop(frame);
    frame_push(frame, v1);
    frame_push(frame, v3);
    frame_push(frame, v2);
    frame_push(frame, v1);
}
void op_dup2(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t v1 = frame_pop(frame);
    int32_t v2 = frame_pop(frame);
    frame_push(frame, v2);
    frame_push(frame, v1);
    frame_push(frame, v2);
    frame_push(frame, v1);
}
void op_dup2_x1(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t v1 = frame_pop(frame);
    int32_t v2 = frame_pop(frame);
    int32_t v3 = frame_pop(frame);
    frame_push(frame, v2);
    frame_push(frame, v1);
    frame_push(frame, v3);
    frame_push(frame, v2);
    frame_push(frame, v1);
}
void op_dup2_x2(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t v1 = frame_pop(frame);
    int32_t v2 = frame_pop(frame);
    int32_t v3 = frame_pop(frame);
    int32_t v4 = frame_pop(frame);
    frame_push(frame, v2);
    frame_push(frame, v1);
    frame_push(frame, v4);
    frame_push(frame, v3);
    frame_push(frame, v2);
    frame_push(frame, v1);
}
void op_swap(JVM *jvm, Frame *frame) {
    (void)jvm;
    int32_t v1 = frame_pop(frame);
    int32_t v2 = frame_pop(frame);
    frame_push(frame, v1);
    frame_push(frame, v2);
}
