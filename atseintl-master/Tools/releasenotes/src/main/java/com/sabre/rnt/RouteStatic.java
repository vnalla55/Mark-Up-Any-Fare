package com.sabre.rnt;

import org.apache.commons.io.FileUtils;
import spark.Request;
import spark.Response;
import spark.Route;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

public class RouteStatic extends Route {

    RouteStatic() {
        super("/static/*");
    }

    private static String filenameToContentType(String filename) {
        if (filename.endsWith(".jpg")) {
            return "image/jpeg;charset=utf-8";
        } else if (filename.endsWith(".gif")) {
            return "image/gif;charset=utf-8";
        } else if (filename.endsWith(".png")) {
            return "image/png;charset=utf-8";
        } else if (filename.endsWith(".js")) {
            return "application/x-javascript";
        } else if (filename.endsWith(".css")) {
            return "text/css;charset=utf-8";
        } else if (filename.endsWith(".htm") || filename.endsWith(".html")) {
            return "text/html;charset=utf-8";
        }
        return "text/plain;charset=utf-8";
    }

    public Object handle(Request request, Response response) {
        response.header("Cache-Control", "max-age=" + (60 * 60 * 24 * 2));
        byte[] out = null;
        try {
            String filename = request.pathInfo().substring(1);
            out = FileUtils.readFileToByteArray(new File(filename));
            response.raw().setContentType(filenameToContentType(filename));
            response.raw().getOutputStream().write(out, 0, out.length);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return new String(out);
    }
}
