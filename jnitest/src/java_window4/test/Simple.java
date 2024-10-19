package java_window4.test;

import java_window4.java.Window;

public class Simple {

    static {
        System.loadLibrary("java_window4");
    }

    public static void main(String[] args) {
        Window window = new Window("Simple Window");
        window.show();
    }
}
