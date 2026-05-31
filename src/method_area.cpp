#include "method_area.h"
#include "class_reader.h"
#include "constant_pool.h"
#include <cstdio>

MethodArea *method_area_create(void) {
    return new MethodArea();
}

void method_area_destroy(MethodArea *ma) {
    if (!ma) return;
    for (auto &pair : ma->classes) {
        free_class_file(pair.second->cf);
        delete pair.second;
    }
    delete ma;
}

ClassEntry *load_class(MethodArea *ma, const std::string &name,
                       const std::string &path) {
    /* Nao recarrega */
    auto it = ma->classes.find(name);
    if (it != ma->classes.end()) return it->second;

    ClassFile *cf = NULL;
    JvmError err = read_class_file(path.c_str(), &cf);
    if (err != JVM_OK || !cf) {
        fprintf(stderr, "Erro ao carregar classe '%s' de '%s' (err=%d)\n",
                name.c_str(), path.c_str(), err);
        return NULL;
    }

    ClassEntry *ce = new ClassEntry();
    ce->name        = name;
    ce->cf          = cf;
    ce->initialized = false;
    ce->super       = NULL;

    /* Conta campos estaticos para alocar static_fields */
    int nstatic = 0;
    for (u2 i = 0; i < cf->fields_count; i++) {
        if (cf->fields[i].access_flags & ACC_STATIC)
            nstatic++;
    }
    ce->static_fields.assign((size_t)nstatic, 0);

    ma->classes[name] = ce;
    return ce;
}

ClassEntry *find_class(const MethodArea *ma, const std::string &name) {
    auto it = ma->classes.find(name);
    if (it == ma->classes.end()) return NULL;
    return it->second;
}

method_info *find_method_ex(const ClassEntry *ce, const std::string &name,
                             const std::string &desc, ClassEntry **declaring_ce) {
    while (ce) {
        if (!ce->cf) { ce = ce->super; continue; }
        for (u2 i = 0; i < ce->cf->methods_count; i++) {
            method_info &m = ce->cf->methods[i];
            std::string mname = resolve_utf8(ce->cf, m.name_index);
            std::string mdesc = resolve_utf8(ce->cf, m.descriptor_index);
            if (mname == name && mdesc == desc) {
                if (declaring_ce) *declaring_ce = const_cast<ClassEntry *>(ce);
                return &m;
            }
        }
        ce = ce->super;
    }
    return NULL;
}

method_info *find_method(const ClassEntry *ce, const std::string &name,
                          const std::string &desc) {
    return find_method_ex(ce, name, desc, NULL);
}

field_info *find_field(const ClassEntry *ce, const std::string &name,
                        const std::string &desc) {
    while (ce) {
        if (!ce->cf) { ce = ce->super; continue; }
        for (u2 i = 0; i < ce->cf->fields_count; i++) {
            field_info &f = ce->cf->fields[i];
            std::string fname = resolve_utf8(ce->cf, f.name_index);
            std::string fdesc = resolve_utf8(ce->cf, f.descriptor_index);
            if (fname == name && fdesc == desc) return &f;
        }
        ce = ce->super;
    }
    return NULL;
}

int count_hierarchy_fields(const ClassEntry *ce) {
    if (!ce) return 0;
    int n = count_hierarchy_fields(ce->super);
    if (ce->cf) {
        for (u2 i = 0; i < ce->cf->fields_count; i++)
            if (!(ce->cf->fields[i].access_flags & ACC_STATIC))
                n++;
    }
    return n;
}

int find_field_index(const ClassEntry *ce, const std::string &name,
                     const std::string &desc) {
    if (!ce || !ce->cf) return -1;

    /* Campos da superclasse ocupam indices menores */
    int base = count_hierarchy_fields(ce->super);

    int local = 0;
    for (u2 i = 0; i < ce->cf->fields_count; i++) {
        field_info &f = ce->cf->fields[i];
        if (f.access_flags & ACC_STATIC) continue;
        std::string fname = resolve_utf8(ce->cf, f.name_index);
        std::string fdesc = resolve_utf8(ce->cf, f.descriptor_index);
        if (fname == name && fdesc == desc) return base + local;
        local++;
    }

    /* Nao encontrado nesta classe — sobe a hierarquia */
    return find_field_index(ce->super, name, desc);
}
