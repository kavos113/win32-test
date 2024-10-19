package java_window4.java;

public class Window extends Component {

    @Override
    protected native void create(Component parent, String title);

    private native void showWindow();

    public Window(String title) {
        create(null, title);
    }

    public Window(Window parent, String title) {
        create(parent, title);
    }

    public void show() {
        showWindow();
    }

}
