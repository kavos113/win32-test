package extend;

public class Super {

    protected String name;

    public Super(String name) {
        this.name = name;
    }

    public String hello() {
        return "Hello, " + name + "from Super";
    }

}
