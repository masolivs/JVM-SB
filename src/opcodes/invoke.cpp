#include "interpreter.h"
#include "constant_pool.h"
#include "frame.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

/**
 * @brief Conta o numero de slots de parametros que um descriptor ocupa na pilha.
 *
 * @details Percorre a string de descriptor a partir do caractere apos '(',
 * contabilizando cada tipo conforme as regras da JVM:
 *   - 'J' (long) e 'D' (double) valem 2 slots.
 *   - Tipos de referencia ('L...;') e arrays ('[...') valem 1 slot.
 *   - Todos os outros tipos primitivos valem 1 slot.
 * O caractere de retorno (apos ')') nao e contabilizado.
 *
 * @param desc  Descriptor de metodo no formato JVM, ex: "(ILjava/lang/String;D)V".
 * @return Numero total de slots que os argumentos ocupam na operand stack.
 */
static int count_params(const std::string &desc) {
    int n = 0;
    size_t i = 1; /* pula '(' */
    while (i < desc.size() && desc[i] != ')') {
        if (desc[i] == 'L') { while (desc[i] != ';') i++; n++; }
        else if (desc[i] == '[') { /* array: consome dimensoes */ while (desc[i]=='[') i++; if (desc[i]=='L') while(desc[i]!=';') i++; n++; }
        else if (desc[i] == 'J' || desc[i] == 'D') n += 2;
        else n++;
        i++;
    }
    return n;
}

/**
 * @brief Simula chamadas a java.io.PrintStream.println e .print via stdout.
 *
 * @details Como o JRE nao e carregado, este handler intercepta invocacoes
 * virtuais a java/io/PrintStream e emula o comportamento de impressao.
 * O algoritmo:
 *   1. Verifica se a classe e "java/io/PrintStream" e o metodo e "println"
 *      ou "print". Se nao, retorna false imediatamente.
 *   2. Se o descriptor e "()V" (sem argumento), descarta apenas a referencia
 *      ao PrintStream e emite endl (se println).
 *   3. Caso contrario, determina o tipo do argumento pelo primeiro caractere
 *      do descriptor:
 *      - I/B/S/Z: pop int32, imprime como inteiro (Z como "true"/"false").
 *      - C: pop int32, imprime como char.
 *      - J: pop long (2 slots), imprime como inteiro de 64 bits.
 *      - F: pop float, formata com 1 casa decimal para inteiros ou %.7g.
 *      - D: pop double, formata com 1 casa decimal ou %.15g.
 *      - L/[: pop referencia; se for JArray de chars, imprime os caracteres;
 *             caso contrario, imprime "null".
 *   4. Apos imprimir, emite endl se o metodo for "println".
 *
 * @param class_name   Classe do metodo (deve ser "java/io/PrintStream").
 * @param method_name  Nome do metodo ("println" ou "print").
 * @param desc         Descriptor do metodo; define o tipo do argumento.
 * @param jvm          JVM em execucao; usada para acessar o heap.
 * @param frame        Frame corrente; argumentos sao retirados da operand stack.
 * @return true se o metodo foi simulado; false se nao era um println/print.
 */
static bool simulate_println(const std::string &class_name,
                             const std::string &method_name,
                             const std::string &desc,
                             JVM *jvm, Frame *frame) {
    if (class_name != "java/io/PrintStream") return false;
    if (method_name != "println" && method_name != "print") return false;

    /* Retira o argumento (se houver) e o objeto PrintStream da pilha */
    if (desc == "()V") {
        frame_pop(frame); /* ref PrintStream */
        if (method_name == "println") std::cout << std::endl;
        return true;
    }

    /* Determina o tipo pelo descriptor */
    char type = desc[1]; /* primeiro char do parametro */

    if (type == 'I' || type == 'B' || type == 'C' || type == 'S' || type == 'Z') {
        int32_t v = frame_pop(frame);
        frame_pop(frame); /* ref */
        if (type == 'C') std::cout << (char)v;
        else if (type == 'Z') std::cout << (v ? "true" : "false");
        else std::cout << v;
    } else if (type == 'J') {
        int64_t v = frame_pop_long(frame);
        frame_pop(frame);
        std::cout << v;
    } else if (type == 'F') {
        float v = frame_pop_float(frame);
        frame_pop(frame);
        /* Java sempre imprime ponto decimal em floats inteiros */
        if (v == (float)(int)v && v == v) printf("%.1f", (double)v);
        else printf("%.7g", (double)v);
    } else if (type == 'D') {
        double v = frame_pop_double(frame);
        frame_pop(frame);
        /* Java sempre imprime ponto decimal em doubles inteiros;
           usa 15 digitos significativos para doubles nao-inteiros */
        if (v == (double)(long long)v && v == v) {
            printf("%.1f", v);
        } else {
            /* %.15g da precisao suficiente sem zeros desnecessarios */
            char buf[64];
            snprintf(buf, sizeof(buf), "%.15g", v);
            printf("%s", buf);
        }
    } else if (type == 'L' || type == '[') {
        /* Referencia — tenta imprimir como array de char (String simulada) */
        int32_t ref = frame_pop(frame);
        frame_pop(frame); /* ref PrintStream */
        JArray *arr = heap_get_array(jvm, ref);
        if (arr) {
            for (int32_t k = 0; k < arr->arraylength; k++) {
                uint16_t ch;
                memcpy(&ch, (char*)arr->elements + k*2, 2);
                std::cout << (char)ch;
            }
        } else {
            std::cout << "null";
        }
    } else {
        frame_pop(frame);
        frame_pop(frame);
    }

    if (method_name == "println") std::cout << std::endl;
    return true;
}

/**
 * @brief Cria um novo frame para o metodo e transfere os argumentos do chamador.
 *
 * @details O algoritmo de transferencia de argumentos:
 *   1. Calcula o numero total de slots: count_params(desc) mais 1 se has_this
 *      (a referencia 'this' ocupa o slot local_vars[0]).
 *   2. Aloca o novo frame via frame_create() e registra o PC do invoke
 *      (jvm->exception_pc) em nf->call_site_pc, permitindo que excecoes
 *      propagadas saibam onde o metodo foi invocado no frame chamador.
 *   3. Retira os argumentos da operand stack do chamador em ordem inversa
 *      (do ultimo para o primeiro) e os armazena em nf->local_vars[0..n-1].
 *      Essa inversao e necessaria porque a pilha empilha argumentos da
 *      esquerda para a direita, mas local_vars os indexa da esquerda.
 *   4. Empurra o novo frame na JvmStack; o loop interpret() passara a
 *      executar a partir dele no proximo ciclo.
 *
 * @param jvm       JVM em execucao.
 * @param caller    Frame do metodo chamador; argumentos sao retirados daqui.
 * @param m         Metodo a ser invocado.
 * @param ce        Classe dona do metodo; usada para resolver o constant pool.
 * @param has_this  true se o metodo e de instancia (inclui 'this' nos params).
 * @param desc      Descriptor do metodo; usado para calcular o numero de slots.
 */
static void invoke_method(JVM *jvm, Frame *caller,
                          method_info *m, ClassEntry *ce,
                          bool has_this, const std::string &desc) {
    /* Conta slots de parametros */
    int nparams = count_params(desc);
    if (has_this) nparams++;

    /* Cria novo frame; salva o PC do invoke no chamador para propagacao de excecao */
    Frame *nf = frame_create(m, ce);
    nf->call_site_pc = jvm->exception_pc;

    /* Retira parametros da pilha do chamador (ordem inversa) e coloca em local_vars */
    for (int i = nparams - 1; i >= 0; i--) {
        nf->local_vars[(size_t)i] = frame_pop(caller);
    }

    jvm_stack_push(jvm->stack, nf);
}

/**
 * @brief invokestatic — invoca metodo estatico (sem receptor 'this').
 *
 * @details
 *   1. Le o indice de 2 bytes do constant pool a partir do PC e avanca pc += 2.
 *   2. Resolve a classe e o par nome:descriptor via resolve_class_name() e
 *      resolve_nameandtype().
 *   3. Carrega a classe via auto_load_class(); aborta com erro em stderr se
 *      nao encontrada.
 *   4. Busca o metodo (e a classe declarante) via find_method_ex(), que sobe
 *      a hierarquia de heranca. Aborta se nao encontrado.
 *   5. Invoca via invoke_method() com has_this = false.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame do metodo chamador.
 */
void op_invokestatic(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    const ClassFile *cf = frame->klass->cf;
    std::string cls  = resolve_class_name(cf, cf->constant_pool[idx].data.ref.class_index);
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t colon = nat.find(':');
    std::string mname = nat.substr(0, colon);
    std::string mdesc = nat.substr(colon + 1);

    /* Simula metodos estaticos conhecidos (ex: valueOf) */
    ClassEntry *ce = auto_load_class(jvm, cls);
    if (!ce) { fprintf(stderr,"Classe nao encontrada: %s\n", cls.c_str()); return; }

    ClassEntry *decl = NULL;
    method_info *m = find_method_ex(ce, mname, mdesc, &decl);
    if (!m) { fprintf(stderr,"Metodo nao encontrado: %s.%s%s\n", cls.c_str(), mname.c_str(), mdesc.c_str()); return; }

    invoke_method(jvm, frame, m, decl ? decl : ce, false, mdesc);
}

/**
 * @brief invokespecial — invoca construtores, metodos privados e super.
 *
 * @details Usado pelo compilador para chamadas que nao passam por despacho
 * virtual: `<init>`, metodos private e chamadas super.method().
 *   1. Le o indice de 2 bytes e resolve classe, nome e descriptor.
 *   2. Caso especial: `<init>` de java/lang/Object e ignorado (descarta
 *      apenas a referencia 'this'), pois Object nao e carregado.
 *   3. Carrega a classe e busca o metodo via find_method_ex().
 *   4. Se o metodo nao for encontrado e for `<init>` de uma classe sintetica
 *      do JRE, descarta os argumentos silenciosamente (no-op seguro).
 *   5. Caso contrario, invoca via invoke_method() com has_this = true.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame do metodo chamador.
 */
void op_invokespecial(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    const ClassFile *cf = frame->klass->cf;
    std::string cls  = resolve_class_name(cf, cf->constant_pool[idx].data.ref.class_index);
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t colon = nat.find(':');
    std::string mname = nat.substr(0, colon);
    std::string mdesc = nat.substr(colon + 1);

    /* <init> do Object pode ser ignorado com seguranca */
    if (cls == "java/lang/Object" && mname == "<init>") {
        frame_pop(frame); /* descarta referencia this */
        return;
    }

    ClassEntry *ce = auto_load_class(jvm, cls);
    if (!ce) { fprintf(stderr,"Classe nao encontrada: %s\n", cls.c_str()); return; }

    ClassEntry *decl = NULL;
    method_info *m = find_method_ex(ce, mname, mdesc, &decl);
    if (!m) {
        /* Construtor de classe sintetica (JRE sem .class): no-op, descarta args */
        if (mname == "<init>") {
            int nparams = count_params(mdesc) + 1; /* +1 = this */
            for (int i = 0; i < nparams; i++) frame_pop(frame);
        } else {
            fprintf(stderr,"Metodo nao encontrado: %s.%s%s\n",
                    cls.c_str(), mname.c_str(), mdesc.c_str());
        }
        return;
    }

    invoke_method(jvm, frame, m, decl ? decl : ce, true, mdesc);
}

/**
 * @brief invokevirtual — invoca metodo de instancia com despacho polimorfico.
 *
 * @details Implementa o despacho virtual da JVM: o metodo executado e
 * determinado pelo tipo real do objeto em tempo de execucao (nao pelo tipo
 * declarado na instrucao).
 *   1. Le o indice de 2 bytes e resolve classe, nome e descriptor.
 *   2. Intercepta System.out.println/print via simulate_println(); se
 *      simulado, retorna imediatamente.
 *   3. Calcula a posicao de 'this' na operand stack: e o slot
 *      stack[top - nparams - 1] (abaixo de todos os argumentos).
 *   4. Recupera o JObject* correspondente ao ref via heap_get_object().
 *      Se o objeto existir, usa obj->klass como classe real para despacho.
 *      Se nao (ref sintetico), usa a classe da instrucao.
 *   5. Busca o metodo via find_method_ex(), que sobe a hierarquia ate
 *      encontrar a implementacao mais especifica.
 *   6. Invoca via invoke_method() com has_this = true.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame do metodo chamador.
 */
void op_invokevirtual(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx = (u2)((c[0]<<8)|c[1]);
    frame->pc += 2;

    const ClassFile *cf = frame->klass->cf;
    std::string cls  = resolve_class_name(cf, cf->constant_pool[idx].data.ref.class_index);
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t colon = nat.find(':');
    std::string mname = nat.substr(0, colon);
    std::string mdesc = nat.substr(colon + 1);

    /* Simula System.out.println / System.out.print */
    if (simulate_println(cls, mname, mdesc, jvm, frame)) return;

    /* Despacha polimorficamente: usa tipo real do objeto (virtual dispatch) */
    int nparams = count_params(mdesc);
    int32_t ref = frame->operand_stack[frame->operand_stack.size() - (size_t)(nparams + 1)];
    JObject *obj = heap_get_object(jvm, ref);

    ClassEntry *ce = obj ? obj->klass : auto_load_class(jvm, cls);
    if (!ce) { fprintf(stderr,"Classe nao encontrada: %s\n", cls.c_str()); return; }

    /* Busca o metodo e a classe que o declara (para CP correto no frame) */
    ClassEntry *decl = NULL;
    method_info *m = find_method_ex(ce, mname, mdesc, &decl);
    if (!m) { fprintf(stderr,"Metodo nao encontrado: %s.%s%s\n", ce->name.c_str(), mname.c_str(), mdesc.c_str()); return; }

    invoke_method(jvm, frame, m, decl ? decl : ce, true, mdesc);
}

/**
 * @brief invokeinterface — invoca metodo de interface com despacho pelo tipo real.
 *
 * @details Semanticamente similar a invokevirtual: o despacho e feito pelo
 * tipo real do objeto. A diferenca e o formato da instrucao no bytecode:
 * alem do indice de 2 bytes, ha 2 bytes adicionais (count e 0) que sao
 * consumidos mas ignorados nesta implementacao.
 *   1. Le o indice de 2 bytes e avanca pc += 4 (indice + count + zero).
 *   2. Resolve classe, nome e descriptor.
 *   3. Localiza 'this' na operand stack e obtem o tipo real via heap.
 *   4. Busca o metodo via find_method() (sem subir hierarquia extra, pois
 *      find_method ja lida com heranca).
 *   5. Invoca via invoke_method() com has_this = true.
 *
 * @param jvm    JVM em execucao.
 * @param frame  Frame do metodo chamador.
 */
void op_invokeinterface(JVM *jvm, Frame *frame) {
    u1 *c = frame->method->code_attr->code + frame->pc;
    u2 idx   = (u2)((c[0]<<8)|c[1]);
    /* c[2] = count, c[3] = 0 — lidos mas ignorados */
    frame->pc += 4;

    const ClassFile *cf = frame->klass->cf;
    std::string cls  = resolve_class_name(cf, cf->constant_pool[idx].data.ref.class_index);
    std::string nat  = resolve_nameandtype(cf, cf->constant_pool[idx].data.ref.name_and_type_index);
    size_t colon = nat.find(':');
    std::string mname = nat.substr(0, colon);
    std::string mdesc = nat.substr(colon + 1);

    int nparams = count_params(mdesc);
    int32_t ref = frame->operand_stack[frame->operand_stack.size() - (size_t)(nparams + 1)];
    JObject *obj = heap_get_object(jvm, ref);

    ClassEntry *ce = obj ? obj->klass : auto_load_class(jvm, cls);
    if (!ce) { fprintf(stderr,"Classe nao encontrada: %s\n", cls.c_str()); return; }

    method_info *m = find_method(ce, mname, mdesc);
    if (!m) { fprintf(stderr,"Metodo nao encontrado: %s.%s%s\n", ce->name.c_str(), mname.c_str(), mdesc.c_str()); return; }

    invoke_method(jvm, frame, m, ce, true, mdesc);
}