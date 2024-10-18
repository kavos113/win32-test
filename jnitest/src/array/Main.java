package array;

public class Main {

    static {
        System.loadLibrary("array");
    }

    public native static String[] newArray(int size);
    public native static void printArray(String[] array);

    public static void main(String[] args) {
        String[] array = newArray(10);
        for (int i = 0; i < array.length; i++) {
            System.out.println("array[" + i + "]: " + array[i]);
        }

        printArray(array);
    }

}
