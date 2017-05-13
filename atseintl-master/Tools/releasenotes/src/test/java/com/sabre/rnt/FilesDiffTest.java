package com.sabre.rnt;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.junit.Test;

import java.io.InputStream;
import java.util.LinkedHashMap;
import java.util.LinkedList;

@RecordConfig(
        newRecordTabLabel = "New",
        editRecordTabLabel = "Edit",
        possibleGroupNames = {
                @PossibleGroupName(name = "group1"),
                @PossibleGroupName(name = "group2")
        })
class FakeRecord extends Record {
    @FormField(title = "Name", type = FormFieldType.ONE_LINE_TEXT)
    private String name;

    String getName() {
        return name;
    }
}

public class FilesDiffTest extends TestCase {

    @Test
    public void testGenerate() throws Exception {
        InputStream streamA = IOUtils.toInputStream("[group1]\n\nName: aaa\n\nName: bbb\n\n[group2]\n\nName: ccc");
        InputStream streamB = IOUtils.toInputStream("[group1]\n\nName: aaa\n\nName: ddd\n\n[group2]\n\nName: ccc\n\nName: eee");
        NotesFile<FakeRecord> fileA = NotesFile.parseFile(FakeRecord.class, streamA);
        NotesFile<FakeRecord> fileB = NotesFile.parseFile(FakeRecord.class, streamB);

        LinkedHashMap<String, LinkedList<FakeRecord>> diff = new FilesDiff<FakeRecord>(fileA, fileB).getDiff();
        assertEquals(2, diff.size());

        assertEquals("ddd", diff.get("group1").get(0).getName());
        assertEquals("eee", diff.get("group2").get(0).getName());
    }
}
