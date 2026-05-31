/* Testa int, long, float, double e conversoes */
public class TiposNumericos {
    public static void main(String[] args) {
        /* int */
        int a = 2147483647;
        System.out.println(a);          /* Integer.MAX_VALUE */

        /* long */
        long b = 9876543210L;
        System.out.println(b);          /* 9876543210 */

        /* float */
        float c = 3.14f;
        System.out.println(c);          /* 3.14 */

        /* double */
        double d = 2.718281828;
        System.out.println(d);          /* 2.718281828 */

        /* conversoes */
        int   i2l_src = 42;
        long  i2l_dst = (long) i2l_src;
        System.out.println(i2l_dst);    /* 42 */

        double f2d_src = 1.5f;
        System.out.println(f2d_src);    /* 1.5 */

        long   big  = 1000000000L * 3;
        System.out.println(big);        /* 3000000000 */
    }
}
