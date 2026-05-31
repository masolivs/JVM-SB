#ifndef METHOD_AREA_H
#define METHOD_AREA_H

#include "class_file.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

/**
 * @brief Entrada da area de metodos — representa uma classe carregada.
 *
 * Armazena o ClassFile parseado, os campos estaticos e a cadeia de heranca.
 * static_fields e inicializado com zeros; cresce conforme fields estaticos.
 */
struct ClassEntry {
    std::string          name;          /**< nome interno, ex: "HelloWorld" */
    ClassFile           *cf;            /**< ClassFile parseado */
    std::vector<int32_t> static_fields; /**< valores dos campos estaticos */
    bool                 initialized;   /**< clinit ja foi executado? */
    ClassEntry          *super;         /**< ClassEntry da superclasse ou NULL */
};

/**
 * @brief Area de metodos da JVM — armazena todas as classes carregadas.
 *
 * Usa unordered_map para busca O(1) por nome.
 */
struct MethodArea {
    std::unordered_map<std::string, ClassEntry *> classes;
};

/**
 * @brief Aloca e inicializa uma MethodArea vazia.
 *
 * @return Ponteiro para a MethodArea criada.
 */
MethodArea *method_area_create(void);

/**
 * @brief Libera todas as classes carregadas e a propria MethodArea.
 *
 * @param ma  MethodArea a destruir; pode ser NULL.
 */
void method_area_destroy(MethodArea *ma);

/**
 * @brief Carrega um .class do disco e registra na MethodArea.
 *
 * Faz read_class_file(), cria ClassEntry, tenta resolver a superclasse
 * recursivamente. Nao recarrega se a classe ja estiver na map.
 *
 * @param ma    MethodArea destino.
 * @param name  Nome interno da classe, ex: "HelloWorld".
 * @param path  Caminho do arquivo .class.
 * @return ClassEntry carregada ou NULL em erro.
 */
ClassEntry *load_class(MethodArea *ma, const std::string &name,
                       const std::string &path);

/**
 * @brief Busca uma classe ja carregada pelo nome interno.
 *
 * @param ma    MethodArea.
 * @param name  Nome interno da classe.
 * @return ClassEntry ou NULL se nao encontrada.
 */
ClassEntry *find_class(const MethodArea *ma, const std::string &name);

/**
 * @brief Busca um metodo na classe e em suas superclasses.
 *
 * @param ce    ClassEntry de partida.
 * @param name  Nome do metodo, ex: "main".
 * @param desc  Descriptor, ex: "([Ljava/lang/String;)V".
 * @return method_info* ou NULL se nao encontrado.
 */
method_info *find_method(const ClassEntry *ce, const std::string &name,
                          const std::string &desc);

/**
 * @brief Busca um metodo e retorna a classe que o declara.
 *
 * Necessario para que o frame use o CP correto (da classe declarante,
 * nao da classe do objeto em runtime).
 *
 * @param ce            ClassEntry de partida (classe do objeto).
 * @param name          Nome do metodo.
 * @param desc          Descriptor.
 * @param declaring_ce  Saida: classe onde o metodo foi encontrado.
 * @return method_info* ou NULL.
 */
method_info *find_method_ex(const ClassEntry *ce, const std::string &name,
                             const std::string &desc, ClassEntry **declaring_ce);

/**
 * @brief Busca um campo (field) na classe e em suas superclasses.
 *
 * @param ce    ClassEntry de partida.
 * @param name  Nome do campo.
 * @param desc  Descriptor do campo.
 * @return field_info* ou NULL se nao encontrado.
 */
field_info *find_field(const ClassEntry *ce, const std::string &name,
                        const std::string &desc);

/**
 * @brief Conta o total de campos de instancia em toda a hierarquia.
 *
 * Campos da superclasse vem primeiro (indices menores).
 * Usado por object_new para alocar o vetor de fields.
 *
 * @param ce  ClassEntry topo da hierarquia.
 * @return Numero total de slots de instancia.
 */
int count_hierarchy_fields(const ClassEntry *ce);

/**
 * @brief Retorna o indice GLOBAL de um campo de instancia.
 *
 * O indice e relativo a toda a hierarquia (superclasse em indices menores).
 * Busca primeiro na propria classe e sobe a cadeia se nao encontrar.
 *
 * @param ce    ClassEntry onde o campo e declarado (ou subclasse).
 * @param name  Nome do campo.
 * @param desc  Descriptor.
 * @return Indice em cf->fields[] ou -1 se nao encontrado.
 */
int find_field_index(const ClassEntry *ce, const std::string &name,
                     const std::string &desc);

#endif /* METHOD_AREA_H */
