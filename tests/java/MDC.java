public class MDC {
    public static int mdc(int a, int b) {
        while (b != 0) {
            int t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    public static int mmc(int a, int b) {
        return a / mdc(a, b) * b;
    }

    public static void main(String[] args) {
        System.out.println(mdc(48, 18));   /* 6  */
        System.out.println(mdc(100, 75));  /* 25 */
        System.out.println(mmc(4, 6));     /* 12 */
        System.out.println(mmc(12, 15));   /* 60 */
    }
}
