package windowjava;

public class Window extends Component {

    private native void createWindow(String windowName);

    public Window(String windowName) {
        create(windowName);
    }

    protected void create() {
        create("Sample Window");
    }

    protected void create(String windowName) {
        createWindow(windowName);
    }
}
