package access;

public class Java {
    private int intPrivate = 0;
    protected int intProtected = 1;
    public int intPublic = 2;
    private static int intStaticPrivate = 3;
    protected static int intStaticProtected = 4;
    public static int intStaticPublic = 5;

    public void printPrivate() {
        System.out.println("intPrivate: " + intPrivate);
    }

    public void printProtected() {
        System.out.println("intProtected: " + intProtected);
    }

    public void printStaticPrivate() {
        System.out.println("intStaticPrivate: " + intStaticPrivate);
    }

    public void printStaticProtected() {
        System.out.println("intStaticProtected: " + intStaticProtected);
    }

    public native void setPrivate(int value);
    public native void setStaticPrivate(int value);

}
