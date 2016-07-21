package org.zheltkov.heapview;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * Created by alex on 15.07.2016.
 */
public class Heapview {

    private String value = "";

    private ArrayList<Heapview> heapview = new ArrayList<Heapview>();

    public Heapview ref;

    public Heapview(String value) {
        this.value = value;
    }

    public static void main(String[] args) {

        Heapview heapview1 = new Heapview("heapview1");
        Heapview heapview2 = new Heapview("heapview2");
        Heapview heapview3 = new Heapview("heapview3");
        Heapview heapview4 = new Heapview("heapview4");

//      heapview1.addHeapview(heapview2);
//      heapview1.addHeapview(heapview3);
//      heapview1.addHeapview(heapview4);

//        heapview4.addHeapview(heapview2);
//        heapview2.addHeapview(heapview3);
//        heapview3.addHeapview(heapview4);

        // String output = heapview4.referenceInfo();

        //heapview1.addHeapview(heapview2);
        //heapview1.ref = heapview3;


        HashMap<String, ArrayList> map = new HashMap<>();
        ArrayList<Heapview> heapList = new ArrayList<>();
        map.put("list", heapList);

        heapList.add(heapview1);
        heapList.add(heapview3);
        heapList.add(heapview4);

        heapview1.ref = heapview2;
        heapview1.addHeapview(heapview2);
        heapview1.addHeapview(heapview3);

        heapview1.references(map);

        heapview1.test();
        heapview2.test();
        heapview3.test();
        heapview4.test();


        //System.out.println(output);
    }

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

    public void addHeapview(Heapview heapview) {
        this.heapview.add(heapview);
    }
}
