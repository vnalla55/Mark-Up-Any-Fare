package com.sabre.rnt;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Set;

public class CheckboxList {
    public static class CheckboxData {
        final private String title;
        private boolean checked;
        private String value;

        CheckboxData(String title, boolean checked, String value) {
            this.title = title;
            this.checked = checked;
            this.value = value;
        }

        public String getTitle() {
            return title;
        }

        public boolean isChecked() {
            return checked;
        }

        public String getValue() {
            return value;
        }

        public void setChecked(boolean checked) {
            this.checked = checked;
        }

        public void setValue(String value) {
            this.value = value;
        }
    }

    private LinkedHashMap<String, CheckboxData> values = new LinkedHashMap<>();

    public CheckboxData getData(String key) {
        return values.get(key);
    }

    public boolean contains(String key) {
        return values.containsKey(key);
    }

    public Set<String> keySet() {
        return values.keySet();
    }

    public Collection<CheckboxData> values() {
        return values.values();
    }

    public void add(CheckboxData checkboxData) {
        values.put(checkboxData.getTitle(), checkboxData);
    }
}