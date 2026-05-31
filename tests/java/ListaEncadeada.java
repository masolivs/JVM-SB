/* Testa objetos com referencias entre si (ponteiros) */
public class ListaEncadeada {
    int valor;
    ListaEncadeada prox;

    public ListaEncadeada(int valor) {
        this.valor = valor;
        this.prox  = null;
    }

    public static ListaEncadeada inserir(ListaEncadeada head, int v) {
        ListaEncadeada novo = new ListaEncadeada(v);
        novo.prox = head;
        return novo;
    }

    public static void imprimir(ListaEncadeada head) {
        ListaEncadeada atual = head;
        while (atual != null) {
            System.out.println(atual.valor);
            atual = atual.prox;
        }
    }

    public static int tamanho(ListaEncadeada head) {
        int count = 0;
        ListaEncadeada atual = head;
        while (atual != null) {
            count++;
            atual = atual.prox;
        }
        return count;
    }

    public static void main(String[] args) {
        ListaEncadeada lista = null;
        lista = inserir(lista, 10);
        lista = inserir(lista, 20);
        lista = inserir(lista, 30);
        System.out.println(tamanho(lista)); /* 3        */
        imprimir(lista);                     /* 30 20 10 */
    }
}
