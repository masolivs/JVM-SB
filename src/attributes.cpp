#include "attributes.h"
#include "constant_pool.h"
#include <cstring>

static u2 buf_read_u2(const u1 *buf, u4 *pos) {
    u2 v = (u2)((buf[*pos] << 8) | buf[*pos + 1]);
    *pos += 2;
    return v;
}

static u4 buf_read_u4(const u1 *buf, u4 *pos) {
    u4 v = ((u4)buf[*pos] << 24) | ((u4)buf[*pos + 1] << 16) |
           ((u4)buf[*pos + 2] << 8) | (u4)buf[*pos + 3];
    *pos += 4;
    return v;
}

static bool attr_matches(const attribute_info &a, const cp_info *cp, const char *name) {
    u2 ni = a.name_index;
    return cp[ni].tag == CP_UTF8 &&
           strcmp(reinterpret_cast<const char *>(cp[ni].data.utf8.bytes), name) == 0;
}

Code_attribute *parse_code_attribute(const attribute_info *attrs, u2 count,
                                     const cp_info *cp) {
    for (u2 i = 0; i < count; i++) {
        if (!attr_matches(attrs[i], cp, "Code")) continue;

        const u1 *buf = attrs[i].info;
        u4 pos = 0;

        Code_attribute *ca = new Code_attribute();
        ca->max_stack  = buf_read_u2(buf, &pos);
        ca->max_locals = buf_read_u2(buf, &pos);

        ca->code_length = buf_read_u4(buf, &pos);
        ca->code = new u1[ca->code_length];
        memcpy(ca->code, buf + pos, ca->code_length);
        pos += ca->code_length;

        ca->exception_table_length = buf_read_u2(buf, &pos);
        if (ca->exception_table_length > 0) {
            ca->exception_table = new exception_entry[ca->exception_table_length];
            for (u2 j = 0; j < ca->exception_table_length; j++) {
                ca->exception_table[j].start_pc   = buf_read_u2(buf, &pos);
                ca->exception_table[j].end_pc     = buf_read_u2(buf, &pos);
                ca->exception_table[j].handler_pc = buf_read_u2(buf, &pos);
                ca->exception_table[j].catch_type = buf_read_u2(buf, &pos);
            }
        } else {
            ca->exception_table = NULL;
        }

        ca->attributes_count = buf_read_u2(buf, &pos);
        if (ca->attributes_count > 0) {
            ca->sub_attributes = new attribute_info[ca->attributes_count];
            for (u2 j = 0; j < ca->attributes_count; j++) {
                ca->sub_attributes[j].name_index = buf_read_u2(buf, &pos);
                ca->sub_attributes[j].length     = buf_read_u4(buf, &pos);
                u4 len = ca->sub_attributes[j].length;
                if (len > 0) {
                    ca->sub_attributes[j].info = new u1[len];
                    memcpy(ca->sub_attributes[j].info, buf + pos, len);
                    pos += len;
                } else {
                    ca->sub_attributes[j].info = NULL;
                }
            }
        } else {
            ca->sub_attributes = NULL;
        }

        return ca;
    }
    return NULL;
}

u2 *parse_linenumber_table(const attribute_info *attrs, u2 count,
                           const cp_info *cp, u2 *out_count) {
    *out_count = 0;
    if (!attrs) return NULL;
    for (u2 i = 0; i < count; i++) {
        if (!attr_matches(attrs[i], cp, "LineNumberTable")) continue;
        const u1 *buf = attrs[i].info;
        if (!buf) return NULL;
        u4 pos = 0;
        u2 len = buf_read_u2(buf, &pos);
        *out_count = len;
        if (len == 0) return NULL;
        u2 *table = new u2[len * 2];
        for (u2 j = 0; j < len; j++) {
            table[j * 2]     = buf_read_u2(buf, &pos);
            table[j * 2 + 1] = buf_read_u2(buf, &pos);
        }
        return table;
    }
    return NULL;
}

u2 *parse_exceptions_attribute(const attribute_info *attrs, u2 count,
                               const cp_info *cp, u2 *out_count) {
    *out_count = 0;
    if (!attrs) return NULL;
    for (u2 i = 0; i < count; i++) {
        if (!attr_matches(attrs[i], cp, "Exceptions")) continue;
        const u1 *buf = attrs[i].info;
        if (!buf) return NULL;
        u4 pos = 0;
        u2 num = buf_read_u2(buf, &pos);
        *out_count = num;
        if (num == 0) return NULL;
        u2 *indices = new u2[num];
        for (u2 j = 0; j < num; j++)
            indices[j] = buf_read_u2(buf, &pos);
        return indices;
    }
    return NULL;
}

u2 parse_constantvalue_attribute(const attribute_info *attrs, u2 count,
                                 const cp_info *cp) {
    if (!attrs) return 0;
    for (u2 i = 0; i < count; i++) {
        if (!attr_matches(attrs[i], cp, "ConstantValue")) continue;
        const u1 *buf = attrs[i].info;
        if (!buf || attrs[i].length < 2) return 0;
        return (u2)((buf[0] << 8) | buf[1]);
    }
    return 0;
}

u2 parse_sourcefile_attribute(const attribute_info *attrs, u2 count,
                              const cp_info *cp) {
    if (!attrs) return 0;
    for (u2 i = 0; i < count; i++) {
        if (!attr_matches(attrs[i], cp, "SourceFile")) continue;
        const u1 *buf = attrs[i].info;
        if (!buf || attrs[i].length < 2) return 0;
        return (u2)((buf[0] << 8) | buf[1]);
    }
    return 0;
}
