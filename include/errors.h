#ifndef ERRORS_H
#define ERRORS_H

/**
 * @file errors.h
 * @brief Codigos de erro da JVM.
 */

/**
 * @enum JvmError
 * @brief Possiveis estados de erro durante parsing ou execucao.
 *
 * @var JVM_OK
 * Sem erro; operacao bem-sucedida.
 *
 * @var ERR_INVALID_MAGIC
 * Arquivo nao comeca com 0xCAFEBABE (magic number invalido).
 *
 * @var ERR_UNSUPPORTED_VERSION
 * Versao do .class nao suportada (acima de Java 8).
 *
 * @var ERR_CP_INVALID_TAG
 * Tag desconhecida no constant pool.
 *
 * @var ERR_OUT_OF_MEMORY
 * Falha de alocacao (malloc/new retornou NULL).
 *
 * @var ERR_CLASS_NOT_FOUND
 * Classe nao carregada na MethodArea.
 *
 * @var ERR_METHOD_NOT_FOUND
 * Metodo nao encontrado na hierarquia de classes.
 *
 * @var ERR_STACK_OVERFLOW
 * Pilha de frames estourou o limite maximo.
 *
 * @var ERR_NULL_POINTER
 * Referencia nula onde um objeto era esperado.
 *
 * @var ERR_ARRAY_BOUNDS
 * Indice fora dos limites do array.
 *
 * @var ERR_UNIMPLEMENTED_OPCODE
 * Opcode ainda nao implementado pelo interpretador.
 */
typedef enum {
    JVM_OK = 0,
    ERR_INVALID_MAGIC,
    ERR_UNSUPPORTED_VERSION,
    ERR_CP_INVALID_TAG,
    ERR_OUT_OF_MEMORY,
    ERR_CLASS_NOT_FOUND,
    ERR_METHOD_NOT_FOUND,
    ERR_STACK_OVERFLOW,
    ERR_NULL_POINTER,
    ERR_ARRAY_BOUNDS,
    ERR_UNIMPLEMENTED_OPCODE,
} JvmError;

#endif
