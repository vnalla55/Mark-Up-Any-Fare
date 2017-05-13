package com.sabre.rnt;
import java.util.HashMap;

import com.sabre.rnt.deployment.DeploymentInfoBundle;

public class ConfigReleaseName extends ReleaseName{
    private String name;

    private static HashMap<String, Class<? extends Record>> nameToType = new HashMap<>();
    private final static Class<? extends Record> DEFAULT_TYPE = ReleaseNote.class;

    static {
        nameToType.put(SubscriptionProcessor.SUBSCRIBERS_FILE, Subscriber.class);
    }

    ConfigReleaseName(String name) {
        this.name = name;
    }

    @Override
    public String toString() {
        return "config_" + name;
    }

    @Override
    public String getDirName() {
        return DeploymentInfoBundle.getInstance().getConfigReleasesDir();
    }

    @Override
    public Class<? extends Record> getRecordType() {
        if (nameToType.containsKey(name)) {
            return nameToType.get(name);
        }
        return DEFAULT_TYPE;
    }
}

class ConfigReleaseNameParserFactory implements ReleaseNameParsingFactory {

    private final String PREFIX = "config_";

    @Override
    public ReleaseName parseString(String s) {
        if (!s.startsWith(PREFIX))
            return null;

        int prefix_len = PREFIX.length();
        return new ConfigReleaseName(s.substring(prefix_len, s.length()));
    }
}
