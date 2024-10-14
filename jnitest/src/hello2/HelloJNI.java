package hello2;

public class HelloJNI {
    public native void sayHello(String name);

    static {
        System.loadLibrary("hello2");
    }

    public static void main(String[] args) {
        new HelloJNI().sayHello("Java");
    }
}
