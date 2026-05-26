#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "class_file.h"

Code_attribute *parse_code_attribute(const attribute_info *attrs, u2 count,
                                     const cp_info *cp);

u2 *parse_linenumber_table(const attribute_info *attrs, u2 count,
                           const cp_info *cp, u2 *out_count);

u2 *parse_exceptions_attribute(const attribute_info *attrs, u2 count,
                               const cp_info *cp, u2 *out_count);

u2  parse_constantvalue_attribute(const attribute_info *attrs, u2 count,
                                  const cp_info *cp);

u2  parse_sourcefile_attribute(const attribute_info *attrs, u2 count,
                               const cp_info *cp);

#endif /* ATTRIBUTES_H */
