package com.sabre.rnt;

import org.apache.velocity.VelocityContext;
import spark.Request;
import spark.Response;
import spark.Route;

public class RouteRemove extends Route {

    public RouteRemove() {
        super("/remove/:release/:digest");
    }

    @Override
    public Object handle(Request request, Response response) {
        ReleaseName release = ReleaseNames.parseReleaseName(request.params(":release"));
        String digest = request.params(":digest");

        Record matchedReleaseNote = null;
        try {
            matchedReleaseNote = ReleaseNames.getRecordByReleaseAndDigest(release, digest);
        } catch (ReleaseNames.RecordExtractionException ex) {
            return ex.getMessage();
        }

        VelocityContext context = ReleaseNotes.createContext(request, release.getRecordType());

        context.put("menu_idx", ReleaseNotes.TabName.EDIT);
        context.put("action", ReleaseNotes.Action.REMOVE);
        context.put("release", release);
        context.put("digest", digest);
        context.put("userDisplayName", ReleaseNotes.getUserDisplayName());
        context.put("releaseNote", matchedReleaseNote.getOriginalText());
        context.put("listOfModifiedFiles", "");
        context.put("record", RecordTools.getRecordFields(release.getRecordType()));

        return ReleaseNotes.render("views/remove.html", context);
    }
}
