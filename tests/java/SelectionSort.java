public class SelectionSort {
    public static void selectionSort(int[] arr) {
        int n = arr.length;
        for (int i = 0; i < n - 1; i++) {
            int minIdx = i;
            for (int j = i + 1; j < n; j++) {
                if (arr[j] < arr[minIdx]) {
                    minIdx = j;
                }
            }
            int temp      = arr[minIdx];
            arr[minIdx]   = arr[i];
            arr[i]        = temp;
        }
    }

    public static void main(String[] args) {
        int[] v = {29, 10, 14, 37, 13};
        selectionSort(v);
        for (int i = 0; i < v.length; i++) {
            System.out.println(v[i]);
        }
    }
}
