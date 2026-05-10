#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    JVM_OK = 0,
    ERR_INVALID_MAGIC,        // arquivo não começa com 0xCAFEBABE
    ERR_UNSUPPORTED_VERSION,  // versão do .class não suportada
    ERR_CP_INVALID_TAG,       // tag desconhecida no constant pool
    ERR_OUT_OF_MEMORY,        // malloc retornou NULL
    ERR_CLASS_NOT_FOUND,      // classe não carregada
    ERR_METHOD_NOT_FOUND,     // método não encontrado
    ERR_STACK_OVERFLOW,       // pilha de frames estourou
    ERR_NULL_POINTER,         // referência nula
    ERR_ARRAY_BOUNDS,         // índice fora do array
    ERR_UNIMPLEMENTED_OPCODE, // opcode não implementado ainda
} JvmError;

#endif