package com.sabre.rnt;

import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

import com.sabre.rnt.deployment.DeploymentInfoBundle;

import java.util.*;

public class SubscriptionProcessor extends Thread {

    public static final String SUBSCRIBERS_FILE = "emails";

    private DeploymentInfoBundle deploymentInfo;

    SubscriptionProcessor(DeploymentInfoBundle deploymentInfo) {
        this.deploymentInfo = deploymentInfo;
    }

    /**
     * @return Map where keys are names of releases and values are email messages.
     */
    Map<String, String> generateStandardMessages() {
        ArrayList<ReleaseName> releases = ReleaseNotes.getReleasesInDir(DeploymentInfoBundle.getInstance().getNumberedReleasesDir(),
                new NumberedReleaseNameParsingFactory());

        Map<String, String> emailMap = new TreeMap<>();
        for (ReleaseName release : releases) {
            String message = generateDocumentForFile(release);
            if (message == null)
                continue;
            emailMap.put(release.toString(), message);
        }

        return emailMap;
    }

    String generateDocumentForFile(ReleaseName release) {
        final String prefix = deploymentInfo.getTmpDirPrefix() + "subscription_processor_";
        String fileNameA = prefix + ReleaseNotes.nextRandomKey(), fileNameB = prefix + ReleaseNotes.nextRandomKey();

        //ReleaseName release = ReleaseNames.parseReleaseName(fileName);
        try {
            NotesFile.parseFile(release.getRecordType(), release.getFileName());
        } catch (RecordParser.NoteParsingException e) {
            // We don't proceed if the current version can't be parsed.
            // There is a race condition here: if the file gets corrupted between the check above and the code below,
            // then this check is futile. It is fixable with two labels, but we'll do this after we decide what VCS
            // we want to use.
            System.err.println("Error while parsing a release note, won't proceed: " + e.toString());
            e.printStackTrace();
            return null;
        }

        //final StringBuilder buffer = new StringBuilder();

        ShellCmdFacade.Logger logger = new ShellCmdFacade.Logger() {
            @Override
            public void addLine(String line) {
                //buffer.append(line);
                //buffer.append("\n");
                System.out.println(line);
            }
        };

        String fileName = release.getFileName();

        try {
            new ShellCmdFacade(deploymentInfo.getCommandForCopyingLabeledFile(fileName, fileNameA), logger).run(false);
            new ShellCmdFacade(deploymentInfo.getCommandForMovingLabelToTop(fileName), logger).run(false);
            new ShellCmdFacade(deploymentInfo.getCommandForCopyingLabeledFile(fileName, fileNameB), logger).run(false);

            NotesFile fileA, fileB;

            try {
                fileA = NotesFile.parseFile(release.getRecordType(), fileNameA);
                fileB = NotesFile.parseFile(release.getRecordType(), fileNameB);
            } catch (RecordParser.NoteParsingException e) {
                System.out.println("Error while parsing a file: " + e.toString());
                e.printStackTrace();
                return null;
            }

            LinkedHashMap<String, LinkedList<ReleaseNote>> newNotes = new FilesDiff(fileA, fileB).getDiff();
            if (newNotes.size() == 0) {
                System.out.println("Nothing to send in " + fileName);
                return null;
            }

            FileRenderer renderer = new FileRenderer("views/email.html");
            return renderer.render(release.getRecordType(), newNotes);

        } finally {
            new ShellCmdFacade(deploymentInfo.getCommandForRemovingFile(fileNameA), logger).run(false);
            new ShellCmdFacade(deploymentInfo.getCommandForRemovingFile(fileNameB), logger).run(false);
        }
    }

    void sendEmail(String to, String title, String message) {
        String from = "Maciej.Krawczyk@sabre.com";

        Properties properties = System.getProperties();
        properties.setProperty("mail.smtp.host", "mail.sgdcelab.sabre.com");

        Session session = Session.getDefaultInstance(properties);

        try{
            MimeMessage mimeMessage = new MimeMessage(session);
            mimeMessage.setFrom(new InternetAddress(from));
            mimeMessage.addRecipient(Message.RecipientType.TO,
                    new InternetAddress(to));
            mimeMessage.setSubject(title);
            //mimeMessage.setText(message);
            mimeMessage.setContent(message, "text/html; charset=utf-8");

            Transport.send(mimeMessage);

            System.out.println("Sent message successfully...");
        } catch (MessagingException mex) {
            mex.printStackTrace();
        }
    }

    void sendMessages(String email, Map<String, String> messages) {
        for (Map.Entry<String, String> entry : messages.entrySet()) {
            String releaseName = entry.getKey();
            String message = entry.getValue();

            sendEmail(email, "[RNT] New notes in " + releaseName, message);
        }
    }

    void processSubscriber(Subscriber subscriber, Map<String, String> standardMessages) {
        CheckboxList subscriptions = subscriber.getSubscriptions();
        if (subscriptions.contains(Subscriber.STANDARD_RELEASES_LBL)) {
            CheckboxList.CheckboxData standardSubscription = subscriptions.getData(Subscriber.STANDARD_RELEASES_LBL);
            if (standardSubscription.isChecked()) {
                sendMessages(subscriber.getEmail(), standardMessages);
            }
        }
    }

    LinkedList<Subscriber> getSubscribers() {
        ReleaseName subscribersFile = new ConfigReleaseNameParserFactory().parseString("config_" + SUBSCRIBERS_FILE);
        assert(subscribersFile.getRecordType() == Subscriber.class);
        NotesFile<Subscriber> notesFile = null;
        try {
            notesFile = NotesFile.parseFile(subscribersFile.getRecordType(), subscribersFile.getFileName());
        } catch (RecordParser.NoteParsingException e) {
            System.err.println("SubscriptionProcessor error: " + e.toString());
            e.printStackTrace();
            return null;
        }

        return notesFile.getNotesByGroup().get(Subscriber.SUBSCIBERS_GROUP_NAME);
    }

    public void process() {
        LinkedList<Subscriber> subscribers = getSubscribers();
        Map<String, String> standardMessages = generateStandardMessages();

        for (Subscriber subscriber : subscribers) {
            processSubscriber(subscriber, standardMessages);
        }
    }

    private boolean running = true;

    public synchronized void pleaseFinish() {
        running = false;
    }

    @Override
    public void run() {

        try {
            sleep(1000 * 7);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        while (true) {

            try {
                process();
            } catch (Exception e) {
                System.out.println("Will retry later, exception thrown while processing subscriptions: " + e.toString());
                e.printStackTrace();
            }

            try {
                sleep(1000 * 60 * 10);
            } catch (InterruptedException e) {
                // Meh, nothing really happened
            }

            synchronized (this) {
                if (!running)
                    break;
            }
        }
    }
}
