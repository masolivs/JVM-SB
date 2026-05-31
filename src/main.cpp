#include "class_reader.h"
#include "displayer.h"
#include "interpreter.h"
#include <cstdio>
#include <cstring>

/**
 * @brief Ponto de entrada da JVM.
 *
 * Modos de operacao:
 *  - Exibidor:      jvm.exe -d [-o saida.txt] <arquivo.class>
 *      Le o .class e exibe suas estruturas + bytecodes resolvidos.
 *      Com -o, redireciona a saida para o arquivo indicado em vez de stdout.
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
                "  Exibidor:      %s -d [-o saida.txt] <arquivo.class>\n"
                "  Interpretador: %s <arquivo.class>\n",
                argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-d") == 0) {
        /* Modo exibidor */
        if (argc < 3) {
            fprintf(stderr, "Uso: %s -d [-o saida.txt] <arquivo.class>\n", argv[0]);
            return 1;
        }

        const char *output_file = NULL;
        const char *class_file  = NULL;

        /* Parseia argumentos apos -d */
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Erro: -o requer um nome de arquivo\n");
                    return 1;
                }
                output_file = argv[++i];
            } else {
                class_file = argv[i];
            }
        }

        if (!class_file) {
            fprintf(stderr, "Uso: %s -d [-o saida.txt] <arquivo.class>\n", argv[0]);
            return 1;
        }

        /* Redireciona stdout para arquivo se -o foi especificado */
        if (output_file) {
            if (!freopen(output_file, "w", stdout)) {
                fprintf(stderr, "Erro: nao foi possivel abrir '%s' para escrita\n", output_file);
                return 1;
            }
        }

        ClassFile *cf = NULL;
        JvmError err = read_class_file(class_file, &cf);
        if (err != JVM_OK) {
            fprintf(stderr, "Erro ao ler '%s': codigo %d\n", class_file, err);
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