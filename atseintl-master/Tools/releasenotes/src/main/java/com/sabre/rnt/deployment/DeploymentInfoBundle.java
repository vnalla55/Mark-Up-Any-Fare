package com.sabre.rnt.deployment;

import com.sabre.rnt.NotesUtils;
import com.sabre.rnt.Publisher;
import com.sabre.rnt.Publisher.Command;
import com.sabre.rnt.Publisher.InsertDeliveryNoteCommand;
import com.sabre.rnt.Publisher.PublicationInfoBundle;
import com.sabre.rnt.Publisher.RemoveOldDeliveryNoteCommand;
import com.sabre.rnt.Publisher.ShellCommand;
import com.sabre.rnt.Publisher.UpdateDeliveryNoteCommand;
import com.sabre.rnt.Publisher.PublicationInfoBundle.FileToUse;

public class DeploymentInfoBundle {

	private String numberedReleasesDir;
    private String specialReleasesDir;
    private String configReleasesDir;
    private String tmpDirPrefix;
    private String currentInstanceURL;
    private String copyLabelledFileCmd;
    private String moveLabelToTheNewestVersionCmd;
    private String removeFileCmd;
    private String repoDirName;
	private Publisher.Command commandsForAdding[];
    private Publisher.Command commandsForUpdating[];
    private Publisher.Command commandsForRemoving[];
    private Publisher.Command commandsForRemoveAndAdd[];
    private Publisher.Command commandsForUpdateWithFileChange[];

	public String getNumberedReleasesDir() {
		return numberedReleasesDir;
	}

    public String getSpecialReleasesDir() {
        return specialReleasesDir;
    }

    public String getConfigReleasesDir() {
        return configReleasesDir;
    }

	public String getTmpDirPrefix() {
		return tmpDirPrefix;
	}

    public String getRepoDirName() {
        return repoDirName;
    }
	
	public Publisher.Command [] getCommandsForAdding() {
		return commandsForAdding;
	}

    public Publisher.Command [] getCommandsForUpdating() {
        return commandsForUpdating;
    }

    public Publisher.Command [] getCommandsForRemoving() {
        return commandsForRemoving;
    }

    public Publisher.Command [] getCommandsForRemoveAndAdd() {
        return commandsForRemoveAndAdd;
    }

    public Publisher.Command [] getCommandsForUpdateWithFileChange() {
        return commandsForUpdateWithFileChange;
    }

    /**
     * Requirements for the shell command:
     *  - When there is no label, it should create an empty file instead of copying the existing one
     * @param fileName
     * @param targetFileName
     * @return Comman that can be run with a Unix shell.
     */
    public String getCommandForCopyingLabeledFile(String fileName, String targetFileName) {
        return copyLabelledFileCmd.replaceAll("%origFile%", fileName).replaceAll("%outputFile%", targetFileName);
    }

    /**
     * Requirements for the shell command:
     *  - When the label is on the file, it should move it to the latest version
     *  - When there is no label, it should create it for the top version
     * @param fileName
     * @return
     */
    public String getCommandForMovingLabelToTop(String fileName) {
        return moveLabelToTheNewestVersionCmd.replaceAll("%origFile%", fileName);
    }

    public String getCommandForRemovingFile(String fileName) {
        return removeFileCmd.replace("%outputFile%", fileName);
    }

	public static DeploymentInfoBundle getInstance() {
		if (null == instance)
			setDevDeployment();
		return instance;
	}

    public static void setDevDeployment() {
        instance = new DeploymentInfoBundle();
        instance.initWindowsDevDeployment();
    }

    public static void setRepoProductionDeployment(String repoDirName) {
        instance = new DeploymentInfoBundle();
        instance.initGitLocalDeployment(repoDirName);
        instance.repoDirName = repoDirName;
    }

    public static void setGitRepoProductionDeployment(String gitRepoName) {
        instance = new DeploymentInfoBundle();
        instance.initGitSharedDeployment(gitRepoName);
        instance.repoDirName = gitRepoName;
    }

    public String getCurrentInstanceURL() {
        return currentInstanceURL;
    }

	private static DeploymentInfoBundle instance = null;

    final Publisher.ShellCommand CMD_VALIDATE =
            new Publisher.ShellCommand("if [[ `basename \"%outputFile%\"` = release* ]]; then /vobs/atseintl/Tools/irelease/irelvalid.py -f %tmpFilename%; fi");

    private void initGitLocalDeployment(String repoDirName) {

        final Publisher.ShellCommand CMD_ASSURE_FILE_EXISTS =
                new Publisher.ShellCommand("if [ ! -f %outputFile% ]; then touch %outputFile%; fi");

        numberedReleasesDir = repoDirName + "/Releases/";
        specialReleasesDir = numberedReleasesDir + "others/";
        configReleasesDir = numberedReleasesDir + "config/";
        tmpDirPrefix = "/tmp/";
        currentInstanceURL = NotesUtils.getCurrentInstanceURL();
        commandsForAdding = new Publisher.Command[] {
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.InsertDeliveryNoteCommand(),
        };
        commandsForUpdating = new Publisher.Command[] {
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.UpdateDeliveryNoteCommand()
        };
        commandsForRemoving = new Publisher.Command[] {
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.ORIGINAL_FILE)
        };
        commandsForRemoveAndAdd = new Publisher.Command[] {
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.TARGET_FILE),
                new Publisher.InsertDeliveryNoteCommand()
        };
        commandsForUpdateWithFileChange = new Publisher.Command[] {
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.ORIGINAL_FILE),
                new Publisher.InsertDeliveryNoteCommand()
        };
    }

    public static String createGitCommand(String repoDir, String cmd) {
        return String.format("cd '%s' && /usr/bin/git %s", repoDir, cmd);
    }

    private void initGitSharedDeployment(String repoDirName) {

        final Publisher.ShellCommand CMD_ASSURE_FILE_EXISTS =
                new Publisher.ShellCommand("if [ ! -f %outputFile% ]; then touch %outputFile% && /usr/bin/git add %outputFile%; fi");

        final Publisher.ShellCommand CMD_GIT_PULL_GIT =
                new Publisher.ShellCommand(createGitCommand(repoDirName, "pull"));

        final Publisher.ShellCommand CMD_GIT_COMMIT_ALL =
                new Publisher.ShellCommand(createGitCommand(repoDirName, "commit -a -m 'Automatic delivery note message'"));

        final Publisher.ShellCommand CMD_GIT_PUSH =
        		new Publisher.ShellCommand(createGitCommand(repoDirName, "push"));


        copyLabelledFileCmd = createGitCommand(repoDirName, "show LAST_REL_NOTE:Releases/$(basename '%origFile%') > '%outputFile%'");
        moveLabelToTheNewestVersionCmd = createGitCommand(repoDirName, "tag -f LAST_REL_NOTE");
        removeFileCmd = "rm '%outputFile%'";

        numberedReleasesDir = repoDirName + "/Releases/";
        specialReleasesDir = numberedReleasesDir + "others/";
        configReleasesDir = numberedReleasesDir + "config/";
        tmpDirPrefix = "/tmp/";
        currentInstanceURL = NotesUtils.getCurrentInstanceURL();
        commandsForAdding = new Publisher.Command[] {
                CMD_GIT_PULL_GIT,
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.InsertDeliveryNoteCommand(),
                CMD_GIT_COMMIT_ALL,
                CMD_GIT_PUSH
        };
        commandsForUpdating = new Publisher.Command[] {
                CMD_GIT_PULL_GIT,
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.UpdateDeliveryNoteCommand(),
                CMD_GIT_COMMIT_ALL,
                CMD_GIT_PUSH
        };
        commandsForRemoving = new Publisher.Command[] {
                CMD_GIT_PULL_GIT,
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.ORIGINAL_FILE),
                CMD_GIT_COMMIT_ALL,
                CMD_GIT_PUSH
        };
        commandsForRemoveAndAdd = new Publisher.Command[] {
                CMD_GIT_PULL_GIT,
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.TARGET_FILE),
                new Publisher.InsertDeliveryNoteCommand(),
                CMD_GIT_COMMIT_ALL,
                CMD_GIT_PUSH
        };
        commandsForUpdateWithFileChange = new Publisher.Command[] {
                CMD_GIT_PULL_GIT,
                CMD_ASSURE_FILE_EXISTS,
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.ORIGINAL_FILE),
                new Publisher.InsertDeliveryNoteCommand(),
                CMD_GIT_COMMIT_ALL,
                CMD_GIT_PUSH
        };
    }

    private void initWindowsDevDeployment() {
        numberedReleasesDir = "c:/rnt/";
        specialReleasesDir = numberedReleasesDir + "others/";
        configReleasesDir = numberedReleasesDir + "config/";
        tmpDirPrefix = numberedReleasesDir + "temp/";
        currentInstanceURL = "127.0.0.1";
        commandsForAdding = new Publisher.Command[] {
                new Publisher.InsertDeliveryNoteCommand(),
        };
        commandsForUpdating = new Publisher.Command[] {
                new Publisher.UpdateDeliveryNoteCommand()
        };
        commandsForRemoving = new Publisher.Command[] {
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.ORIGINAL_FILE)
        };
        commandsForRemoveAndAdd = new Publisher.Command[] {
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.TARGET_FILE),
                new Publisher.InsertDeliveryNoteCommand()
        };
        commandsForUpdateWithFileChange = new Publisher.Command[] {
                new Publisher.RemoveOldDeliveryNoteCommand(Publisher.PublicationInfoBundle.FileToUse.ORIGINAL_FILE),
                new Publisher.InsertDeliveryNoteCommand()
        };
    }
}
