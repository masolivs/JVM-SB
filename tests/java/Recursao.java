/* Testa recursao mútua e varios casos de retorno */
public class Recursao {
    public static int somatorio(int n) {
        if (n <= 0) return 0;
        return n + somatorio(n - 1);
    }

    public static int hanoi(int n) {
        if (n == 1) return 1;
        return 2 * hanoi(n - 1) + 1;
    }

    public static int ackermann(int m, int n) {
        if (m == 0) return n + 1;
        if (n == 0) return ackermann(m - 1, 1);
        return ackermann(m - 1, ackermann(m, n - 1));
    }

    public static void main(String[] args) {
        System.out.println(somatorio(10));      /* 55  */
        System.out.println(somatorio(100));     /* 5050 */
        System.out.println(hanoi(5));           /* 31  */
        System.out.println(ackermann(3, 4));    /* 125 */
    }
}
