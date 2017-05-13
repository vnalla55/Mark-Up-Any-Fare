package com.sabre.rnt;

import org.apache.velocity.VelocityContext;
import spark.Request;
import spark.Response;
import spark.Route;

public class RouteEdit extends Route {

    RouteEdit() {
        super("/edit/:release/:digest");
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
        context.put("action", ReleaseNotes.Action.EDIT);
        context.put("release", release);
        context.put("digest", digest);
        context.put("origGroup", matchedReleaseNote.getGroupName());

//        String appliesToOther = matchedReleaseNote.getAppliesToOther() ? matchedReleaseNote.getAppliesToOtherText() : null;
//        context.put("appliesTo", ReleaseNotes.generateHTMLForBoolList(matchedReleaseNote.getAppliesTo(),
//                "appliesTo", appliesToOther, 6)); TODO
//        String applicationImpactOther = matchedReleaseNote.getOtherApplicationImpact() ? matchedReleaseNote.getOtherApplicationImpactText() : null;
//        context.put("applicationImpact", ReleaseNotes.generateHTMLForBoolList(matchedReleaseNote.getApplicationImpact(),
//                "applicationImpact", applicationImpactOther, 4));
        context.put("groupRadio", ReleaseNotes.generateHTMLForGroupRadioList("groupRadio", matchedReleaseNote.getGroupName()));
        context.put("selectedGroup", matchedReleaseNote.getGroupName());
        context.put("userDisplayName", ReleaseNotes.getUserDisplayName());
        context.put("listOfModifiedFiles", "");

        context.put("releaseNote", matchedReleaseNote);

        context.put("record", RecordTools.getRecordFields(release.getRecordType()));

        return ReleaseNotes.render("views/edit.html", context);
    }
}
