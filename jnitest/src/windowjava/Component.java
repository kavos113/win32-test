package windowjava;

public class Component {

    private static native void CreateWindowClass(String className);
    private static native void CreateComponent(
            String className,
            String windowName
    );

    static {
        System.loadLibrary("window_java");
    }

    protected void createWindowClass(String className) {
        CreateWindowClass(className);
    }

    protected void createComponent(
            Component parent,
            String className,
            String windowName
    ) {
        CreateComponent(className, windowName);
    }

    public void setPosition(int x, int y, int width, int height) {

    }
}
