package com.sabre.rnt;

import org.apache.velocity.VelocityContext;

import com.sabre.rnt.deployment.DeploymentInfoBundle;

import spark.Request;
import spark.Response;
import spark.Route;

import java.io.PrintWriter;
import java.io.StringWriter;

public class RouteBrowse extends Route {

    RouteBrowse() {
        super("/browse/:release");
    }

    @Override
    public Object handle(Request request, Response response) {
        try {
            String releaseName = request.params(":release");
            if (releaseName.equals("none") && request.queryParams().contains("specialRelease")) {
                releaseName = request.queryParams("specialRelease");
            }
            ReleaseName release = ReleaseNames.parseReleaseName(releaseName);

            // TODO: Refactoring: We don't want to use ReleaseNote class explicitly here
            Class<? extends Record> recordType = release == null ? ReleaseNote.class : release.getRecordType();
            VelocityContext context = ReleaseNotes.createContext(request, recordType);
            context.put("release", release);
            context.put("menu_idx", ReleaseNotes.TabName.EDIT);

            if (releaseName.equals("none")) {
                context.put("releases", ReleaseNotes.getReleasesInDir(DeploymentInfoBundle.getInstance().getNumberedReleasesDir(),
                        new NumberedReleaseNameParsingFactory()));
                context.put("stage", "listfiles");
            } else
            if (releaseName.equals("specials")) {
                context.put("releases", ReleaseNotes.getReleasesInDir(DeploymentInfoBundle.getInstance().getSpecialReleasesDir(),
                        new SpecialReleaseNameParserFactory()));
                context.put("stage", "listspecials");
                context.put("menu_idx", ReleaseNotes.TabName.OTHER);
            } else {
                try {
                    NotesFile notesFile = NotesFile.parseFile(release.getRecordType(), release.getFileName());

                    context.put("notes", notesFile);
                    context.put("stage", "showfile");
                    //context.put("recordType", release.getRecordType());
                } catch (RecordParser.NoteParsingException ex) {
                    return ReleaseNotes.parsingFailureMsg(ex);
                }
            }

            return ReleaseNotes.render("views/browse.html", context);
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
