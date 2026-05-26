#ifndef DISPLAYER_H
#define DISPLAYER_H

#include "class_file.h"

void display_class_file(const ClassFile *cf);
void display_bytecodes(const ClassFile *cf, const Code_attribute *code);

#endif /* DISPLAYER_H */
