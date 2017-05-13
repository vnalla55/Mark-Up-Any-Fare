// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "TaxDisplay/EntryHelp.h"

#include "TaxDisplay/Response/LineParams.h"
#include "TaxDisplay/Response/ResponseFormatter.h"


namespace tax
{
namespace display
{

bool EntryHelp::buildBody()
{
  if (_request.entryType == TaxDisplayRequest::EntryType::ENTRY_HELP_CALCULATION_BREAKDOWN)
    printHelpCalculationBreakdown();
  else
    printHelp();

  return true;
}

void EntryHelp::printHelp()
{
  const LineParams withMargin5 = LineParams::withLeftMargin(5);
  const LineParams withMargin11 = LineParams::withLeftMargin(11);

  LineParams margin9HeadWithdrawn4;
  margin9HeadWithdrawn4.setLeftMargin(9);
  margin9HeadWithdrawn4.setParagraphIndent(-4);
  LineParams margin9HeadWithdrawn5;
  margin9HeadWithdrawn5.setLeftMargin(9);
  margin9HeadWithdrawn5.setParagraphIndent(-5);

  _formatter.addLine("TAX FORMAT TX* AND TX1* HELP DISPLAY", withMargin11)
            .addLine("TX* COMMANDS REFLECT FREE TEXT TAX FILING DETAILS "
                     "PROVIDED BY IATA TTBS AND/OR TAX CARRIERS TO ATPCO ", margin9HeadWithdrawn4)
            .addBlankLine()
            .addLine("TX1* COMMANDS REFLECT ATPCO/SABRE TAX CODING DETAILS "
                     "REFLECTING TAX CODE SEQUENCES APPLIED IN "
                     "PRICING/TICKETING", margin9HeadWithdrawn5)
            .addBlankLine()
            .addLine("COUNTRY ENTRY:")
            .addLine("TX* COMMANDS:", withMargin5)
            .addBlankLine()
            .addLine("TX*CC")
            .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE", withMargin5)
            .addLine("DISPLAYS LIST OF TAXES FOR A SPECIFIC COUNTRY", withMargin5)
            .addLine("TX*US DISPLAYS LIST OF TAXES FOR UNITED STATES", withMargin5)
            .addBlankLine()
            .addLine("TX*CN")
            .addLine("WHERE CN IS A COUNTRY NAME ", withMargin5)
            .addLine("DISPLAYS LIST OF TAXES FOR A SPECIFIC COUNTRY", withMargin5)
            .addLine("TX*MEXICO DISPLAYS LIST OF TAXES FOR MEXICO", withMargin5)
            .addBlankLine()
            .addLine("TX*AAA")
            .addLine("WHERE AAA IS A THREE CHARACTER AIRPORT/CITY CODE", withMargin5)
            .addLine("DISPLAYS LIST OF TAXES FOR A SPECIFIC COUNTRY", withMargin5)
            .addLine("TX*YVR DISPLAYS LIST OF TAXES FOR CANADA", withMargin5)
            .addBlankLine()
            .addLine("COUNTRY ENTRY: ")
            .addLine("TX1* COMMANDS:", withMargin5)
            .addBlankLine()
            .addLine("TX1*CC")
            .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE", withMargin5)
            .addLine("DISPLAYS LIST OF TAXES FOR A SPECIFIC COUNTRY", withMargin5)
            .addLine("TX1*RU DISPLAYS LIST OF TAXES FOR RUSSIA", withMargin5)
            .addLine("TX1*CN")
            .addLine("WHERE CN IS A COUNTRY NAME ", withMargin5)
            .addLine("DISPLAYS LIST OF TAXES FOR A SPECIFIC COUNTRY", withMargin5)
            .addLine("TX1*PHILIPPINES DISPLAYS LIST OF TAXES FOR PHILIPPINES", withMargin5)
            .addBlankLine()
            .addLine("TX1*AAA")
            .addLine("WHERE AAA IS A THREE CHARACTER AIRPORT/CITY CODE", withMargin5)
            .addLine("DISPLAYS LIST OF TAXES FOR A SPECIFIC COUNTRY", withMargin5)
            .addLine("TX1*HKG DISPLAYS LIST OF TAXES FOR HONG KONG SAR CHINA", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine()
            .addLine("COUNTRY/TAX CODE ENTRY:")
            .addLine("TX* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX*CC/TC")
            .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE AND TC IS A "
                     "TWO CHARACTER TAX CODE", withMargin5)
            .addLine("DISPLAYS A SPECIFIC TAX CODE FOR A COUNTRY", withMargin5)
            .addLine("TX*BY/B3 DISPLAYS VALUE ADDED TAX DOMESTIC FOR BELARUS", withMargin5)
            .addBlankLine()
            .addLine("COUNTRY/TAX CODE ENTRY:")
            .addLine("TX1* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1*CC/TC")
            .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE AND TC IS A "
                     "TWO CHARACTER TAX CODE", withMargin5)
            .addLine("DISPLAYS A SPECIFIC TAX CODE FOR A COUNTRY", withMargin5)
            .addLine("TX1*AU/UO DISPLAYS GOODS AND SERVICES TAX GST FOR "
                     "AUSTRALIA", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine();

  if (!_request.isUserTN())
  {

    _formatter.addLine("COUNTRY/TAX CODE/TAX CODE TYPE ENTRY:")
              .addLine("TX* COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX*CC/TC/TCT")
              .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE, TC IS A TWO "
                       "CHARACTER TAX CODE AND TCT IS A THREE CHARACTER TAX TYPE", withMargin5)
              .addLine("DISPLAYS A SPECIFIC TAX CODE/TAX TYPE FOR A COUNTRY", withMargin5)
              .addLine("TX*UY/UY/001 DISPLAYS TICKET SALES TAX FOR URUGUAY", withMargin5)
              .addBlankLine()
              .addLine("COUNTRY/TAX CODE/TAX CODE TYPE ENTRY:")
              .addLine("TX1* COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX1*CC/TC/TCT")
              .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE, TC IS A TWO "
                       "CHARACTER TAX CODE AND TCT IS A THREE CHARACTER TAX TYPE", withMargin5)
              .addLine("DISPLAYS A SPECIFIC TAX CODE/TAX TYPE FOR A COUNTRY", withMargin5)
              .addLine("TX1*BR/BR/001 DISPLAYS EMBARKATION FEE DOMESTIC FOR BRAZIL", withMargin5)
              .addBlankLine()
              .addSeparatorLine('*')
              .addBlankLine();
  }

  _formatter.addLine("COUNTRY/CARRIER CODE* ENTRY: TX COMMAND ONLY")
            .addLine("TX* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX*CC/XX*")
            .addLine("TX*CC/XX-XX*")
            .addLine("TX*CC/XX-XX-XX*")
            .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE AND XX IS A "
                     "TWO CHARACTER AIRLINE CODE FOLLOWED BY * (ASTRISK)", withMargin5)
            .addLine("DISPLAYS LIST OF TAXES FOR A SPECIFIC COUNTRY FOR ONLY", withMargin5)
            .addLine("REQUESTED CARRIER/S – ALSO DISPLAY TAXES FOR YY CARRIER "
                     "IN RESPONSE. UP TO 3 AIRLINE CODES ALLOWED.", withMargin5)
            .addLine("TX*TT/BA* DISPLAYS LIST OF TRINIDAD TAXES SPECIFIC TO "
                     "BA AIRLINE – WILL RETURN TAXES FOR YY CARRIER AS WELL.", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine()
            .addLine("TAX CODE ENTRY: NOTE- DOUBLE ASTRISK (**)COMMAND")
            .addLine("TX** COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX**TC")
            .addLine("WHERE TC IS A TWO CHARACTER TAX CODE", withMargin5)
            .addLine("DISPLAYS A SPECIFIC TAX CODE – SINGLE OR MULTI-COUNTRY", withMargin5)
            .addLine("TX**AY DISPLAYS AY TAX FOR UNITED STATES, CANADA, MEXICO", withMargin5)
            .addBlankLine()
            .addLine("TAX CODE ENTRY: NOTE- DOUBLE ASTRISK (**)COMMAND")
            .addLine("TX1** COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1**TC")
            .addLine("WHERE TC IS A TWO CHARACTER TAX CODE", withMargin5)
            .addLine("DISPLAYS A SPECIFIC TAX CODE – SINGLE OR MULTI-COUNTRY", withMargin5)
            .addLine("TX1**FR DISPLAYS FR TAX FOR FRANCE, FRENCH GUIANA, "
                     "MARTINIQUE, NEW CALEDONIA, FRENCH POLYNESIA, REUNION", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine();

  if (!_request.isUserTN())
  {
    _formatter.addLine("TAX CODE/TAX CODE TYPE ENTRY: NOTE- DOUBLE ASTRISK (**)COMMAND")
              .addLine("TX** COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX**TC/TCT")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE AND TCT IS A THREE "
                       "CHARACTER TAX TYPE", withMargin5)
              .addLine("DISPLAYS A SPECIFIC TAX CODE/TAX TYPE", withMargin5)
              .addLine("TX**HM/001 DISPLAYS HM TAX FOR BAHRAIN", withMargin5)
              .addBlankLine()
              .addLine("TAX CODE/TAX CODE TYPE ENTRY: NOTE- DOUBLE ASTRISK (**)COMMAND")
              .addLine("TX1** COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX1**TC/TCT")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE AND TCT IS A THREE "
                       "CHARACTER TAX TYPE", withMargin5)
              .addLine("DISPLAYS A SPECIFIC TAX CODE/TAX TYPE", withMargin5)
              .addLine("TX1**OI/001 DISPLAYS OI TAX FOR JAPAN", withMargin5)
              .addBlankLine()
              .addSeparatorLine('*')
              .addBlankLine()
              .addLine("TAX CODE/TAX CODE TYPE/SEQUENCE# ENTRY: TX1 COMMNAD ONLY ")
              .addLine("TX1** COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX1**TC/TCT/S#")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, TCT IS A THREE "
                       "CHARACTER TAX TYPE AND S# IS SEQUENCE NUMBER", withMargin5)
              .addLine("DISPLAYS A SPECIFIC TAX CODE/TAX TYPE/SEQUENCE NUMBER", withMargin5)
              .addLine("TX1**C4/001/S110000 DISPLAYS C4 TAX FOR VIETNAM", withMargin5)
              .addBlankLine()
              .addSeparatorLine('*')
              .addBlankLine();
  }

  _formatter.addLine("TAX CODE/C# NUMBER ENTRY: ONE OR MORE C# ALLOWED")
            .addLine("TX** COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX**TC/C# (SINGLE CATEGORY REQUEST)")
            .addLine("TX**TC/C#,#,# (MULTIPLE CATEGORY REQUEST)")
            .addLine("TX**TC/C#-# (RANGE CATEGORY REQUEST) ")
            .addLine("WHERE TC IS A TWO CHARACTER TAX CODE AND C# IS CATEGORY "
                     "NUMBER/S FOR CATEGORY DISPLAY OF A TAX CODE", withMargin5)
            .addLine("TX**OP/C3,7 DISPLAYS CATEGORY 3 AND 7 FOR TAX CODE OP", withMargin5)
            .addBlankLine()
            .addLine("TAX CODE/C# NUMBER ENTRY: ONE OR MORE C# ALLOWED")
            .addLine("TX1** COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1**TC/C#(SINGLE CATEGORY REQUEST)")
            .addLine("TX1**TC/C#,#,#(MULTIPLE CATEGORY REQUEST)")
            .addLine("TX1**TC/C#-#(RANGE CATEGORY REQUEST)")
            .addLine("WHERE TC IS A TWO CHARACTER TAX CODE AND C# IS CATEGORY "
                     "NUMBER/S FOR CATEGORY DISPLAY OF A TAX CODE ", withMargin5)
            .addLine("CATEGORY#1 WILL DISPLAY WITH ANY REQUESTED CATEGORY# ", withMargin5)
            .addLine("TX1**OP/C3,7 DISPLAYS CATEGORY 3 AND 7 FOR TAX CODE OP", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine();

  if (!_request.isUserTN())
  {
    _formatter.addLine("TAX CODE/TAX CODE TYPE/C# NUMBER ENTRY: ONE OR MORE C# ALLOWED")
              .addLine("TX** COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX**TC/TCT/C# (SINGLE CATEGORY REQUEST)")
              .addLine("TX**TC/TCT/C#,#,# (MULTIPLE CATEGORY REQUEST)")
              .addLine("TX**TC/TCT/C#-# (RANGE CATEGORY REQUEST) ")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, TCT IS A THREE "
                       "CHARACTER TAX TYPE AND C# IS CATEGORY NUMBER/S FOR", withMargin5)
              .addLine("CATEGORY DISPLAY OF A TAX CODE/TAX TYPE", withMargin5)
              .addLine("TX**OP/001/C3,7 DISPLAYS CATEGORY 3 AND 7 FOR TAX CODE OP", withMargin5)
              .addBlankLine()
              .addLine("TAX CODE/TAX CODE TYPE/C# NUMBER ENTRY: ONE OR MORE C# ALLOWED")
              .addLine("TX1** COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX1**TC/TCT/C#(SINGLE CATEGORY REQUEST)")
              .addLine("TX1**TC/TCT/C#,#,#(MULTIPLE CATEGORY REQUEST)")
              .addLine("TX1**TC/TCT/C#-#(RANGE CATEGORY REQUEST)")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, TCT IS A THREE "
                       "CHARACTER TAX TYPE AND C# IS CATEGORY NUMBER/S FOR "
                       "CATEGORY DISPLAY OF A TAX CODE/TAX TYPE ", withMargin5)
              .addLine("CATEGORY#1 WILL DISPLAY WITH ANY REQUESTED CATEGORY# ", withMargin5)
              .addLine("TX1**OP/001/C3,7 DISPLAYS CATEGORY 3 AND 7 FOR TAX CODE OP", withMargin5)
              .addBlankLine()
              .addSeparatorLine('*')
              .addBlankLine();
  }

  _formatter.addLine("TAX CODE/S#/C# NUMBER ENTRY: TX1 COMMAND ONLY")
            .addLine("TX1** COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1**TC/S#/C# (SINGLE CATEGORY REQUEST)")
            .addLine("TX1**TC/S#/C#,#,# (MULTIPLE CATEGROY REQUEST)")
            .addLine("TX1**TC/S#/C#-# (RANGE CATEGORY REQUEST)")
            .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, S# IS A SEQUENCE "
                     "NUMBER AND C# IS CATEGORY NUMBER/S FOR CATEGORY DISPLAY "
                     "OF A TAX CODE/SEQUENCE #", withMargin5)
            .addLine("CATEGORY#1 WILL DISPLAY WITH ANY REQUESTED CATEGORY#", withMargin5)
            .addLine("TX1**ZV/S115000/C2,4,6,7,9 DISPLAYS CATEGORY 1,2,4,7,9 "
                     "FOR TAX CODE ZV", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine();

  if (!_request.isUserTN())
  {
    _formatter.addLine("TAX CODE/TAX CODE TYPE/S#/C# NUMBER ENTRY: TX1 COMMAND ONLY")
              .addLine("TX1** COMMAND:", withMargin5)
              .addBlankLine()
              .addLine("TX1**TC/TCT/S#/C# (SINGLE CATEGORY REQUEST)")
              .addLine("TX1**TC/TCT/S#/C#,#,# (MULTIPLE CATEGROY REQUEST)")
              .addLine("TX1**TC/TCT/S#/C#-# (RANGE CATEGORY REQUEST)")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, TCT IS A THREE "
                       "CHARACTER TAX TYPE, S# IS A SEQUENCE NUMBER AND C# "
                       "IS CATEGORY NUMBER/S FOR CATEGORY DISPLAY OF A "
                       "TAX CODE/TAX TYPE/SEQUENCE #", withMargin5)
              .addLine("CATEGORY#1 WILL DISPLAY WITH ANY REQUESTED CATEGORY#", withMargin5)
              .addLine("TX1**ZV/001/S115000/C2,4,6,7,9 DISPLAYS CATEGORY 1,2,4,7,9 "
                       "FOR TAX CODE ZV", withMargin5)
              .addBlankLine()
              .addSeparatorLine('*')
              .addBlankLine();

  }

  _formatter.addLine("TAX CODE/C# NUMBER ENTRY FOR TAX CODE CABIN DATA:")
            .addLine("TX** COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX**TC/C2");


  if (!_request.isUserTN())
  {
    _formatter.addLine("TX**TC/TCT/C2")
              .addLine("(CABIN DATA NORMALLY REFLECTED IN CATEGORY # 2)", withMargin5)
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, TCT IS A THREE "
                       "CHARACTER TAX TYPE, AND C2 IS FOR CATEGORY #2 FOR DISPLAY "
                       "OF CABIN DATA", withMargin5);
  }
  else
  {
    _formatter.addLine("(CABIN DATA NORMALLY REFLECTED IN CATEGORY # 2)", withMargin5)
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE AND C2 IS FOR CATEGORY #2 FOR DISPLAY "
                       "OF CABIN DATA", withMargin5);
  }

  _formatter.addLine("TX**GB/C2 DISPLAYS CATEGORY 2 CABIN DATA FOR TAX CODE GB", withMargin5)
            .addBlankLine()
            .addLine("TAX CODE/C# NUMBER ENTRY FOR TAX CODE CABIN DATA:")
            .addLine("TX1** COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1**TC/C11");

  if (!_request.isUserTN())
  {
    _formatter.addLine("TX1**TC/TCT/C11")
              .addLine("TX1**TC/S#/C11")
              .addLine("TX1**TC/TCT/S#/C11")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, TCT IS THREE "
                       "CHARACTER TAX TYPE, S# IS SEQUENCE NUMBER, AND C11 IS FOR "
                       "CATEGORY 11 – CABIN DATA", withMargin5);

  }
  else
  {
    _formatter.addLine("TX1**TC/S#/C11")
              .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, "
                       "S# IS SEQUENCE NUMBER, AND C11 IS FOR "
                       "CATEGORY 11 – CABIN DATA", withMargin5);
  }

  _formatter.addLine("CATEGORY#1 WILL DISPLAY WITH ANY REQUESTED CATEGORY# ", withMargin5)
            .addLine("TX1**GB/C11 DISPLAYS CATEGORY 11 AND 1 FOR TAX CODE GB", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine()
            .addLine("TAX # ENTRY: BASED ON PREVIOUS DISPLAY REFLECTING LINE NUMBERS ")
            .addLine("TX* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX*#")
            .addLine("WHERE # IS A LINE NUMBER ASSOCIATED TO PREVIOUS TX* ENTRY", withMargin5)
            .addLine("WHERE A LINE NUMBER IS DISPLAYED", withMargin5)
            .addBlankLine()
            .addLine("ADDITIONAL TX*# LINE NUMBER QUALIFIER ENTRIES BASED ON PREVIOUS")
            .addLine("TX ENTRIES WHICH RETURN A DISPLAY WITH LINE #:")
            .addBlankLine()
            .addLine("TX*#/C# (SINGLE CATEGORY REQUEST)")
            .addLine("TX*#/C#,#,#,# (MULTIPLE CATEGORY REQUEST)")
            .addLine("TX*#/C#-# (CATEGORY RANGE REQUEST)")
            .addLine("TX*#/R(TAX REISSUE REQUEST FOR SELECTED TAX CODE)")
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine()
            .addLine("TAX # ENTRY: BASED ON PREVIOUS DISPLAY REFLECTING LINE NUMBERS ")
            .addLine("TX1* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1*#")
            .addLine("WHERE # IS A LINE NUMBER ASSOCIATED TO PREVIOUS TX1* ENTRY")
            .addLine("WHERE A LINE NUMBER IS DISPLAYED")
            .addBlankLine()
            .addLine("ADDITIONAL TX1*# LINE NUMBER QUALIFIER ENTRIES BASED ON PREVIOUS")
            .addLine("TX1 ENTRIES WHICH RETURN A DISPLAY WITH LINE #:")
            .addBlankLine()
            .addLine("TX1*#/C#(SINGLE CATEGORY REQUEST)")
            .addLine("TX1*#/C#,#,#,#(MULTIPLE CATEGORY REQUEST)")
            .addLine("TX1*#/C#-#(CATEGORY RANGE REQUEST)")
            .addLine("TX1*#/S#(SEQ NUMBER REQUEST)")
            .addLine("TX1*#/S#/C# (SEQ #/CAT SINGLE # REQUEST)")
            .addLine("TX1*#/S#/C#,#,# (SEQ #/CAT MULTIPLE # REQUEST)")
            .addLine("TX1*#/S#/C#-# (SEQ #/CAT RANGE # REQUEST)")
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine()
            .addLine("TAX RE-DISPLAY: RE-DISPLAY PREVIOUS TX* OR TX1* ENTRY")
            .addLine("TX* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX*")
            .addLine("WHERE TX* DISPLAYS PREVIOUS TX* REQUEST/DISPLAY RESPONSE", withMargin5)
            .addBlankLine()
            .addLine("TAX RE-DISPLAY: RE-DISPLAY PREVIOUS TX* OR TX1* ENTRY")
            .addLine("TX1* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1*")
            .addLine("WHERE TX1* DISPLAYS PREVIOUS TX1* REQUEST/DISPLAY RESPONSE", withMargin5)
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine()
            .addLine("TAX CODE/R REFUND/REISSUE DETAILS – TX COMMAND ONLY")
            .addLine("TX* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX*CC/R")
            .addLine("WHERE CC IS A TWO CHARACTER COUNTRY CODE, FOLLOWED BY /R", withMargin5)
            .addLine("DISPLAYS A COUNTRIES REFUND/REISSUE DETAILS ASSOCIATED TO "
                     "SPECIFIC COUNTRY TAXES", withMargin5)
            .addLine("TX*SA/R DISPLAYS SAUDI ARABIA TAX DETAILS", withMargin5)
            .addBlankLine()
            .addLine("TX**TC/R")
            .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, FOLLOWED BY /R", withMargin5)
            .addLine("DISPLAYS TAX CODE REFUND/REISSUE DETAILS", withMargin5)
            .addLine("TX**MX/R DISPLAYS MX TAX CODE DETAILS", withMargin5)
            .addBlankLine();

  if (!_request.isUserTN())
  {
    _formatter.addLine("TX**TC/TCT/R")
            .addLine("WHERE TC IS A TWO CHARACTER TAX CODE, TCT IS A THREE "
                     "CHARACTER TAX CODE TYPE, FOLLOWED BY /R", withMargin5)
            .addLine("DISPLAYS TAX CODE/TAX TYPE REFUND/REISSUE DETAILS", withMargin5)
            .addLine("TX**MX/103/R DISPLAYS MX TAX CODE/TYPE 103 TAX DETAILS", withMargin5)
            .addBlankLine();
  }

  _formatter.addSeparatorLine('*')
            .addBlankLine()
            .addLine("HISTORICAL DATE ENTRY: ADD –DDMMMYY AT END OF TX* / TX1* ENTRY "
                     "SPECIFIC TX* AND TX1* ENTRIES ALLOWED FOR DATE SELECTION ", withMargin5)
            .addBlankLine()
            .addLine("EFFECTIVE DATE ENTRIES:")
            .addLine("TX* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX*CC-DDMMMYY ")
            .addLine("TX*CC/TC-DDMMMYY ");

  if (!_request.isUserTN())
    _formatter.addLine("TX*CC/TC/TCT-DDMMMYY ");

  _formatter.addLine("TX**TC-DDMMMYY ");

  if (!_request.isUserTN())
    _formatter.addLine("TX**TC/TCT-DDMMMYY");

  _formatter.addLine("TX**TC/C#-DDMMMYY")
            .addLine("TX**TC/C#,#,#-DDMMMYY")
            .addLine("TX**TC/C#-#-DDMMMYY ");

  if (!_request.isUserTN())
  {
    _formatter.addLine("TX**TC/TCT/C#-DDMMMYY ")
              .addLine("TX**TC/TCT/C#,#,#-DDMMMYY ")
              .addLine("TX**TC/TCT/C#-#-DDMMMYY ");
  }

  _formatter.addLine("TX*#-DDMMMYY ")
            .addLine("TX*CC/XX*-DDMMMYY")
            .addLine("TX**TC/R-DDMMMYY")
            .addLine("TX*#-DDMMMYY ")
            .addBlankLine()
            .addLine("EFFECTIVE DATE ENTRIES: ")
            .addLine("TX1* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1*CC-DDMMMYY")
            .addLine("TX1*CC/TC-DDMMMYY");

  if (!_request.isUserTN())
  {
    _formatter.addLine("TX1*CC/TC/TCT-DDMMMYY")
              .addLine("TX1*CC/TC/TCT/S#-DDMMMYY");
  }

  _formatter.addLine("TX1**TC-DDMMMYY")
            .addLine("TX1**TC/S#-DDMMMYY")
            .addLine("TX1**TC/C#-DDMMMYY")
            .addLine("TX1**TC/C#,#,#-DDMMMYY")
            .addLine("TX1**TC/C#-#-DDMMMYY")
            .addLine("TX1**TC/S#/C#-DDMMMYY")
            .addLine("TX1**TC/S#/C#,#,#-DDMMMYY")
            .addLine("TX1**TC/S#/C#-#-DDMMMYY");

  if (!_request.isUserTN())
  {
    _formatter.addLine("TX1**TC/TCT-DDMMMYY")
              .addLine("TX1**TC/TCT/S#-DDMMMYY")
              .addLine("TX1**TC/TCT/C#-DDMMMYY")
              .addLine("TX1**TC/TCT/C#,#,#-DDMMMYY")
              .addLine("TX1**TC/TCT/C#-#-DDMMMYY")
              .addLine("TX1**TC/TCT/S#/C#-DDMMMYY")
              .addLine("TX1**TC/TCT/S#/C#,#,#-DDMMMYY")
              .addLine("TX1**TC/TCT/S#/C#-#-DDMMMYY");
  }

  _formatter.addLine("TX1*#-DDMMMYY")
            .addBlankLine()
            .addLine("EFFECTIVE DATE AND TRAVEL DATE ENTRIES: TX1 COMMAND ONLY ")
            .addLine("TX1* COMMAND:", withMargin5)
            .addBlankLine()
            .addLine("TX1*CC-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC/S#-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC/C#-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC/C#,#,#-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC/C#-#-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC/S#/C#-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC/S#/C#,#,#-DDMMMYY-TDDMMMYY")
            .addLine("TX1**TC/S#/C#-#-DDMMMYY-TDDMMMYY");

  if (!_request.isUserTN())
  {
    _formatter.addLine("TX1**TC/TCT-DDMMMYY-TDDMMMYY")
              .addLine("TX1**TC/TCT/S#-DDMMMYY-TDDMMMYY")
              .addLine("TX1**TC/TCT/C#-DDMMMYY-TDDMMMYY")
              .addLine("TX1**TC/TCT/C#,#,#-DDMMMYY-TDDMMMYY")
              .addLine("TX1**TC/TCT/C#-#-DDMMMYY-TDDMMMYY")
              .addLine("TX1**TC/TCT/S#/C#-DDMMMYY-TDDMMMYY")
              .addLine("TX1**TC/TCT/S#/C#,#,#-DDMMMYY-TDDMMMYY")
              .addLine("TX1**TC/TCT/S#/C#-#-DDMMMYY-TDDMMMYY");
  }

  _formatter.addLine("TX1*#-DDMMMYY-TDDMMMYY")
            .addBlankLine()
            .addSeparatorLine('*')
            .addBlankLine()
            .addLine("FOR COMPLETE DETAILS OR ADDITIONAL DETAILS")
            .addLine("REFER TO YOUR APPROPRIATE REFERENCE SOURCE ")
            .addLine(".");
}

void EntryHelp::printHelpCalculationBreakdown()
{
  const LineParams withIndent3 = LineParams::withLeftMargin(3);
  const LineParams withIndent5 = LineParams::withLeftMargin(5);

_formatter.addLine("SPECIFIC TAX CALCULATION ENTRIES APPLICABLE TO "
                   "US TAXES AND US MARKETS ONLY. ", withIndent5)
          .addLine("TAX CALCULATION FOR FARE BASE/TOTAL ONLY REFLECT "
                   "US DOMESTIC TAX BREAKDOWN. ", withIndent5)
          .addLine("TAX CALCULATION INVOLVING ALASKA/HAWAII BY MARKET PAIR, "
                   "BETWEEN ALASKA/HAWAII AND CONTINENTAL U.S.OR BETWEEN/WITHIN "
                   "ALASKA AND HAWAII REFLECT BREAKDOWN OF: ", withIndent5)
          .addLine("BASE/TOTAL FARE, US INTERNATIONAL TAX,PART OF US DOMESTIC "
                   "TAX APPLIED, AND THE ALASKA/HAWAII PERCENTAGE FACTOR.", withIndent5)
          .addBlankLine()
          .addBlankLine()
          .addLine("TX/T99999.99 - T=TOTAL")
          .addLine("TX/B99999.99 - B=BASE")
          .addLine("ALASKA/HAWAII-CONTINENTAL US – CCCCCC=MARKET PAIR")
          .addLine("TX/TCCCCCC/X/99999.99 - T=TOTAL/X=ONE WAY (OW)")
          .addLine("TX/TCCCCCC/R/99999.99 - T=TOTAL/R=ROUND TRIP (RT)")
          .addLine("TX/BCCCCCC/X/99999.99 - B=BASE/X=ONE WAY (OW)")
          .addLine("TX/BCCCCCC/R/99999.99 - B=BASE/R=ROUND TRIP (RT)")
          .addBlankLine()
          .addLine("EXAMPLE –")
          .addLine("CALCULATION WITHIN CONTINENTAL US - TOTAL (1000.00)", withIndent3)
          .addLine("TX/T1000.00")
          .addLine("930.23/69.77")
          .addLine(".")
          .addLine("CALCULATION WITHIN CONTINENTAL US - BASE (930.23)", withIndent3)
          .addLine("TX/B930.23«")
          .addLine("1000.00/69.77")
          .addLine(".")
          .addLine("CALCULATION BETWEEN HNL/ANC - TOTAL (1000.00) - OW", withIndent3)
          .addLine("TX/THNLANC/X/1000.00")
          .addLine("987.45/8.90/3.65 - TAX PCT. 0.37")
          .addLine(".")
          .addLine(" CALCULATION BETWEEN HNL/LAX-BASE (983.70 ) - RT ", withIndent3)
          .addLine("TX/BHNLLAX/R/981.91«")
          .addLine("1000.00/17.80/0.29 - TAX PCT. 0.03")
          .addLine(".")
          .addLine("NOTE -")
          .addLine("AN OPTIONAL DATE MAY BE APPENDED TO THE INPUTS.")
          .addLine("I.E. APPEND -DDMMMYY DEFAULT IS TODAY.")
          .addBlankLine()
          .addLine("EXAMPLE –")
          .addLine("TX/T1000-01APR16«")
          .addLine("930.23/69.77")
          .addLine(".")
          .addLine("TX/B930.23-10APR16«")
          .addLine("1000.00/69.77 ")
          .addLine(".")
          .addLine("TX/THNLLAX/R/1000.00-01DEC14«")
          .addLine("982.31/17.40/0.29 - TAX PCT. 0.03 ")
          .addLine(".")
          .addLine("TX/BHNLANC/X/987.65-31DEC14« ")
          .addLine("1000.00/8.70/3.65 - TAX PCT. 0.37 ")
          .addLine(".")
          .addLine("END OF SCROLL");
}

} /* namespace display */
} /* namespace tax */
