package com.sabre.rnt;
import java.util.*;

public class FilesDiff<T extends Record> {

    private NotesFile<T> fileOld, fileNew;
    private LinkedHashMap<String, LinkedList<T>> newNotesByGroup = null;

    public FilesDiff(NotesFile<T> fileOld, NotesFile<T> fileNew) {
        this.fileOld = fileOld;
        this.fileNew = fileNew;
    }

    /**
     * Lazily generates differences between two files.
     * The result contains only records which are present in the
     * new file and aren't present in the old one. No information about the
     * ones which disappeared is returned.
     * Also, there is no notion of a changed record - if any data in a record
     * changes, then it becomes a new record.
     * @return Record which are present in fileNew and aren't in fileOld.
     */
    public LinkedHashMap<String, LinkedList<T>> getDiff() {
        if (null == newNotesByGroup) {
            generate();
        }
        return newNotesByGroup;
    }

    private void generate() {

        newNotesByGroup = new LinkedHashMap<>();

        LinkedHashMap<String, LinkedList<T>> oldNotes = fileOld.getNotesByGroup(),
                newNotes = fileNew.getNotesByGroup();

        HashMap<String, HashMap<String, T>> oldNotesMaps = new HashMap<>();

        for (Map.Entry<String, LinkedList<T>> notesInGroup : oldNotes.entrySet()) {
            String groupName = notesInGroup.getKey();

            HashMap<String, T> oldNotesMap;
            if (!oldNotesMaps.containsKey(groupName)) {
                oldNotesMap = new HashMap<>();
                oldNotesMaps.put(groupName, oldNotesMap);
            } else
                oldNotesMap = oldNotesMaps.get(groupName);

            for (T note : notesInGroup.getValue()) {
                assert(! oldNotesMap.containsKey(note.getDigest()));
                oldNotesMap.put(note.getDigest(), note);
            }
        }

        for (Map.Entry<String, LinkedList<T>> notesInGroup : newNotes.entrySet()) {
            String groupName = notesInGroup.getKey();
            for (T note : notesInGroup.getValue()) {
                if (!oldNotesMaps.containsKey(groupName) || !oldNotesMaps.get(groupName).containsKey(note.getDigest())) {
                    LinkedList<T> newNotesList;

                    if (!newNotesByGroup.containsKey(groupName)) {
                        newNotesList = new LinkedList<>();
                        newNotesByGroup.put(groupName, newNotesList);
                    } else
                        newNotesList = newNotesByGroup.get(groupName);

                    newNotesList.add(note);
                }
            }
        }
    }

}
