public class Vetores {
    public static void main(String[] args) {
        int[] v = new int[5];
        for (int i = 0; i < 5; i++) {
            v[i] = i * i;
        }
        for (int i = 0; i < v.length; i++) {
            System.out.println(v[i]);
        }
    }
}
