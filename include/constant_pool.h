#ifndef CONSTANT_POOL_H
#define CONSTANT_POOL_H

#include "class_file.h"
#include <string>

std::string resolve_utf8(const ClassFile *cf, u2 index);
std::string resolve_class_name(const ClassFile *cf, u2 index);
std::string resolve_string(const ClassFile *cf, u2 index);
std::string resolve_methodref(const ClassFile *cf, u2 index);
std::string resolve_fieldref(const ClassFile *cf, u2 index);
std::string resolve_nameandtype(const ClassFile *cf, u2 index);
std::string resolve_cp_value(const ClassFile *cf, u2 index);

#endif /* CONSTANT_POOL_H */
