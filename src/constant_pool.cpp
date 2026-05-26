#include "constant_pool.h"
#include <cstring>
#include <cstdio>
#include <cstdint>

std::string resolve_utf8(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    if (e.tag != CP_UTF8) return "<invalid-utf8>";
    return std::string(reinterpret_cast<const char *>(e.data.utf8.bytes), e.data.utf8.length);
}

std::string resolve_class_name(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    if (e.tag != CP_CLASS) return "<invalid-class>";
    return resolve_utf8(cf, e.data.class_info.name_index);
}

std::string resolve_string(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    if (e.tag != CP_STRING) return "<invalid-string>";
    return resolve_utf8(cf, e.data.string_info.string_index);
}

std::string resolve_nameandtype(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    if (e.tag != CP_NAME_AND_TYPE) return "<invalid-nat>";
    return resolve_utf8(cf, e.data.name_and_type.name_index) + ":" +
           resolve_utf8(cf, e.data.name_and_type.descriptor_index);
}

std::string resolve_methodref(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    if (e.tag != CP_METHODREF && e.tag != CP_INTERFACE_METHODREF)
        return "<invalid-methodref>";
    return resolve_class_name(cf, e.data.ref.class_index) + "." +
           resolve_nameandtype(cf, e.data.ref.name_and_type_index);
}

std::string resolve_fieldref(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    if (e.tag != CP_FIELDREF) return "<invalid-fieldref>";
    return resolve_class_name(cf, e.data.ref.class_index) + "." +
           resolve_nameandtype(cf, e.data.ref.name_and_type_index);
}

std::string resolve_cp_value(const ClassFile *cf, u2 index) {
    const cp_info &e = cf->constant_pool[index];
    char buf[64];
    switch (e.tag) {
        case CP_INTEGER: {
            int32_t v; memcpy(&v, &e.data.integer_val.bytes, 4);
            snprintf(buf, sizeof(buf), "%d", v);
            return buf;
        }
        case CP_FLOAT: {
            float v; memcpy(&v, &e.data.float_val.bytes, 4);
            snprintf(buf, sizeof(buf), "%ff", v);
            return buf;
        }
        case CP_LONG: {
            int64_t v = ((int64_t)e.data.long_val.high_bytes << 32) |
                         (int64_t)e.data.long_val.low_bytes;
            snprintf(buf, sizeof(buf), "%lldL", (long long)v);
            return buf;
        }
        case CP_DOUBLE: {
            uint64_t bits = ((uint64_t)e.data.double_val.high_bytes << 32) |
                             (uint64_t)e.data.double_val.low_bytes;
            double v; memcpy(&v, &bits, 8);
            snprintf(buf, sizeof(buf), "%f", v);
            return buf;
        }
        case CP_STRING:  return "\"" + resolve_string(cf, index) + "\"";
        case CP_CLASS:   return resolve_class_name(cf, index);
        default:
            snprintf(buf, sizeof(buf), "<tag-%u>", e.tag);
            return buf;
    }
}
