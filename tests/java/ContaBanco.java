/* Testa heranca, polimorfismo e campos de instancia */
public class ContaBanco {
    int saldo;

    public ContaBanco(int saldoInicial) {
        this.saldo = saldoInicial;
    }

    public void depositar(int valor) {
        this.saldo += valor;
    }

    public void sacar(int valor) {
        if (valor <= this.saldo) {
            this.saldo -= valor;
        }
    }

    public int getSaldo() {
        return this.saldo;
    }

    public static void main(String[] args) {
        ContaBanco c = new ContaBanco(1000);
        c.depositar(500);
        System.out.println(c.getSaldo()); /* 1500 */
        c.sacar(200);
        System.out.println(c.getSaldo()); /* 1300 */
        c.sacar(9999);
        System.out.println(c.getSaldo()); /* 1300 — saque negado */
    }
}
