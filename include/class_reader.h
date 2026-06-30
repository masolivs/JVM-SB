#ifndef CLASS_READER_H
#define CLASS_READER_H

/**
 * @file class_reader.h
 * @brief Leitura e validacao de arquivos .class do disco.
 *
 * Implementa o parsing binario do formato .class conforme JVMS 8,
 * com leitura big-endian e alocacao dinamica dos arrays internos.
 */

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
 *
 * @param path  Caminho para o arquivo .class.
 * @param out   Ponteiro que recebe o ClassFile alocado; permanece NULL em erro.
 * @return JVM_OK ou codigo de erro JvmError.
 *
 * @details Todos os arrays internos do ClassFile sao alocados com new[].
 * A leitura segue o formato big-endian definido na especificacao JVM.
 *
 * @warning O ponteiro em *out deve ser liberado com free_class_file().
 *
 * @see free_class_file()
 * @see JvmError
 * @see ClassFile
 *
 * @code
 * ClassFile *cf = NULL;
 * JvmError err = read_class_file("build/HelloWorld.class", &cf);
 * if (err == JVM_OK) {
 *     // usar cf...
 *     free_class_file(cf);
 * }
 * @endcode
 */
JvmError read_class_file(const char *path, ClassFile **out);

/**
 * @brief Libera toda a memoria alocada por read_class_file.
 *
 * @param cf  ClassFile retornado por read_class_file; pode ser NULL.
 *
 * @see read_class_file()
 */
void free_class_file(ClassFile *cf);

#ifdef __cplusplus
}
#endif

#endif /* CLASS_READER_H */
