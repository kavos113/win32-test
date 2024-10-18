package java_window3.java;

public class Button extends Component {

    @Override
    protected native void create(Component parent, String title);

    public Button(Component parent, String title) {
        create(parent, title);
    }

}
