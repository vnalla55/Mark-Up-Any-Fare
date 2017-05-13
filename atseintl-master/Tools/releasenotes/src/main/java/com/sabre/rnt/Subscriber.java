package com.sabre.rnt;

@RecordConfig(
        newRecordTabLabel = "New subscription",
        editRecordTabLabel = "Edit subscriptions",
        possibleGroupNames = {
                @PossibleGroupName(name = Subscriber.SUBSCIBERS_GROUP_NAME)
        })
public class Subscriber extends Record {

    public final static String SUBSCIBERS_GROUP_NAME = "emails";
    public final static String STANDARD_RELEASES_LBL = "Standard releases";
    public final static String OTHER_RELEASES_LBL = "Other";

    @FormField(title = "E-mail address", type = FormFieldType.ONE_LINE_TEXT, required = true, inSummary = true)
    private String email = "";

    @FormField(
            title = "Subscriptions",
            type = FormFieldType.CHECKBOX_LIST,
            cols = 1,
            possibleValues = {
                    @PossibleVal(title = STANDARD_RELEASES_LBL, isCheckedByDefault = true),
                    //@PossibleVal(title = OTHER_RELEASES_LBL, isInputEnabled = true, comment = "separate with commas")
            }
    )
    private CheckboxList subscriptions = new CheckboxList();

    public CheckboxList getSubscriptions() {
        return subscriptions;
    }

    public String getEmail() {
        return email;
    }
}
