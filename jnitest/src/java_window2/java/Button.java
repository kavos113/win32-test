package java_window2.java;

public class Button extends Component {

        protected native void create(Component parent, String title);

        public Button(Component parent, String title) {
            create(parent, title);
        }
}
