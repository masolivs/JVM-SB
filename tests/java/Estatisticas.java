/* Testa float/double em operacoes reais */
public class Estatisticas {
    public static double media(int[] v) {
        long soma = 0;
        for (int i = 0; i < v.length; i++) soma += v[i];
        return (double) soma / v.length;
    }

    public static int maximo(int[] v) {
        int max = v[0];
        for (int i = 1; i < v.length; i++) {
            if (v[i] > max) max = v[i];
        }
        return max;
    }

    public static int minimo(int[] v) {
        int min = v[0];
        for (int i = 1; i < v.length; i++) {
            if (v[i] < min) min = v[i];
        }
        return min;
    }

    public static void main(String[] args) {
        int[] v = {4, 7, 2, 9, 1, 5, 8, 3, 6};
        System.out.println(media(v));   /* 5.0   */
        System.out.println(maximo(v));  /* 9     */
        System.out.println(minimo(v));  /* 1     */
    }
}
