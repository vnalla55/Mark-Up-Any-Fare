package com.sabre.rnt;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Vector;

public class RecordTools {

    /* This is for Apache Velocity templates - this class is used as a Tool. */
    public static final java.lang.String DEFAULT_KEY = "recordTools";

    /**
     * Get record fields declared for a given Record class (record fields
     * are fields marked with special annotations).
     */
    public static FormFieldDesc[] getRecordFields(Class<? extends Record> recordType) {
        Vector<FormFieldDesc> formFields = new Vector<>();
        for (Field f : recordType.getDeclaredFields()) {
            FormField formField = f.getAnnotation(FormField.class);
            if (formField == null)
                continue;
            formFields.add(new FormFieldDesc(f.getName(), formField));
        }
        FormFieldDesc[] result = new FormFieldDesc[formFields.size()];
        return formFields.toArray(result);
    }

    public static <T extends Record> void fillWithDefaults(T record) {
        Class<? extends Record> classOf = record.getClass();
        FormFieldDesc[] fields = getRecordFields(classOf);
        for (FormFieldDesc f : fields) {
            if (f.getType() == FormFieldType.CHECKBOX_LIST) {
                CheckboxList checkboxListValues = getCheckboxListValues(record, f.getFieldName());

                for (FormFieldDesc.PossibleValDesc possibleVal : f.getPossibleValues()) {
                    CheckboxList.CheckboxData checkboxData = null;
                    if (checkboxListValues.contains(possibleVal.getTitle())) {
                        checkboxData = checkboxListValues.getData(possibleVal.getTitle());

                        checkboxData.setChecked(possibleVal.isCheckedByDefault());
                        checkboxData.setValue("");
                    } else {
                        checkboxData = new CheckboxList.CheckboxData(possibleVal.getTitle(),
                                                     possibleVal.isCheckedByDefault(),
                                                     "");
                        checkboxListValues.add(checkboxData);
                    }
                }
            } else {
                setRecordFieldVal(record, f.getFieldName(), "");
            }
        }
    }

    public static Object getRecordFieldVal(Record record, String fieldName) {
        Class<?> cl = record.getClass();
        do {
            try {
                Field field = cl.getDeclaredField(fieldName);
                if (field != null) {
                    field.setAccessible(true);
                    return field.get(record);
                }
            } catch (NoSuchFieldException e) {
            } catch (IllegalAccessException e) {
            }
            // Keep digging
            cl = cl.getSuperclass();
        } while (cl != null);

        return null;
    }

    public static void setRecordFieldVal(Record record, String fieldName, Object value) {
        Class<?> cl = record.getClass();
        do {
            try {
                Field field = cl.getDeclaredField(fieldName);
                if (field != null) {
                    field.setAccessible(true);
                    field.set(record, value);
                    return;
                }
            } catch (NoSuchFieldException e) {
            } catch (IllegalAccessException e) {
            }
            // Keep digging
            cl = cl.getSuperclass();
        } while (cl != null);
    }

    /**
     * Extract a field value from a record object, treating it as a String.
     * If the field is of the String type, then it is simply returned,
     * otherwise .toString() is called to get its string representation.
     * @param record Object of the Record class.
     * @param fieldName Name of the field to be extracted.
     * @return String value of the field.
     */
    public static String getRecordString(Record record, String fieldName) {
        Object val = getRecordFieldVal(record, fieldName);
        if (val instanceof String) {
            return (String) val;
        } else {
            return val.toString();
        }
    }

    public static CheckboxList getCheckboxListValues(Record record, String fieldName) {
        Object val = getRecordFieldVal(record, fieldName);
        return (CheckboxList)val;
//        if (val instanceof Map<?, ?>)
//            return (Map<String, Boolean>)val;
//        return null;
    }

    public static String[] getSummaryFieldTitles(Class<? extends Record> recordType) {
        ArrayList<String> result = new ArrayList<>();
        for (FormFieldDesc formFieldDesc : getRecordFields(recordType)) {
            if (formFieldDesc.isInSummary()) {
                result.add(formFieldDesc.getTitle());
            }
        }
        return result.toArray(new String[result.size()]);
    }

    public static String[] getSummaryFieldValues(Record record) {
        ArrayList<String> result = new ArrayList<>();
        for (FormFieldDesc formFieldDesc : getRecordFields(record.getClass())) {
            if (formFieldDesc.isInSummary()) {
                result.add(getRecordString(record, formFieldDesc.getFieldName()));
            }
        }
        return result.toArray(new String[result.size()]);
    }

    public static String getNewRecordTabLabel(Class<? extends Record> recordType) {
        RecordConfig recordConfig = recordType.getAnnotation(RecordConfig.class);
        return recordConfig.newRecordTabLabel();
    }

    public static String getEditRecordTabLabel(Class<? extends Record> recordType) {
        RecordConfig recordConfig = recordType.getAnnotation(RecordConfig.class);
        return recordConfig.editRecordTabLabel();
    }

    public static String[] getPossibleGroupNames(Class<? extends Record> recordType) {
        RecordConfig recordConfig = recordType.getAnnotation(RecordConfig.class);
        ArrayList<String> result = new ArrayList<>();
        for (PossibleGroupName groupName : recordConfig.possibleGroupNames()) {
            result.add(groupName.name());
        }
        return result.toArray(new String[result.size()]);
    }
}