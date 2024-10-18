package extend;

public class Sub extends Super {

    public Sub(String name) {
        super(name);
    }

    public String hello() {
        return "Hello, " + name + " from Sub";
    }

}
