#include "displayer.h"
#include "opcodes.h"
#include "constant_pool.h"
#include <cstdio>
#include <cstring>
#include <cstdint>

/* ------------------------------------------------------------------ */
/* Helpers de formatacao                                                */
/* ------------------------------------------------------------------ */

/**
 * @brief Converte access_flags de classe em string legivel.
 */
static void flags_to_str(u2 flags, char *buf, int bufsz) {
    buf[0] = '\0';
    if (flags & ACC_PUBLIC)    strncat(buf, "public ",    bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_FINAL)     strncat(buf, "final ",     bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_SUPER)     strncat(buf, "super ",     bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_INTERFACE) strncat(buf, "interface ", bufsz - (int)strlen(buf) - 1);
    if (flags & ACC_ABSTRACT)  strncat(buf, "abstract ",  bufsz - (int)strlen(buf) - 1);
    /* Remove espaco final */
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == ' ') buf[len - 1] = '\0';
}

/**
 * @brief Converte access_flags de metodo/campo em string legivel.
 */
static void method_flags_to_str(u2 flags, char *buf, int bufsz) {
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
}

/**
 * @brief Retorna o nome Java da versao major.
 */
static const char *java_version_name(u2 major) {
    switch (major) {
        case 46: return "Java 2";
        case 47: return "Java 3";
        case 48: return "Java 4";
        case 49: return "Java 5";
        case 50: return "Java 6";
        case 51: return "Java 7";
        case 52: return "Java 8";
        case 53: return "Java 9";
        case 54: return "Java 10";
        case 55: return "Java 11";
        default: return "unknown";
    }
}

/* ------------------------------------------------------------------ */
/* Exibicao do constant pool                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Imprime uma entrada do constant pool com valor resolvido.
 */
static void display_cp_entry(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    printf("  #%-4u = ", index);

    switch (e.tag) {
        case CP_UTF8:
            printf("Utf8          \"%s\"\n",
                   reinterpret_cast<const char *>(e.data.utf8.bytes));
            break;
        case CP_INTEGER: {
            int32_t v;
            memcpy(&v, &e.data.integer_val.bytes, 4);
            printf("Integer       %d\n", v);
            break;
        }
        case CP_FLOAT: {
            float v;
            memcpy(&v, &e.data.float_val.bytes, 4);
            printf("Float         %f\n", v);
            break;
        }
        case CP_LONG: {
            int64_t v = ((int64_t)e.data.long_val.high_bytes << 32) |
                         e.data.long_val.low_bytes;
            printf("Long          %lld\n", (long long)v);
            break;
        }
        case CP_DOUBLE: {
            uint64_t bits = ((uint64_t)e.data.double_val.high_bytes << 32) |
                             e.data.double_val.low_bytes;
            double v;
            memcpy(&v, &bits, 8);
            printf("Double        %f\n", v);
            break;
        }
        case CP_CLASS: {
            std::string name = resolve_class_name(cf, index);
            printf("Class         #%u\t// %s\n",
                   e.data.class_info.name_index, name.c_str());
            break;
        }
        case CP_STRING: {
            std::string s = resolve_string(cf, index);
            printf("String        #%u\t// \"%s\"\n",
                   e.data.string_info.string_index, s.c_str());
            break;
        }
        case CP_FIELDREF: {
            std::string resolved = resolve_fieldref(cf, index);
            printf("Fieldref      #%u.#%u\t// %s\n",
                   e.data.ref.class_index,
                   e.data.ref.name_and_type_index,
                   resolved.c_str());
            break;
        }
        case CP_METHODREF: {
            std::string resolved = resolve_methodref(cf, index);
            printf("Methodref     #%u.#%u\t// %s\n",
                   e.data.ref.class_index,
                   e.data.ref.name_and_type_index,
                   resolved.c_str());
            break;
        }
        case CP_INTERFACE_METHODREF: {
            std::string resolved = resolve_methodref(cf, index);
            printf("InterfaceMethodref #%u.#%u\t// %s\n",
                   e.data.ref.class_index,
                   e.data.ref.name_and_type_index,
                   resolved.c_str());
            break;
        }
        case CP_NAME_AND_TYPE: {
            std::string nat = resolve_nameandtype(cf, index);
            printf("NameAndType   #%u.#%u\t// %s\n",
                   e.data.name_and_type.name_index,
                   e.data.name_and_type.descriptor_index,
                   nat.c_str());
            break;
        }
        default:
            printf("(empty/invalid tag %u)\n", e.tag);
            break;
    }
}

/* ------------------------------------------------------------------ */
/* Tamanho de cada instrucao para avanco correto do PC                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Retorna o numero de bytes de uma instrucao (opcode + operandos).
 *
 * tableswitch e lookupswitch tem tamanho variavel; o offset do inicio
 * da instrucao e necessario para calcular o alinhamento.
 *
 * @param code  Array de bytecodes.
 * @param pc    Offset da instrucao atual.
 * @return Numero de bytes a avancar.
 */
static int opcode_size(const u1 *code, u4 pc) {
    u1 op = code[pc];
    switch (op) {
        /* sem operandos */
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

        /* 1 operando de 1 byte */
        case 0x10: /* bipush */
        case 0x12: /* ldc */
        case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: /* xload */
        case 0x36: case 0x37: case 0x38: case 0x39: case 0x3A: /* xstore */
        case 0xA9: /* ret */
        case 0xBC: /* newarray */
            return 2;

        /* 1 operando de 2 bytes */
        case 0x11: /* sipush */
        case 0x13: /* ldc_w */
        case 0x14: /* ldc2_w */
        case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E:
        case 0x9F: case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4:
        case 0xA5: case 0xA6:
        case 0xA7: /* goto */
        case 0xA8: /* jsr */
        case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
        case 0xB8: case 0xBB: case 0xBD: case 0xC0: case 0xC1:
        case 0xC6: case 0xC7: /* ifnull, ifnonnull */
            return 3;

        /* iinc: indice(1) + constante(1) */
        case 0x84:
            return 3;

        /* invokeinterface: index(2) + count(1) + 0(1) */
        case 0xB9:
            return 5;

        /* invokedynamic: index(2) + 0(1) + 0(1) */
        case 0xBA:
            return 5;

        /* goto_w, jsr_w: 4 bytes */
        case 0xC8: case 0xC9:
            return 5;

        /* multianewarray: index(2) + dim(1) */
        case 0xC5:
            return 4;

        /* wide: depende do opcode seguinte */
        case 0xC4: {
            u1 next = code[pc + 1];
            if (next == 0x84) return 6; /* wide iinc */
            return 4;                   /* wide xload/xstore */
        }

        /* tableswitch: alinhamento variavel */
        case 0xAA: {
            u4 aligned = (pc + 4) & ~3u;
            u4 off = aligned - pc;
            /* default(4) + low(4) + high(4) */
            int32_t low  = (int32_t)(((u4)code[pc + off + 4] << 24) |
                                     ((u4)code[pc + off + 5] << 16) |
                                     ((u4)code[pc + off + 6] <<  8) |
                                      (u4)code[pc + off + 7]);
            int32_t high = (int32_t)(((u4)code[pc + off + 8] << 24) |
                                     ((u4)code[pc + off + 9] << 16) |
                                     ((u4)code[pc + off +10] <<  8) |
                                      (u4)code[pc + off +11]);
            return (int)(off + 12 + (high - low + 1) * 4);
        }

        /* lookupswitch */
        case 0xAB: {
            u4 aligned = (pc + 4) & ~3u;
            u4 off = aligned - pc;
            int32_t npairs = (int32_t)(((u4)code[pc + off + 4] << 24) |
                                       ((u4)code[pc + off + 5] << 16) |
                                       ((u4)code[pc + off + 6] <<  8) |
                                        (u4)code[pc + off + 7]);
            return (int)(off + 8 + npairs * 8);
        }

        default:
            return 1;
    }
}

/* ------------------------------------------------------------------ */
/* Exibicao de bytecodes                                                */
/* ------------------------------------------------------------------ */

void display_bytecodes(const ClassFile *cf, const Code_attribute *code) {
    if (!code) return;

    u4 pc = 0;
    while (pc < code->code_length) {
        u1 op = code->code[pc];
        printf("  %4u: %-15s", pc, mnemonic[op]);

        /* Imprime operandos e valor resolvido do CP quando aplicavel */
        switch (op) {
            case 0x10: /* bipush */
                printf(" %d", (int8_t)code->code[pc + 1]);
                break;
            case 0x11: /* sipush */
                printf(" %d", (int16_t)((code->code[pc + 1] << 8) | code->code[pc + 2]));
                break;
            case 0x12: { /* ldc */
                u1 idx = code->code[pc + 1];
                printf(" #%u\t// %s", idx,
                       resolve_string(cf, idx).c_str());
                break;
            }
            case 0x13: { /* ldc_w */
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" #%u", idx);
                break;
            }
            case 0x14: { /* ldc2_w */
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" #%u", idx);
                break;
            }
            case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: /* xload */
            case 0x36: case 0x37: case 0x38: case 0x39: case 0x3A: /* xstore */
            case 0xBC: /* newarray */
            case 0xA9: /* ret */
                printf(" %u", code->code[pc + 1]);
                break;
            case 0x84: /* iinc */
                printf(" %u, %d", code->code[pc + 1], (int8_t)code->code[pc + 2]);
                break;
            case 0xB2: case 0xB3: case 0xB4: case 0xB5: { /* getstatic/putstatic/getfield/putfield */
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                std::string resolved = resolve_fieldref(cf, idx);
                printf(" #%u\t// %s", idx, resolved.c_str());
                break;
            }
            case 0xB6: case 0xB7: case 0xB8: { /* invokevirtual/invokespecial/invokestatic */
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                std::string resolved = resolve_methodref(cf, idx);
                printf(" #%u\t// %s", idx, resolved.c_str());
                break;
            }
            case 0xB9: { /* invokeinterface */
                u2 idx   = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                u1 count = code->code[pc + 3];
                std::string resolved = resolve_methodref(cf, idx);
                printf(" #%u, %u\t// %s", idx, count, resolved.c_str());
                break;
            }
            case 0xBB: case 0xBD: case 0xC0: case 0xC1: { /* new/anewarray/checkcast/instanceof */
                u2 idx = (u2)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                std::string resolved = resolve_class_name(cf, idx);
                printf(" #%u\t// %s", idx, resolved.c_str());
                break;
            }
            case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E:
            case 0x9F: case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4:
            case 0xA5: case 0xA6:
            case 0xA7: case 0xA8:
            case 0xC6: case 0xC7: { /* branches */
                int16_t offset = (int16_t)((code->code[pc + 1] << 8) | code->code[pc + 2]);
                printf(" %d  (-> %u)", offset, (u4)((int32_t)pc + offset));
                break;
            }
            case 0xC8: case 0xC9: { /* goto_w / jsr_w */
                int32_t offset = (int32_t)(((u4)code->code[pc + 1] << 24) |
                                           ((u4)code->code[pc + 2] << 16) |
                                           ((u4)code->code[pc + 3] <<  8) |
                                            (u4)code->code[pc + 4]);
                printf(" %d  (-> %u)", offset, (u4)((int32_t)pc + offset));
                break;
            }
            case 0xAA: { /* tableswitch */
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
                    printf("             %8d: %d\n", low + k, (int32_t)pc + tgt);
                }
                printf("               default: %d\n            }", (int32_t)pc + def);
                break;
            }
            case 0xAB: { /* lookupswitch */
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
                    printf("             %8d: %d\n", key, (int32_t)pc + tgt);
                }
                printf("               default: %d\n            }", (int32_t)pc + def);
                break;
            }
            default:
                break;
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

    /* 1. Cabecalho */
    printf("Magic:         0x%08X\n", cf->magic);
    printf("Minor version: %u\n", cf->minor_version);
    printf("Major version: %u  (%s)\n", cf->major_version,
           java_version_name(cf->major_version));
    printf("\n");

    /* 2. Constant Pool */
    printf("Constant Pool (%u entradas):\n", cf->constant_pool_count - 1);
    for (u2 i = 1; i < cf->constant_pool_count; i++) {
        if (cf->constant_pool[i].tag == 0) {
            printf("  #%-4u   (slot vazio — parte de Long/Double anterior)\n", i);
            continue;
        }
        display_cp_entry(cf, i);
    }
    printf("\n");

    /* 3. Access flags */
    flags_to_str(cf->access_flags, buf, sizeof(buf));
    printf("Access flags:  0x%04X (%s)\n", cf->access_flags, buf);

    /* 4. This class e Super class */
    printf("This class:    #%u  // %s\n",
           cf->this_class, resolve_class_name(cf, cf->this_class).c_str());
    if (cf->super_class != 0)
        printf("Super class:   #%u  // %s\n",
               cf->super_class, resolve_class_name(cf, cf->super_class).c_str());
    else
        printf("Super class:   #0  // (nenhuma)\n");
    printf("\n");

    /* 5. Interfaces */
    printf("Interfaces (%u):\n", cf->interfaces_count);
    for (u2 i = 0; i < cf->interfaces_count; i++)
        printf("  #%u  // %s\n",
               cf->interfaces[i], resolve_class_name(cf, cf->interfaces[i]).c_str());
    printf("\n");

    /* 6. Fields */
    printf("Fields (%u):\n", cf->fields_count);
    for (u2 i = 0; i < cf->fields_count; i++) {
        const field_info &fi = cf->fields[i];
        method_flags_to_str(fi.access_flags, buf, sizeof(buf));
        printf("  %s %s %s\n",
               buf,
               resolve_utf8(cf, fi.name_index).c_str(),
               resolve_utf8(cf, fi.descriptor_index).c_str());
    }
    printf("\n");

    /* 7. Methods */
    printf("Methods (%u):\n", cf->methods_count);
    for (u2 i = 0; i < cf->methods_count; i++) {
        const method_info &mi = cf->methods[i];
        method_flags_to_str(mi.access_flags, buf, sizeof(buf));
        printf("  Method: %s %s  [%s]\n",
               resolve_utf8(cf, mi.name_index).c_str(),
               resolve_utf8(cf, mi.descriptor_index).c_str(),
               buf);
        if (mi.code_attr) {
            printf("    max_stack=%u  max_locals=%u  code_length=%u\n",
                   mi.code_attr->max_stack,
                   mi.code_attr->max_locals,
                   mi.code_attr->code_length);
            display_bytecodes(cf, mi.code_attr);
            if (mi.code_attr->exception_table_length > 0) {
                printf("    Exception table:\n");
                printf("      start  end  handler  type\n");
                for (u2 j = 0; j < mi.code_attr->exception_table_length; j++) {
                    const exception_entry &e = mi.code_attr->exception_table[j];
                    printf("      %-6u %-4u %-8u  ",
                           e.start_pc, e.end_pc, e.handler_pc);
                    if (e.catch_type == 0)
                        printf("any\n");
                    else
                        printf("%s\n", resolve_class_name(cf, e.catch_type).c_str());
                }
            }
        }
        printf("\n");
    }
}
