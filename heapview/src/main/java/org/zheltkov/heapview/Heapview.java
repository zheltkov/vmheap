package org.zheltkov.heapview;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * Created by alex on 15.07.2016.
 */
public class Heapview {

    private String value = "test static value";
    private ArrayList<Heapview> heapview = new ArrayList<Heapview>();

    public native int references(Object object);

    public native int instances();

    public String instanceInfo() {
        int instances = instances();
        return String.format("\nClass instances %d\n", instances);
    }

    public String referenceInfo() {
        return String.format("ref %d\n", references(this));
    }

    public void test() {
        value = "test";
    }

    public static void main(String[] args) {

        Heapview heapview1 = new Heapview();
        Heapview heapview2 = new Heapview();
        Heapview heapview3 = new Heapview();
        Heapview heapview4 = new Heapview();

        heapview1.addHeapview(heapview2);
        heapview1.addHeapview(heapview3);
        heapview1.addHeapview(heapview4);

        heapview4.addHeapview(heapview3);
        heapview3.addHeapview(heapview4);

        String output = heapview1.referenceInfo();

        heapview1.test();
        heapview2.test();
        heapview3.test();
        heapview4.test();


        //System.out.println(output);
    }

    public void addHeapview(Heapview heapview) {
        this.heapview.add(heapview);
    }
}
