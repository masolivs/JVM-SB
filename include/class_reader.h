#ifndef CLASS_READER_H
#define CLASS_READER_H

#include "class_file.h"
#include "errors.h"

JvmError read_class_file(const char *path, ClassFile **out);
void     free_class_file(ClassFile *cf);

#endif /* CLASS_READER_H */
