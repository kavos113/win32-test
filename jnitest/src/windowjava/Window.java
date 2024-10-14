package windowjava;

public class Window extends Component {
    public Window(String windowName) {
        createWindowClass("WindowClass");
        createComponent(null, "WindowClass", windowName);
    }
}
