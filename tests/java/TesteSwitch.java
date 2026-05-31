public class TesteSwitch {
    public static void main(String[] args) {
        for (int i = 0; i <= 4; i++) {
            switch (i) {
                case 0: System.out.println("zero");  break;
                case 1: System.out.println("um");    break;
                case 2: System.out.println("dois");  break;
                case 3: System.out.println("tres");  break;
                default: System.out.println("outro"); break;
            }
        }
    }
}
