package access;

public class Java {
    private int intPrivate = 0;
    private static int intStaticPrivate = 3;

    public void printPrivate() {
        System.out.println("intPrivate: " + intPrivate);
    }

    public void printStaticPrivate() {
        System.out.println("intStaticPrivate: " + intStaticPrivate);
    }

    public native void setPrivate(int value);
    public native void setStaticPrivate(int value);

}
