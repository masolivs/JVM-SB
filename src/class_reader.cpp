#include "class_reader.h"
#include "attributes.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static u1 read_u1(FILE *f) {
    u1 b = 0;
    fread(&b, 1, 1, f);
    return b;
}

static u2 read_u2(FILE *f) {
    u1 b[2];
    fread(b, 1, 2, f);
    return (u2)((b[0] << 8) | b[1]);
}

static u4 read_u4(FILE *f) {
    u1 b[4];
    fread(b, 1, 4, f);
    return ((u4)b[0] << 24) | ((u4)b[1] << 16) | ((u4)b[2] << 8) | b[3];
}

static JvmError read_cp_entry(FILE *f, cp_info *entry, bool *skip) {
    *skip = false;
    entry->tag = read_u1(f);

    switch (entry->tag) {
        case CP_UTF8: {
            u2 len = read_u2(f);
            entry->data.utf8.length = len;
            entry->data.utf8.bytes  = new u1[len + 1];
            fread(entry->data.utf8.bytes, 1, len, f);
            entry->data.utf8.bytes[len] = 0;
            break;
        }
        case CP_INTEGER:
            entry->data.integer_val.bytes = read_u4(f);
            break;
        case CP_FLOAT:
            entry->data.float_val.bytes = read_u4(f);
            break;
        case CP_LONG:
            entry->data.long_val.high_bytes = read_u4(f);
            entry->data.long_val.low_bytes  = read_u4(f);
            *skip = true;
            break;
        case CP_DOUBLE:
            entry->data.double_val.high_bytes = read_u4(f);
            entry->data.double_val.low_bytes  = read_u4(f);
            *skip = true;
            break;
        case CP_CLASS:
            entry->data.class_info.name_index = read_u2(f);
            break;
        case CP_STRING:
            entry->data.string_info.string_index = read_u2(f);
            break;
        case CP_FIELDREF:
        case CP_METHODREF:
        case CP_INTERFACE_METHODREF:
            entry->data.ref.class_index         = read_u2(f);
            entry->data.ref.name_and_type_index = read_u2(f);
            break;
        case CP_NAME_AND_TYPE:
            entry->data.name_and_type.name_index       = read_u2(f);
            entry->data.name_and_type.descriptor_index = read_u2(f);
            break;
        default:
            return ERR_CP_INVALID_TAG;
    }
    return JVM_OK;
}

static void read_attributes(FILE *f, u2 count, attribute_info *attrs) {
    for (u2 i = 0; i < count; i++) {
        attrs[i].name_index = read_u2(f);
        attrs[i].length     = read_u4(f);
        if (attrs[i].length > 0) {
            attrs[i].info = new u1[attrs[i].length];
            fread(attrs[i].info, 1, attrs[i].length, f);
        } else {
            attrs[i].info = NULL;
        }
    }
}

static void read_fields(FILE *f, u2 count, field_info *fields) {
    for (u2 i = 0; i < count; i++) {
        fields[i].access_flags     = read_u2(f);
        fields[i].name_index       = read_u2(f);
        fields[i].descriptor_index = read_u2(f);
        fields[i].attributes_count = read_u2(f);
        if (fields[i].attributes_count > 0) {
            fields[i].attributes = new attribute_info[fields[i].attributes_count];
            read_attributes(f, fields[i].attributes_count, fields[i].attributes);
        } else {
            fields[i].attributes = NULL;
        }
    }
}

static void read_methods(FILE *f, u2 count, method_info *methods, cp_info *cp) {
    for (u2 i = 0; i < count; i++) {
        methods[i].access_flags     = read_u2(f);
        methods[i].name_index       = read_u2(f);
        methods[i].descriptor_index = read_u2(f);
        methods[i].attributes_count = read_u2(f);
        methods[i].code_attr        = NULL;

        if (methods[i].attributes_count > 0) {
            methods[i].attributes = new attribute_info[methods[i].attributes_count];
            read_attributes(f, methods[i].attributes_count, methods[i].attributes);
            methods[i].code_attr = parse_code_attribute(
                methods[i].attributes,
                methods[i].attributes_count,
                cp);
        } else {
            methods[i].attributes = NULL;
        }
    }
}

JvmError read_class_file(const char *path, ClassFile **out) {
    *out = NULL;

    FILE *f = fopen(path, "rb");
    if (!f) return ERR_CLASS_NOT_FOUND;

    ClassFile *cf = new ClassFile();
    memset(cf, 0, sizeof(*cf));

    cf->magic = read_u4(f);
    if (cf->magic != 0xCAFEBABE) {
        fclose(f);
        delete cf;
        return ERR_INVALID_MAGIC;
    }

    cf->minor_version = read_u2(f);
    cf->major_version = read_u2(f);

    cf->constant_pool_count = read_u2(f);
    cf->constant_pool = new cp_info[cf->constant_pool_count]();

    for (u2 i = 1; i < cf->constant_pool_count; i++) {
        bool skip = false;
        JvmError err = read_cp_entry(f, &cf->constant_pool[i], &skip);
        if (err != JVM_OK) {
            fclose(f);
            free_class_file(cf);
            return err;
        }
        if (skip) i++;  /* Long/Double ocupam dois slots */
    }

    cf->access_flags = read_u2(f);
    cf->this_class   = read_u2(f);
    cf->super_class  = read_u2(f);

    cf->interfaces_count = read_u2(f);
    if (cf->interfaces_count > 0) {
        cf->interfaces = new u2[cf->interfaces_count];
        for (u2 i = 0; i < cf->interfaces_count; i++)
            cf->interfaces[i] = read_u2(f);
    }

    cf->fields_count = read_u2(f);
    if (cf->fields_count > 0) {
        cf->fields = new field_info[cf->fields_count]();
        read_fields(f, cf->fields_count, cf->fields);
    }

    cf->methods_count = read_u2(f);
    if (cf->methods_count > 0) {
        cf->methods = new method_info[cf->methods_count]();
        read_methods(f, cf->methods_count, cf->methods, cf->constant_pool);
    }

    cf->attributes_count = read_u2(f);
    if (cf->attributes_count > 0) {
        cf->attributes = new attribute_info[cf->attributes_count];
        read_attributes(f, cf->attributes_count, cf->attributes);
    }

    fclose(f);
    *out = cf;
    return JVM_OK;
}

void free_class_file(ClassFile *cf) {
    if (!cf) return;

    if (cf->constant_pool) {
        for (u2 i = 1; i < cf->constant_pool_count; i++)
            if (cf->constant_pool[i].tag == CP_UTF8)
                delete[] cf->constant_pool[i].data.utf8.bytes;
        delete[] cf->constant_pool;
    }

    delete[] cf->interfaces;

    if (cf->fields) {
        for (u2 i = 0; i < cf->fields_count; i++) {
            if (cf->fields[i].attributes) {
                for (u2 j = 0; j < cf->fields[i].attributes_count; j++)
                    delete[] cf->fields[i].attributes[j].info;
                delete[] cf->fields[i].attributes;
            }
        }
        delete[] cf->fields;
    }

    if (cf->methods) {
        for (u2 i = 0; i < cf->methods_count; i++) {
            if (cf->methods[i].code_attr) {
                delete[] cf->methods[i].code_attr->code;
                delete[] cf->methods[i].code_attr->exception_table;
                if (cf->methods[i].code_attr->sub_attributes) {
                    for (u2 j = 0; j < cf->methods[i].code_attr->attributes_count; j++)
                        delete[] cf->methods[i].code_attr->sub_attributes[j].info;
                    delete[] cf->methods[i].code_attr->sub_attributes;
                }
                delete cf->methods[i].code_attr;
            }
            if (cf->methods[i].attributes) {
                for (u2 j = 0; j < cf->methods[i].attributes_count; j++)
                    delete[] cf->methods[i].attributes[j].info;
                delete[] cf->methods[i].attributes;
            }
        }
        delete[] cf->methods;
    }

    if (cf->attributes) {
        for (u2 i = 0; i < cf->attributes_count; i++)
            delete[] cf->attributes[i].info;
        delete[] cf->attributes;
    }

    delete cf;
}
