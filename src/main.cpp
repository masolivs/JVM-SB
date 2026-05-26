#include "class_reader.h"
#include "displayer.h"
#include <cstdio>
#include <cstring>

static void print_usage(const char *prog) {
    fprintf(stderr, "Uso: %s [-o <saida.txt>] <arquivo.class>\n", prog);
}

int main(int argc, char **argv) {
    const char *output_file = NULL;
    const char *class_file  = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -o requer um nome de arquivo\n");
                print_usage(argv[0]);
                return 1;
            }
            output_file = argv[++i];
        } else {
            class_file = argv[i];
        }
    }

    if (!class_file) {
        print_usage(argv[0]);
        return 1;
    }

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
    return 0;
}
