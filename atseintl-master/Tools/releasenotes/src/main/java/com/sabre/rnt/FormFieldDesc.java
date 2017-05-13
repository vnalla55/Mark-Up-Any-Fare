package com.sabre.rnt;
import java.util.Arrays;
import java.util.HashSet;

/**
 * This is a class used to represent a field from the Record class (or, more likely, a class
 * extending the Record class) which is annotated to be a form field (fillable through
 * the web interface, stored in generated text files, etc.)
 *
 * {@link FormField} is the annotation type used to mark fields described by FormFieldDesc class.
 * FormFieldDesc exposes data stored by FormField through a more typical interface (which can be
 * used directly by Velocity), and it might store some additional information as well (like the
 * name of the field).
 */
public class FormFieldDesc {
    private String fieldName;
    private FormField formField;
    private PossibleValDesc[] possibleValDescs = null;

    FormFieldDesc(String fieldName, FormField formField) {
        this.fieldName = fieldName;
        this.formField = formField;
    }

    public static class PossibleValDesc {
        private PossibleVal possibleVal;

        PossibleValDesc(PossibleVal possibleVal) {
            this.possibleVal = possibleVal;
        }

        public String getTitle() {
            return possibleVal.title();
        }

        public boolean isCheckedByDefault() {
            return possibleVal.isCheckedByDefault();
        }

        public boolean isCheckable() {
            return possibleVal.isCheckable();
        }

        public boolean isInputEnabled() {
            return possibleVal.isInputEnabled();
        }

        public String getComment() {
            return possibleVal.comment();
        }
    }

    public String getFieldName() {
        return fieldName;
    }

    public String getTitle() {
        return formField.title();
    }

    public boolean isRequired() {
        return formField.required();
    }

    public boolean isInSummary() {
        return formField.inSummary();
    }

    public FormFieldType getType() {
        return formField.type();
    }

    public HashSet<String> getDefaultValues() {
        return new HashSet<String>(Arrays.asList(formField.defaultValues()));
    }

    public int getRows() {
        return formField.rows();
    }

    public int getCols() {
        return formField.cols();
    }

    public PossibleValDesc[] getPossibleValues() {
        if (possibleValDescs == null) {
            possibleValDescs = new PossibleValDesc[formField.possibleValues().length];
            for (int i = 0; i != possibleValDescs.length; ++i) {
                possibleValDescs[i] = new PossibleValDesc(formField.possibleValues()[i]);
            }
        }
        return possibleValDescs;
    }
}