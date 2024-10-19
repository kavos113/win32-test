package java_window4.test;

import java_window4.java.Window;

public class ChangeSize {

    static {
        System.loadLibrary("java_window4");
    }

    public static void main(String[] args) {
        Window window = new Window("Change Size");
        window.setRectangle(300, 300, 640, 480);
        // window.setSize(640, 480);
        // window.setPosition(1300, 300);
        window.show();
    }
}
