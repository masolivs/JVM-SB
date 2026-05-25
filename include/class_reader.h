#ifndef CLASS_READER_H
#define CLASS_READER_H

#include "class_file.h"
#include "errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Le e valida um arquivo .class do disco.
 *
 * Abre o arquivo binario, valida o magic number 0xCAFEBABE,
 * le todo o constant pool, campos, metodos e atributos.
 * Todos os arrays internos sao alocados com new[].
 *
 * @param path  Caminho para o arquivo .class.
 * @param out   Ponteiro que recebers o ClassFile alocado; NULL em erro.
 * @return JVM_OK ou codigo de erro JvmError.
 */
JvmError read_class_file(const char *path, ClassFile **out);

/**
 * @brief Libera toda a memoria alocada por read_class_file.
 *
 * @param cf  ClassFile retornado por read_class_file; pode ser NULL.
 */
void free_class_file(ClassFile *cf);

#ifdef __cplusplus
}
#endif

#endif /* CLASS_READER_H */
