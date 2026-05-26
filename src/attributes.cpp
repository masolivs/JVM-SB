/**
 * @file attributes.cpp
 * @brief Implementacao do parse dos atributos obrigatorios do formato .class.
 *
 * Atributos implementados:
 *   - Code          (JVMS 4.7.3)
 *   - LineNumberTable (JVMS 4.7.12)
 *   - Exceptions    (JVMS 4.7.5)
 *   - SourceFile    (JVMS 4.7.10)
 *   - ConstantValue (JVMS 4.7.2)
 */

#include "attributes.h"
#include "constant_pool.h"
#include <cstring>

/* ------------------------------------------------------------------ */
/* Helpers de leitura big-endian a partir de buffer em memoria         */
/* ------------------------------------------------------------------ */

/**
 * @brief Le 2 bytes big-endian de um buffer e avanca a posicao.
 * @param buf  Buffer de bytes.
 * @param pos  Posicao atual (modificada em +2 apos a leitura).
 * @return Valor u2 lido.
 */
static u2 buf_read_u2(const u1 *buf, u4 *pos) {
    u2 v = (u2)((buf[*pos] << 8) | buf[*pos + 1]);
    *pos += 2;
    return v;
}

/**
 * @brief Le 4 bytes big-endian de um buffer e avanca a posicao.
 * @param buf  Buffer de bytes.
 * @param pos  Posicao atual (modificada em +4 apos a leitura).
 * @return Valor u4 lido.
 */
static u4 buf_read_u4(const u1 *buf, u4 *pos) {
    u4 v = ((u4)buf[*pos]     << 24) |
           ((u4)buf[*pos + 1] << 16) |
           ((u4)buf[*pos + 2] <<  8) |
            (u4)buf[*pos + 3];
    *pos += 4;
    return v;
}

/* ------------------------------------------------------------------ */
/* Helper interno: encontra atributo por nome                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Procura um atributo por nome no array de atributos genericos.
 *
 * @param attrs  Array de atributos.
 * @param count  Numero de entradas.
 * @param cp     Constant pool para resolver o nome.
 * @param name   Nome do atributo a procurar (ex: "Code").
 * @return Ponteiro para o atributo encontrado, ou NULL.
 */
static const attribute_info *find_attribute(const attribute_info *attrs,
                                             u2                    count,
                                             const cp_info        *cp,
                                             const char           *name) {
    for (u2 i = 0; i < count; i++) {
        u2 ni = attrs[i].name_index;
        if (cp[ni].tag != CP_UTF8) continue;
        const char *aname = reinterpret_cast<const char *>(cp[ni].data.utf8.bytes);
        if (strcmp(aname, name) == 0)
            return &attrs[i];
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Code_attribute                                                       */
/* ------------------------------------------------------------------ */

Code_attribute *parse_code_attribute(const attribute_info *attrs,
                                     u2                    count,
                                     const cp_info        *cp) {
    const attribute_info *attr = find_attribute(attrs, count, cp, "Code");
    if (!attr || !attr->info) return NULL;

    const u1 *buf = attr->info;
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

    /* Guarda os sub-atributos do Code para uso posterior (LineNumberTable etc.) */
    ca->attributes_count = buf_read_u2(buf, &pos);
    if (ca->attributes_count > 0) {
        ca->sub_attributes = new attribute_info[ca->attributes_count];
        for (u2 i = 0; i < ca->attributes_count; i++) {
            ca->sub_attributes[i].name_index = buf_read_u2(buf, &pos);
            ca->sub_attributes[i].length     = buf_read_u4(buf, &pos);
            if (ca->sub_attributes[i].length > 0) {
                ca->sub_attributes[i].info = new u1[ca->sub_attributes[i].length];
                memcpy(ca->sub_attributes[i].info, buf + pos, ca->sub_attributes[i].length);
                pos += ca->sub_attributes[i].length;
            } else {
                ca->sub_attributes[i].info = NULL;
            }
        }
    } else {
        ca->sub_attributes = NULL;
    }

    return ca;
}

/* ------------------------------------------------------------------ */
/* LineNumberTable                                                      */
/* ------------------------------------------------------------------ */

u2 *parse_linenumber_table(const attribute_info *attrs,
                           u2                    count,
                           const cp_info        *cp,
                           u2                   *out_count) {
    *out_count = 0;
    const attribute_info *attr = find_attribute(attrs, count, cp, "LineNumberTable");
    if (!attr || !attr->info) return NULL;

    const u1 *buf = attr->info;
    u4 pos = 0;

    u2 len = buf_read_u2(buf, &pos);
    *out_count = len;
    if (len == 0) return NULL;

    /* Array plano de pares {start_pc, line_number} */
    u2 *table = new u2[len * 2];
    for (u2 i = 0; i < len; i++) {
        table[i * 2]     = buf_read_u2(buf, &pos); /* start_pc */
        table[i * 2 + 1] = buf_read_u2(buf, &pos); /* line_number */
    }
    return table;
}

/* ------------------------------------------------------------------ */
/* Exceptions (clausula throws)                                        */
/* ------------------------------------------------------------------ */

u2 *parse_exceptions_attribute(const attribute_info *attrs,
                               u2                    count,
                               const cp_info        *cp,
                               u2                   *out_count) {
    *out_count = 0;
    const attribute_info *attr = find_attribute(attrs, count, cp, "Exceptions");
    if (!attr || !attr->info) return NULL;

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

/* ------------------------------------------------------------------ */
/* SourceFile                                                           */
/* ------------------------------------------------------------------ */

u2 parse_sourcefile_attribute(const attribute_info *attrs,
                              u2                    count,
                              const cp_info        *cp) {
    const attribute_info *attr = find_attribute(attrs, count, cp, "SourceFile");
    if (!attr || !attr->info || attr->length < 2) return 0;
    return (u2)((attr->info[0] << 8) | attr->info[1]);
}

/* ------------------------------------------------------------------ */
/* ConstantValue                                                        */
/* ------------------------------------------------------------------ */

u2 parse_constantvalue_attribute(const attribute_info *attrs,
                                 u2                    count,
                                 const cp_info        *cp) {
    const attribute_info *attr = find_attribute(attrs, count, cp, "ConstantValue");
    if (!attr || !attr->info || attr->length < 2) return 0;
    return (u2)((attr->info[0] << 8) | attr->info[1]);
}
