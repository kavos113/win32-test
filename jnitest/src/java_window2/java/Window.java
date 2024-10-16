package java_window2.java;

public class Window extends Component {

    @Override
    protected native void create(Component parent, String title);

    private native void showWinodw();

    public Window(String windowTitle) {
        create(null, windowTitle);
    }

    public Window(Window parent, String windowTitle) {
        create(parent, windowTitle);
    }

    public void show() {
        showWinodw();
    }
}
