package com.sabre.rnt;

import spark.Request;
import spark.Response;
import spark.Route;

import java.io.PrintWriter;
import java.io.StringWriter;

public class RouteStatus extends Route {

    private boolean exitWhenAllFlushed;

    RouteStatus(boolean exitWhenAllFlushed) {
        super("/status/:id");

        this.exitWhenAllFlushed = exitWhenAllFlushed;
    }

    public Object handle(Request request, Response response) {
        try {
            Publisher job = ReleaseNotes.getJobs().get(request.params(":id"));
            if (job == null) {
                response.status(404);
                return "Requested job doesn't exist (maybe it has timed out)\n";
            }

            if (exitWhenAllFlushed) new Thread() {
                public void run() {
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException ignored) {
                    } finally {
                        System.out.println("Publishing is finished, exiting.");
                        System.exit(0);
                    }
                }
            }.start();
            return job.getOutput();
        } catch (Exception ex) {
            StringWriter sw = new StringWriter();
            ex.printStackTrace(new PrintWriter(sw));
            String stacktrace = sw.toString();

            System.err.println("THROW:");
            System.err.println(stacktrace);
            throw ex;
        }
    }
}
