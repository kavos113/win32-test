package window;

public class WindowTest {
    public native void CreateWindow(String windowName);
    public native void ShowWindow();

    static {
        System.loadLibrary("window_test");
    }

    public static void main(String[] args) {
        WindowTest window = new WindowTest();
        window.CreateWindow("Sample Window");
        window.ShowWindow();
    }
}
