package java_window4.test;

import java_window4.java.Text;
import java_window4.java.Window;

public class TextTest {

    static {
        System.loadLibrary("java_window4");
    }

    public static void main(String[] args) {
        Window window = new Window("Text Test");
        window.setRectangle(300, 300, 640, 480);

        Text text = new Text(window, "Hello, World!");
        text.setRectangle(10, 10, 200, 100);
        text.setText("Hello, Java!");

        window.show();
    }
}
