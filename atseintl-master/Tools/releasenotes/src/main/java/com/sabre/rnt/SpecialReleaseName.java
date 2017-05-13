package com.sabre.rnt;

import com.sabre.rnt.deployment.DeploymentInfoBundle;

public class SpecialReleaseName extends ReleaseName {

    SpecialReleaseName(String name) {
        this.name = name;
    }

    @Override
    public String toString() {
        return "release_" + name;
    }

    private String name;

    @Override
    public String getDirName() {
        return DeploymentInfoBundle.getInstance().getSpecialReleasesDir();
    }

    @Override
    public Class<? extends Record> getRecordType() {
        return ReleaseNote.class;
    }
}

class SpecialReleaseNameParserFactory implements ReleaseNameParsingFactory {

    private String PREFIX = "release_";
    private NumberedReleaseNameParsingFactory numberedParser = new NumberedReleaseNameParsingFactory();

    @Override
    public ReleaseName parseString(String s) {
        if (! s.startsWith(PREFIX))
            return null;

        try {
            if (numberedParser.parseStringToNumbered(s) != null)
                // Numbered releases are not special releases.
                return null;
        } catch (NumberFormatException ignore) {}

        int prefix_len = PREFIX.length();
        return new SpecialReleaseName(s.substring(prefix_len, s.length()));
    }
}
