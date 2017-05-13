package com.sabre.rnt;

@RecordConfig(
        newRecordTabLabel = "New delivery notice",
        editRecordTabLabel = "Edit release files",
        possibleGroupNames = {
                @PossibleGroupName(name = "ATSEv2"),
                @PossibleGroupName(name = "RTG"),
                @PossibleGroupName(name = "PDC")
        })
public class ReleaseNote extends Record {

    @FormField(title = "Title", type = FormFieldType.ONE_LINE_TEXT, required = true, inSummary = true)
    private String title;

    @FormField(title = "Date", type = FormFieldType.DATE, required = true, inSummary = true)
    private String date;

    @FormField(title = "Description", type = FormFieldType.RICH_TEXT, required = true)
    private String description;

    @FormField(
            title = "Applies to",
            type = FormFieldType.CHECKBOX_LIST,
            possibleValues = {
                    @PossibleVal(title = "all agencies", isCheckedByDefault = true),
                    @PossibleVal(title = "all airlines", isCheckedByDefault = true),
                    @PossibleVal(title = "Abacus"),
                    @PossibleVal(title = "Axess"),
                    @PossibleVal(title = "LAN"),
                    @PossibleVal(title = "Other", isInputEnabled = true)
            },
            defaultValues = {
                    "all agencies",
                    "all airlines"
            }
        )
    private CheckboxList appliesTo = new CheckboxList();

    @FormField(title = "Activated by/Configuration change", type = FormFieldType.RICH_TEXT, required = true)
    private String activatedByConfigurationChange;

    @FormField(title = "Resolution/QA recommendation", type = FormFieldType.RICH_TEXT)
    private String resolutionQARecommendation;

    @FormField(title = "Entry/XML to recreate", type = FormFieldType.RICH_TEXT)
    private String entryXMLToRecreate;

    @FormField(title = "Example before", type = FormFieldType.RICH_TEXT, required = true)
    private String exampleBefore;

    @FormField(title = "Example after", type = FormFieldType.RICH_TEXT, required = true)
    private String exampleAfter;

    @FormField(title = "Files updated", type = FormFieldType.RICH_TEXT)
    private String filesUpdated;

    @FormField(title = "Unit tests files", type = FormFieldType.RICH_TEXT)
    private String unitTestsFiles;

    @FormField(title = "DB",
            type = FormFieldType.CHECKBOX_LIST,
            cols = 1,
            possibleValues = {
                    @PossibleVal(title = "DB query update", isCheckable = true, isInputEnabled = true),
                    @PossibleVal(title = "DAO version number change", isCheckable = true, isInputEnabled = true),
                    @PossibleVal(title = "DBA contact", isCheckable = false, isInputEnabled = true)
            }
    )
    private CheckboxList dbChanges = new CheckboxList();

    @FormField(title = "Application impact",
            type = FormFieldType.CHECKBOX_LIST,
            cols = 4,
            possibleValues = {
                    @PossibleVal(title = "WP", isCheckedByDefault = true),
                    @PossibleVal(title = "WPNC", isCheckedByDefault = true),
                    @PossibleVal(title = "WPA", isCheckedByDefault = true),
                    @PossibleVal(title = "Shopping", isCheckedByDefault = true),
                    @PossibleVal(title = "Fare Display", isCheckedByDefault = true),
                    @PossibleVal(title = "Historical", isCheckedByDefault = true),
                    @PossibleVal(title = "Taxes"),
                    @PossibleVal(title = "Currency"),
                    @PossibleVal(title = "Mileage"),
                    @PossibleVal(title = "Baggage"),
                    @PossibleVal(title = "Reissue/Exchange"),
                    @PossibleVal(title = "Other", isInputEnabled = true)
            }
    )
    private CheckboxList applicationImpact = new CheckboxList();

    @FormField(title = "Schema update",
            type = FormFieldType.CHECKBOX_LIST,
            possibleValues = {
                    @PossibleVal(title = "Yes")
            }
    )
    private CheckboxList schemaUpdate = new CheckboxList();

    @FormField(title = "Developer",
            type = FormFieldType.ONE_LINE_TEXT,
            required = true,
            inSummary = true
    )
    private String developer;

    @FormField(title = "Code reviewers",
            type = FormFieldType.ONE_LINE_TEXT,
            required = true
    )
    private String codeReviewers;

    @FormField(title = "BA",
            type = FormFieldType.ONE_LINE_TEXT
    )
    private String ba;

    @FormField(title = "QA",
            type = FormFieldType.ONE_LINE_TEXT
    )
    private String qa;

    /**
     * @return the title
     */
    public String getTitle() {
        return title;
    }

    /**
     * @param title the title to set
     */
    public void setTitle(String title) {
        this.title = title;
    }

    /**
     * @return the date
     */
    public String getDate() {
        return date;
    }

    /**
     * @param date the date to set
     */
    public void setDate(String date) {
        this.date = date;
    }

    /**
     * @return the appliesTo
     */
    public CheckboxList getAppliesTo() {
        return appliesTo;
    }

    /**
     * @param appliesTo the appliesTo to set
     */
    public void setAppliesTo(CheckboxList appliesTo) {
        this.appliesTo = appliesTo;
    }

    /**
     * @return the activatedByConfigurationChange
     */
    public String getActivatedByConfigurationChange() {
        return activatedByConfigurationChange;
    }

    /**
     * @param activatedByConfigurationChange the activatedByConfigurationChange to set
     */
    public void setActivatedByConfigurationChange(String activatedByConfigurationChange) {
        this.activatedByConfigurationChange = activatedByConfigurationChange;
    }

    /**
     * @return the resolutionQARecommendation
     */
    public String getResolutionQARecommendation() {
        return resolutionQARecommendation;
    }

    /**
     * @param resolutionQARecommendation the resolutionQARecommendation to set
     */
    public void setResolutionQARecommendation(String resolutionQARecommendation) {
        this.resolutionQARecommendation = resolutionQARecommendation;
    }

    /**
     * @return the entryXMLToRecreate
     */
    public String getEntryXMLToRecreate() {
        return entryXMLToRecreate;
    }

    public void setEntryXMLToRecreate(String entryXMLToRecreate) {
        this.entryXMLToRecreate = entryXMLToRecreate;
    }

    public String getExampleBefore() {
        return exampleBefore;
    }

    public void setExampleBefore(String exampleBefore) {
        this.exampleBefore = exampleBefore;
    }

    public String getExampleAfter() {
        return exampleAfter;
    }

    public void setExampleAfter(String exampleAfter) {
        this.exampleAfter = exampleAfter;
    }

    public String getFilesUpdated() {
        return filesUpdated;
    }

    public void setFilesUpdated(String filesUpdated) {
        this.filesUpdated = filesUpdated;
    }

    /**
     * @return the unitTestsFiles
     */
    public String getUnitTestsFiles() {
        return unitTestsFiles;
    }

    /**
     * @param unitTestsFiles the unitTestsFiles to set
     */
    public void setUnitTestsFiles(String unitTestsFiles) {
        this.unitTestsFiles = unitTestsFiles;
    }

    /**
     * @return the developer
     */
    public String getDeveloper() {
        return developer;
    }

    /**
     * @param developer the developer to set
     */
    public void setDeveloper(String developer) {
        this.developer = developer;
    }

    /**
     * @return the codeReviewers
     */
    public String getCodeReviewers() {
        return codeReviewers;
    }

    /**
     * @param codeReviewers the codeReviewers to set
     */
    public void setCodeReviewers(String codeReviewers) {
        this.codeReviewers = codeReviewers;
    }

    /**
     * @return the ba
     */
    public String getBa() {
        return ba;
    }

    /**
     * @param ba the ba to set
     */
    public void setBa(String ba) {
        this.ba = ba;
    }

    /**
     * @return the qa
     */
    public String getQa() {
        return qa;
    }

    /**
     * @param qa the qa to set
     */
    public void setQa(String qa) {
        this.qa = qa;
    }

    /**
     * @return the description
     */
    public String getDescription() {
        return description;
    }

    /**
     * @param description the description to set
     */
    public void setDescription(String description) {
        this.description = description;
    }

}
