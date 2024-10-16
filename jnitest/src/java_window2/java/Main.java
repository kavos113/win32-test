package java_window2.java;

public class Main {
    public static void main(String[] args) {
        Window w = new Window("Hello World");
        Button b = new Button(w, "Click me");

        w.show();
    }
}
