package org.zheltkov.heapview;

/**
 * Created by alex on 15.07.2016.
 */
public class Heapview {

    private static native int references();

    private static native int instances();

    static String fromNative() {
        int references = references();
        int instances = instances();
        return String.format("\nClass references %d\nClass instances %d\n", references, instances);
    }

    public static void main(String[] args) {
        Heapview heapview1 = new Heapview();
        Heapview heapview2 = new Heapview();

        System.out.println(heapview1.fromNative());
    }
}
