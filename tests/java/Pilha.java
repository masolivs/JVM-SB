/* Pilha de inteiros usando array — testa campos de instancia + metodos */
public class Pilha {
    int[] dados;
    int   topo;

    public Pilha(int capacidade) {
        this.dados = new int[capacidade];
        this.topo  = -1;
    }

    public void push(int v) {
        this.topo++;
        this.dados[this.topo] = v;
    }

    public int pop() {
        int v = this.dados[this.topo];
        this.topo--;
        return v;
    }

    public int peek() {
        return this.dados[this.topo];
    }

    public int tamanho() {
        return this.topo + 1;
    }

    public static void main(String[] args) {
        Pilha p = new Pilha(10);
        p.push(10);
        p.push(20);
        p.push(30);
        System.out.println(p.tamanho()); /* 3  */
        System.out.println(p.peek());    /* 30 */
        System.out.println(p.pop());     /* 30 */
        System.out.println(p.pop());     /* 20 */
        System.out.println(p.tamanho()); /* 1  */
    }
}
