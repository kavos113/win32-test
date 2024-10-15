package windowjava;

public class Button extends Component {

    private native void createButton(String buttonName);

    public Button(String buttonName) {
        create(buttonName);
    }

    protected void create() {
        create("Sample Button");
    }

    protected void create(String buttonName) {
        createButton(buttonName);
    }
}
