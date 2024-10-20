package java_window3.java;

public abstract class Component {
    protected volatile long hwnd = 0;

    protected abstract void create(Component parent, String title);

    private native void reshape(long hwnd, int x, int y, int width, int height);

    public void setPosition(int x, int y, int width, int height) {
        reshape(hwnd, x, y, width, height);
    }

}
