package com.sabre.rnt;

import org.apache.commons.io.FileUtils;
import org.apache.velocity.VelocityContext;

import java.io.File;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.LinkedList;

public class FileRenderer<T extends Record> {

    private String templateName;

    public FileRenderer(String templateName) {
        this.templateName = templateName;
    }

    public String render(Class<? extends Record> recordType, NotesFile<T> file) {
        return render(recordType, file.getNotesByGroup());
    }

    public String render(Class<? extends Record> recordType, LinkedHashMap<String, LinkedList<T>> notesByGroup) {
        VelocityContext context = ReleaseNotes.createNakedContext();
        context.put("notesByGroup", notesByGroup);
        context.put("recordType", recordType);
        context.put("record", RecordTools.getRecordFields(recordType));

        return ReleaseNotes.render(templateName, context);
    }

    public static void main(String args[]) {
        if (args.length < 3) {
            System.err.println("Not enough parameters.");
            System.err.println("Usage: java FileRenderer <old_file> <new_file> <output_file>");
            return;
        }

        NotesFile oldFile, newFile;

        try {
            oldFile = NotesFile.parseFile(ReleaseNote.class, args[0]);
            newFile = NotesFile.parseFile(ReleaseNote.class, args[1]);
        } catch (RecordParser.NoteParsingException e) {
            System.err.println(e.toString());
            e.printStackTrace();
            return;
        }

        LinkedHashMap<String, LinkedList<ReleaseNote>> newNotes = new FilesDiff(oldFile, newFile).getDiff();
        FileRenderer renderer = new FileRenderer("views/email.html");

        try {
            FileUtils.writeStringToFile(new File(args[2]), renderer.render(ReleaseNote.class, newNotes));
        } catch (IOException e) {
            System.err.println(e.toString());
            e.printStackTrace();
            return;
        }
    }
}
