package com.sabre.rnt;
import java.io.*;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.Vector;
import java.util.regex.Pattern;

import org.apache.commons.lang3.StringUtils;

public class NotesFile<T extends Record> {

	public final static String DEFAULT_GROUP_NAME = "_default";

    T getNoteByDigest(String digest) {

        T matchedReleaseNote = null;

        for (Map.Entry<String, LinkedList<T>> group : getNotesByGroup().entrySet()) {
            String groupName = group.getKey();
            for (T publishedReleaseNote : group.getValue()) {
                if (publishedReleaseNote.getDigest().equals(digest)) {
                    matchedReleaseNote = publishedReleaseNote;
                    break;
                }
            }
        }

        return matchedReleaseNote;
    }

    public static <T extends Record> NotesFile parseFile(Class<T> recordType, String fileName) throws RecordParser.NoteParsingException {
        try {
            InputStream in = new DataInputStream(new FileInputStream(fileName));
            return parseFile(recordType, in);
        } catch (IOException e) {
            e.printStackTrace();
            throw new RecordParser.NoteParsingException(e.toString());
        }
    }

	public static<T extends Record> NotesFile parseFile(Class<T> recordType, InputStream inputStream) throws RecordParser.NoteParsingException {
		DataInputStream in;
        NotesFile<T> notesFile = null;
		try {
			BufferedReader br = new BufferedReader(new InputStreamReader(inputStream));
			String line;
            int noteFirstLineNum = 0, currentLineNum = 0;

			notesFile = new NotesFile<T>();

			Pattern groupPattern = Pattern.compile("\\[\\p{Alnum}+\\]\\s*");

			String currentGroupName = DEFAULT_GROUP_NAME;

			Vector<String> currentNoteLines = new Vector<>();

			while ((line = br.readLine()) != null) {
				if (line.trim().equals("") || groupPattern.matcher(line).matches()) {
					if (!currentNoteLines.isEmpty()) {
						LinkedList<T> group = notesFile.getOrCreateGroup(currentGroupName);

                        T record = recordType.newInstance();
                        group.add(RecordParser.fillFromText(record, StringUtils
                                .join(currentNoteLines, "\n"), noteFirstLineNum, currentGroupName));
					}

                    currentNoteLines = new Vector<>();

                    if (groupPattern.matcher(line).matches()) {
                        currentGroupName = NotesUtils.removeSquareBrackets(line);
                    }
				} else {
                    if (currentNoteLines.isEmpty())
                        noteFirstLineNum = currentLineNum;
					currentNoteLines.add(line);
				}

                currentLineNum++;
			}

            if (!currentNoteLines.isEmpty()) {
                LinkedList<T> group = notesFile.getOrCreateGroup(currentGroupName);

                T record = recordType.newInstance();
                group.add(RecordParser.fillFromText(record, StringUtils
                        .join(currentNoteLines, "\n"), noteFirstLineNum, currentGroupName));
            }

			inputStream.close();
		} catch (IOException e) {
			e.printStackTrace();
            throw new RecordParser.NoteParsingException(e.toString());
		} catch (InstantiationException e) {
            e.printStackTrace();
            throw new RecordParser.NoteParsingException(e.toString());
        } catch (IllegalAccessException e) {
            e.printStackTrace();
            throw new RecordParser.NoteParsingException(e.toString());
        }
		return notesFile;
	}
	
	private LinkedList<T> getOrCreateGroup(String groupName) {
		if (! notesByGroup.containsKey(groupName))
			notesByGroup.put(groupName, new LinkedList<T>());

		return notesByGroup.get(groupName);
	}

	private LinkedHashMap<String, LinkedList<T>> notesByGroup = new LinkedHashMap<>();
	
	private NotesFile() {}

    public LinkedHashMap<String, LinkedList<T>> getNotesByGroup() {
        return notesByGroup;
    }
}
