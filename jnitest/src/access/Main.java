package access;

public class Main {
    static {
        System.loadLibrary("access");
    }

    public static void main(String[] args) {
        Java java = new Java();
        java.printPrivate();
        java.printStaticPrivate();
        java.setPrivate(10);
        java.printPrivate();
        java.setStaticPrivate(20);
        java.printStaticPrivate();
    }
}

