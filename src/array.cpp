#include "array.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

/**
 * @brief Retorna o tamanho em bytes de um elemento do tipo dado.
 *
 * @details Mapeia cada ArrayType para o seu tamanho conforme a especificacao JVM:
 *  - boolean, byte: 1 byte.
 *  - char, short:   2 bytes.
 *  - int, float:    4 bytes.
 *  - long, double:  8 bytes.
 *  - T_REF:         4 bytes (referencia armazenada como int32_t, indice na heap).
 *
 * Usado por array_new() para calcular o tamanho total do buffer e por
 * array_get()/array_set() para calcular o offset de cada elemento.
 *
 * @param type  Tipo do elemento do array.
 * @return Tamanho em bytes do elemento (1, 2, 4 ou 8).
 */
int element_size(ArrayType type) {
    switch (type) {
        case T_BOOLEAN: return 1;
        case T_BYTE:    return 1;
        case T_CHAR:    return 2;
        case T_SHORT:   return 2;
        case T_INT:     return 4;
        case T_FLOAT:   return 4;
        case T_LONG:    return 8;
        case T_DOUBLE:  return 8;
        case T_REF:     return 4; /* referencia como int32_t (indice na heap) */
        default:        return 4;
    }
}

/**
 * @brief Aloca e inicializa um novo JArray com todos os elementos zerados.
 *
 * @details Calcula o tamanho total do buffer como length * element_size(type)
 * e aloca com `new char[total]()` (zero-initialization garantida pelo ()
 * em C++). O campo elements aponta para esse buffer; arraylength e type
 * sao preenchidos com os valores fornecidos.
 *
 * @param type    Tipo dos elementos (define o tamanho por elemento).
 * @param length  Numero de elementos; deve ser >= 0.
 * @return Ponteiro para o JArray alocado.
 */
JArray *array_new(ArrayType type, int32_t length) {
    JArray *arr = new JArray();
    arr->type        = type;
    arr->arraylength = length;

    size_t total = (size_t)length * (size_t)element_size(type);
    arr->elements = new char[total]();  /* zero-initialized */
    return arr;
}

/**
 * @brief Libera um JArray alocado por array_new().
 *
 * @details Libera o buffer de elementos (alocado como char[]) e em seguida
 * a estrutura JArray em si. E seguro chamar com arr == NULL.
 *
 * @param arr  Array a destruir; pode ser NULL.
 */
void array_free(JArray *arr) {
    if (!arr) return;
    delete[] static_cast<char *>(arr->elements);
    delete arr;
}

/**
 * @brief Stub de verificacao de bounds (validacao feita em array_ops.cpp).
 *
 * @details Esta funcao e intencionalmente um no-op: a verificacao de limites
 * e realizada antes de chamar array_get/array_set pelos opcodes em
 * array_ops.cpp via jvm_throw(), que integra o mecanismo de excecoes da JVM.
 * O stub existe para documentar o ponto de extensao caso a verificacao
 * precise ser centralizada aqui no futuro.
 *
 * @param arr    Array a verificar (nao utilizado).
 * @param index  Indice a verificar (nao utilizado).
 */
static void check_bounds(const JArray *arr, int32_t index) {
    (void)arr; (void)index;
}

/**
 * @brief Le um elemento do array e retorna seu valor como int32_t.
 *
 * @details O algoritmo usa memcpy para leitura segura (evita problemas de
 * alinhamento) e despacha pelo tipo do array:
 *  - T_BOOLEAN/T_BYTE:  le 1 byte, interpreta como int8_t (sign-extend).
 *  - T_CHAR:            le 2 bytes como uint16_t (zero-extend, sem sinal).
 *  - T_SHORT:           le 2 bytes como int16_t (sign-extend).
 *  - T_INT/T_FLOAT/T_REF: le 4 bytes como int32_t.
 *  - T_LONG/T_DOUBLE:   le os 4 bytes da high word na posicao index*8;
 *                        o chamador deve ler index+1 para a low word
 *                        (convencao de 2 slots da JVM).
 *
 * @param arr    Array de origem; nao pode ser NULL.
 * @param index  Indice do elemento (bounds ja verificados pelo chamador).
 * @return Valor do elemento como int32_t.
 */
int32_t array_get(const JArray *arr, int32_t index) {
    check_bounds(arr, index);
    const char *base = static_cast<const char *>(arr->elements);
    switch (arr->type) {
        case T_BOOLEAN:
        case T_BYTE:
            return (int32_t)((int8_t)base[index]);
        case T_CHAR: {
            uint16_t v;
            memcpy(&v, base + index * 2, 2);
            return (int32_t)v;
        }
        case T_SHORT: {
            int16_t v;
            memcpy(&v, base + index * 2, 2);
            return (int32_t)v;
        }
        case T_INT:
        case T_FLOAT:
        case T_REF: {
            int32_t v;
            memcpy(&v, base + index * 4, 4);
            return v;
        }
        /* Long/Double: retorna word high; caller usa index+1 para low */
        case T_LONG:
        case T_DOUBLE: {
            int32_t v;
            memcpy(&v, base + index * 8, 4);
            return v;
        }
        default:
            return 0;
    }
}

/**
 * @brief Escreve um valor int32_t em uma posicao do array.
 *
 * @details Simetrico a array_get: usa memcpy para escrita segura e despacha
 * pelo tipo para determinar quantos bytes escrever e em qual offset:
 *  - T_BOOLEAN/T_BYTE:  escreve 1 byte (trunca para int8_t).
 *  - T_CHAR:            escreve 2 bytes como uint16_t.
 *  - T_SHORT:           escreve 2 bytes como int16_t.
 *  - T_INT/T_FLOAT/T_REF: escreve 4 bytes.
 *  - T_LONG/T_DOUBLE:   escreve 4 bytes na posicao index*8 (high word);
 *                        o chamador deve chamar array_set com index+1
 *                        para a low word (ou usar op_lastore/op_dastore
 *                        que escrevem os 8 bytes diretamente via memcpy).
 *
 * @param arr    Array de destino; nao pode ser NULL.
 * @param index  Indice do elemento (bounds ja verificados pelo chamador).
 * @param value  Valor a armazenar como int32_t.
 */
void array_set(JArray *arr, int32_t index, int32_t value) {
    check_bounds(arr, index);
    char *base = static_cast<char *>(arr->elements);
    switch (arr->type) {
        case T_BOOLEAN:
        case T_BYTE:
            base[index] = (char)(int8_t)value;
            break;
        case T_CHAR: {
            uint16_t v = (uint16_t)value;
            memcpy(base + index * 2, &v, 2);
            break;
        }
        case T_SHORT: {
            int16_t v = (int16_t)value;
            memcpy(base + index * 2, &v, 2);
            break;
        }
        case T_INT:
        case T_FLOAT:
        case T_REF:
            memcpy(base + index * 4, &value, 4);
            break;
        /* Long/Double: word high no index*8; caller passa word high */
        case T_LONG:
        case T_DOUBLE:
            memcpy(base + index * 8, &value, 4);
            break;
        default:
            break;
    }
}