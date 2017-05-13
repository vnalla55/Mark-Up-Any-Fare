package com.sabre.rnt;

public abstract class ReleaseName {
    public static String RELEASE_FILE_PREFIX = "release_";

    public abstract String getDirName();

    public abstract Class<? extends Record> getRecordType();

    public String getFileName() {
        return String.format("%s%s", getDirName(), toString());
    }
}

interface ReleaseNameParsingFactory {
    public ReleaseName parseString(String s);
}

class ReleaseNames {

    private static NumberedReleaseNameParsingFactory numberedParser = new NumberedReleaseNameParsingFactory();
    private static SpecialReleaseNameParserFactory specialParser = new SpecialReleaseNameParserFactory();
    private static ConfigReleaseNameParserFactory configParser = new ConfigReleaseNameParserFactory();

    /* Order might be important! */
    private static ReleaseNameParsingFactory nameParsers[] = {numberedParser, specialParser, configParser};

    /**
     * Handles both kinds of release names (numbered, special).
     * @param s String representing a release.
     * @return
     */
    public static ReleaseName parseReleaseName(String s) {

        ReleaseName result = null;

        if (s == null)
            return null;

        for (ReleaseNameParsingFactory parser : nameParsers) {
            try {
                result = parser.parseString(s);
            } catch (NumberFormatException ex) {
                continue;
            }

            if (result != null)
                return result;
        }

        return result;
    }

    public static class RecordExtractionException extends Exception {
        public RecordExtractionException(String message) {
            super(message);
        }
    }

    public static Record getRecordByReleaseAndDigest(ReleaseName release, String digest) throws RecordExtractionException {
        NotesFile notesFile = null;
        try {
            //notesFile = NotesFile.parseFile(String.format("%s/%s", DeploymentInfoBundle.getInstance().getNumberedReleasesDir(), release));
            notesFile = NotesFile.parseFile(release.getRecordType(), release.getFileName());
        } catch (RecordParser.NoteParsingException ex) {
            throw new RecordExtractionException(ReleaseNotes.parsingFailureMsg(ex));
        }

        Record matchedReleaseNote = notesFile.getNoteByDigest(digest);

        if (matchedReleaseNote == null) {
            throw new RecordExtractionException("Can't find the release note! It might have been modified/removed in the meanwhile.");
        }

        return matchedReleaseNote;
    }
}
