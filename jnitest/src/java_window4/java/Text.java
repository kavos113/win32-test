package java_window4.java;

public class Text extends Component {

    String text;

    private native void setNativeText(String text);
    private native void setTextColor(int color);

    public native void setTextHorizontalAlignment(TextHorizontalAlignment alignment);
    public native void setTextVerticalAlignment(TextVerticalAlignment alignment);

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

    public void setTextColor(Color color) {
        setTextColor(color.getColorValue());
    }

}
