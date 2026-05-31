public class Fatorial {
    public static int fatorial(int n) {
        if (n <= 1) return 1;
        return n * fatorial(n - 1);
    }

    public static void main(String[] args) {
        for (int i = 0; i <= 10; i++) {
            System.out.println(fatorial(i));
        }
    }
}
