package com.sabre.rnt;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target(ElementType.ANNOTATION_TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface PossibleVal {
    public String title();
    public boolean isCheckedByDefault() default false;
    public boolean isCheckable() default true;
    public boolean isInputEnabled() default false;
    public String comment() default "";
}
