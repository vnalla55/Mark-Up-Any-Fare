package com.sabre.rnt;

import static org.junit.Assert.assertEquals;
import org.junit.Test;

import java.util.HashMap;

public class ReleaseNotesTest {
    @Test
    public void testCreateUrlParams() {
        HashMap<String, String> map = new HashMap<>();

        map.put("aaa", "bbb");
        map.put("ccc", "ddd");
        map.put("eee", "fff");

        assertEquals("aaa=bbb&ccc=ddd&eee=fff", ReleaseNotes.createUrlParams(map));
    }

    @Test
    public void testCreateUrlParams_URLEncoder() {
        HashMap<String, String> map = new HashMap<>();

        map.put("aaa", "b b b");
        map.put("ccc", "ddd/xxx");
        map.put("eee", "fff");

        assertEquals("aaa=b+b+b&ccc=ddd%2Fxxx&eee=fff", ReleaseNotes.createUrlParams(map));
    }
}
