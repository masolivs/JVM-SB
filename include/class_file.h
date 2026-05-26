#ifndef CLASS_FILE_H
#define CLASS_FILE_H

#include "types.h"

#define CP_UTF8                1
#define CP_INTEGER             3
#define CP_FLOAT               4
#define CP_LONG                5
#define CP_DOUBLE              6
#define CP_CLASS               7
#define CP_STRING              8
#define CP_FIELDREF            9
#define CP_METHODREF           10
#define CP_INTERFACE_METHODREF 11
#define CP_NAME_AND_TYPE       12

#define ACC_PUBLIC     0x0001
#define ACC_FINAL      0x0010
#define ACC_SUPER      0x0020
#define ACC_INTERFACE  0x0200
#define ACC_ABSTRACT   0x0400
#define ACC_SYNTHETIC  0x1000
#define ACC_ANNOTATION 0x2000
#define ACC_ENUM       0x4000

#define ACC_PRIVATE      0x0002
#define ACC_PROTECTED    0x0004
#define ACC_STATIC       0x0008
#define ACC_VOLATILE     0x0040
#define ACC_TRANSIENT    0x0080
#define ACC_NATIVE       0x0100
#define ACC_SYNCHRONIZED 0x0020
#define ACC_STRICT       0x0800
#define ACC_BRIDGE       0x0040
#define ACC_VARARGS      0x0080

typedef union {
    struct {
        u2  length;
        u1 *bytes;
    } utf8;
    struct { u4 bytes; } integer_val;
    struct { u4 bytes; } float_val;
    struct { u4 high_bytes; u4 low_bytes; } long_val;
    struct { u4 high_bytes; u4 low_bytes; } double_val;
    struct { u2 name_index; } class_info;
    struct { u2 string_index; } string_info;
    struct {
        u2 class_index;
        u2 name_and_type_index;
    } ref;
    struct {
        u2 name_index;
        u2 descriptor_index;
    } name_and_type;
} CpData;

typedef struct {
    u1     tag;
    CpData data;
} cp_info;

typedef struct {
    u2  name_index;
    u4  length;
    u1 *info;
} attribute_info;

typedef struct {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
} exception_entry;

typedef struct {
    u2              max_stack;
    u2              max_locals;
    u4              code_length;
    u1             *code;
    u2              exception_table_length;
    exception_entry *exception_table;
    u2              attributes_count;
    attribute_info *sub_attributes;
} Code_attribute;

typedef struct {
    u2              access_flags;
    u2              name_index;
    u2              descriptor_index;
    u2              attributes_count;
    attribute_info *attributes;
} field_info;

typedef struct {
    u2              access_flags;
    u2              name_index;
    u2              descriptor_index;
    u2              attributes_count;
    attribute_info *attributes;
    Code_attribute *code_attr;
} method_info;

typedef struct {
    u4          magic;
    u2          minor_version;
    u2          major_version;
    u2          constant_pool_count;
    cp_info    *constant_pool;
    u2          access_flags;
    u2          this_class;
    u2          super_class;
    u2          interfaces_count;
    u2         *interfaces;
    u2          fields_count;
    field_info *fields;
    u2          methods_count;
    method_info*methods;
    u2          attributes_count;
    attribute_info *attributes;
} ClassFile;

#endif /* CLASS_FILE_H */
