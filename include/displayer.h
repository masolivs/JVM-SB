#ifndef DISPLAYER_H
#define DISPLAYER_H

/**
 * @file displayer.h
 * @brief Interface de exibicao formatada de arquivos .class.
 *
 * Modo exibidor da CLI (flag -d). Toda saida e escrita em stdout.
 */

#include "class_file.h"

/**
 * @brief Exibe estrutura completa de um arquivo .class em formato legivel.
 *
 * Imprime na ordem exigida:
 *  1. Cabecalho (magic, minor/major version, SourceFile)
 *  2. Constant Pool com indices, tags e valores resolvidos
 *  3. Access flags legiveis (public, super, final...)
 *  4. This class e Super class resolvidos
 *  5. Interfaces
 *  6. Fields com flags e atributos
 *  7. Methods com flags, bytecodes e mnemonicos resolvidos
 *
 * @param cf  ClassFile carregado por read_class_file().
 *
 * @details A saida e enviada para stdout via printf().
 *
 * @see read_class_file()
 * @see display_bytecodes()
 *
 * @code
 * ClassFile *cf = NULL;
 * if (read_class_file("build/HelloWorld.class", &cf) == JVM_OK)
 *     display_class_file(cf);
 * @endcode
 */
void display_class_file(const ClassFile *cf);

/**
 * @brief Exibe disassembly de bytecode de um metodo.
 *
 * Converte bytes de bytecode em mnemonicos JVM legiveis, com operandos
 * resolvidos a partir do constant pool.
 *
 * @param cf    ClassFile (necessario para resolver CP).
 * @param code  Atributo Code do metodo.
 *
 * @details Operandos que sao indices no CP sao impressos com o valor
 * resolvido. tableswitch e lookupswitch exibem a estrutura completa.
 * Offsets relativos sao apresentados como PC (program counter).
 *
 * @see display_class_file()
 * @see mnemonic
 */
void display_bytecodes(const ClassFile *cf, const Code_attribute *code);

#endif /* DISPLAYER_H */
