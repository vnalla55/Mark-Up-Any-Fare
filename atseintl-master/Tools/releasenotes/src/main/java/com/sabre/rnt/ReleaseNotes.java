package com.sabre.rnt;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.Velocity;
import org.apache.velocity.tools.ToolManager;

import com.sabre.rnt.deployment.DeploymentInfoBundle;

import spark.Filter;
import spark.Request;
import spark.Response;
import spark.Route;

import java.io.*;
import java.math.BigInteger;
import java.net.URLEncoder;
import java.security.SecureRandom;
import java.util.*;

import static spark.Spark.*;

public class ReleaseNotes {

	private static SecureRandom random = new SecureRandom();
    private static HashMap<String, Publisher> jobs = new HashMap<>();
	private static String instanceURL;
    private static String instanceKey;
    private static String userDisplayName;
	volatile private static boolean exitWhenAllFlushed = false;

    private static NumberedReleaseNameParsingFactory numberedReleaseParser = new NumberedReleaseNameParsingFactory();
	
	public static boolean NO_KEY_REQUIRED = false;

	public final static String [] groupNames = new String[] { "ATSEv2", "RTG", "PDC" };
	
	public static void setExitWhenAllFlushed(boolean exitWhenAllFlushed) {
		ReleaseNotes.exitWhenAllFlushed = exitWhenAllFlushed;
	}
	
	public static String nextRandomKey() {
		return new BigInteger(130, random).toString(32);
	}

    enum TabName {
        NEW,
        EDIT,
        OTHER
    }

	private static final Map<String, Object> settings = new HashMap<String, Object>();

    public static String render(String file, VelocityContext context) {
        org.apache.velocity.Template template = Velocity.getTemplate(file);
        StringWriter sw = new StringWriter();
        template.merge(context, sw);
        return sw.toString();
    }

    enum Action {
        EDIT,
        ADD,
        REMOVE
    }

    public static String createUrlParams(Map<String, String> params) {
        if (params.isEmpty())
            return "";

        StringBuilder builder = new StringBuilder();
        //builder.append("?");

        int i = 1;
        for (Map.Entry<String, String> param : params.entrySet()) {
            builder.append(param.getKey()).append("=").append(URLEncoder.encode(param.getValue()));
            if (i < params.size())
                builder.append("&");
            i++;
        }
        return builder.toString();
    }

    public static VelocityContext createNakedContext() {
        ToolManager manager = new ToolManager();
        manager.configure("velocity-tools.xml");
        return new VelocityContext(manager.createContext());
    }

    public static VelocityContext createContext(Request request, Class<? extends Record> recordType) {
        VelocityContext context = createNakedContext();

        context.put("recordType", recordType);
        context.put("instanceURL", instanceURL);
        context.put("instanceKey", instanceKey);
        context.put("noKeyRequired", NO_KEY_REQUIRED);
        context.put("keyRequired", !NO_KEY_REQUIRED);


        HashMap<String, String> urlParamsMap = new HashMap<>();

        if (!NO_KEY_REQUIRED)
            urlParamsMap.put("key", instanceKey);

        if (request.queryParams().contains("specialRelease")) {
            context.put("specialRelease", request.queryParams("specialRelease"));
            urlParamsMap.put("specialRelease", request.queryParams("specialRelease"));
        }

        context.put("urlParams", createUrlParams(urlParamsMap));

        urlParamsMap.remove("specialRelease");
        context.put("urlParamsWithoutSpecialRelease", createUrlParams(urlParamsMap));

        return context;
    }

	public static String safeQuotes(Object html) {
		return html.toString().replace("\"", "&quot;");
	}

    public static String generateHTMLForGroupRadioList(String name) {
        return generateHTMLForGroupRadioList(name, groupNames[0]);
    }

	public static String generateHTMLForGroupRadioList(String name, String checkedByDefault) {
		StringBuilder result = new StringBuilder();
		for (String groupName : groupNames) {
			result.append(String
					.format("<input type=\"radio\" name=\"%s\" id=\"%s_radio\" value=\"%s\" class=\"%s\" %s><label for=\"%s_radio\">%s</label><br>",
							name, groupName, groupName, name, checkedByDefault.equals(groupName) ? "checked=\"checked\"" : "", groupName, groupName));
		}
		return result.toString();
	}

    public static ArrayList<ReleaseName> getReleasesInDir(String dir, ReleaseNameParsingFactory nameParser) {
        File filedir = new File(dir);
        String [] filenames = filedir.list();
        Arrays.sort(filenames, Collections.reverseOrder());
        ArrayList<ReleaseName> releases = new ArrayList<>();
        for (String filename : filenames) {
            if (! filename.startsWith("release"))
                continue;

            try {
                ReleaseName releaseName = nameParser.parseString(filename);
                if (releaseName == null)
                    continue;
                releases.add(releaseName);
            } catch (NumberFormatException ignored) {}
        }
        return releases;
    }

	static String returnMsg(String msg) {
		return "<!doctype html><style type=\"text/css\">body { font-family: sans-serif; }</style>\n" + msg;
	}
	
	public static String execCmd(String cmd) {
		StringBuilder result = new StringBuilder();
		try {
			String line;
			Process p = Runtime.getRuntime().exec(
					new String[] { "sh", "-c", cmd });
			BufferedReader bri = new BufferedReader(new InputStreamReader(
					p.getInputStream()));
			while ((line = bri.readLine()) != null) {
				result.append(line);
			}
			bri.close();
			p.waitFor();
		} catch (Exception err) {
			err.printStackTrace();
		}
		return result.toString();
	}
	
	private static HashMap<String, String> generateArgsMap(String[] args) {
		String key = null;
        HashMap<String, String> result = new HashMap<>();
		boolean wasKeyBefore = false;
		
		for (String arg : args) {
			if (arg.startsWith("-")) {
				if (wasKeyBefore) {
					result.put(key, null);
				}
				key = arg;
				wasKeyBefore = true;
			} else {
				if (wasKeyBefore) {
					result.put(key, arg);
				} else {
					result.put(arg, null);
				}
				wasKeyBefore = false;
			}
		}
		
		if (wasKeyBefore) {
			result.put(key, null);
		}
		
		return result;
	}

    public static String parsingFailureMsg(RecordParser.NoteParsingException ex) {
        return "Unfortunately, the requested file is currently corrupted and cannot be parsed. This means that " +
                "this tool is not able to update this file (unless the file is fixed first).\n" +
                "Debugging errors are attached below.\n\n" + ex.toString();
    }

	public static void main(String[] args) {

		Random rand = new Random();
        int port = 4433;
		instanceKey = nextRandomKey();
        userDisplayName = "";
        boolean runSubscriptionProcessor = true;

        Velocity.init();

		HashMap<String, String> argsMap = generateArgsMap(args);

		if (argsMap.containsKey("-nokey")) {
			NO_KEY_REQUIRED = true;
            instanceKey = "0";
		}

        if (! argsMap.containsKey("-nouser")) {
            userDisplayName = NotesUtils.safeHTML(execCmd("finger `whoami` | grep 'Name:'  | awk -F 'Name: ' '{print $2}'"));
        }

        if (argsMap.containsKey("-repo")) {
            String repoDirName = argsMap.get("-repo");
            System.out.println("Running on external codebase repository: " + repoDirName);
            DeploymentInfoBundle.setRepoProductionDeployment(repoDirName);
            runSubscriptionProcessor = false;
        }

        if (argsMap.containsKey("-gitrepo")) {
            String repoDirName = argsMap.get("-gitrepo");
            System.out.println("Running on Git repository: " + repoDirName);
            DeploymentInfoBundle.setGitRepoProductionDeployment(repoDirName);
            //runSubscriptionProcessor = false;
        }

        if (argsMap.containsKey("-port")) {
            port = Integer.parseInt(argsMap.get("-port"));
        } else {
            int min = 5000, max = 15000;
            port = rand.nextInt(max - min + 1) + min;
        }

		if (argsMap.containsKey("-dev")) {
			System.out.println("Running a development instance.");
            DeploymentInfoBundle.setDevDeployment();
		}

        String urlSuffix = String.format(":%d/form%s", port, NO_KEY_REQUIRED ? "" : "?key=" + instanceKey);
        instanceURL = String.format("http://%s%s", DeploymentInfoBundle.getInstance().getCurrentInstanceURL(), urlSuffix);
        String localhostURL = "http://127.0.0.1" + urlSuffix;

        System.out.println("*************************************************");
        System.out.println("* URL of this instance: " + instanceURL);
        System.out.println("*************************************************");

        if (! argsMap.containsKey("-p")) {
            try {
                Runtime.getRuntime().exec("gnome-open " + localhostURL);
            } catch (IOException e) {
                System.err.println("Running gnome-open failed");
            }
        }

        setPort(port);

        if (runSubscriptionProcessor) {
            new SubscriptionProcessor(DeploymentInfoBundle.getInstance()).start();
        }

		
		before(new Filter("/*") {
			public void handle(Request request, Response response) {
				if (!NO_KEY_REQUIRED && !request.pathInfo().startsWith("/static")
						&& !instanceKey.equals(request.queryParams("key"))) {
					halt(401, "Unauthorized.");
                    return;
				}

                // Without the header below, IE sometimes switches to lower version
                // compatibility mode, and things stop working.
                response.header("X-UA-Compatible", "IE=edge");
            }
        });

        get(new Route("/") {
            @Override
            public Object handle(Request request, Response response) {
                response.status(301);
                response.header("Location", instanceURL);
                return String.format("<a href=\"%s\">%s</a>", instanceURL, instanceURL);
            }
        });

        get(new RouteForm());
        get(new RouteStatic());
        get(new RouteEdit());
        get(new RouteRemove());
        get(new RouteBrowse());
        post(new RouteWorker());
        get(new RouteStatus(exitWhenAllFlushed));
        post(new RoutePublish());
	}

    public static String getUserDisplayName() {
        return userDisplayName;
    }

    public static HashMap<String, Publisher> getJobs() {
        return jobs;
    }

}