package java_window2.java;

public abstract class Component {

    protected abstract void create(Component parent, String title);

    private native void reshape(int x, int y, int width, int height);

    static {
        System.loadLibrary("window_java");
    }

    public void setPosition(int x, int y, int width, int height) {
        reshape(x, y, width, height);
    }
}
