package org.zheltkov.heapview;

import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by alex on 17.07.2016.
 */
@WebServlet(name = "HeapServlet", urlPatterns = "/heap")
public class HeapServlet extends HttpServlet {

    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {

        response.setContentType("text/html");

        String heapInfo;
        try {
            heapInfo = new Heapview("web").instanceInfo();
        } catch (UnsatisfiedLinkError e) {
            heapInfo = "None";
        }

        PrintWriter writer = response.getWriter();
        writer.println("Heap view info.");
        writer.println(heapInfo);
        writer.close();
    }
}
