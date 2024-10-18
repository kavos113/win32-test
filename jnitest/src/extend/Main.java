package extend;

public class Main {

    static {
        System.loadLibrary("extend");
    }

    public static native void nativeHello(Super sup);

    public static void main(String[] args) {
        Sub sub = new Sub("Sub");
        System.out.println(sub.hello());
        nativeHello(sub);
    }

}
