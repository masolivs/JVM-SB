#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "method_area.h"
#include "jvm_stack.h"
#include "object.h"
#include "array.h"
#include <vector>

struct JVM;

/**
 * @brief Tipo de um handler de opcode.
 *
 * Cada instrucao JVM e implementada como uma funcao desse tipo.
 * O handler le seus operandos de frame->method->code_attr->code[frame->pc]
 * e avanca frame->pc o numero correto de bytes.
 */
typedef void (*OpcodeHandler)(JVM *jvm, Frame *frame);

/**
 * @brief Entrada da heap table — diferencia objeto de array.
 */
typedef enum { HEAP_NULL = 0, HEAP_OBJECT, HEAP_ARRAY } HeapTag;

struct HeapEntry {
    HeapTag tag;
    void   *ptr; /**< JObject* ou JArray* conforme tag */
};

/**
 * @brief Estrutura principal da JVM.
 *
 * Reune a area de metodos, a pilha de frames e a dispatch table O(1).
 * heap[0] e sempre a referencia nula (null).
 */
struct JVM {
    MethodArea    *method_area;
    JvmStack      *stack;
    OpcodeHandler  dispatch[256]; /**< dispatch table indexada pelo byte do opcode */
    std::vector<HeapEntry> heap;  /**< heap[0] = null; indices positivos = objetos/arrays */
    std::string    class_dir;     /**< diretorio onde buscar .class automaticamente */
    std::string    main_class;    /**< nome interno da classe principal carregada em jvm_create */

    /* Estado de excecao — setado por jvm_throw(), consumido pelo interpret loop */
    bool        exception_pending; /**< true quando ha excecao propagando */
    std::string exception_class;   /**< nome interno da classe da excecao */
    int32_t     exception_ref;     /**< referencia na heap (0 para excecoes sinteticas) */
    uint32_t    exception_pc;      /**< PC da instrucao que lancou a excecao */
};

/**
 * @brief Sinaliza uma excecao pendente na JVM.
 *
 * Opcodes internos (array bounds, null pointer, aritmetica) e op_athrow
 * chamam esta funcao em vez de exit(). O interpret loop captura e busca
 * o handler na exception_table do frame corrente.
 *
 * @param jvm        JVM em execucao.
 * @param class_name Nome interno da classe da excecao, ex: "java/lang/ArrayIndexOutOfBoundsException".
 * @param ref        Referencia na heap (0 para excecoes sinteticas sem objeto real).
 */
void jvm_throw(JVM *jvm, const std::string &class_name, int32_t ref = 0);

/**
 * @brief Busca uma classe, carregando-a automaticamente do class_dir se necessario.
 *
 * Tenta find_class primeiro; se nao encontrar, faz load_class a partir de
 * class_dir/NomeClasse.class. Retorna NULL se o arquivo nao existir.
 *
 * @param jvm   JVM em execucao.
 * @param name  Nome interno da classe, ex: "Jogador".
 * @return ClassEntry* carregada ou NULL.
 */
ClassEntry *auto_load_class(JVM *jvm, const std::string &name);

/**
 * @brief Preenche a dispatch table com os handlers de cada opcode.
 *
 * Opcodes nao implementados apontam para op_not_implemented.
 *
 * @param jvm  JVM a inicializar.
 */
void init_dispatch_table(JVM *jvm);

/**
 * @brief Loop principal de interpretacao.
 *
 * Executa ate a pilha de frames ficar vazia:
 *   frame = jvm_stack_current(stack)
 *   op    = code[frame->pc++]
 *   dispatch[op](jvm, frame)
 *
 * @param jvm  JVM em execucao.
 */
void interpret(JVM *jvm);

/**
 * @brief Cria um frame para o metodo e o empurra na pilha.
 *
 * O loop interpret() executa a partir desse frame.
 *
 * @param jvm    JVM em execucao.
 * @param m      Metodo a invocar.
 * @param klass  Classe dona do metodo.
 */
void execute_method(JVM *jvm, method_info *m, ClassEntry *klass);

/**
 * @brief Cria e inicializa uma JVM completa.
 *
 * Carrega a classe principal a partir do caminho dado,
 * inicializa a dispatch table e prepara o heap (heap[0] = null).
 *
 * @param class_path  Caminho para o arquivo .class principal.
 * @return JVM criada ou NULL em erro.
 */
JVM *jvm_create(const char *class_path);

/**
 * @brief Executa o metodo main da classe principal.
 *
 * @param jvm  JVM inicializada por jvm_create.
 */
void jvm_run(JVM *jvm);

/**
 * @brief Libera todos os recursos da JVM.
 *
 * @param jvm  JVM a destruir; pode ser NULL.
 */
void jvm_destroy(JVM *jvm);

/* ------------------------------------------------------------------ */
/* Helpers para acesso ao heap da JVM                                   */
/* ------------------------------------------------------------------ */

/**
 * @brief Aloca um JObject no heap da JVM e retorna o indice (referencia).
 *
 * @param jvm    JVM.
 * @param klass  Classe do objeto.
 * @return Indice int32_t na heap table (nunca 0; 0 = null).
 */
int32_t heap_alloc_object(JVM *jvm, ClassEntry *klass);

/**
 * @brief Aloca um JArray no heap da JVM e retorna o indice.
 *
 * @param jvm     JVM.
 * @param type    Tipo do array.
 * @param length  Tamanho do array.
 * @return Indice int32_t na heap table.
 */
int32_t heap_alloc_array(JVM *jvm, ArrayType type, int32_t length);

/**
 * @brief Retorna o JObject* associado ao indice do heap.
 *
 * @param jvm  JVM.
 * @param ref  Indice na heap table.
 * @return JObject* ou NULL se ref == 0.
 */
JObject *heap_get_object(JVM *jvm, int32_t ref);

/**
 * @brief Retorna o JArray* associado ao indice do heap.
 *
 * @param jvm  JVM.
 * @param ref  Indice na heap table.
 * @return JArray* ou NULL se ref == 0.
 */
JArray *heap_get_array(JVM *jvm, int32_t ref);

#endif /* INTERPRETER_H */