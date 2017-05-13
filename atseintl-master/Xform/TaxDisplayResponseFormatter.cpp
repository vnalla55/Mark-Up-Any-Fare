//----------------------------------------------------------------------------
//
//  File:  TaxDisplayResponseFormatter.cpp
//  Description: See TaxDisplayResponseFormatter.h file
//  Created: August, 2007
//  Authors:  Dean van Decker
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Xform/TaxDisplayResponseFormatter.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category1.h"
#include "Taxes/LegacyTaxes/Category10.h"
#include "Taxes/LegacyTaxes/Category11.h"
#include "Taxes/LegacyTaxes/Category12.h"
#include "Taxes/LegacyTaxes/Category13.h"
#include "Taxes/LegacyTaxes/Category14.h"
#include "Taxes/LegacyTaxes/Category15.h"
#include "Taxes/LegacyTaxes/Category16.h"
#include "Taxes/LegacyTaxes/Category17.h"
#include "Taxes/LegacyTaxes/Category18.h"
#include "Taxes/LegacyTaxes/Category2.h"
#include "Taxes/LegacyTaxes/Category3.h"
#include "Taxes/LegacyTaxes/Category4.h"
#include "Taxes/LegacyTaxes/Category5.h"
#include "Taxes/LegacyTaxes/Category6.h"
#include "Taxes/LegacyTaxes/Category7.h"
#include "Taxes/LegacyTaxes/Category8.h"
#include "Taxes/LegacyTaxes/Category9.h"
#include "Taxes/LegacyTaxes/CategorySeqDataVerbiage.h"
#include "Taxes/LegacyTaxes/Reissue.h"
#include "Taxes/LegacyTaxes/TaxDisplayFormatter.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxPercentageUS.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Xform/PricingResponseXMLTags.h"

#include <vector>

using namespace tse;
using namespace std;

static Logger
logger("atseintl.Xform.TaxDisplayResponseFormatter");

const std::string TaxDisplayResponseFormatter::XML_DECLARATION_TAG_TEXT =
    "xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"";

const std::string TaxDisplayResponseFormatter::TAX_DISPLAY_XML_VERSION_TEXT = "2003A.TsabreXML1.0";

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::TaxDisplayResponseFormatter
//----------------------------------------------------------------------------
TaxDisplayResponseFormatter::TaxDisplayResponseFormatter()
  : XML_NAMESPACE_TEXT(readConfigXMLNamespace("OTA_XML_NAMESPACE")),
    XML_NAMESPACE_XS_TEXT(readConfigXMLNamespace("OTA_XML_NAMESPACE_XS")),
    XML_NAMESPACE_XSI_TEXT(readConfigXMLNamespace("OTA_XML_NAMESPACE_XSI")),
    isAxess(false)
{
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::~TaxDisplayResponseFormatter
//----------------------------------------------------------------------------
TaxDisplayResponseFormatter::~TaxDisplayResponseFormatter() {}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::vdPrefix(
//----------------------------------------------------------------------------

void
TaxDisplayResponseFormatter::vdPrefix(TaxTrx& taxTrx)
{
  // AXESS fix (VD prefix)
  std::vector<Customer*> customerVec =
      taxTrx.dataHandle().getCustomer(taxTrx.billing()->userPseudoCityCode());
  if (!customerVec.empty())
  {
    Customer* customer = customerVec.front();

    if (customer->crsCarrier() == AXESS_MULTIHOST_ID)
    {
      isAxess = true;
      taxTrx.response() << "VD" << endl;
    }
  }
}
//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::formatResponse(TaxTrx& taxTrx)
{
  XMLConstruct construct;
  addResponseHeader(taxTrx.taxDisplayRequestRootElementType(), construct);

  construct.openElement("Success");
  construct.closeElement();

  if (taxTrx.getRequest()->help())
  {
    vdPrefix(taxTrx);
    if (taxTrx.getRequest()->helpTaxUS())
    {
      buildTaxUSHelpInfo(taxTrx);
    }
    else
    {
      buildHelpInfo(taxTrx);
    }
    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (TaxPercentageUS::validConUSTrip(taxTrx.getRequest()))
  {
    vdPrefix(taxTrx);
    TaxPercentageUS percentage(*(taxTrx.getRequest()));
    percentage.taxContinentalBuild();
    taxTrx.response() << percentage.display() << std::endl;

    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (TaxPercentageUS::validHiAkConUSTrip(taxTrx.getRequest()))
  {
    vdPrefix(taxTrx);
    TaxPercentageUS percentage(*(taxTrx.getRequest()));
    percentage.taxFactorsBuild();
    taxTrx.response() << percentage.display() << std::endl;

    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (taxTrx.itin().empty())
  {
    LOG4CXX_DEBUG(logger, "No Itinerary");
    taxTrx.response() << "SYSTEM MEMORY ERROR" << endl;
    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (taxTrx.itin().front()->getTaxResponses().empty())
  {
    LOG4CXX_DEBUG(logger, "No Tax Response");
    taxTrx.response() << "SYSTEM MEMORY ERROR" << endl;
    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (taxTrx.itin().front()->getTaxResponses().front()->taxDisplayItemVector().empty())
  {
    LOG4CXX_DEBUG(logger, "INVALID TAX INFORMATION");

    taxTrx.response() << "INVALID TAX INFORMATION" << endl;

    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (taxTrx.itin()
          .front()
          ->getTaxResponses()
          .front()
          ->taxDisplayItemVector()
          .front()
          ->errorMsg() != EMPTY_STRING())
  {

    taxTrx.response() << taxTrx.itin()
                             .front()
                             ->getTaxResponses()
                             .front()
                             ->taxDisplayItemVector()
                             .front()
                             ->errorMsg() << std::endl;

    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (taxTrx.getRequest()->sequenceNumber() != TaxRequest::EMPTY_SEQUENCE)
  {
    if (taxTrx.getRequest()->sequenceNumber() == taxTrx.getRequest()->sequenceNumber2())
    {
      LOG4CXX_DEBUG(logger, "same comparision sequence");
      taxTrx.response() << "SAME SEQUENCE COMPARISON NOT VALID" << endl;
      buildMessage(taxTrx.response().str(), construct);
      addResponseFooter(construct);
      taxTrx.response().str(EMPTY_STRING());
      taxTrx.response() << construct.getXMLData();
      return;
    }
  }

  if (taxTrx.getRequest()->getReissue())
  {
    vdPrefix(taxTrx);
    buildStandardHeader(taxTrx);
    buildReissue(taxTrx);
    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (!taxTrx.getRequest()->nation().empty())
  {
    vdPrefix(taxTrx);
    buildListNationDisplay(taxTrx, construct);
  }

  if (taxTrx.getRequest()->menu())
  {
    vdPrefix(taxTrx);
    buildMenu(taxTrx);
    buildMessage(taxTrx.response().str(), construct);
    addResponseFooter(construct);
    taxTrx.response().str(EMPTY_STRING());
    taxTrx.response() << construct.getXMLData();
    return;
  }

  if (!taxTrx.getRequest()->taxCode().empty())
  {
    vdPrefix(taxTrx);
    buildStandardHeader(taxTrx);
    TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();

    if (taxResponse->taxDisplayItemVector().front() != taxResponse->taxDisplayItemVector().back())
    {
      buildMultiSequence(taxTrx);
    }
    else
    {
      buildSequence(taxTrx);
    }
  }

  buildMessage(taxTrx.response().str(), construct);
  addResponseFooter(construct);
  taxTrx.response().str(EMPTY_STRING());
  taxTrx.response() << construct.getXMLData();
  return;
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildMessage
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildMessage(const std::string& response,
                                          XMLConstruct& construct,
                                          std::string msgType)
{
  // Attaching MSG elements

  std::string tmpResponse = response;

  TaxDisplayFormatter dispFmt(60);
  dispFmt.format(tmpResponse);

  LOG4CXX_DEBUG(logger, "Before XML:\n" << tmpResponse);

  unsigned int lastPos = 0;
  int recNum = 2;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  char tmpBuf[BUF_SIZE];

  // Clobber the trailing newline
  while (1)
  {
    lastPos = tmpResponse.rfind("\n");
    if (lastPos > 0 && lastPos == (tmpResponse.length() - 1))
      tmpResponse.replace(lastPos, 1, "\0");
    else
      break;
  }
  char* pHolder = nullptr;
  int tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), "\n", &pHolder); token != nullptr;
       token = strtok_r(nullptr, "\n", &pHolder), recNum++)
  {
    tokenLen = strlen(token);
    if (tokenLen == 0)
      continue;
    else if (tokenLen > AVAILABLE_SIZE)
    {
      LOG4CXX_WARN(logger, "Line: [" << token << "] too long!");
      continue;
    }
    construct.openElement(xml2::MessageInformation);
    construct.addAttribute(xml2::MessageType, msgType);

    sprintf(tmpBuf, "%06d", msgType == "E" ? 0 : recNum + 1);
    construct.addAttribute(xml2::MessageFailCode, tmpBuf);
    construct.addAttribute(xml2::MessageText, token);
    construct.closeElement();
  }

  recNum++;
  construct.openElement(xml2::MessageInformation);
  construct.addAttribute(xml2::MessageType, msgType);

  sprintf(tmpBuf, "%06d", msgType == "E" ? 0 : recNum + 1);
  construct.addAttribute(xml2::MessageFailCode, tmpBuf);
  construct.addAttribute(xml2::MessageText, ".");
  construct.closeElement();

  LOG4CXX_DEBUG(logger, "XML output:\n" << construct.getXMLData());
  LOG4CXX_DEBUG(logger, "Finished in formatResponse");
}
//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::formatResponse(TaxTrx& taxTrx, ErrorResponseException& ere)
{
  std::string response;
  formatResponse(taxTrx.taxDisplayRequestRootElementType(), ere, response);
  taxTrx.response().str(EMPTY_STRING());
  taxTrx.response() << response;
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildTaxUSHelpInfo
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildTaxUSHelpInfo(TaxTrx& taxTrx)
{
  taxTrx.response() << "          SPECIFIC TAX CALCULATION ENTRIES APPLICABLE TO" << endl;
  taxTrx.response() << "          US TAXES AND US MARKETS ONLY" << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "TXN/TCCCCCC/X/99999.99 - T=TOTAL/X=ONE WAY" << endl;
  taxTrx.response() << "TXN/TCCCCCC/R/99999.99 - T=TOTAL/R=ROUND TRIP" << endl;
  taxTrx.response() << "TXN/BCCCCCC/X/99999.99 - B=BASE/X=ONE WAY" << endl;
  taxTrx.response() << "TXN/BCCCCCC/R/99999.99 - B=BASE/R=ROUND TRIP" << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "          DISPLAYS TAXES FOR TRAVEL BETWEEN ALASKA/HAWAII" << endl;
  taxTrx.response() << "          AND THE CONTINENTAL U.S., OR BETWEEN ALASKA AND HAWAII." << endl;
  taxTrx.response() << "          RETURNS BASE/TOTAL FARE, US2 TAX, PART OF US1 TAX" << endl;
  taxTrx.response() << "          APPLIED, AND THE ALASKA/HAWAII PERCENTAGE FACTOR." << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "EXAMPLE - " << endl;
  taxTrx.response() << "          ON TAX PERCENTAGE CALCULATION BETWEEN HNL/ANC" << endl;
  taxTrx.response() << "TXN/THNLANC/X/1000" << endl;
  taxTrx.response() << "988.34/8.00/3.66 - TAX PCT. 0.37" << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "          RT BASE TAX PERCENTAGE CALCULATION BETWEEN HNL/LAX" << endl;
  taxTrx.response() << "TXN/BHNLLAX/R/983.70" << endl;
  taxTrx.response() << "1000.00/16.00/0.30 - TAX PCT. 0.03" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "          AT LEAST ONE ALASKA OR HAWAII MKT REQUIRED" << endl;
  taxTrx.response() << "TXN/TTULDFW/X/1000" << endl;
  taxTrx.response() << "NO TAX FACTOR FOUND FOR CITY PAIR - MODIFY AND REENTER  " << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "FOR TRAVEL WITHIN THE CONTINENTAL U.S. THE FOLLOWING ENTRIES" << endl;
  taxTrx.response() << "ARE APPLICABLE FOR BASE/TOTAL FARE AND US1 TAX BREAKDOWN" << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "TXN/T99999.99 - T=TOTAL" << endl;
  taxTrx.response() << "TXN/B99999.99 - B=BASE" << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "EXAMPLE -" << endl;
  taxTrx.response() << "TXN/T1000.00" << endl;
  taxTrx.response() << "930.23/69.77" << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "TXN/B930.23" << endl;
  taxTrx.response() << "1000.00/69.77" << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "NOTE -" << endl;
  taxTrx.response() << "AN OPTIONAL DATE MAY BE APPENDED TO THE INPUTS." << endl;
  taxTrx.response() << "I.E. APPEND -DDMMMYY DEFAULT IS TODAY." << endl;
  taxTrx.response() << " " << endl;
  taxTrx.response() << "EXAMPLE -" << endl;
  taxTrx.response() << "TXN/THNLANC/X/1000-01JAN09" << endl;
  taxTrx.response() << "988.34/8.00/3.66 - TAX PCT. 0.37" << endl;
  taxTrx.response() << "." << endl;
  taxTrx.response() << "TXN/B930.23-01DEC07" << endl;
  taxTrx.response() << "1000.00/69.77" << endl;
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildHelpInfo
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildHelpInfo(TaxTrx& taxTrx)
{
  taxTrx.response() << "               TAX DISPLAY HELP" << endl << endl;
  taxTrx.response() << "TXN*CC  WHERE CC IS THE COUNTRY CODE" << endl;
  taxTrx.response() << "       DISPLAYS LIST OF TAXES FOR THIS COUNTRY" << endl;
  taxTrx.response() << "       TXN*MX DISPLAYS LIST OF TAXES FOR MEXICO" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       DISPLAYS DETAIL OF THIS TAX" << endl;
  taxTrx.response() << "       I.E.  TXN**US2 DISPLAYS USA ITT TAX" << endl;
  taxTrx.response() << "       AND TXN**EG DISPLAYS EGYPT TRANSPORTATION TAX" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/M  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       DISPLAYS CATEGORY MENU OF THIS TAX SEQUENCE" << endl;
  taxTrx.response() << "       I.E.  TXN**UK/M DISPLAYS MENU FOR MEXICO" << endl;
  taxTrx.response() << "       TOURISM TAX LISTED CATERGORIES 01 THROUGH 18" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/S#/M  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND S# IS SEQUENCE NUMBER" << endl;
  taxTrx.response() << "       DISPLAYS CATEGORY MENU OF THIS TAX SEQUENCE" << endl;
  taxTrx.response() << "       I.E.  TXN**UK/S10/M DISPLAYS MEXICO TOURISM" << endl;
  taxTrx.response() << "       TAX MENU LISTED CATERGORIES 01 THROUGH 18 FOR SEQUENCE 10" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/R  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       DISPLAYS TAX CODE REISSUE DATA" << endl;
  taxTrx.response() << "       I.E.  TXN**MX/R DISPLAYS MEXICO MX TAX CODE" << endl;
  taxTrx.response() << "       REISSUE INFORMATION" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/S#  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND S# IS SEQUENCE NUMBER" << endl;
  taxTrx.response() << "       DISPLAYS RULE SEQUENCE DETAIL OF THIS TAX" << endl;
  taxTrx.response() << "       I.E.  TXN**BR1/S2 DISPLAYS BRAZIL EMBARKATION" << endl;
  taxTrx.response() << "       TAX SEQUENCE 2 DETAILS FOR BR1 TAX DETAILS-" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/##  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND ## IS CATEGORY NUMBER" << endl;
  taxTrx.response() << "       DISPLAYS RULE CATEGORY DETAIL OF THIS TAX" << endl;
  taxTrx.response() << "       I.E.  TXN**BR1/2 DISPLAYS BRAZIL EMBARKATION" << endl;
  taxTrx.response() << "       TAX CATEGORY 2 DETAILS FOR 02 TAX DETAILS-" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/##,##  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND ##,## IS CATEGORY NUMBERS" << endl;
  taxTrx.response() << "       DISPLAYS RULE CATEGORY DETAIL OF THIS TAX" << endl;
  taxTrx.response() << "       I.E.  TXN**BR1/2,5 DISPLAYS BRAZIL" << endl;
  taxTrx.response() << "       EMBARKATION TAX CATEGORY 2 AND 5" << endl;
  taxTrx.response() << "       DETAILS FOR BR1 TAX-" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/##-##  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND ##-## IS CATEGORY RANGE" << endl;
  taxTrx.response() << "       DISPLAYS RULE CATEGORY DETAIL OF THIS TAX" << endl;
  taxTrx.response() << "       I.E.  TXN**BR1/2-9 DISPLAYS BRAZIL" << endl;
  taxTrx.response() << "       EMBARKATION TAX CATEGORY 2 THROUGH 9 DETAILS FOR BR1 TAX-" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/S#/##  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND S# IS SEQUENCE NUMBER AND ## IS CATEGORY" << endl;
  taxTrx.response() << "       NUMBER DISPLAYS RULE SEQUENCE/CATEGORY" << endl;
  taxTrx.response()
      << "       DETAIL OF THIS TAX I.E.  TXN**BR1/S2/3 DISPLAYS BRAZIL EMBARKATION TAX" << endl;
  taxTrx.response() << "       SEQUENCE 2/CATEGORY 3 DETAILS FOR BR1 TAX DETAILS-" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/S#/##,##  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND S# IS SEQUENCE NUMBER AND ##,## ARE" << endl;
  taxTrx.response()
      << "       CATEGORY NUMBERS DISPLAYS RULE SEQUENCE/CATERGORIES DETAIL OF THIS TAX" << endl;
  taxTrx.response() << "       I.E.  TXN**BR1/S2/3,6 DISPLAYS BRAZIL" << endl;
  taxTrx.response()
      << "       EMBARKATION TAX SEQUENCE 2/CATEGORY 3 AND 6 DETAILS FOR BR1 TAX DETAILS-" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN**TTT/S#/##-##  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND S# IS SEQUENCE NUMBER AND ##-## IS" << endl;
  taxTrx.response()
      << "       CATEGORY RANGE, DISPLAYS RULE SEQUENCE/CATEGORY RANGE DETAIL OF THIS TAX" << endl;
  taxTrx.response() << "       I.E.  TXN**BR1/S2/3-6 DISPLAYS BRAZIL" << endl;
  taxTrx.response() << "       EMBARKATION TAX SEQUENCE 2/CATEGORY 3-6 DETAILS FOR BR1 TAX DETAILS-"
                    << endl;
  taxTrx.response() << "  " << endl;
  // PL 17849
  taxTrx.response() << "TXN**TTT/S#/13/CC  WHERE TTT IS THE TAX CODE 2 OR 3 CHARACTERS" << endl;
  taxTrx.response() << "       AND S# IS SEQUENCE NUMBER, 13 IS CATEGORY 13," << endl;
  taxTrx.response() << "       CC IS CARRIER CODE" << endl;
  taxTrx.response() << "       DISPLAYS RULE SEQUENCE/CATEGORY 13/CARRIER" << endl;
  taxTrx.response() << "       CABIN DATA I.E.  TXN**GB1/S2/13/BA DISPLAYS" << endl;
  taxTrx.response() << "       GB1 TAX SEQUENCE 2/CATEGORY 13/CARRIER BA CABIN DETAILS FOR BA-"
                    << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN*XXXX   WHERE XXXX IS THE COUNTRY NAME" << endl;
  taxTrx.response() << "       DISPLAYS LIST OF TAXES FOR THIS COUNTRY" << endl;
  taxTrx.response() << "       I.E.  TXN*ITALY DISPLAYS LIST OF TAXES FOR ITALY" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN*AAA    WHERE AAA IS THE AIRPORT OR CITY CODE" << endl;
  taxTrx.response() << "       DISPLAYS LIST OF TAXES FOR COUNTRY OF AIRPORT/CITY" << endl;
  taxTrx.response() << "       I.E.  TXN*PAR DISPLAYS LIST OF TAXES FOR FRANCE" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "TXN*   RE-DISPLAYS PREVIOUS TAX DISPLAY" << endl;
  taxTrx.response() << endl << endl;
  taxTrx.response() << "HISTORY   HISTORICAL DISPLAYS FOR TXN* ENTRIES" << endl;
  taxTrx.response() << "       ADD -DDMMMYY TO THE END OF THE TXN* ENTRY" << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "ADDITIONAL HISTORICAL DATE FOR SPECIFIC TRAVEL DATE CAN BE ENTERED AFTER "
                       "THE EFFECTIVE DATE ENTRY - DDMMMYY-TDDMMMYY (EFFECTIVE/TRAVEL DATES)"
                    << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "       FOR COMPLETE DETAILS AND ADDITIONAL FORMATS" << endl;
  taxTrx.response() << "       REFER TO YOUR APPROPRIATE REFERENCE SOURCE" << endl;
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildReissue
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildReissue(TaxTrx& taxTrx)
{
  TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();

  ostringstream displayCat;
  string catResponse = EMPTY_STRING();

  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIter =
      taxResponse->taxDisplayItemVector().begin();
  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIterEnd =
      taxResponse->taxDisplayItemVector().end();

  for (; taxDisplayItemIter != taxDisplayItemIterEnd; taxDisplayItemIter++)
  {

    std::string reissueTax = (*taxDisplayItemIter)->reissue()->subCat0();

    if (reissueTax.empty())
    {
      taxTrx.response() << "01 TAX-" << endl;
      taxTrx.response() << (*taxDisplayItemIter)->reissue()->subCat0();
      taxTrx.response() << "* TAX CODE SEQUENCE DOES NOT APPLY FOR REISSUE\n";
      taxTrx.response() << "   " << endl;
      continue;
    }

    taxTrx.response() << "01 TAX-" << endl;
    taxTrx.response() << (*taxDisplayItemIter)->reissue()->subCat0();
    taxTrx.response() << "   " << endl;

    taxTrx.response() << "02 TAX DETAIL-REFUND APPLICATION-" << endl;
    taxTrx.response() << (*taxDisplayItemIter)->reissue()->subCat1();
    taxTrx.response() << "   " << endl;

    taxTrx.response() << "03 SALE-REISSUE-" << endl;
    taxTrx.response() << (*taxDisplayItemIter)->reissue()->subCat2();
    taxTrx.response() << "   " << endl;

    taxTrx.response() << "05 VALIDATING CARRIER-TICKETING REISSUE CARRIER-" << endl;
    taxTrx.response() << (*taxDisplayItemIter)->reissue()->subCat3();
    taxTrx.response() << "   " << endl;

    taxTrx.response() << "06 CURRENCY-" << endl;
    taxTrx.response() << (*taxDisplayItemIter)->reissue()->subCat4();
    taxTrx.response() << "   " << endl;
  }
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildMenu  (star display?)
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildMenu(TaxTrx& taxTrx)
{
  TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();

  TaxDisplayItem* taxDisplayItem = taxResponse->taxDisplayItemVector().front();
  taxTrx.response() << "TAX CODE         "
                    << "TAX NAME" << endl;
  taxTrx.response().setf(std::ios::left, std::ios::adjustfield);

  ostringstream msg;
  msg.setf(std::ios::left, std::ios::adjustfield);

  msg << setw(3) << taxDisplayItem->taxCode();
  msg << "              ";
  msg << taxDisplayItem->taxDescription();
  std::string tmp = msg.str();

  TaxDisplayFormatter formatter;
  formatter.offsetWidth(17);
  formatter.format(tmp);

  taxTrx.response() << tmp;

  // if there is no any sign, endline is not working...
  taxTrx.response() << " " << endl << endl;
  taxTrx.response() << "COUNTRY CODE     COUNTRY NAME" << endl;

  taxTrx.response() << taxDisplayItem->taxCodeReg()->nation();
  taxTrx.response() << "               ";
  taxTrx.response() << taxDisplayItem->taxNation();

  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIter =
      taxResponse->taxDisplayItemVector().begin();
  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIterEnd =
      taxResponse->taxDisplayItemVector().end();

#define ISEMPTY(x, y) (!(*taxDisplayItemIter)->category##x()->subCat##y().empty())
#define STRSUB(x, y) (*taxDisplayItemIter)->category##x()->subCat##y()

  for (; taxDisplayItemIter != taxDisplayItemIterEnd; taxDisplayItemIter++)
  {

    taxTrx.response() << std::endl << std::endl;
    taxTrx.response() << " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - "
                      << std::endl;

    if ((*taxDisplayItemIter)->category1() != nullptr)
    {
      if ((*taxDisplayItemIter)->category1()->displayOnly().size())
        taxTrx.response() << (*taxDisplayItemIter)->category1()->displayOnly() << std::endl;

      if (ISEMPTY(1, 1) && ISEMPTY(1, 2) && ISEMPTY(1, 3) && ISEMPTY(1, 4) && ISEMPTY(1, 5) &&
          ISEMPTY(1, 6))
        taxTrx.response() << "  ";
      else
        taxTrx.response() << "* ";

      taxTrx.response() << " 01 " << (*taxDisplayItemIter)->category1()->name() << std::endl;
      taxTrx.response() << (*taxDisplayItemIter)->category1()->subCat1();
      taxTrx.response() << std::endl << std::endl;
    }

    if ((*taxDisplayItemIter)->category2() != nullptr)
    {
      std::string displayCat = STRSUB(2, 1) + STRSUB(2, 2) + STRSUB(2, 3) + STRSUB(2, 4) +
                               STRSUB(2, 5) + STRSUB(2, 6) + STRSUB(2, 7) + STRSUB(2, 8) +
                               STRSUB(2, 9);

      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 02 " << setw(22) << (*taxDisplayItemIter)->category2()->name();

    if ((*taxDisplayItemIter)->category3() != nullptr)
    {
      std::string displayCat = STRSUB(3, 1) + STRSUB(3, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 03 " << setw(22) << (*taxDisplayItemIter)->category3()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category4() != nullptr)
    {
      std::string displayCat =
          STRSUB(4, 1) + STRSUB(4, 2) + STRSUB(4, 3) + STRSUB(4, 4) + STRSUB(4, 5);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 04 " << setw(22) << (*taxDisplayItemIter)->category4()->name();

    if ((*taxDisplayItemIter)->category5() != nullptr)
    {
      std::string displayCat = STRSUB(5, 1) + STRSUB(5, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 05 " << setw(22) << (*taxDisplayItemIter)->category5()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category6() != nullptr)
    {
      std::string displayCat = STRSUB(6, 1) + STRSUB(6, 2) + STRSUB(6, 3) + STRSUB(6, 4) +
                               STRSUB(6, 5) + STRSUB(6, 6) + STRSUB(6, 7);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 06 " << setw(22) << (*taxDisplayItemIter)->category6()->name();

    if ((*taxDisplayItemIter)->category7() != nullptr)
    {
      std::string displayCat = STRSUB(7, 1) + STRSUB(7, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 07 " << setw(22) << (*taxDisplayItemIter)->category7()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category8() != nullptr)
    {
      std::string displayCat = STRSUB(8, 1) + STRSUB(8, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 08 " << setw(22) << (*taxDisplayItemIter)->category8()->name();

    if ((*taxDisplayItemIter)->category9() != nullptr)
    {
      std::string displayCat = STRSUB(9, 1) + STRSUB(9, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 09 " << setw(22) << (*taxDisplayItemIter)->category9()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category10() != nullptr)
    {
      std::string displayCat = STRSUB(10, 1) + STRSUB(10, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 10 " << setw(22) << (*taxDisplayItemIter)->category10()->name();

    if ((*taxDisplayItemIter)->category11() != nullptr)
    {
      std::string displayCat = STRSUB(11, 1) + STRSUB(11, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 11 " << setw(22) << (*taxDisplayItemIter)->category11()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category12() != nullptr)
    {
      std::string displayCat = STRSUB(12, 1) + STRSUB(12, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 12 " << setw(22) << (*taxDisplayItemIter)->category12()->name();

    if ((*taxDisplayItemIter)->category13() != nullptr)
    {
      std::string displayCat = STRSUB(13, 1) + STRSUB(13, 2) + STRSUB(13, 3);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 13 " << setw(22) << (*taxDisplayItemIter)->category13()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category14() != nullptr)
    {
      std::string displayCat = STRSUB(14, 1) + STRSUB(14, 2);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 14 " << setw(22) << (*taxDisplayItemIter)->category14()->name();

    if ((*taxDisplayItemIter)->category15() != nullptr)
    {
      std::string displayCat = STRSUB(15, 1) + STRSUB(15, 2) + STRSUB(15, 3) + STRSUB(15, 4);
      taxTrx.response() << setw(2) << displayCat.substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 15 " << setw(22) << (*taxDisplayItemIter)->category15()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category16() != nullptr)
    {
      taxTrx.response() << setw(2) << (*taxDisplayItemIter)->category16()->subCat1().substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 16 " << setw(22) << (*taxDisplayItemIter)->category16()->name();

    if ((*taxDisplayItemIter)->category17() != nullptr)
    {
      taxTrx.response() << setw(2) << (*taxDisplayItemIter)->category17()->subCat1().substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 17 " << setw(22) << (*taxDisplayItemIter)->category17()->name();

    taxTrx.response() << std::endl << std::endl;

    if ((*taxDisplayItemIter)->category18() != nullptr)
    {
      taxTrx.response() << setw(2) << (*taxDisplayItemIter)->category18()->subCat1().substr(0, 2);
    }
    else
    {
      taxTrx.response() << "  ";
    }
    taxTrx.response() << " 18 " << setw(22) << (*taxDisplayItemIter)->category18()->name();
  }
  taxTrx.response() << std::endl << " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - "
                    << std::endl;
  taxTrx.response() << "CATEGORIES WITH * CONTAIN SPECIFIC DATA FOR TAX CODE" << std::endl;
  taxTrx.response() << "CATEGORIES WITH ** ONLY CONTAIN INFORMATIONAL TEXT DATA" << std::endl;
  taxTrx.response() << endl;
#undef ISNEMPTY
}
//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildListNationDisplay
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildListNationDisplay(TaxTrx& taxTrx, XMLConstruct& construct)
{
  TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();
  TaxDisplayItem* taxDisplayItem = taxResponse->taxDisplayItemVector().front();

  taxTrx.response() << "COUNTRY NAME- " << taxDisplayItem->taxNation() << endl;
  taxTrx.response() << "  " << endl;
  taxTrx.response() << "      TAX     TAX" << endl;
  taxTrx.response() << "      CODE    NAME" << endl;
  taxTrx.response() << "  " << endl;

  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIter =
      taxResponse->taxDisplayItemVector().begin();
  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIterEnd =
      taxResponse->taxDisplayItemVector().end();

  uint16_t taxCount = 1;

  TaxDisplayFormatter formatter;
  formatter.offsetWidth(14);

  for (; taxDisplayItemIter != taxDisplayItemIterEnd; taxDisplayItemIter++, taxCount++)
  {
    char tmpBuf[100];
    ostringstream msg;
    msg.setf(std::ios::left, std::ios::adjustfield);
    msg << setw(3);
    sprintf(tmpBuf, "%u", taxCount);
    msg << tmpBuf;
    msg << "   ";
    msg << setw(3);
    msg << (*taxDisplayItemIter)->taxCode();
    msg << "     ";
    msg << (*taxDisplayItemIter)->taxDescription();
    msg << endl;

    std::string tmp = msg.str();
    formatter.format(tmp);

    taxTrx.response() << tmp;
    construct.openElement(xml2::TaxInformation);
    construct.addAttribute(xml2::ATaxCode, (*taxDisplayItemIter)->taxCode());
    construct.closeElement();
  }

  taxTrx.response() << "  " << endl;
  // removed due to SPR 17007
  // taxTrx.response() << taxDisplayItem->taxNation() << " TAXES FOR COLLECTION" << endl;
  taxTrx.response() << "COUNTRY TAX ROUNDING - ";

  RoundingRule rr = taxDisplayItem->roundingRule();

  if (rr == UP)
  {
    taxTrx.response() << "ROUND UP TO NEXT " << taxDisplayItem->roundingUnit();
  }
  else if (rr == DOWN)
  {
    taxTrx.response() << "ROUND DOWN TO NEXT " << taxDisplayItem->roundingUnit();
  }
  else if (rr == NEAREST)
  {
    taxTrx.response() << "ROUND TO NEAREST " << taxDisplayItem->roundingUnit();
  }
  else
  {
    taxTrx.response() << "NO ROUNDING SPECIFIED";
  }

  taxTrx.response() << endl;
  taxTrx.response() << taxDisplayItem->message();
  taxTrx.response() << endl;
  if (!isAxess)
    taxTrx.response() << "USE TXN*# WHERE # IS LINE NUMBER" << endl;
  taxTrx.response() << "3 - CHARACTER CODE FOR DISPLAY PURPOSES ONLY" << endl;
  return;
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildListNationDisplay
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildStandardHeader(TaxTrx& taxTrx)
{
  TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();
  TaxDisplayItem* taxDisplayItem = taxResponse->taxDisplayItemVector().front();

  taxTrx.response() << "TAX CODE         "
                    << "TAX NAME" << endl;
  taxTrx.response().setf(std::ios::left, std::ios::adjustfield);

  ostringstream msg;
  msg.setf(std::ios::left, std::ios::adjustfield);

  msg << setw(3) << taxDisplayItem->taxCode();
  msg << "              ";
  msg << taxDisplayItem->taxDescription();
  std::string tmp = msg.str();

  TaxDisplayFormatter formatter;
  formatter.offsetWidth(17);
  formatter.format(tmp);

  taxTrx.response() << tmp;
  taxTrx.response() << endl;
  taxTrx.response() << "   " << endl;
  taxTrx.response() << "COUNTRY CODE     COUNTRY NAME" << endl;

  taxTrx.response() << taxDisplayItem->taxCodeReg()->nation();
  taxTrx.response() << "               ";
  taxTrx.response() << taxDisplayItem->taxNation();

  if (taxTrx.getRequest()->getReissue())
  {
    taxTrx.response() << endl;
    taxTrx.response() << "............................................................" << endl;
    taxTrx.response() << "   " << endl;
    return;
  }

  if (taxDisplayItem->message() != "")
  {
    taxTrx.response() << "   " << endl;
    taxTrx.response() << taxDisplayItem->message() << endl;
  }

  taxTrx.response() << endl;

  taxTrx.response() << "............................................................" << endl;

  if ((taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate()) &&
      (taxTrx.getRequest()->effectiveDate() < DateTime::localTime()))
  {
    taxTrx.response() << "         HISTORICAL TXN REQUEST - DATE DIFFERENCES NOTED ON" << endl;
    taxTrx.response() << "         SPECIFIC TAX SEQUENCE" << endl;
    taxTrx.response() << "   " << endl;
  }
  else
  {
    taxTrx.response() << "         CURRENTLY EFFECTIVE FOR SALES AND TRAVEL" << endl;
    taxTrx.response() << "         FUTURE DATE DIFFERENCES NOTED ON SPECIFIC TAX SEQUENCE" << endl;
    taxTrx.response() << "   " << endl;
  }
  return;
}
//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildListNationDisplay
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildMultiSequence(TaxTrx& taxTrx)
{

  if (!isAxess)
  {
    taxTrx.response() << "MULTIPLE SEQUENCES EXIST - USE SEQUENCE ENTRY TO DISPLAY" << endl;
    taxTrx.response() << "TAX DETAILS FOR EACH SEQUENCE - REFER TO TXNHELP FOR" << endl;
    taxTrx.response() << "                                SEQUENCE ENTRY" << endl;
  }
  else
  {
    taxTrx.response() << "MULTIPLE SEQUENCES EXIST - USE SEQUENCE ENTRY TO DISPLAY" << endl;
    taxTrx.response() << "TAX DETAILS FOR EACH SEQUENCE" << endl;
    taxTrx.response() << " " << endl;
  }
  // AXESS fix end.-----------------------------------------------

  TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();

  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIter =
      taxResponse->taxDisplayItemVector().begin();
  std::vector<TaxDisplayItem*>::const_iterator taxDisplayItemIterEnd =
      taxResponse->taxDisplayItemVector().end();

  bool catVecEmpty = taxTrx.getRequest()->categoryVec().empty();
  unsigned int secondSeqNum = taxTrx.getRequest()->sequenceNumber2();

  for (; taxDisplayItemIter != taxDisplayItemIterEnd; taxDisplayItemIter++)
  {

    if ((*taxDisplayItemIter)->category1() != nullptr &&
        (catVecEmpty && secondSeqNum == TaxRequest::EMPTY_SEQUENCE))
    {
      Category1* ct1 = (*taxDisplayItemIter)->category1();
      taxTrx.response() << "\n";
      if ((*taxDisplayItemIter)->categorySeqDataVerb())
      {
        taxTrx.response() << (*taxDisplayItemIter)->categorySeqDataVerb()->subCat1();
      }

      if ((*taxDisplayItemIter)->category1()->displayOnly().size())
        taxTrx.response() << (*taxDisplayItemIter)->category1()->displayOnly() << std::endl;

      taxTrx.response() << "01 TAX-" << endl;
      taxTrx.response() << ct1->subCat1();
      taxTrx.response() << ct1->subCat2();
      taxTrx.response() << ct1->subCat3();
      taxTrx.response() << ct1->subCat4();
      taxTrx.response() << ct1->subCat5();
      taxTrx.response() << ct1->subCat6();
      taxTrx.response() << ct1->subCat7();
      taxTrx.response() << ct1->subCat8();
      taxTrx.response() << ct1->subCat9();
      taxTrx.response() << ct1->subCat10();
      taxTrx.response() << ct1->subCat11();
      taxTrx.response() << "   " << endl;
      continue;
    }
    // if this exists probably all cats exists.. in multi sequences.
    /*
    if ( (*taxDisplayItemIter)->category2())
    {
       taxTrx.response() << "..............................................................." <<
    std::endl;
       taxTrx.response() << "SEQ:  " <<  (*taxDisplayItemIter)->taxCodeReg()->seqNo() << std::endl;
    }*/
    buildOneSequence(taxTrx, (*taxDisplayItemIter));
  }
  return;
}
//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::buildListNationDisplay
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::buildSequence(TaxTrx& taxTrx)
{
  TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();
  TaxDisplayItem* taxDisplayItem = taxResponse->taxDisplayItemVector().front();
  buildOneSequence(taxTrx, taxDisplayItem);
  return;
}

void
TaxDisplayResponseFormatter::buildOneSequence(TaxTrx& taxTrx, TaxDisplayItem* taxDisplayItem)
{
  if (taxDisplayItem->categorySeqDataVerb())
  {
    taxTrx.response() << taxDisplayItem->categorySeqDataVerb()->subCat1();
    taxTrx.response() << "   " << endl;
  }

  if (taxDisplayItem->category1())
  {
    if (taxDisplayItem->category1()->displayOnly().size())
      taxTrx.response() << taxDisplayItem->category1()->displayOnly() << std::endl;

    taxTrx.response() << "01 TAX-" << endl;
    taxTrx.response() << taxDisplayItem->category1()->subCat1();
    taxTrx.response() << "   " << endl;
  }

  ostringstream displayCat;
  string catResponse = EMPTY_STRING();

  if (taxDisplayItem->category2())
  {
    displayCat << taxDisplayItem->category2()->subCat1();
    displayCat << taxDisplayItem->category2()->subCat2();
    displayCat << taxDisplayItem->category2()->subCat3();
    displayCat << taxDisplayItem->category2()->subCat4();
    displayCat << taxDisplayItem->category2()->subCat5();
    displayCat << taxDisplayItem->category2()->subCat6();
    displayCat << taxDisplayItem->category2()->subCat7();
    displayCat << taxDisplayItem->category2()->subCat8();
    displayCat << taxDisplayItem->category2()->subCat9();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "02 TAX DETAIL-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category3())
  {
    displayCat << taxDisplayItem->category3()->subCat1();
    displayCat << taxDisplayItem->category3()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "03 SALE-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category4())
  {
    displayCat << taxDisplayItem->category4()->subCat1();
    displayCat << taxDisplayItem->category4()->subCat2();
    displayCat << taxDisplayItem->category4()->subCat3();
    displayCat << taxDisplayItem->category4()->subCat4();
    displayCat << taxDisplayItem->category4()->subCat5();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "04 TRAVEL-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category5())
  {
    displayCat << taxDisplayItem->category5()->subCat1();
    displayCat << taxDisplayItem->category5()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "05 VALIDATING CARRIER-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category6())
  {
    displayCat << taxDisplayItem->category6()->subCat1();
    displayCat << taxDisplayItem->category6()->subCat2();
    displayCat << taxDisplayItem->category6()->subCat3();
    displayCat << taxDisplayItem->category6()->subCat4();
    displayCat << taxDisplayItem->category6()->subCat5();
    displayCat << taxDisplayItem->category6()->subCat6();
    displayCat << taxDisplayItem->category6()->subCat7();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "06 CURRENCY-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category7())
  {
    displayCat << taxDisplayItem->category7()->subCat1();
    displayCat << taxDisplayItem->category7()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "07 AIRLINE-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category8())
  {
    displayCat << taxDisplayItem->category8()->subCat1();
    displayCat << taxDisplayItem->category8()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "08 EQUIPMENT-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category9())
  {
    displayCat << taxDisplayItem->category9()->subCat1();
    displayCat << taxDisplayItem->category9()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "09 PASSENGER TYPE-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category10())
  {
    displayCat << taxDisplayItem->category10()->subCat1();
    displayCat << taxDisplayItem->category10()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "10 FARE TYPE/CLASS-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category11())
  {
    displayCat << taxDisplayItem->category11()->subCat1();
    displayCat << taxDisplayItem->category11()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "11 TRANSIT-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category12())
  {
    displayCat << taxDisplayItem->category12()->subCat1();
    displayCat << taxDisplayItem->category12()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "12 TICKET DESIGNATOR-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category13())
  {
    if (std::find(taxTrx.getRequest()->categoryVec().begin(),
                  taxTrx.getRequest()->categoryVec().end(),
                  Category13::NUMBER) != taxTrx.getRequest()->categoryVec().end() &&
        taxTrx.getRequest()->categoryVec().size() < 18)
    {
      displayCat << taxDisplayItem->category13()->subCat2(); // detail info
      displayCat << taxDisplayItem->category13()->subCat3();
    }
    else
    {
      displayCat << taxDisplayItem->category13()->subCat1(); // general info
      displayCat << taxDisplayItem->category13()->subCat2();
    }

    // prevent from empty, display default
    if (displayCat.str().empty())
      displayCat << taxDisplayItem->category13()->subCat1();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "13 CABIN-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category14())
  {
    displayCat << taxDisplayItem->category14()->subCat1();
    displayCat << taxDisplayItem->category14()->subCat2();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "14 DISCOUNT-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category15())
  {
    displayCat << taxDisplayItem->category15()->subCat1();
    displayCat << taxDisplayItem->category15()->subCat2();
    displayCat << taxDisplayItem->category15()->subCat3();
    displayCat << taxDisplayItem->category15()->subCat4();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "15 MISCELLANEOUS INFORMATION-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category16())
  {
    displayCat << taxDisplayItem->category16()->subCat1();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "16 REISSUE INFORMATION-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category17())
  {
    displayCat << taxDisplayItem->category17()->subCat1();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "17 ROUTING-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }

  if (taxDisplayItem->category18())
  {
    displayCat << taxDisplayItem->category18()->subCat1();

    catResponse = displayCat.str();
    displayCat.str(EMPTY_STRING());

    if (!catResponse.empty())
    {
      taxTrx.response() << "18 REFUND-" << endl;
      taxTrx.response() << catResponse;
      taxTrx.response() << "   " << endl;
      catResponse = EMPTY_STRING();
    }
  }
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::addResponseHeader
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::addResponseHeader(const std::string& reqRootElement, XMLConstruct& construct)
{
  construct.addSpecialElement(XML_DECLARATION_TAG_TEXT.c_str());
  if (reqRootElement == "AIRTAXDISPLAYRQ")
    construct.openElement("AirTaxDisplayRS");
  else
    construct.openElement("TaxDisplayRS");
  construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);
  construct.addAttribute("xmlns:xs", XML_NAMESPACE_XS_TEXT);
  construct.addAttribute("xmlns:xsi", XML_NAMESPACE_XSI_TEXT);
  construct.addAttribute("Version", TAX_DISPLAY_XML_VERSION_TEXT);
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::addResponseFooter
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::addResponseFooter(XMLConstruct& construct)
{
  construct.closeElement(); // TaxRS
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::readConfigXMLNamespace
//----------------------------------------------------------------------------
const std::string
TaxDisplayResponseFormatter::readConfigXMLNamespace(const std::string& configName)
{
  tse::ConfigMan& config = Global::config();

  std::string xmlNamespace;

  if (!config.getValue(configName, xmlNamespace, "TAX_SVC"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, configName, "TAX_SVC");
  }

  return xmlNamespace;
}

//----------------------------------------------------------------------------
// TaxDisplayResponseFormatter::formatResposne
//----------------------------------------------------------------------------
void
TaxDisplayResponseFormatter::formatResponse(const std::string& reqRootElement, const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  addResponseHeader(reqRootElement, construct);

  buildMessage(ere.message(), construct, "E");

  addResponseFooter(construct);
  response = construct.getXMLData();
}
