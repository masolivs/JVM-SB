#include "displayer.h"
#include "opcodes.h"
#include "constant_pool.h"
#include "attributes.h"
#include <cstdio>
#include <cstring>
#include <cstdint>

static void class_flags_to_str(u2 flags, char *buf, int bufsz) {
    buf[0] = '\0';
    if (flags & ACC_PUBLIC)     strncat(buf, "public ",     bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_FINAL)      strncat(buf, "final ",      bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_SUPER)      strncat(buf, "super ",      bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_INTERFACE)  strncat(buf, "interface ",  bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ABSTRACT)   strncat(buf, "abstract ",   bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ANNOTATION) strncat(buf, "annotation ", bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ENUM)       strncat(buf, "enum ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_SYNTHETIC)  strncat(buf, "synthetic ",  bufsz - (int)strlen(buf) - 1);
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == ' ') buf[len - 1] = '\0';
}

static void member_flags_to_str(u2 flags, char *buf, int bufsz) {
    buf[0] = '\0';
    if (flags & ACC_PUBLIC)       strncat(buf, "public ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_PRIVATE)      strncat(buf, "private ",      bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_PROTECTED)    strncat(buf, "protected ",    bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_STATIC)       strncat(buf, "static ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_FINAL)        strncat(buf, "final ",        bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_SYNCHRONIZED) strncat(buf, "synchronized ", bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_VOLATILE)     strncat(buf, "volatile ",     bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_TRANSIENT)    strncat(buf, "transient ",    bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_NATIVE)       strncat(buf, "native ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ABSTRACT)     strncat(buf, "abstract ",     bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_STRICT)       strncat(buf, "strict ",       bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ENUM)         strncat(buf, "enum ",         bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_SYNTHETIC)    strncat(buf, "synthetic ",    bufsz - (int)strlen(buf) - 1);
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == ' ') buf[len - 1] = '\0';
}

static const char *java_version_name(u2 major) {
    switch (major) {
        case 45: return "Java 1.1";
        case 46: return "Java 1.2";
        case 47: return "Java 1.3";
        case 48: return "Java 1.4";
        case 49: return "Java 5";
        case 50: return "Java 6";
        case 51: return "Java 7";
        case 52: return "Java 8";
        case 53: return "Java 9";
        case 54: return "Java 10";
        case 55: return "Java 11";
        case 56: return "Java 12";
        case 57: return "Java 13";
        case 58: return "Java 14";
        case 59: return "Java 15";
        case 60: return "Java 16";
        case 61: return "Java 17";
        case 62: return "Java 18";
        case 63: return "Java 19";
        case 64: return "Java 20";
        case 65: return "Java 21";
        default: return "unknown";
    }
}

static void display_cp_entry(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    printf("  #%-4u = ", index);

    switch (e.tag) {
        case CP_UTF8:
            printf("Utf8               \"%s\"\n",
                   reinterpret_cast<const char *>(e.data.utf8.bytes));
            break;
        case CP_INTEGER: {
            int32_t v; memcpy(&v, &e.data.integer_val.bytes, 4);
            printf("Integer            %d\n", v);
            break;
        }
        case CP_FLOAT: {
            float v; memcpy(&v, &e.data.float_val.bytes, 4);
            printf("Float              %f\n", v);
            break;
        }
        case CP_LONG: {
            int64_t v = ((int64_t)e.data.long_val.high_bytes << 32) |
                         e.data.long_val.low_bytes;
            printf("Long               %lld\n", (long long)v);
            break;
        }
        case CP_DOUBLE: {
            uint64_t bits = ((uint64_t)e.data.double_val.high_bytes << 32) |
                             e.data.double_val.low_bytes;
            double v; memcpy(&v, &bits, 8);
            printf("Double             %f\n", v);
            break;
        }
        case CP_CLASS:
            printf("Class              #%u\t// %s\n",
                   e.data.class_info.name_index,
                   resolve_class_name(cf, index).c_str());
            break;
        case CP_STRING:
            printf("String             #%u\t// \"%s\"\n",
                   e.data.string_info.string_index,
                   resolve_string(cf, index).c_str());
            break;
        case CP_FIELDREF:
            printf("Fieldref           #%u.#%u\t// %s\n",
                   e.data.ref.class_index, e.data.ref.name_and_type_index,
                   resolve_fieldref(cf, index).c_str());
            break;
        case CP_METHODREF:
            printf("Methodref          #%u.#%u\t// %s\n",
                   e.data.ref.class_index, e.data.ref.name_and_type_index,
                   resolve_methodref(cf, index).c_str());
            break;
        case CP_INTERFACE_METHODREF:
            printf("InterfaceMethodref #%u.#%u\t// %s\n",
                   e.data.ref.class_index, e.data.ref.name_and_type_index,
                   resolve_methodref(cf, index).c_str());
            break;
        case CP_NAME_AND_TYPE:
            printf("NameAndType        #%u.#%u\t// %s\n",
                   e.data.name_and_type.name_index,
                   e.data.name_and_type.descriptor_index,
                   resolve_nameandtype(cf, index).c_str());
            break;
        default:
            printf("(tag invalido: %u)\n", e.tag);
            break;
    }
}

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
        case 0xB2: case 0xB3: case 0xB4: case 0xB5:
        case 0xB6: case 0xB7: case 0xB8:
        case 0xBB: case 0xBD: case 0xC0: case 0xC1:
        case 0xC6: case 0xC7:
            return 3;

        case 0xB9: case 0xBA: case 0xC8: case 0xC9:
            return 5;

        case 0xC5:
            return 4;

        case 0xC4: {
            u1 next = code[pc + 1];
            return (next == 0x84) ? 6 : 4;
        }

        case 0xAA: {
            u4 aligned = (pc + 4) & ~3u;
            u4 off = aligned - pc;
            const u1 *p = code + pc + off;
            int32_t low  = (int32_t)(((u4)p[4]<<24)|((u4)p[5]<<16)|((u4)p[6]<<8)|(u4)p[7]);
            int32_t high = (int32_t)(((u4)p[8]<<24)|((u4)p[9]<<16)|((u4)p[10]<<8)|(u4)p[11]);
            return (int)(off + 12 + (high - low + 1) * 4);
        }

        case 0xAB: {
            u4 aligned = (pc + 4) & ~3u;
            u4 off = aligned - pc;
            const u1 *p = code + pc + off;
            int32_t npairs = (int32_t)(((u4)p[4]<<24)|((u4)p[5]<<16)|((u4)p[6]<<8)|(u4)p[7]);
            return (int)(off + 8 + npairs * 8);
        }

        default:
            return 1;
    }
}

void display_bytecodes(const ClassFile *cf, const Code_attribute *code) {
    if (!code) return;

    u4 pc = 0;
    while (pc < code->code_length) {
        u1 op = code->code[pc];
        printf("    %4u: %-16s", pc, mnemonic[op]);

        switch (op) {
            case 0x10:
                printf(" %d", (int8_t)code->code[pc + 1]);
                break;
            case 0x11:
                printf(" %d", (int16_t)((code->code[pc + 1] << 8) | code->code[pc + 2]));
                break;
            case 0x12: {
                u1 idx = code->code[pc + 1];
                printf(" #%u\t// %s", idx, resolve_cp_value(cf, idx).c_str());
                break;
            }
            case 0x13: case 0x14: {
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" #%u\t// %s", idx, resolve_cp_value(cf, idx).c_str());
                break;
            }
            case 0x15: case 0x16: case 0x17: case 0x18: case 0x19:
            case 0x36: case 0x37: case 0x38: case 0x39: case 0x3A:
            case 0xA9: case 0xBC:
                printf(" %u", code->code[pc + 1]);
                break;
            case 0x84:
                printf(" %u, %d", code->code[pc + 1], (int8_t)code->code[pc + 2]);
                break;
            case 0xB2: case 0xB3: case 0xB4: case 0xB5: {
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" #%u\t// %s", idx, resolve_fieldref(cf, idx).c_str());
                break;
            }
            case 0xB6: case 0xB7: case 0xB8: {
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" #%u\t// %s", idx, resolve_methodref(cf, idx).c_str());
                break;
            }
            case 0xB9: {
                u2 idx   = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                u1 count = code->code[pc + 3];
                printf(" #%u, %u\t// %s", idx, count, resolve_methodref(cf, idx).c_str());
                break;
            }
            case 0xBA: {
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" #%u", idx);
                break;
            }
            case 0xBB: case 0xBD: case 0xC0: case 0xC1: {
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" #%u\t// %s", idx, resolve_class_name(cf, idx).c_str());
                break;
            }
            case 0xC5: {
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                u1 dim = code->code[pc + 3];
                printf(" #%u, %u\t// %s", idx, dim, resolve_class_name(cf, idx).c_str());
                break;
            }
            case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E:
            case 0x9F: case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4:
            case 0xA5: case 0xA6: case 0xA7: case 0xA8: case 0xC6: case 0xC7: {
                int16_t offset = (int16_t)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                u4 target = (u4)((int32_t)pc + offset);
                printf(" %d\t(-> %u)", offset, target);
                break;
            }
            case 0xC8: case 0xC9: {
                int32_t offset = (int32_t)(((u4)code->code[pc + 1] << 24) |
                                           ((u4)code->code[pc + 2] << 16) |
                                           ((u4)code->code[pc + 3] <<  8) |
                                            (u4)code->code[pc + 4]);
                u4 target = (u4)((int32_t)pc + offset);
                printf(" %d\t(-> %u)", offset, target);
                break;
            }
            case 0xAA: {
                u4 aligned = (pc + 4) & ~3u;
                u4 off = aligned - pc;
                const u1 *p = code->code + pc + off;
                int32_t def  = (int32_t)(((u4)p[0]<<24)|((u4)p[1]<<16)|((u4)p[2]<<8)|(u4)p[3]);
                int32_t low  = (int32_t)(((u4)p[4]<<24)|((u4)p[5]<<16)|((u4)p[6]<<8)|(u4)p[7]);
                int32_t high = (int32_t)(((u4)p[8]<<24)|((u4)p[9]<<16)|((u4)p[10]<<8)|(u4)p[11]);
                printf(" { // %d to %d\n", low, high);
                for (int32_t k = 0; k <= high - low; k++) {
                    const u1 *ep = p + 12 + k * 4;
                    int32_t tgt = (int32_t)(((u4)ep[0]<<24)|((u4)ep[1]<<16)|((u4)ep[2]<<8)|(u4)ep[3]);
                    printf("               %8d: %d\n", low + k, (int32_t)pc + tgt);
                }
                printf("                 default: %d\n              }", (int32_t)pc + def);
                break;
            }
            case 0xAB: {
                u4 aligned = (pc + 4) & ~3u;
                u4 off = aligned - pc;
                const u1 *p = code->code + pc + off;
                int32_t def    = (int32_t)(((u4)p[0]<<24)|((u4)p[1]<<16)|((u4)p[2]<<8)|(u4)p[3]);
                int32_t npairs = (int32_t)(((u4)p[4]<<24)|((u4)p[5]<<16)|((u4)p[6]<<8)|(u4)p[7]);
                printf(" { // %d\n", npairs);
                for (int32_t k = 0; k < npairs; k++) {
                    const u1 *ep = p + 8 + k * 8;
                    int32_t key = (int32_t)(((u4)ep[0]<<24)|((u4)ep[1]<<16)|((u4)ep[2]<<8)|(u4)ep[3]);
                    int32_t tgt = (int32_t)(((u4)ep[4]<<24)|((u4)ep[5]<<16)|((u4)ep[6]<<8)|(u4)ep[7]);
                    printf("               %8d: %d\n", key, (int32_t)pc + tgt);
                }
                printf("                 default: %d\n              }", (int32_t)pc + def);
                break;
            }
            default:
                break;
        }
        printf("\n");
        pc += (u4)opcode_size(code->code, pc);
    }
}

static void display_constant_value(const ClassFile *cf, u2 idx) {
    const cp_info &e = cf->constant_pool[idx];
    switch (e.tag) {
        case CP_INTEGER: {
            int32_t v; memcpy(&v, &e.data.integer_val.bytes, 4);
            printf(" = %d", v);
            break;
        }
        case CP_FLOAT: {
            float v; memcpy(&v, &e.data.float_val.bytes, 4);
            printf(" = %ff", v);
            break;
        }
        case CP_LONG: {
            int64_t v = ((int64_t)e.data.long_val.high_bytes << 32) |
                         e.data.long_val.low_bytes;
            printf(" = %lldL", (long long)v);
            break;
        }
        case CP_DOUBLE: {
            uint64_t bits = ((uint64_t)e.data.double_val.high_bytes << 32) |
                             e.data.double_val.low_bytes;
            double v; memcpy(&v, &bits, 8);
            printf(" = %f", v);
            break;
        }
        case CP_STRING:
            printf(" = \"%s\"", resolve_string(cf, idx).c_str());
            break;
        default:
            break;
    }
}

static void display_field(const ClassFile *cf, const field_info &fi) {
    char buf[256];
    member_flags_to_str(fi.access_flags, buf, sizeof(buf));
    printf("    [0x%04X] %-20s  %s  %s",
           fi.access_flags, buf,
           resolve_utf8(cf, fi.name_index).c_str(),
           resolve_utf8(cf, fi.descriptor_index).c_str());

    if (fi.attributes_count > 0) {
        u2 cv_idx = parse_constantvalue_attribute(fi.attributes, fi.attributes_count,
                                                  cf->constant_pool);
        if (cv_idx != 0)
            display_constant_value(cf, cv_idx);
    }
    printf("\n");
}

void display_class_file(const ClassFile *cf) {
    char buf[256];

    printf("=== Cabecalho ============================================\n");
    printf("  Magic:          0x%08X\n", cf->magic);
    printf("  Minor version:  %u\n",     cf->minor_version);
    printf("  Major version:  %u  (%s)\n", cf->major_version,
           java_version_name(cf->major_version));
    if (cf->attributes_count > 0) {
        u2 sf_idx = parse_sourcefile_attribute(cf->attributes, cf->attributes_count,
                                               cf->constant_pool);
        if (sf_idx != 0)
            printf("  SourceFile:     %s\n", resolve_utf8(cf, sf_idx).c_str());
    }
    printf("\n");

    printf("=== Constant Pool (%u entradas) ==========================\n",
           cf->constant_pool_count - 1);
    for (u2 i = 1; i < cf->constant_pool_count; i++) {
        if (cf->constant_pool[i].tag == 0) {
            printf("  #%-4u   (slot vazio — parte de Long/Double anterior)\n", i);
            continue;
        }
        display_cp_entry(cf, i);
    }
    printf("\n");

    printf("=== Informacoes da Classe ================================\n");
    class_flags_to_str(cf->access_flags, buf, sizeof(buf));
    printf("  Access flags:  0x%04X  (%s)\n", cf->access_flags, buf);
    printf("  This class:    #%u  // %s\n",
           cf->this_class, resolve_class_name(cf, cf->this_class).c_str());
    if (cf->super_class != 0)
        printf("  Super class:   #%u  // %s\n",
               cf->super_class, resolve_class_name(cf, cf->super_class).c_str());
    else
        printf("  Super class:   #0  // (nenhuma — java/lang/Object)\n");
    printf("  Interfaces (%u):\n", cf->interfaces_count);
    for (u2 i = 0; i < cf->interfaces_count; i++)
        printf("    #%u  // %s\n",
               cf->interfaces[i], resolve_class_name(cf, cf->interfaces[i]).c_str());
    printf("\n");

    printf("=== Fields (%u) ==========================================\n",
           cf->fields_count);
    u2 n_static = 0;
    for (u2 i = 0; i < cf->fields_count; i++)
        if (cf->fields[i].access_flags & ACC_STATIC) n_static++;
    u2 n_instance = cf->fields_count - n_static;

    if (n_static > 0) {
        printf("  --- Fields estaticos (%u) ---\n", n_static);
        for (u2 i = 0; i < cf->fields_count; i++)
            if (cf->fields[i].access_flags & ACC_STATIC)
                display_field(cf, cf->fields[i]);
    }
    if (n_instance > 0) {
        printf("  --- Fields de instancia (%u) ---\n", n_instance);
        for (u2 i = 0; i < cf->fields_count; i++)
            if (!(cf->fields[i].access_flags & ACC_STATIC))
                display_field(cf, cf->fields[i]);
    }
    if (cf->fields_count == 0)
        printf("  (nenhum campo)\n");
    printf("\n");

    printf("=== Methods (%u) =========================================\n",
           cf->methods_count);
    for (u2 i = 0; i < cf->methods_count; i++) {
        const method_info &mi = cf->methods[i];
        member_flags_to_str(mi.access_flags, buf, sizeof(buf));

        printf("  Method #%u: %s  %s\n", i,
               resolve_utf8(cf, mi.name_index).c_str(),
               resolve_utf8(cf, mi.descriptor_index).c_str());
        printf("    Access flags: 0x%04X  (%s)\n", mi.access_flags, buf);

        if (mi.attributes_count > 0) {
            u2 ex_count = 0;
            u2 *ex_indices = parse_exceptions_attribute(mi.attributes, mi.attributes_count,
                                                        cf->constant_pool, &ex_count);
            if (ex_indices && ex_count > 0) {
                printf("    throws:");
                for (u2 j = 0; j < ex_count; j++)
                    printf(" %s", resolve_class_name(cf, ex_indices[j]).c_str());
                printf("\n");
                delete[] ex_indices;
            }
        }

        if (mi.code_attr) {
            const Code_attribute *ca = mi.code_attr;
            printf("    Code:\n");
            printf("      max_stack=%u  max_locals=%u  code_length=%u\n",
                   ca->max_stack, ca->max_locals, ca->code_length);
            display_bytecodes(cf, ca);

            if (ca->exception_table_length > 0) {
                printf("    Exception table:\n");
                printf("      %6s  %5s  %9s  type\n", "start", "end", "handler");
                for (u2 j = 0; j < ca->exception_table_length; j++) {
                    const exception_entry &ex = ca->exception_table[j];
                    printf("      %-6u %-5u %-9u  ", ex.start_pc, ex.end_pc, ex.handler_pc);
                    if (ex.catch_type == 0)
                        printf("any (finally)\n");
                    else
                        printf("%s\n", resolve_class_name(cf, ex.catch_type).c_str());
                }
            }

            if (ca->sub_attributes && ca->attributes_count > 0) {
                u2 lnt_count = 0;
                u2 *lnt = parse_linenumber_table(ca->sub_attributes, ca->attributes_count,
                                                 cf->constant_pool, &lnt_count);
                if (lnt && lnt_count > 0) {
                    printf("    LineNumberTable:\n");
                    for (u2 j = 0; j < lnt_count; j++)
                        printf("      line %u: %u\n", lnt[j * 2 + 1], lnt[j * 2]);
                    delete[] lnt;
                }
            }
        } else {
            printf("    (sem Code — metodo nativo ou abstrato)\n");
        }
        printf("\n");
    }
}
