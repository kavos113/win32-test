package java_window3.java;

public class Main {

    static {
        System.loadLibrary("java_window3");
    }

    public static void main(String[] args) {
        Window w = new Window("Hello, World!");
        Button b = new Button(w, "Press me!");
        System.out.println("Window hwnd: " + Long.toHexString(w.hwnd));
        System.out.println("Button hwnd: " + Long.toHexString(b.hwnd));
        w.show();
    }
}
