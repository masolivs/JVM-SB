public class Primos {
    public static boolean ehPrimo(int n) {
        if (n < 2) return false;
        for (int i = 2; i * i <= n; i++) {
            if (n % i == 0) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        /* Imprime todos os primos ate 50 */
        for (int i = 2; i <= 50; i++) {
            if (ehPrimo(i)) {
                System.out.println(i);
            }
        }
    }
}
