package com.sabre.rnt;
import java.io.*;
import java.math.BigInteger;
import java.security.SecureRandom;
import java.util.LinkedList;
import java.util.List;
import java.util.Vector;
import java.util.regex.Pattern;

import com.sabre.rnt.deployment.DeploymentInfoBundle;

public class Publisher extends Thread implements ShellCmdFacade.Logger {

    private Class<? extends Record> recordType;
    private String releaseNote, outputFile, groupName, origOutputFile, origGroup, origDigest;
    ReleaseNotes.Action action;
    private LinkedList<String> lines = new LinkedList<String>();
    private static SecureRandom random = new SecureRandom();
    private int lineNumForNoteInsert = -1;

	public static class PublicationInfoBundle {
        private Class<? extends Record> recordType;
		private String tmpFilename;
        private String outputFile;
        private String groupName;
        private String origFile;
        private String releaseNoteToOverwriteDigest;

        public enum FileToUse {
            TARGET_FILE,
            ORIGINAL_FILE
        }

        public String getFilename(FileToUse whatFile) {
            switch (whatFile) {
                case TARGET_FILE: return getOutputFile();
                case ORIGINAL_FILE: return getOrigFile();
                default: throw new IllegalArgumentException("Unknown file requested");
            }
        }

		/**
		 * @return Filename of a temporary file where the delivery notice is saved.
		 */
		public String getTmpFilename() {
			return tmpFilename;
		}

		public void setTmpFilename(String tmpFilename) {
			this.tmpFilename = tmpFilename;
		}

		/**
		 * @return Filename of the final file with the delivery notices.
		 */
		public String getOutputFile() {
			return outputFile;
		}

		public void setOutputFile(String outputFile) {
			this.outputFile = outputFile;
		}

		/**
		 * @return Name of a group to which the delivery notice is to be inserted.
		 */
		public String getGroupName() {
			return groupName;
		}

		public void setGroupName(String groupName) {
			this.groupName = groupName;
		}

        /**
         * This is used for updating notes which are already published.
         *
         * @return Digest of an published release note that is to be updated (~overwritten).
         */
        public String getReleaseNoteToOverwriteDigest() {
            return releaseNoteToOverwriteDigest;
        }

        public void setReleaseNoteToOverwriteDigest(String releaseNoteToOverwriteDigest) {
            this.releaseNoteToOverwriteDigest = releaseNoteToOverwriteDigest;
        }

        public String getOrigFile() {
            return origFile;
        }

        public void setOrigFile(String origFile) {
            this.origFile = origFile;
        }

        Class<? extends Record> getRecordType() {
            return recordType;
        }

        void setRecordType(Class<? extends Record> recordType) {
            this.recordType = recordType;
        }
    }

	public static abstract class Command {
		abstract public int run(Publisher parent, PublicationInfoBundle bundle);
		
		protected static String fillCommandWithPublicationInfoBundle(String command, PublicationInfoBundle bundle) {
			return command.replaceAll("%tmpFilename%", bundle.getTmpFilename())
						  .replaceAll("%outputFile%",  bundle.getOutputFile())
						  .replaceAll("%groupName%", bundle.getGroupName())
                          .replaceAll("%origFile%", bundle.getOrigFile());
			
		}

		boolean isQuiet() {
			return quiet;
		}

		void setQuiet(boolean quiet) {
			this.quiet = quiet;
		}

		protected boolean quiet = false;
	}

    static abstract class FileModifyingCommand extends Command {

        protected Publisher parent;
        protected PublicationInfoBundle bundle;
        protected PublicationInfoBundle.FileToUse affectedFile = PublicationInfoBundle.FileToUse.TARGET_FILE;

        protected void appendWholeFile(Publisher parent, String fileName) {
            try {
                DataInputStream in = new DataInputStream(new FileInputStream(
                        fileName));

                BufferedReader br = new BufferedReader(
                        new InputStreamReader(in));
                String line;

                if (! resultLines.lastElement().equals("\n")) {
                    // Notes have to be separated by a blank line.
                    // This tool takes care of appending blank lines after each note in advance,
                    // but users who add notes manually tend to forget about them.
                    resultLines.add("\n");
                }

                while ((line = br.readLine()) != null) {
                    resultLines.add(line + "\n");
                }
                in.close();
            } catch (Exception e) {
                parent.addLine("Error: " + e.getMessage());
            }
        }

        protected boolean tryWritingLinesToFile(Publisher parent, Vector<String> lines, String fileName) {
            try {
                FileWriter fstream = new FileWriter(fileName);
                BufferedWriter out = new BufferedWriter(fstream);

                for (String line : lines) {
                    out.write(line);
                }
                out.close();
            } catch (Exception e) {
                parent.addLine("Error: " + e.getMessage());
                return false;
            }

            return true;
        }

        protected Vector<String> resultLines = null;

        @Override
        public int run(Publisher parent, PublicationInfoBundle bundle) {
            this.parent = parent;
            this.bundle = bundle;

            int setUpResult = setUp();
            if (setUpResult != 0)
                return setUpResult;

            resultLines = new Vector<>();
            int lineNum = 0;

            try {
                DataInputStream in = new DataInputStream(new FileInputStream(
                        bundle.getFilename(affectedFile)));

                BufferedReader br = new BufferedReader(
                        new InputStreamReader(in));
                String line;

                while ((line = br.readLine()) != null) {

                    processLine(line, lineNum);

                    lineNum++;
                }
                in.close();
            } catch (Exception e) {
                StringWriter sw = new StringWriter();
                e.printStackTrace(new PrintWriter(sw));
                String stacktrace = sw.toString();

                parent.addLine("Error: " + e.getMessage());
                parent.addLine(stacktrace);
                return -1;
            }

            int tearDownResult = tearDown();
            if (tearDownResult != 0)
                return tearDownResult;

            return tryWritingLinesToFile(parent, resultLines, bundle.getFilename(affectedFile)) ? 0 : 1;
        }

        abstract int setUp();
        abstract void processLine(String line, int lineNum);
        abstract int tearDown();
    }
	
	public static class ShellCommand extends Command {
		private String cmdPattern;
		private Publisher parent;
		
		public ShellCommand(String command) {
			cmdPattern = command;
		}
		
		public ShellCommand(String command, boolean quiet) {
			cmdPattern = command;
			setQuiet(quiet);
		}
		
		@Override
		public int run(Publisher parent, PublicationInfoBundle bundle) {
			int result = 1;
			String cmd = fillCommandWithPublicationInfoBundle(cmdPattern, bundle);
            return new ShellCmdFacade(cmd, parent).run(isQuiet());
		}
	}

    public static class RemoveOldDeliveryNoteCommand extends FileModifyingCommand {

        private Record noteToRemove = null;

        public RemoveOldDeliveryNoteCommand(PublicationInfoBundle.FileToUse affectedFile) {
            this.affectedFile = affectedFile;
        }

        @Override
        int setUp() {
            parent.addLine("Parsing current release notes file...");
            NotesFile notesFile = null;

            try {
                notesFile = NotesFile.parseFile(bundle.getRecordType(), bundle.getFilename(affectedFile));
            } catch (RecordParser.NoteParsingException ex) {
                parent.addLine(ex.toString());
                ex.printStackTrace();
                return -1;
            } catch (Exception ex) {
                StringWriter strace = new StringWriter();
                ex.printStackTrace(new PrintWriter(strace));
                parent.addLine(strace.toString());
                ex.printStackTrace();
                return -1;
            }

            noteToRemove = notesFile.getNoteByDigest(bundle.getReleaseNoteToOverwriteDigest());

            if (noteToRemove == null) {
                parent.addLine("Cannot find the release note to be removed. The file might have been modified in the meanwhile.");
                return -1;
            }

            parent.addLine("Modifying the file...");

            return 0;
        }

        @Override
        void processLine(String line, int lineNum) {
            if (lineNum < noteToRemove.getFirstLineNum() ||
                    lineNum >= (noteToRemove.getFirstLineNum() + noteToRemove.getLinesCount())) {
                resultLines.add(line + "\n");
            }
        }

        @Override
        int tearDown() {
            return 0;
        }
    }

    public static class UpdateDeliveryNoteCommand extends FileModifyingCommand {

        Record noteToOverwrite = null;
        boolean inserted = false;

        @Override
        int setUp() {
            parent.addLine("Parsing current release notes file...");
            NotesFile notesFile = null;
            try {
                notesFile = NotesFile.parseFile(bundle.getRecordType(), bundle.getOutputFile());
            } catch (RecordParser.NoteParsingException ex) {
                parent.addLine(ex.toString());
                ex.printStackTrace();
                return -1;
            }

            inserted = false;

            noteToOverwrite = notesFile.getNoteByDigest(bundle.getReleaseNoteToOverwriteDigest());

            if (noteToOverwrite == null) {
                parent.addLine("Cannot find the release note to be updated. The file might have been modified in the meanwhile.");
                return -1;
            }

            parent.addLine("Modifying the file...");

            return 0;
        }

        @Override
        int tearDown() {
            return 0;
        }

        @Override
        void processLine(String line, int lineNum) {
            if (lineNum < noteToOverwrite.getFirstLineNum() ||
                    lineNum >= (noteToOverwrite.getFirstLineNum() + noteToOverwrite.getLinesCount())) {
                resultLines.add(line + "\n");
            } else {
                if (!inserted) {
                    appendWholeFile(parent, bundle.getTmpFilename());
                    inserted = true;
                }
            }
        }

    }

	public static class InsertDeliveryNoteCommand extends FileModifyingCommand {
        boolean foundDesiredGroup = false, noteAppended = false;
        Pattern groupPattern = Pattern.compile("\\[\\p{Alnum}+\\]\\s*");

        @Override
        int setUp() {
            parent.addLine("Inserting the delivery notice to a release note file...");
            foundDesiredGroup = false;
            noteAppended = false;

            return 0;
        }

        @Override
        void processLine(String line, int lineNum) {
            if (!noteAppended) {
                if (foundDesiredGroup) {
                    if (groupPattern.matcher(line).matches()) {
                        // Another group starts -> we're at the end of
                        // the desired group

                        appendWholeFile(parent, bundle.getTmpFilename());

                        resultLines.add("\n");
                        noteAppended = true;
                    }
                } else if (line.startsWith("[" + bundle.getGroupName() + "]")) {
                    foundDesiredGroup = true;
                }
            }

            resultLines.add(line + "\n");
        }

        @Override
        int tearDown() {
            if (! noteAppended) {
                if (! foundDesiredGroup) {
                    parent.addLine("Group [" + bundle.getGroupName() + "] doesn't exist in the file, I will add it...");
                    resultLines.add("[" + bundle.getGroupName() + "]\n");
                    resultLines.add("\n");
                }

                appendWholeFile(parent, bundle.getTmpFilename());
                resultLines.add("\n");
            }
            return 0;
        }
	}

	public Publisher(Class<? extends Record> recordType, String groupName, String releaseNote,
                     ReleaseName targetRelease, ReleaseName origRelease, String origGroup, String origDigest,
                     ReleaseNotes.Action action) {
        this.recordType = recordType;
		this.groupName = groupName;
		this.releaseNote = releaseNote;
        this.outputFile = targetRelease.getFileName();
        this.origOutputFile = origRelease == null ? "" : origRelease.getFileName();
        this.origGroup = origGroup;
        this.origDigest = origDigest;
        this.action = action;
	}

	private static String nextRandomKey() {
		return new BigInteger(130, random).toString(32);
	}

    @Override
	public void addLine(String line) {
        synchronized (this) {
		    lines.add(line);
        }
	}

	synchronized List<String> getLines() {
		List<String> result = lines;
		lines = new LinkedList<>();
		return result;
	}
	
	String getOutput() {
		StringBuffer result = new StringBuffer();
		for (String line : getLines()) {
			result.append(line);
			result.append("\r\n");
		}
		return result.toString();
	}

	String saveToTmpFile() {
		String tmpFilename = DeploymentInfoBundle.getInstance().getTmpDirPrefix() + "/release_note_" + nextRandomKey()
				+ ".txt";
		addLine("Saving the delivery notice to a temporary file '" + tmpFilename + "'...");
		PrintWriter outputFile = null;
		try {
			outputFile = new PrintWriter(tmpFilename);
			outputFile.append(releaseNote);
		} catch (FileNotFoundException ex) {
			addLine("Saving to '" + tmpFilename + "' has failed.");
			return null;
		} finally {
			if (null != outputFile) {
				outputFile.close();
			}
		}
		return tmpFilename;
	}

	@Override
	public void run() {
		addLine("Starting processing...");

		PublicationInfoBundle publicationInfoBundle = new PublicationInfoBundle();
        publicationInfoBundle.setRecordType(recordType);
		publicationInfoBundle.setGroupName(groupName);
		publicationInfoBundle.setOutputFile(outputFile);

        if (action != ReleaseNotes.Action.REMOVE) {
            String tmpFilename = saveToTmpFile();
            if (null == tmpFilename) {
                return;
            }
            publicationInfoBundle.setTmpFilename(tmpFilename);
        }

        Command commandsToRun[] = null;

        if (action == ReleaseNotes.Action.EDIT) {
            if (!origOutputFile.equals(outputFile)) {
                addLine("Notice to be moved to a different file.");
                commandsToRun = DeploymentInfoBundle.getInstance().getCommandsForUpdateWithFileChange();
                publicationInfoBundle.setOrigFile(origOutputFile);
            } else
            if (!origGroup.equals(groupName)) {
                addLine("Notice to be moved to a different group/section.");
                commandsToRun = DeploymentInfoBundle.getInstance().getCommandsForRemoveAndAdd();
            } else
            {
                addLine("Simple update: notice stays in the same group and file.");
                commandsToRun = DeploymentInfoBundle.getInstance().getCommandsForUpdating();
            }
            publicationInfoBundle.setReleaseNoteToOverwriteDigest(origDigest);
        } 
        else if (action == ReleaseNotes.Action.ADD) {
            commandsToRun = DeploymentInfoBundle.getInstance().getCommandsForAdding();
        } 
        else if (action == ReleaseNotes.Action.REMOVE) {
            commandsToRun = DeploymentInfoBundle.getInstance().getCommandsForRemoving();
            publicationInfoBundle.setOrigFile(origOutputFile);
            publicationInfoBundle.setReleaseNoteToOverwriteDigest(origDigest);
        }

		boolean fail = false;
        int statusCode = 0;
		for (Command command : commandsToRun) {
			if ((statusCode = command.run(this, publicationInfoBundle)) != 0) {
				if (command.isQuiet())
					continue;
				fail = true;
				break;
			}
		}

        if (fail) {
            addLine(String.format("Failure (command exited with status code %d). I will try to remove the commit, just in case...", statusCode));
            new ShellCommand(DeploymentInfoBundle.createGitCommand(DeploymentInfoBundle.getInstance().getRepoDirName(),
                             "reset --hard HEAD~1")).run(this, publicationInfoBundle);
        }

        addLine("Publishing " + (fail ? "has failed." : "is finished, you can close this window now."));

		if (!ReleaseNotes.NO_KEY_REQUIRED) {
			ReleaseNotes.setExitWhenAllFlushed(true);
		}
	}
}