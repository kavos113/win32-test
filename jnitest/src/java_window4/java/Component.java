package java_window4.java;

public abstract class Component {

    protected volatile long nativeWindow = 0;

    protected abstract void create(Component parent, String title);

    private native void destroy();
    private native void reshape(int x, int y, int width, int height, int operation);
    private native void setBackgroundColor(int color);

    private static final int SET_POSITION = 1;
    private static final int SET_SIZE = 2;
    private static final int SET_POSITION_AND_SIZE = 3;

    public void release() {
        destroy();
    }

    public void setRectangle(int x, int y, int width, int height) {
        reshape(x, y, width, height, SET_POSITION_AND_SIZE);
    }

    public void setPosition(int x, int y) {
        reshape(x, y, 0, 0, SET_POSITION);
    }

    public void setSize(int width, int height) {
        reshape(0, 0, width, height, SET_SIZE);
    }

    public void setBackgroundColor(Color color) {
        setBackgroundColor(color.getColorValue());
    }

    public long getNativeWindow() {
        return nativeWindow;
    }
}
