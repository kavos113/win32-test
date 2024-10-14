package hello;

public class Hello {
    public native void sayHello();

    static {
        System.loadLibrary("hello");
    }

    public static void main(String[] args) {
        new Hello().sayHello();   
    }
}