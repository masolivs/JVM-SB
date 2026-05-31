#include "object.h"
#include "method_area.h"

JObject *object_new(ClassEntry *klass) {
    JObject *obj = new JObject();
    obj->klass = klass;
    /* Conta campos de toda a hierarquia: superclasse ocupa indices menores */
    obj->fields.assign((size_t)count_hierarchy_fields(klass), 0);
    return obj;
}

void object_free(JObject *obj) {
    delete obj;
}

int32_t object_get_field(const JObject *obj, int index) {
    return obj->fields[(size_t)index];
}

void object_set_field(JObject *obj, int index, int32_t value) {
    obj->fields[(size_t)index] = value;
}
