#include "class_reader.h"
#include "displayer.h"
#include "interpreter.h"
#include <cstdio>
#include <cstring>

/**
 * @brief Ponto de entrada da JVM.
 *
 * Modos de operacao:
 *  - Exibidor: jvm.exe -d <arquivo.class>
 *      Le o .class e exibe suas estruturas + bytecodes resolvidos.
 *  - Interpretador: jvm.exe <arquivo.class>
 *      Executa o metodo main da classe.
 *
 * @param argc  Numero de argumentos.
 * @param argv  Vetor de argumentos.
 * @return 0 em sucesso, 1 em erro.
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "Uso:\n"
                "  Exibidor:      %s -d <arquivo.class>\n"
                "  Interpretador: %s <arquivo.class>\n",
                argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-d") == 0) {
        /* Modo exibidor */
        if (argc < 3) {
            fprintf(stderr, "Uso: %s -d <arquivo.class>\n", argv[0]);
            return 1;
        }

        ClassFile *cf = NULL;
        JvmError err = read_class_file(argv[2], &cf);
        if (err != JVM_OK) {
            fprintf(stderr, "Erro ao ler '%s': codigo %d\n", argv[2], err);
            return 1;
        }

        display_class_file(cf);
        free_class_file(cf);

    } else {
        /* Modo interpretador */
        JVM *jvm = jvm_create(argv[1]);
        if (!jvm) return 1;

        jvm_run(jvm);
        jvm_destroy(jvm);
    }

    return 0;
}
