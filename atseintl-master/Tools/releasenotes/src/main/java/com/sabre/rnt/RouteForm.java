package com.sabre.rnt;

import org.apache.velocity.VelocityContext;
import spark.Request;
import spark.Response;
import spark.Route;

import java.io.PrintWriter;
import java.io.StringWriter;

public class RouteForm extends Route {

    RouteForm() {
        super("/form");
    }

    @Override
    public Object handle(Request request, Response response) {
        try {
            response.header("Content-Type", "text/html; charset=utf-8");

            Class<? extends Record> recordType = ReleaseNote.class;
            if (request.queryParams().contains("specialRelease")) {
                String releaseName = request.queryParams("specialRelease");
                recordType = ReleaseNames.parseReleaseName(releaseName).getRecordType();
            }

            VelocityContext context = ReleaseNotes.createContext(request, recordType);
            context.put("menu_idx", ReleaseNotes.TabName.NEW);

            context.put("action", ReleaseNotes.Action.ADD);
            context.put("groupRadio", ReleaseNotes.generateHTMLForGroupRadioList("groupRadio"));
            context.put("userDisplayName", ReleaseNotes.getUserDisplayName());
            context.put("listOfModifiedFiles", "");

            Record emptyRecord = recordType.newInstance();
            RecordTools.fillWithDefaults(emptyRecord);
            context.put("releaseNote", emptyRecord);

            context.put("record", RecordTools.getRecordFields(recordType));

            return ReleaseNotes.render("views/edit.html", context);
        } catch (Exception ex) {
            StringWriter sw = new StringWriter();
            ex.printStackTrace(new PrintWriter(sw));
            String stacktrace = sw.toString();

            System.err.println("THROW:");
            System.err.println(stacktrace);
            return "Failed to display page!\n\n" + stacktrace;
        }
    }
}
