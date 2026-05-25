#include "class_reader.h"
#include "displayer.h"
#include <cstdio>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.class>\n", argv[0]);
        return 1;
    }

    ClassFile *cf = NULL;
    JvmError err = read_class_file(argv[1], &cf);
    if (err != JVM_OK) {
        fprintf(stderr, "Erro ao ler '%s': codigo %d\n", argv[1], err);
        return 1;
    }

    display_class_file(cf);
    free_class_file(cf);
    return 0;
}
