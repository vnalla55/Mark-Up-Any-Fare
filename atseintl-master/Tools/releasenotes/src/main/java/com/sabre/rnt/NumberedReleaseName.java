package com.sabre.rnt;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import com.sabre.rnt.deployment.DeploymentInfoBundle;

public class NumberedReleaseName extends ReleaseName implements Comparable<NumberedReleaseName> {

    @Override
    public String getDirName() {
        return DeploymentInfoBundle.getInstance().getNumberedReleasesDir();
    }

    @Override
    public Class<? extends Record> getRecordType() {
        return ReleaseNote.class;
    }

    // The "major" part of a release name usually refers to the year, "minor" to the month.
    private int major;
    private int minor;

    NumberedReleaseName(int major, int minor) {
        setMajor(major);
        setMinor(minor);
    }

    /**
     * Major part of a release name usually refers to year.
     * @return Return the major part of a version name.
     */
    public int getMajor() {
        return major;
    }

    public void setMajor(int major) {
        this.major = major;

        if (major < 2000) {
            this.major += 2000;
        }
    }

    /**
     * Minor part of a release name usually refers to month.
     * @return Return the minor part of a version name.
     */
    public int getMinor() {
        return minor;
    }

    public void setMinor(int minor) {
        this.minor = minor;
    }

    @Override
    public String toString() {
        return String.format("%s%04d_%02d", RELEASE_FILE_PREFIX, major, minor);
    }



    public static NumberedReleaseName getLatestActiveName(Date date) {
        Calendar calendar = new GregorianCalendar();
        calendar.setTime(date);

        final int skipBy = 4 * 7; // 4 weeks

        Calendar leftBound = new GregorianCalendar(2012, 10, 2);
        Calendar rightBound = ((Calendar) leftBound.clone());
        rightBound.add(Calendar.DAY_OF_YEAR, skipBy);

        int currentMajor = 13, currentMinor = 0;

        while (! ((leftBound.before(calendar) || leftBound.equals(calendar)) && rightBound.after(calendar))) {
            leftBound.add(Calendar.DAY_OF_YEAR, skipBy);
            rightBound.add(Calendar.DAY_OF_YEAR, skipBy);

            if (currentMinor == 12) {
                currentMinor = 0;
                currentMajor++;
            } else {
                currentMinor++;
            }
        }

        return new NumberedReleaseName(currentMajor, currentMinor);
    }

    @Override
    public int compareTo(NumberedReleaseName o) {
        if (major != o.major)
            return major - o.major;
        else
            return minor - o.minor;
    }

    @Override
    public boolean equals(Object o) {
        if (! (o instanceof NumberedReleaseName))
            return false;

        NumberedReleaseName other = (NumberedReleaseName) o;

        return major == other.major && minor == other.minor;
    }
}

class NumberedReleaseNameParsingFactory implements ReleaseNameParsingFactory {

    /**
     * Should be able to parse both 'raw' version names and release note filename.
     * @param s String representing a release.
     * @return
     */
    @Override
    public ReleaseName parseString(String s) {
        return parseStringToNumbered(s);
    }

    public NumberedReleaseName parseStringToNumbered(String s) {
        if (s == null)
            return null;

        s = s.trim();

        if (s.startsWith(ReleaseName.RELEASE_FILE_PREFIX)) {
            s = s.substring(ReleaseName.RELEASE_FILE_PREFIX.length(), s.length());
        }

        String splitted[] = s.split("[._]", 2);
        for (String part : splitted) {
            if (!part.matches("\\d+"))
                return null;
        }

        return new NumberedReleaseName(Integer.valueOf(splitted[0]), Integer.valueOf(splitted[1]));
    }
}