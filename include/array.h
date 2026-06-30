#ifndef ARRAY_H
#define ARRAY_H

#include <cstdint>

/**
 * @brief Tipos de array primitivo conforme especificacao JVM.
 *
 * Valores correspondem ao operando do opcode newarray.
 */
typedef enum {
    T_BOOLEAN = 4,
    T_CHAR    = 5,
    T_FLOAT   = 6,
    T_DOUBLE  = 7,
    T_BYTE    = 8,
    T_SHORT   = 9,
    T_INT     = 10,
    T_LONG    = 11,
    T_REF     = 12  /**< array de referencias (anewarray) */
} ArrayType;

/**
 * @brief Representa um array Java alocado no heap.
 *
 * arraylength e um CAMPO DO STRUCT — o opcode arraylength (0xBE)
 * le diretamente arr->arraylength, nao calcula tamanho.
 * elements aponta para bloco alocado com new[].
 */
struct JArray {
    ArrayType  type;        /**< tipo dos elementos */
    int32_t    arraylength; /**< numero de elementos (obrigatorio pelo spec) */
    void      *elements;    /**< bloco de elementos alocado com new[] */
};

/**
 * @brief Cria um novo JArray do tipo e tamanho dados.
 *
 * Aloca elements com new[] do tamanho correto e zera tudo.
 *
 * @param type    Tipo primitivo ou T_REF.
 * @param length  Numero de elementos.
 * @return Ponteiro para o JArray alocado.
 *
 * @see array_free()
 * @see element_size()
 *
 * @code
 * JArray *arr = array_new(T_INT, 10);   // int[10], zerado
 * array_set(arr, 0, 42);
 * int32_t v = array_get(arr, 0);        // 42
 * array_free(arr);
 * @endcode
 */
JArray *array_new(ArrayType type, int32_t length);

/**
 * @brief Libera um JArray alocado por array_new.
 *
 * @param arr  Array a destruir; pode ser NULL.
 *
 * @see array_new()
 */
void array_free(JArray *arr);

/**
 * @brief Le um elemento do array como int32_t.
 *
 * Para long/double, usa os dois elementos adjacentes.
 *
 * @param arr    Array alvo; nao pode ser NULL.
 * @param index  Indice do elemento.
 * @return Valor do elemento convertido para int32.
 *
 * @warning Esta funcao NAO verifica limites (index < 0 ou index >=
 * arraylength). A verificacao e responsabilidade do chamador e e feita
 * pelos opcodes xaload em array_ops.cpp via macro CHECK_BOUNDS, que
 * dispara ArrayIndexOutOfBoundsException antes de invocar array_get().
 * Chamar diretamente com indice invalido e comportamento indefinido.
 *
 * @see array_set()
 * @see element_size()
 */
int32_t array_get(const JArray *arr, int32_t index);

/**
 * @brief Escreve um elemento no array como int32_t.
 *
 * @param arr    Array alvo; nao pode ser NULL.
 * @param index  Indice do elemento.
 * @param value  Valor a armazenar.
 *
 * @warning Assim como array_get(), esta funcao NAO verifica limites.
 * A validacao acontece no chamador (array_ops.cpp, macro CHECK_BOUNDS)
 * antes de array_set() ser invocada.
 *
 * @see array_get()
 */
void array_set(JArray *arr, int32_t index, int32_t value);

/**
 * @brief Retorna o tamanho em bytes de cada elemento para o tipo dado.
 *
 * @param type  Tipo do array.
 * @return Bytes por elemento.
 */
int element_size(ArrayType type);

#endif /* ARRAY_H */