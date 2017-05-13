package com.sabre.rnt;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@interface RecordConfig {
    public String newRecordTabLabel() default "New delivery notice";
    public String editRecordTabLabel() default "Edit release files";
    public PossibleGroupName[] possibleGroupNames() default {};
}

@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface FormField {
    public String title();
    public FormFieldType type();
    public boolean required() default false;
    public boolean inSummary() default false;

    public PossibleVal[] possibleValues() default {};

    public String [] defaultValues() default {};
    //public String [] possibleValues() default {};
    public boolean otherPossible() default false;

    public int rows() default 1;
    public int cols() default Integer.MAX_VALUE;
}
