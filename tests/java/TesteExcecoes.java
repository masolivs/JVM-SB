public class TesteExcecoes {

    /* Testa catch de ArrayIndexOutOfBoundsException */
    static void testeArrayBounds() {
        try {
            int[] arr = new int[3];
            arr[10] = 42;
            System.out.println(0); /* nao deve chegar aqui */
        } catch (ArrayIndexOutOfBoundsException e) {
            System.out.println(1); /* deve imprimir 1 */
        }
    }

    /* Testa catch de ArithmeticException (divisao por zero) */
    static void testeDivisaoZero() {
        try {
            int x = 10 / 0;
            System.out.println(0);
        } catch (ArithmeticException e) {
            System.out.println(2);
        }
    }

    /* Testa finally */
    static void testeFinally() {
        try {
            int[] arr = new int[1];
            arr[5] = 1;
        } catch (ArrayIndexOutOfBoundsException e) {
            System.out.println(3);
        } finally {
            System.out.println(4); /* sempre executa */
        }
    }

    /* Testa excecao lancada em metodo chamado (propagacao entre frames) */
    static int metodoQueJoga(int x) {
        if (x == 0) throw new ArithmeticException();
        return 100 / x;
    }

    static void testeProxagacao() {
        try {
            metodoQueJoga(0);
            System.out.println(0);
        } catch (ArithmeticException e) {
            System.out.println(5);
        }
    }

    /* Testa catch generico com Exception */
    static void testeCatchGenerico() {
        try {
            int[] arr = new int[2];
            arr[99] = 1;
        } catch (Exception e) {
            System.out.println(6);
        }
    }

    public static void main(String[] args) {
        testeArrayBounds();   /* 1 */
        testeDivisaoZero();   /* 2 */
        testeFinally();       /* 3 4 */
        testeProxagacao();    /* 5 */
        testeCatchGenerico(); /* 6 */
        System.out.println(7); /* chegou ao fim */
    }
}
