package java_window4.java;

public class Text extends Component {

    String text;

    private native void setNativeText(String text);
    private native void setTextColor(int color);
    private native void setFont(String family, float size, float stretch, int style, int weight, float lineHeight, int operation);

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

    private static final int SET_FONT_FAMILY = 0b1;
    private static final int SET_FONT_SIZE = 0b10;
    private static final int SET_FONT_STRETCH = 0b100;
    private static final int SET_FONT_STYLE = 0b1000;
    private static final int SET_FONT_WEIGHT = 0b10000;
    private static final int SET_FONT_LINE_HEIGHT = 0b100000;
}
