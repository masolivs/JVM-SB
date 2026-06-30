#include "object.h"
#include "method_area.h"

/**
 * @brief Cria uma nova instancia de classe no heap.
 *
 * @details Aloca JObject, associa klass e inicializa o vetor fields
 * com zeros (padrao Java). O tamanho e determinado por
 * count_hierarchy_fields(), incluindo campos herdados de superclasses
 * (indices menores = superclasse).
 *
 * @see object_free()
 * @see count_hierarchy_fields()
 */
JObject *object_new(ClassEntry *klass) {
    JObject *obj = new JObject();
    obj->klass = klass;
    /* Conta campos de toda a hierarquia: superclasse ocupa indices menores */
    obj->fields.assign((size_t)count_hierarchy_fields(klass), 0);
    return obj;
}

/**
 * @brief Libera memoria de uma instancia de objeto.
 *
 * @details Chama delete sobre o JObject. Seguro com obj == NULL.
 *
 * @see object_new()
 */
void object_free(JObject *obj) {
    delete obj;
}

/**
 * @brief Obtem valor de um campo de instancia.
 *
 * @details Acesso direto a fields[index]. O indice e global na hierarquia
 * (ver find_field_index()).
 *
 * @see object_set_field()
 * @see find_field_index()
 */
int32_t object_get_field(const JObject *obj, int index) {
    return obj->fields[(size_t)index];
}

/**
 * @brief Define valor de um campo de instancia.
 *
 * @details Escrita direta em fields[index].
 *
 * @see object_get_field()
 */
void object_set_field(JObject *obj, int index, int32_t value) {
    obj->fields[(size_t)index] = value;
}
