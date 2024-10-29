package java_window4.test;

import java_window4.java.Color;
import java_window4.java.Text;
import java_window4.java.TextHorizontalAlignment;
import java_window4.java.TextVerticalAlignment;
import java_window4.java.Window;

public class TextTest {

    static {
        System.loadLibrary("java_window4");
    }

    public static void main(String[] args) {
        Window window = new Window("Text Test");
        window.setRectangle(300, 300, 640, 480);
        window.setBackgroundColor(Color.LIGHT_GRAY);

        Text text = new Text(window, "Hello, World!");
        text.setRectangle(0, 0, 640, 400);
        text.setText("This is a text component. Who knows what will happen next?");
        text.setBackgroundColor(Color.AZURE2);
        text.setTextColor(Color.RED);
        text.setTextHorizontalAlignment(TextHorizontalAlignment.JUSTIFY);
        text.setTextVerticalAlignment(TextVerticalAlignment.CENTER);

        window.show();
        
        window.release();
        text.release();
    }
}
