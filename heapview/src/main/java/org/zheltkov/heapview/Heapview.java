package org.zheltkov.heapview;

import java.util.concurrent.TimeUnit;

/**
 * Created by alex on 15.07.2016.
 */
public class Heapview {

    private String value = "test static value";

    private static native int references();

    private static native int instances();

    static String fromNative() {
        int references = 0;// references();
        int instances = instances();

        return String.format("\nClass references %d\nClass instances %d\n", references, instances);
    }

    public void test() {
        value = "test";
    }

    public static void main(String[] args) {
        Heapview heapview1 = new Heapview();

        Heapview heapview2 = new Heapview();

        String output = heapview2.fromNative();

        heapview1.test();
        heapview2.test();

        //System.out.println(output);
    }
}
