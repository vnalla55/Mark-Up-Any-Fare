package com.sabre.rnt;
import org.apache.commons.lang3.StringUtils;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.lang.annotation.Target;
import java.lang.reflect.Field;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;
import java.lang.annotation.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public abstract class Record {

    private int firstLineNum;
    private int linesCount;
    private String originalText;
    private String digest;
    private String groupName;

    public int getFirstLineNum() {
        return firstLineNum;
    }

    public void setFirstLineNum(int firstLineNum) {
        this.firstLineNum = firstLineNum;
    }

    public String getOriginalText() {
        return originalText;
    }

    public void setOriginalText(String originalText) {
        this.originalText = originalText;
        computeOriginalTextDigest();
    }

    public String getDigest() {
        return digest;
    }

    private void computeOriginalTextDigest() {
        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance("SHA-1");

            byte [] sha1hash = new byte[40];
            md.update(originalText.getBytes("utf-8"), 0, originalText.length());
            sha1hash = md.digest();

            StringBuffer sb = new StringBuffer("");
            for (int i = 0; i < sha1hash.length; i++) {
                sb.append(Integer.toString((sha1hash[i] & 0xff) + 0x100, 16).substring(1));
            }

            digest = sb.toString();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    /**
     * Published release notes always belong to a group (~section in a file). If no group is specified
     * in the file, then they belong to the default group.
     * Beginnings of a group in a file are marked by lines such as:
     * [ABC]
     * Notes following this line belong to the "ABC" group (until another group begins).
     *
     * @return Name of a group to which this object belongs.
     */
    public String getGroupName() {
        return groupName;
    }

    public void setGroupName(String groupName) {
        this.groupName = groupName;
    }

    public int getLinesCount() {
        return linesCount;
    }

    public void setLinesCount(int linesCount) {
        this.linesCount = linesCount;
    }


    private void appSimpleString(StringBuffer buf, String title, String data) {
        buf.append(title);
        buf.append(": ");
        buf.append(data == null ? "" : data.replace("\n", " "));
        buf.append("\n");
    }

    private void appIndentedString(StringBuffer buf, String title, String data) {
        buf.append("        ");
        appSimpleString(buf, title, data == null ? "" : data);
    }

    private void appMlineString(StringBuffer buf, String title, String data) {
        buf.append(title);
        buf.append(":\n");
        if (data == null) {
            buf.append("\n");
            return;
        }

        for (String line : NotesUtils.normalizeLineEndings(data).trim().split("\n")) {
            if (line.trim().equals("")) {
                continue;
            }

            buf.append("        ");
            buf.append(line);
            buf.append("\n");
        }
    }
/*
    private void appBoolList(StringBuffer buf, String title, CheckboxList data) {
        buf.append(title);
        buf.append(":\n");
        if (data == null) {
            return;
        }
        for (CheckboxList.CheckboxData entry : data.values()) {
            appSingleBool(buf, entry.getTitle(), entry.isChecked());
        }
    }

    private void appBoolList(StringBuffer buf, String title, Map<String, Boolean> data) {
        buf.append(title);
        buf.append(":\n");
        if (data == null) {
            return;
        }
        for (Map.Entry<String, Boolean> entry : data.entrySet()) {
            appSingleBool(buf, entry.getKey(), entry.getValue());
        }
    }  */

    private void appSingleBool(StringBuffer buf, String title, Boolean data) {
        if (data) {
            buf.append("        [X]");
        } else {
            buf.append("        [ ]");
        }
        buf.append(" ");
        buf.append(title);
        buf.append("\n");
    }

    String generateReleaseNote() {
        StringBuffer result = new StringBuffer();

        for (FormFieldDesc field : RecordTools.getRecordFields(this.getClass())) {
            switch (field.getType()) {
                case DATE:
                case ONE_LINE_TEXT:
                    appSimpleString(result, field.getTitle(), RecordTools.getRecordString(this, field.getFieldName()));
                    break;
                case RICH_TEXT:
                    appMlineString(result, field.getTitle(), RecordTools.getRecordString(this, field.getFieldName()));
                    break;
                case CHECKBOX_LIST:
                    appSimpleString(result, field.getTitle(), "");

                    CheckboxList values = RecordTools.getCheckboxListValues(this, field.getFieldName());
                    for (FormFieldDesc.PossibleValDesc possibleVal : field.getPossibleValues()) {

                        CheckboxList.CheckboxData data = values.getData(possibleVal.getTitle());

                        boolean checked = false;
                        if (possibleVal.isCheckable()) {
                            String textValue = possibleVal.getTitle();
                            if (possibleVal.isInputEnabled()) {
                                textValue = textValue + ": " + data.getValue();
                            }
                            appSingleBool(result, textValue, data.isChecked());
                        } else {
                            appIndentedString(result, possibleVal.getTitle(), data.getValue());
                        }
                    }
            }
        }

        // Remove any lines with just the whitespace chars (they shouldn't be there anyway,
        // but we want to make sure because they cause scripts to parse files wrongly).
        return result.toString().replaceAll("(?m)^\\s+$", "");
    }
}

