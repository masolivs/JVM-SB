#include "attributes.h"
#include "constant_pool.h"
#include <cstring>

/* Leitura big-endian de um buffer em memoria */
static u2 buf_read_u2(const u1 *buf, u4 *pos) {
    u2 v = (u2)((buf[*pos] << 8) | buf[*pos + 1]);
    *pos += 2;
    return v;
}

static u4 buf_read_u4(const u1 *buf, u4 *pos) {
    u4 v = ((u4)buf[*pos]     << 24) |
           ((u4)buf[*pos + 1] << 16) |
           ((u4)buf[*pos + 2] <<  8) |
            (u4)buf[*pos + 3];
    *pos += 4;
    return v;
}

Code_attribute *parse_code_attribute(const attribute_info *attrs,
                                     u2                    count,
                                     const cp_info        *cp) {
    for (u2 i = 0; i < count; i++) {
        /* Verifica pelo nome "Code" via CP */
        u2 ni = attrs[i].name_index;
        if (cp[ni].tag != CP_UTF8) continue;
        const char *name = reinterpret_cast<const char *>(cp[ni].data.utf8.bytes);
        if (strcmp(name, "Code") != 0) continue;

        /* Parseia os bytes do atributo */
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

        return ca;
    }
    return NULL;
}

u2 *parse_linenumber_table(const attribute_info *attr, u2 *out_count) {
    if (!attr || !attr->info) { *out_count = 0; return NULL; }

    const u1 *buf = attr->info;
    u4 pos = 0;

    u2 len = buf_read_u2(buf, &pos); /* line_number_table_length */
    *out_count = len;
    if (len == 0) return NULL;

    /* Retorna pares {start_pc, line_number} como array plano de u2 */
    u2 *table = new u2[len * 2];
    for (u2 i = 0; i < len; i++) {
        table[i * 2]     = buf_read_u2(buf, &pos); /* start_pc */
        table[i * 2 + 1] = buf_read_u2(buf, &pos); /* line_number */
    }
    return table;
}

u2 *parse_exceptions_attribute(const attribute_info *attr, u2 *out_count) {
    if (!attr || !attr->info) { *out_count = 0; return NULL; }

    const u1 *buf = attr->info;
    u4 pos = 0;

    u2 num = buf_read_u2(buf, &pos);
    *out_count = num;
    if (num == 0) return NULL;

    u2 *indices = new u2[num];
    for (u2 i = 0; i < num; i++)
        indices[i] = buf_read_u2(buf, &pos);
    return indices;
}
