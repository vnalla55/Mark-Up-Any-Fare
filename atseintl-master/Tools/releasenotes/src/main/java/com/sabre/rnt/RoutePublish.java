package com.sabre.rnt;

import org.apache.velocity.VelocityContext;
import com.sabre.rnt.deployment.DeploymentInfoBundle;
import spark.Request;
import spark.Response;
import spark.Route;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.*;

public class RoutePublish extends Route {

    RoutePublish() {
        super("/publish");
    }

    public static ArrayList<ReleaseName> getPossibleReleases(String dir) {
        ArrayList<ReleaseName> relsInDir = ReleaseNotes.getReleasesInDir(dir, new NumberedReleaseNameParsingFactory());
        Collections.sort(relsInDir, Collections.reverseOrder());

        NumberedReleaseName latestRel = NumberedReleaseName.getLatestActiveName(new Date());

        if (relsInDir.size() == 0 || !relsInDir.get(0).equals(latestRel)) {
            relsInDir.add(0, latestRel);
        }

        return relsInDir;
    }

    public static boolean paramToBool(String param) {
        return param != null && param.equals("on");
    }

    private static void propagateBoolListFromForm(LinkedHashMap<String, Boolean> list, Request request) {
        for (String s : list.keySet()) {
            String shortS = NotesUtils.produceFormName(s);
            list.put(s, paramToBool(request.queryParams(shortS)));
        }
    }

    public static String generateHTMLForComboOptionsFromDir(String dir, ReleaseName selected) {
        StringBuilder result = new StringBuilder();
        for (ReleaseName filename : getPossibleReleases(dir)) {
            result.append(String.format(
                    "<option name=\"%s\" value=\"%s\" %s>%s</option>",
                    ReleaseNotes.safeQuotes(filename), ReleaseNotes.safeQuotes(filename),
                    filename.equals(selected) ? "selected=\"selected\"" : "", NotesUtils.safeHTML(filename)));
        }
        return result.toString();
    }

    Record extractRecordFromRequest(Class<? extends Record> classOfRecord, Request request) {
        try {
            Record r = classOfRecord.newInstance();
            FormFieldDesc [] fields = RecordTools.getRecordFields(classOfRecord);

            for (FormFieldDesc field : fields) {
                switch (field.getType()) {
                    case DATE:
                    case ONE_LINE_TEXT:
                        RecordTools.setRecordFieldVal(r, field.getFieldName(),
                                request.queryParams(field.getFieldName()));
                        break;
                    case RICH_TEXT:
                        RecordTools.setRecordFieldVal(r, field.getFieldName(),
                                NotesUtils.filterTinyMCE(request.queryParams(field.getFieldName())));
                        break;
                    case CHECKBOX_LIST:
                        CheckboxList values = RecordTools.getCheckboxListValues(r, field.getFieldName());
                        for (FormFieldDesc.PossibleValDesc possibleVal : field.getPossibleValues()) {

                            String inputNameSuffix = NotesUtils.produceFormName(field.getFieldName()) + "_" +
                                    NotesUtils.produceFormName(possibleVal.getTitle());

                            boolean checked = false;
                            if (possibleVal.isCheckable()) {
                                checked = paramToBool(request.queryParams("checkbox_" + inputNameSuffix));
                            }

                            String textValue = "";
                            if (possibleVal.isInputEnabled()) {
                                textValue = request.queryParams("text_" + inputNameSuffix);
                            }

                            CheckboxList.CheckboxData newData = new CheckboxList.CheckboxData(
                                    possibleVal.getTitle(), checked, textValue);
                            values.add(newData);
                        }
                        break;
                }
            }

            return r;
        } catch (InstantiationException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        } catch (IllegalAccessException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }
        return null;
    }

    private ReleaseName determineDefaultTargetReleaseName(Request request) {
        if (request.queryParams().contains("specialRelease")) {
            return ReleaseNames.parseReleaseName(request.queryParams("specialRelease"));
        }

        ReleaseName rel = ReleaseNames.parseReleaseName(request.queryParams("origRelease"));
        if (null == rel) {
            return NumberedReleaseName.getLatestActiveName(new Date());
        }
        return rel;
    }

    public Object handle(Request request, Response response) {
        response.header("Content-Type", "text/html; charset=utf-8");

        ReleaseName releaseName = determineDefaultTargetReleaseName(request);
        Class<? extends Record> recordType = releaseName.getRecordType();

        VelocityContext context = ReleaseNotes.createContext(request, recordType);

        try {
            Record note = extractRecordFromRequest(recordType, request);

            try {
                context.put("targetReleaseOptions",
                        generateHTMLForComboOptionsFromDir(DeploymentInfoBundle.getInstance().getNumberedReleasesDir(), releaseName));
            } catch (NullPointerException ex) {
                return ReleaseNotes.returnMsg("ERROR: Couldn't fetch contents of the directory: "
                        + DeploymentInfoBundle.getInstance().getNumberedReleasesDir()
                        + "<br>Did you forget to set the proper ClearCase view?");
            }
            context.put("action", ReleaseNotes.Action.valueOf(request.queryParams("action")));
            context.put("groupName", request.queryParams("groupRadio"));
            context.put("releaseNote", NotesUtils.safeHTML(note.generateReleaseNote()));
            context.put("release", request.queryParams("origRelease"));
            context.put("origGroup", request.queryParams("origGroup"));
            context.put("digest", request.queryParams("digest"));
            return ReleaseNotes.render("views/publish.html", context);

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
