#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <cstdint>

struct ClassEntry;

/**
 * @brief Representa um objeto Java alocado no heap.
 *
 * Campos de instancia sao armazenados em 'fields' como slots int32_t.
 * Longs e doubles ocupam dois slots consecutivos.
 * Referencias sao indices int32_t na heap table da JVM.
 */
struct JObject {
    ClassEntry           *klass;  /**< tipo do objeto */
    std::vector<int32_t>  fields; /**< campos de instancia (inicializados em 0) */
};

/**
 * @brief Aloca um novo JObject para a classe dada.
 *
 * O numero de campos e determinado pelo count de fields da ClassFile.
 * Todos os campos sao inicializados em 0.
 *
 * @param klass  ClassEntry da classe a instanciar.
 * @return Ponteiro para o JObject alocado.
 */
JObject *object_new(ClassEntry *klass);

/**
 * @brief Libera um JObject alocado por object_new.
 *
 * @param obj  Objeto a destruir; pode ser NULL.
 */
void object_free(JObject *obj);

/**
 * @brief Retorna o valor de um campo de instancia.
 *
 * @param obj    Objeto alvo.
 * @param index  Indice do campo em fields[].
 * @return Valor do campo.
 */
int32_t object_get_field(const JObject *obj, int index);

/**
 * @brief Escreve o valor de um campo de instancia.
 *
 * @param obj    Objeto alvo.
 * @param index  Indice do campo em fields[].
 * @param value  Novo valor.
 */
void object_set_field(JObject *obj, int index, int32_t value);

#endif /* OBJECT_H */
