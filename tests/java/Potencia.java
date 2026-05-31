public class Potencia {
    public static long potencia(long base, int exp) {
        long resultado = 1;
        while (exp > 0) {
            if (exp % 2 == 1) resultado *= base;
            base *= base;
            exp /= 2;
        }
        return resultado;
    }

    public static void main(String[] args) {
        System.out.println(potencia(2, 10));   /* 1024       */
        System.out.println(potencia(3, 5));    /* 243        */
        System.out.println(potencia(2, 30));   /* 1073741824 */
    }
}
