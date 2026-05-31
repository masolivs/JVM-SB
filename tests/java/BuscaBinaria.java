public class BuscaBinaria {
    public static int busca(int[] arr, int alvo) {
        int esq = 0;
        int dir = arr.length - 1;
        while (esq <= dir) {
            int meio = esq + (dir - esq) / 2;
            if (arr[meio] == alvo) return meio;
            if (arr[meio] < alvo)  esq = meio + 1;
            else                   dir = meio - 1;
        }
        return -1;
    }

    public static void main(String[] args) {
        int[] v = {1, 3, 5, 7, 9, 11, 13, 15};
        System.out.println(busca(v, 7));   /* 3  */
        System.out.println(busca(v, 1));   /* 0  */
        System.out.println(busca(v, 15));  /* 7  */
        System.out.println(busca(v, 4));   /* -1 */
    }
}
