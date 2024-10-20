package java_window4.java;

public class Text extends Component {

    String text;

    private native void setNativeText(String text);

    @Override
    protected native void create(Component parent, String text);

    public Text(Component parent, String text) {
        create(parent, text);
    }

    public void setText(String text) {
        this.text = text;
        setNativeText(text);
    }

    public String getText() {
        return text;
    }

}
