#ifndef DISPLAYER_H
#define DISPLAYER_H

#include "class_file.h"

/**
 * @brief Exibe todas as informacoes do ClassFile no stdout.
 *
 * Imprime na ordem exigida pelo professor:
 *  1. Cabecalho (magic, minor/major version)
 *  2. Constant Pool com indices, tags e valores resolvidos
 *  3. Access flags legíveis (public, super, final...)
 *  4. This class e Super class resolvidos
 *  5. Interfaces
 *  6. Fields com flags e atributos
 *  7. Methods com flags e bytecodes + mnemônicos resolvidos
 *
 * @param cf  ClassFile carregado por read_class_file().
 */
void display_class_file(const ClassFile *cf);

/**
 * @brief Exibe os bytecodes de um metodo com offsets e mnemônicos.
 *
 * Operandos que sao indices no CP sao impressos com o valor resolvido
 * apos "//". Ex: "  3: ldc  #3  // \"Hello, World!\""
 *
 * @param cf    ClassFile (necessario para resolver CP).
 * @param code  Atributo Code do metodo.
 */
void display_bytecodes(const ClassFile *cf, const Code_attribute *code);

#endif /* DISPLAYER_H */
