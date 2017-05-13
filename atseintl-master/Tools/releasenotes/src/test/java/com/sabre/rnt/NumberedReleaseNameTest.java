package com.sabre.rnt;

import junit.framework.TestCase;
import org.junit.Test;

import java.util.Calendar;
import java.util.GregorianCalendar;

import static junit.framework.TestCase.assertEquals;
import static org.junit.Assert.assertNotEquals;


public class NumberedReleaseNameTest extends TestCase {

    private NumberedReleaseNameParsingFactory parsingFactory;

    @Override
    protected void setUp() {
        parsingFactory = new NumberedReleaseNameParsingFactory();
    }

    @org.junit.Test
    public void testParseStringJustReleaseDotLong() throws Exception {

        NumberedReleaseName rel = parsingFactory.parseStringToNumbered("2013.01");

        assertEquals(2013, rel.getMajor());
        assertEquals(1, rel.getMinor());
    }

    @org.junit.Test
    public void testParseStringJustReleaseUnderscoreShort() throws Exception {

        NumberedReleaseName rel = parsingFactory.parseStringToNumbered("13_01");

        assertEquals(2013, rel.getMajor());
        assertEquals(1, rel.getMinor());
    }

    @org.junit.Test
    public void testParseStringReleaseNote() throws Exception {

        NumberedReleaseName rel = parsingFactory.parseStringToNumbered("release_2013.01");

        assertEquals(2013, rel.getMajor());
        assertEquals(1, rel.getMinor());
    }

    @org.junit.Test
    public void testGetReleaseNoteFilename() throws Exception {
        NumberedReleaseName rel = new NumberedReleaseName(13, 1);
        assertEquals("release_2013_01", rel.toString());
    }

    @org.junit.Test
    public void testCurrentRelease_MiddleOfRelease() throws Exception {
        Calendar calendar = new GregorianCalendar(2013, 3, 16);
        NumberedReleaseName rel = NumberedReleaseName.getLatestActiveName(calendar.getTime());

        assertEquals(2013, rel.getMajor());
        assertEquals(5, rel.getMinor());
    }

    @org.junit.Test
    public void testCurrentRelease_FirstDayOfRelease() throws Exception {
        Calendar calendar = new GregorianCalendar(2013, 2, 29);
        NumberedReleaseName rel = NumberedReleaseName.getLatestActiveName(calendar.getTime());

        assertEquals(2013, rel.getMajor());
        assertEquals(5, rel.getMinor());
    }

    @org.junit.Test
    public void testCurrentRelease_LastDayOfRelease() throws Exception {
        Calendar calendar = new GregorianCalendar(2013, 2, 21);
        NumberedReleaseName rel = NumberedReleaseName.getLatestActiveName(calendar.getTime());

        assertEquals(2013, rel.getMajor());
        assertEquals(4, rel.getMinor());
    }

    @org.junit.Test
    public void testCurrentRelease_FirstDayOfRelease2() throws Exception {
        Calendar calendar = new GregorianCalendar(2013, 3, 19);
        NumberedReleaseName rel = NumberedReleaseName.getLatestActiveName(calendar.getTime());

        assertEquals(2013, rel.getMajor());
        assertEquals(6, rel.getMinor());
    }

    @org.junit.Test
    public void testCurrentRelease_NextYear() throws Exception {
        Calendar calendar = new GregorianCalendar(2013, 11, 19);
        NumberedReleaseName rel = NumberedReleaseName.getLatestActiveName(calendar.getTime());

        assertEquals(2014, rel.getMajor());
        assertEquals(1, rel.getMinor());
    }

    @Test
    public void testEquals() throws Exception {
        NumberedReleaseName r1, r2, r3;
        r1 = new NumberedReleaseName(13, 1);
        r2 = new NumberedReleaseName(13, 1);
        r3 = new NumberedReleaseName(13, 2);

        assertEquals(r1, r1);
        assertEquals(r1, r2);
        assertNotEquals(r1, r3);
    }

    @Test
    public void testParsingNullGivesNull() throws Exception {
        assertEquals(null, parsingFactory.parseStringToNumbered(null));
    }
}
