package com.sabre.rnt;

import org.apache.velocity.VelocityContext;
import spark.Request;
import spark.Response;
import spark.Route;

public class RouteWorker extends Route {

    RouteWorker() {
        super("/worker");
    }

    public Object handle(Request request, Response response) {
        try {
            response.header("Content-Type", "text/html; charset=utf-8");

            ReleaseName targetReleaseName = ReleaseNames.parseReleaseName(request.queryParams("targetRelease")),
                    origReleaseName = ReleaseNames.parseReleaseName(request.queryParams("origRelease"));

            assert(origReleaseName.getRecordType().equals(targetReleaseName.getRecordType()));

            Class<? extends Record> recordType = targetReleaseName.getRecordType();
            VelocityContext context = ReleaseNotes.createContext(request, recordType);

            String jobID = ReleaseNotes.nextRandomKey();
            context.put("jobID", jobID);

            Publisher job = new Publisher(
                    recordType,
                    request.queryParams("groupName"),
                    request.queryParams("releaseNote"),
                    targetReleaseName,
                    origReleaseName,
                    request.queryParams("origGroup"),
                    request.queryParams("digest"),
                    ReleaseNotes.Action.valueOf(request.queryParams("action")));
            ReleaseNotes.getJobs().put(jobID, job);

            job.start();
            return ReleaseNotes.render("views/worker.html", context);
        } catch (Exception ex) {
            ex.printStackTrace();
            return "Exception: " + ex.toString();
        }
    }
}
