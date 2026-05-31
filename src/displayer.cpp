#include "displayer.h"
#include "opcodes.h"
#include "constant_pool.h"
#include <cstdio>
#include <cstring>
#include <cstdint>

/* ------------------------------------------------------------------ */
/* Helpers visuais                                                       */
/* ------------------------------------------------------------------ */

static void print_section(const char *title) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  %-60s║\n", title);
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

static void class_flags_to_str(u2 flags, char *buf, int bufsz) {
    buf[0] = '\0';
    if (flags & ACC_PUBLIC)     strncat(buf, "public ",     bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_FINAL)      strncat(buf, "final ",      bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_SUPER)      strncat(buf, "super ",      bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_INTERFACE)  strncat(buf, "interface ",  bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ABSTRACT)   strncat(buf, "abstract ",   bufsz - (int)strlen(buf) - 1);
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == ' ') buf[len - 1] = '\0';
    if (buf[0] == '\0') snprintf(buf, (size_t)bufsz, "(nenhum)");
}

static void member_flags_to_str(u2 flags, char *buf, int bufsz) {
    buf[0] = '\0';
    if (flags & ACC_PUBLIC)       strncat(buf, "public ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_PRIVATE)      strncat(buf, "private ",      bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_PROTECTED)    strncat(buf, "protected ",    bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_STATIC)       strncat(buf, "static ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_FINAL)        strncat(buf, "final ",        bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_SYNCHRONIZED) strncat(buf, "synchronized ", bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_NATIVE)       strncat(buf, "native ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ABSTRACT)     strncat(buf, "abstract ",     bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_STRICT)       strncat(buf, "strict ",       bufsz - (int)strlen(buf) - 1);
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == ' ') buf[len - 1] = '\0';
    if (buf[0] == '\0') snprintf(buf, (size_t)bufsz, "(nenhum)");
}

static const char *java_version_name(u2 major) {
    switch (major) {
        case 45: return "Java 1.1";  case 46: return "Java 1.2";
        case 47: return "Java 1.3";  case 48: return "Java 1.4";
        case 49: return "Java 5";    case 50: return "Java 6";
        case 51: return "Java 7";    case 52: return "Java 8";
        case 53: return "Java 9";    case 54: return "Java 10";
        case 55: return "Java 11";   case 56: return "Java 12";
        case 57: return "Java 13";   case 58: return "Java 14";
        case 59: return "Java 15";   case 60: return "Java 16";
        case 61: return "Java 17";   case 65: return "Java 21";
        default: return "desconhecida";
    }
}

static const char *cp_tag_name(u1 tag) {
    switch (tag) {
        case  1: return "Utf8";
        case  3: return "Integer";
        case  4: return "Float";
        case  5: return "Long";
        case  6: return "Double";
        case  7: return "Class";
        case  8: return "String";
        case  9: return "Fieldref";
        case 10: return "Methodref";
        case 11: return "InterfaceMethodref";
        case 12: return "NameAndType";
        default: return "?";
    }
}

static u2 buf_u2(const u1 *buf, u4 pos) {
    return (u2)((buf[pos] << 8) | buf[pos+1]);
}
static u4 buf_u4(const u1 *buf, u4 pos) {
    return ((u4)buf[pos]<<24)|((u4)buf[pos+1]<<16)|((u4)buf[pos+2]<<8)|(u4)buf[pos+3];
}

static u2 find_sourcefile(const ClassFile *cf) {
    for (u2 i = 0; i < cf->attributes_count; i++) {
        u2 ni = cf->attributes[i].name_index;
        if (cf->constant_pool[ni].tag == CP_UTF8 &&
            strcmp((const char *)cf->constant_pool[ni].data.utf8.bytes, "SourceFile") == 0) {
            const u1 *buf = cf->attributes[i].info;
            if (buf && cf->attributes[i].length >= 2)
                return buf_u2(buf, 0);
        }
    }
    return 0;
}

static void display_linenumber_table(const method_info &mi,
                                     const Code_attribute *ca,
                                     const cp_info *cp) {
    for (u2 i = 0; i < mi.attributes_count; i++) {
        u2 ni = mi.attributes[i].name_index;
        if (cp[ni].tag != CP_UTF8 ||
            strcmp((const char *)cp[ni].data.utf8.bytes, "Code") != 0) continue;
        const u1 *buf = mi.attributes[i].info;
        if (!buf) return;
        u4 pos = 2 + 2 + 4 + ca->code_length;
        u2 exc = buf_u2(buf, pos); pos += 2 + exc * 8;
        u2 sub = buf_u2(buf, pos); pos += 2;
        for (u2 j = 0; j < sub; j++) {
            u2 sni  = buf_u2(buf, pos);
            u4 slen = buf_u4(buf, pos+2);
            pos += 6;
            if (cp[sni].tag == CP_UTF8 &&
                strcmp((const char *)cp[sni].data.utf8.bytes, "LineNumberTable") == 0) {
                u2 n = buf_u2(buf, pos);
                printf("    │\n");
                printf("    └─ LineNumberTable (%u entrada%s):\n", n, n == 1 ? "" : "s");
                for (u2 k = 0; k < n; k++) {
                    u2 start = buf_u2(buf, pos+2+k*4);
                    u2 line  = buf_u2(buf, pos+4+k*4);
                    printf("         linha %-4u  →  offset %u\n", line, start);
                }
                return;
            }
            pos += slen;
        }
        return;
    }
}

/* ------------------------------------------------------------------ */
/* Tamanho das instrucoes                                               */
/* ------------------------------------------------------------------ */

static int opcode_size(const u1 *code, u4 pc) {
    u1 op = code[pc];
    switch (op) {
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
        case 0x06: case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B:
        case 0x0C: case 0x0D: case 0x0E: case 0x0F: case 0x1A: case 0x1B:
        case 0x1C: case 0x1D: case 0x1E: case 0x1F: case 0x20: case 0x21:
        case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
        case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D:
        case 0x2E: case 0x2F: case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x3B: case 0x3C: case 0x3D: case 0x3E:
        case 0x3F: case 0x40: case 0x41: case 0x42: case 0x43: case 0x44:
        case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4A:
        case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: case 0x50:
        case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56:
        case 0x57: case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C:
        case 0x5D: case 0x5E: case 0x5F: case 0x60: case 0x61: case 0x62:
        case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68:
        case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E:
        case 0x6F: case 0x70: case 0x71: case 0x72: case 0x73: case 0x74:
        case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7A:
        case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: case 0x80:
        case 0x81: case 0x82: case 0x83: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D:
        case 0x8E: case 0x8F: case 0x90: case 0x91: case 0x92: case 0x93:
        case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0xAC:
        case 0xAD: case 0xAE: case 0xAF: case 0xB0: case 0xB1: case 0xBE:
        case 0xBF: case 0xC2: case 0xC3:
            return 1;
        case 0x10: case 0x12:
        case 0x15: case 0x16: case 0x17: case 0x18: case 0x19:
        case 0x36: case 0x37: case 0x38: case 0x39: case 0x3A:
        case 0xA9: case 0xBC:
            return 2;
        case 0x11: case 0x13: case 0x14: case 0x84:
        case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E:
        case 0x9F: case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4:
        case 0xA5: case 0xA6: case 0xA7: case 0xA8:
        case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
        case 0xB8: case 0xBB: case 0xBD: case 0xC0: case 0xC1:
        case 0xC6: case 0xC7:
            return 3;
        case 0xC5: return 4;
        case 0xB9: case 0xBA: case 0xC8: case 0xC9: return 5;
        case 0xC4: return (code[pc+1] == 0x84) ? 6 : 4;
        case 0xAA: {
            u4 al = (pc+4)&~3u, off = al-pc;
            int32_t lo = (int32_t)(((u4)code[pc+off+4]<<24)|((u4)code[pc+off+5]<<16)|
                                   ((u4)code[pc+off+6]<<8)|(u4)code[pc+off+7]);
            int32_t hi = (int32_t)(((u4)code[pc+off+8]<<24)|((u4)code[pc+off+9]<<16)|
                                   ((u4)code[pc+off+10]<<8)|(u4)code[pc+off+11]);
            return (int)(off+12+(hi-lo+1)*4);
        }
        case 0xAB: {
            u4 al = (pc+4)&~3u, off = al-pc;
            int32_t np = (int32_t)(((u4)code[pc+off+4]<<24)|((u4)code[pc+off+5]<<16)|
                                   ((u4)code[pc+off+6]<<8)|(u4)code[pc+off+7]);
            return (int)(off+8+np*8);
        }
        default: return 1;
    }
}

/* ------------------------------------------------------------------ */
/* Disassembly                                                           */
/* ------------------------------------------------------------------ */

void display_bytecodes(const ClassFile *cf, const Code_attribute *code) {
    if (!code) return;
    u4 pc = 0;
    while (pc < code->code_length) {
        u1 op = code->code[pc];
        printf("    │   %4u: %-16s", pc, mnemonic[op]);

        switch (op) {
            case 0x10: printf("  %d", (int8_t)code->code[pc+1]); break;
            case 0x11: printf("  %d", (int16_t)((code->code[pc+1]<<8)|code->code[pc+2])); break;
            case 0x12: {
                u1 idx = code->code[pc+1];
                printf("  #%-4u → %s", idx, resolve_cp_value(cf, idx).c_str());
                break;
            }
            case 0x13: case 0x14: {
                u2 idx = (u2)((code->code[pc+1]<<8)|code->code[pc+2]);
                printf("  #%-4u → %s", idx, resolve_cp_value(cf, idx).c_str());
                break;
            }
            case 0x15: case 0x16: case 0x17: case 0x18: case 0x19:
            case 0x36: case 0x37: case 0x38: case 0x39: case 0x3A:
            case 0xA9: case 0xBC:
                printf("  %u", code->code[pc+1]);
                break;
            case 0x84:
                printf("  var=%u  inc=%d", code->code[pc+1], (int8_t)code->code[pc+2]);
                break;
            case 0xB2: case 0xB3: case 0xB4: case 0xB5: {
                u2 idx = (u2)((code->code[pc+1]<<8)|code->code[pc+2]);
                printf("  #%-4u → %s", idx, resolve_fieldref(cf, idx).c_str());
                break;
            }
            case 0xB6: case 0xB7: case 0xB8: {
                u2 idx = (u2)((code->code[pc+1]<<8)|code->code[pc+2]);
                printf("  #%-4u → %s", idx, resolve_methodref(cf, idx).c_str());
                break;
            }
            case 0xB9: {
                u2 idx = (u2)((code->code[pc+1]<<8)|code->code[pc+2]);
                u1 cnt = code->code[pc+3];
                printf("  #%-4u  count=%u → %s", idx, cnt, resolve_methodref(cf, idx).c_str());
                break;
            }
            case 0xBB: case 0xBD: case 0xC0: case 0xC1: {
                u2 idx = (u2)((code->code[pc+1]<<8)|code->code[pc+2]);
                printf("  #%-4u → %s", idx, resolve_class_name(cf, idx).c_str());
                break;
            }
            case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E:
            case 0x9F: case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4:
            case 0xA5: case 0xA6: case 0xA7: case 0xA8: case 0xC6: case 0xC7: {
                int16_t off = (int16_t)((code->code[pc+1]<<8)|code->code[pc+2]);
                printf("  %d  (→ pc %u)", off, (u4)((int32_t)pc+off));
                break;
            }
            case 0xC8: case 0xC9: {
                int32_t off = (int32_t)(((u4)code->code[pc+1]<<24)|((u4)code->code[pc+2]<<16)|
                                        ((u4)code->code[pc+3]<<8)|(u4)code->code[pc+4]);
                printf("  %d  (→ pc %u)", off, (u4)((int32_t)pc+off));
                break;
            }
            case 0xAA: {
                u4 al = (pc+4)&~3u, off = al-pc;
                const u1 *c = code->code;
                int32_t def  = (int32_t)(((u4)c[pc+off]  <<24)|((u4)c[pc+off+1]<<16)|((u4)c[pc+off+2]<<8)|(u4)c[pc+off+3]);
                int32_t low  = (int32_t)(((u4)c[pc+off+4]<<24)|((u4)c[pc+off+5]<<16)|((u4)c[pc+off+6]<<8)|(u4)c[pc+off+7]);
                int32_t high = (int32_t)(((u4)c[pc+off+8]<<24)|((u4)c[pc+off+9]<<16)|((u4)c[pc+off+10]<<8)|(u4)c[pc+off+11]);
                printf("  { // range %d..%d\n", low, high);
                for (int32_t k = 0; k <= high-low; k++) {
                    int32_t tgt = (int32_t)(((u4)c[pc+off+12+k*4]<<24)|((u4)c[pc+off+13+k*4]<<16)|
                                            ((u4)c[pc+off+14+k*4]<<8)|(u4)c[pc+off+15+k*4]);
                    printf("    │        case %-6d → pc %d\n", low+k, (int32_t)pc+tgt);
                }
                printf("    │        default    → pc %d\n", (int32_t)pc+def);
                printf("    │              }");
                break;
            }
            case 0xAB: {
                u4 al = (pc+4)&~3u, off = al-pc;
                const u1 *c = code->code;
                int32_t def = (int32_t)(((u4)c[pc+off]  <<24)|((u4)c[pc+off+1]<<16)|((u4)c[pc+off+2]<<8)|(u4)c[pc+off+3]);
                int32_t np  = (int32_t)(((u4)c[pc+off+4]<<24)|((u4)c[pc+off+5]<<16)|((u4)c[pc+off+6]<<8)|(u4)c[pc+off+7]);
                printf("  { // %d pares\n", np);
                for (int32_t k = 0; k < np; k++) {
                    int32_t key = (int32_t)(((u4)c[pc+off+8+k*8]  <<24)|((u4)c[pc+off+9+k*8] <<16)|
                                            ((u4)c[pc+off+10+k*8] <<8)|(u4)c[pc+off+11+k*8]);
                    int32_t tgt = (int32_t)(((u4)c[pc+off+12+k*8]<<24)|((u4)c[pc+off+13+k*8]<<16)|
                                            ((u4)c[pc+off+14+k*8]<<8)|(u4)c[pc+off+15+k*8]);
                    printf("    │        chave %-6d → pc %d\n", key, (int32_t)pc+tgt);
                }
                printf("    │        default    → pc %d\n", (int32_t)pc+def);
                printf("    │              }");
                break;
            }
            default: break;
        }
        printf("\n");
        pc += (u4)opcode_size(code->code, pc);
    }
}

/* ------------------------------------------------------------------ */
/* Exibicao principal                                                   */
/* ------------------------------------------------------------------ */

void display_class_file(const ClassFile *cf) {
    char buf[256];

    /* ── 1. Informacoes da Classe ──────────────────────────────────── */
    print_section("INFORMACOES DA CLASSE");
    u2 sf = find_sourcefile(cf);
    class_flags_to_str(cf->access_flags, buf, sizeof(buf));

    printf("  %-22s  0x%08X\n",       "Magic Number:",     cf->magic);
    printf("  %-22s  %u\n",           "Minor Version:",    cf->minor_version);
    printf("  %-22s  %u  (%s)\n",     "Major Version:",    cf->major_version,
           java_version_name(cf->major_version));
    if (sf)
        printf("  %-22s  %s\n", "Arquivo Fonte:",
               (const char *)cf->constant_pool[sf].data.utf8.bytes);
    printf("  %-22s  0x%04X  (%s)\n", "Access Flags:",     cf->access_flags, buf);
    printf("  %-22s  #%-4u  %s\n",    "This Class:",
           cf->this_class, resolve_class_name(cf, cf->this_class).c_str());
    if (cf->super_class)
        printf("  %-22s  #%-4u  %s\n", "Super Class:",
               cf->super_class, resolve_class_name(cf, cf->super_class).c_str());
    else
        printf("  %-22s  (nenhuma)\n", "Super Class:");
    if (cf->interfaces_count == 0) {
        printf("  %-22s  (nenhuma)\n", "Interfaces:");
    } else {
        printf("  %-22s  %u\n", "Interfaces:", cf->interfaces_count);
        for (u2 i = 0; i < cf->interfaces_count; i++)
            printf("    %-20s  #%-4u  %s\n", "",
                   cf->interfaces[i],
                   resolve_class_name(cf, cf->interfaces[i]).c_str());
    }

    /* ── 2. Constant Pool ─────────────────────────────────────────── */
    print_section("CONSTANT POOL");
    printf("  Total de entradas: %u\n", cf->constant_pool_count - 1);
    printf("  │\n");
    printf("  │  %-6s  %-3s  %-20s  %s\n",
           "Indice", "Tag", "Tipo", "Valor / Referencia");
    printf("  │  %-6s  %-3s  %-20s  %s\n",
           "------", "---", "--------------------", "---------------------");

    for (u2 i = 1; i < cf->constant_pool_count; i++) {
        const cp_info &e = cf->constant_pool[i];
        if (e.tag == 0) {
            printf("  │  #%-5u  ---  %-20s\n", i, "(slot duplo: Long/Double)");
            continue;
        }

        char resolved[256] = "";
        switch (e.tag) {
            case CP_UTF8:
                snprintf(resolved, sizeof(resolved), "\"%s\"",
                         (const char *)e.data.utf8.bytes); break;
            case CP_INTEGER: {
                int32_t v; memcpy(&v, &e.data.integer_val.bytes, 4);
                snprintf(resolved, sizeof(resolved), "%d", v); break;
            }
            case CP_FLOAT: {
                float v; memcpy(&v, &e.data.float_val.bytes, 4);
                snprintf(resolved, sizeof(resolved), "%f", v); break;
            }
            case CP_LONG: {
                int64_t v = ((int64_t)e.data.long_val.high_bytes<<32)|e.data.long_val.low_bytes;
                snprintf(resolved, sizeof(resolved), "%lldL", (long long)v); break;
            }
            case CP_DOUBLE: {
                uint64_t b = ((uint64_t)e.data.double_val.high_bytes<<32)|e.data.double_val.low_bytes;
                double v; memcpy(&v, &b, 8);
                snprintf(resolved, sizeof(resolved), "%f", v); break;
            }
            case CP_CLASS:
                snprintf(resolved, sizeof(resolved), "#%-3u → %s",
                         e.data.class_info.name_index,
                         resolve_class_name(cf, i).c_str()); break;
            case CP_STRING:
                snprintf(resolved, sizeof(resolved), "#%-3u → \"%s\"",
                         e.data.string_info.string_index,
                         resolve_string(cf, i).c_str()); break;
            case CP_FIELDREF:
                snprintf(resolved, sizeof(resolved), "#%u.#%-2u → %s",
                         e.data.ref.class_index, e.data.ref.name_and_type_index,
                         resolve_fieldref(cf, i).c_str()); break;
            case CP_METHODREF:
            case CP_INTERFACE_METHODREF:
                snprintf(resolved, sizeof(resolved), "#%u.#%-2u → %s",
                         e.data.ref.class_index, e.data.ref.name_and_type_index,
                         resolve_methodref(cf, i).c_str()); break;
            case CP_NAME_AND_TYPE:
                snprintf(resolved, sizeof(resolved), "#%u.#%-2u → %s",
                         e.data.name_and_type.name_index,
                         e.data.name_and_type.descriptor_index,
                         resolve_nameandtype(cf, i).c_str()); break;
            default:
                snprintf(resolved, sizeof(resolved), "(desconhecido)"); break;
        }

        printf("  │  #%-5u  %-3u  %-20s  %s\n",
               i, e.tag, cp_tag_name(e.tag), resolved);
    }
    printf("  │\n");

    /* ── 3. Fields ────────────────────────────────────────────────── */
    print_section("FIELDS");
    printf("  Total de campos: %u\n", cf->fields_count);

    if (cf->fields_count == 0) {
        printf("  (nenhum campo definido)\n");
    } else {
        for (u2 i = 0; i < cf->fields_count; i++) {
            const field_info &fi = cf->fields[i];
            member_flags_to_str(fi.access_flags, buf, sizeof(buf));
            printf("  │\n");
            printf("  ├─ Campo #%u\n", i);
            printf("  │   %-18s  %s\n", "Nome:",       resolve_utf8(cf, fi.name_index).c_str());
            printf("  │   %-18s  %s\n", "Descriptor:", resolve_utf8(cf, fi.descriptor_index).c_str());
            printf("  │   %-18s  0x%04X  (%s)\n", "Access Flags:", fi.access_flags, buf);
        }
        printf("  │\n");
    }

    /* ── 4. Methods ───────────────────────────────────────────────── */
    print_section("METHODS");
    printf("  Total de metodos: %u\n", cf->methods_count);

    for (u2 i = 0; i < cf->methods_count; i++) {
        const method_info &mi = cf->methods[i];
        member_flags_to_str(mi.access_flags, buf, sizeof(buf));

        printf("  │\n");
        printf("  ├─ Metodo #%u\n", i);
        printf("  │   %-18s  %s\n", "Nome:",       resolve_utf8(cf, mi.name_index).c_str());
        printf("  │   %-18s  %s\n", "Descriptor:", resolve_utf8(cf, mi.descriptor_index).c_str());
        printf("  │   %-18s  0x%04X  (%s)\n", "Access Flags:", mi.access_flags, buf);

        if (mi.code_attr) {
            printf("  │\n");
            printf("  │   %-18s  %u\n", "max_stack:",  mi.code_attr->max_stack);
            printf("  │   %-18s  %u\n", "max_locals:", mi.code_attr->max_locals);
            printf("  │   %-18s  %u bytes\n", "Codigo:",  mi.code_attr->code_length);

            if (mi.code_attr->exception_table_length > 0) {
                printf("  │\n");
                printf("  │   Tabela de Excecoes (%u entrada%s):\n",
                       mi.code_attr->exception_table_length,
                       mi.code_attr->exception_table_length == 1 ? "" : "s");
                printf("  │   %-8s  %-6s  %-8s  %s\n",
                       "start_pc", "end_pc", "handler", "Tipo");
                for (u2 j = 0; j < mi.code_attr->exception_table_length; j++) {
                    const exception_entry &e = mi.code_attr->exception_table[j];
                    printf("  │   %-8u  %-6u  %-8u  ", e.start_pc, e.end_pc, e.handler_pc);
                    if (e.catch_type == 0) printf("any (finally)\n");
                    else printf("%s\n", resolve_class_name(cf, e.catch_type).c_str());
                }
            }

            printf("  │\n");
            printf("  │   Bytecodes:\n");
            printf("  │   %-6s  %-16s  %s\n",
                   "offset", "mnemonico", "operandos / CP resolvido");
            printf("  │   %-6s  %-16s  %s\n",
                   "------", "----------------", "------------------------");
            display_bytecodes(cf, mi.code_attr);
            display_linenumber_table(mi, mi.code_attr, cf->constant_pool);
        }
    }
    printf("  │\n\n");
}
