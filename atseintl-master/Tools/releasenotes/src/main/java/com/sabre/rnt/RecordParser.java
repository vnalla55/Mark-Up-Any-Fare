package com.sabre.rnt;
import org.apache.commons.lang3.StringUtils;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class RecordParser {

    static class NoteParsingException extends Exception {
        public NoteParsingException(String message) {
            super(message);
        }
    }

    /**
     * Finds the smallest common indent (indent = amount of whitespace chars at the beginning of a line),
     * and merges all the lines removing the indent from each one.
     * Lines are merged using '\n' character.
     *
     * @param lines Lines to be merged.
     * @return Merged lines.
     */
    private static String mergeLinesAndDropLowestCommonIndent(Vector<String> lines) {
        int shortestIndent = Integer.MAX_VALUE;
        for (String line : lines) {

            int whitespaceCounter = 0;
            for (int i = 0; i < line.length(); i++) {
                if (line.charAt(i) != ' ' && line.charAt(i) != '\t')
                    break;
                whitespaceCounter++;
            }

            if (whitespaceCounter < shortestIndent)
                shortestIndent = whitespaceCounter;
        }

        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < lines.size() - 1; i++) {
            String line = lines.get(i);
            buf.append(line.substring(shortestIndent, line.length()));
            buf.append('\n');
        }

        if (! lines.isEmpty()) {
            String lastElement = lines.lastElement();
            buf.append(lastElement.substring(shortestIndent, lastElement.length()));
        }

        return buf.toString();
    }

    private static HashMap<String, String> parseNoteText(String text) throws NoteParsingException {
        text = NotesUtils.normalizeLineEndings(text);

        Vector<String> parsingErrors = new Vector<>();

        HashMap<String, String> noteValues = new HashMap<>();

        String fieldName = null;
        //StringBuffer fieldContent = new StringBuffer();

        Vector<String> fieldContentLines = new Vector<>();

        for (String line : text.split("\n")) {
            if (line.startsWith(" ") || line.startsWith("\t")) {
                //if (fieldContent.length() > 0)
                //    fieldContent.append("\n");
                //fieldContent.append(line);
                fieldContentLines.add(line);
            } else {
                if (! line.contains(":")) {
                    parsingErrors.add("Bad line (no indent, no colon): " + line);
                    continue;
                }

                //noteValues.put(fieldName, fieldContent.toString());
                noteValues.put(fieldName, mergeLinesAndDropLowestCommonIndent(fieldContentLines));

                String [] splitted = line.split(":", 2);
                fieldName = splitted[0].trim();
                //fieldContent = new StringBuffer(splitted[1].trim());

                fieldContentLines.clear();

                String valuePart = splitted[1].trim();
                if (!valuePart.equals("")) {
                    // Multi-line values usually don't begin in the same line as their name,
                    // so we don't want to add this empty line to the value.
                    fieldContentLines.add(valuePart);
                }
            }
        }

        if (fieldName != null) {
            //noteValues.put(fieldName, fieldContent.toString());
            noteValues.put(fieldName, mergeLinesAndDropLowestCommonIndent(fieldContentLines));
        }

        if (! parsingErrors.isEmpty())
            throw new NoteParsingException(StringUtils.join(parsingErrors, "\n"));

        return noteValues;
    }

    private static HashMap<String, CheckboxList.CheckboxData> parseInnerNoteList(String noteText) {

        HashMap<String, CheckboxList.CheckboxData> result = new HashMap<>();

        for (String line : noteText.split("\n")) {
            String trimmedLine = line.trim();

            /* Lines can look like this:
                1) [x] name
                2) [x] name: text
                3) name: text
                4) name

               There might be some additional whitespace.
               [x], [X] in the beginning => this field is checked
               [ ] => this field is unchecked.
             */

            String innerName = null, innerText = null;
            boolean innerChecked = false;

            String nameAndText = trimmedLine;

            Pattern booleanItem = Pattern.compile("^\\[([Xx ]?)\\].*");
            Matcher matcher = null;

            if ((matcher = booleanItem.matcher(trimmedLine)).matches()) {
                if (matcher.group(1).toLowerCase().equals("x")) {
                    innerChecked = true;
                }

                int pos = trimmedLine.indexOf(']');
                nameAndText = trimmedLine.substring(pos + 1, trimmedLine.length());
            }

            if (nameAndText.contains(":")) {
                String [] splitted = nameAndText.split(":", 2);
                innerName = splitted[0].trim();
                innerText = splitted[1].trim();
            } else {
                innerName = nameAndText.trim();
            }

            result.put(innerName, new CheckboxList.CheckboxData(innerName, innerChecked, innerText));
        }
        return result;
    }

    /**
     * Parses a piece of file containing a generated record.
     * @param text Lines containing a record, NOT the whole file.
     * @param firstLineNum Number of the line in the file where the lines from "text" parameter start.
     * @param groupName Name of the group in the file to which this record belongs.
     * @return Object representing parsed record.
     * @throws NoteParsingException
     */
    public static <T extends Record> T fillFromText(T recordToFill, String text, int firstLineNum, String groupName) throws NoteParsingException {
        HashMap<String, String> noteValues = parseNoteText(text);

        try {
            recordToFill.setOriginalText(text);
            recordToFill.setFirstLineNum(firstLineNum);
            recordToFill.setGroupName(groupName);
            recordToFill.setLinesCount(StringUtils.countMatches(text, "\n") + 1);

            FormFieldDesc [] fields = RecordTools.getRecordFields(recordToFill.getClass());
            for (FormFieldDesc field : fields) {
                switch (field.getType()) {
                    case DATE:
                    case ONE_LINE_TEXT:
                    case RICH_TEXT:
                        RecordTools.setRecordFieldVal(recordToFill, field.getFieldName(), noteValues.get(field.getTitle()));
                        break;
                    case CHECKBOX_LIST:
                        HashMap<String, CheckboxList.CheckboxData> parsedValues = parseInnerNoteList(noteValues.get(field.getTitle()));
                        CheckboxList checkboxList = RecordTools.getCheckboxListValues(recordToFill, field.getFieldName());
                        for (FormFieldDesc.PossibleValDesc possibleVal : field.getPossibleValues()) {
                            checkboxList.add(parsedValues.get(possibleVal.getTitle()));
                        }
                        break;
                }
            }
        } catch (NullPointerException ex) {
            StringWriter sw = new StringWriter();
            ex.printStackTrace(new PrintWriter(sw));
            String stacktrace = sw.toString();

            throw new NoteParsingException("Null pointer exception while parsing. " +
                    "Probably some required field in a note doesn't exist. Stacktrace: " +
                    ex.toString() + " " + stacktrace);
        }

        return recordToFill;
    }
}
