public class MatrizMult {
    public static void main(String[] args) {
        int[][] a = { {1, 2}, {3, 4} };
        int[][] b = { {5, 6}, {7, 8} };
        int n = 2;
        int[] c0 = new int[n];
        int[] c1 = new int[n];

        for (int j = 0; j < n; j++) {
            c0[j] = 0;
            for (int k = 0; k < n; k++) {
                c0[j] = c0[j] + a[0][k] * b[k][j];
            }
        }
        for (int j = 0; j < n; j++) {
            c1[j] = 0;
            for (int k = 0; k < n; k++) {
                c1[j] = c1[j] + a[1][k] * b[k][j];
            }
        }
        /* resultado: [[19,22],[43,50]] */
        System.out.println(c0[0]);
        System.out.println(c0[1]);
        System.out.println(c1[0]);
        System.out.println(c1[1]);
    }
}
