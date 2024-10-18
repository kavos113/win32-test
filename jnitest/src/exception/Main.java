package exception;

public class Main {
    static {
        System.loadLibrary("exception");
    }

    public static native void nativeThrowException(String message) throws Exception;
    public static native void nativeThrowNewException() throws Exception;

    public static void main(String[] args) {
        try {
            nativeThrowException("Exception message");
        } catch (Exception e) {
            e.printStackTrace();
        }

        try {
            nativeThrowNewException();
        } catch (Exception e) {
            System.out.println("e.getMessage() = " + e.getMessage());
            e.printStackTrace();
        }

        System.out.println("Main.main() done");
    }
}
