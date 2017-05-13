//----------------------------------------------------------------------------
//
//  File:  FareDisplayResponseFormatter.cpp
//  Description: See FareDisplayResponseFormatter.h file
//  Created:  February 17, 2005
//  Authors:  Mike Carroll
//
//  Copyright Sabre 2003
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

#include "Xform/FareDisplayResponseFormatter.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/NUCCollectionResults.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PrivateIndicator.h"
#include "DataModel/SurchargeData.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/Routing.h"
#include "Diagnostic/Diagnostic.h"
#include "DSS/FlightCount.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/RuleConst.h"
#include "Xform/FareDisplayResponseRoutingXMLTags.h"
#include "Xform/FareDisplayResponseXMLTags.h"
#include "Xform/FareDisplaySDSTaxInfo.h"

#include <boost/algorithm/string.hpp>

#include <vector>

namespace xmlAttr
{
static const std::string BrandCode = "SB2";
static const std::string BrandName = "SB3";
static const std::string ProgramCode = "SC0";
static const std::string ProgramName = "SC2";
} // end namespace xmlAttr

namespace tse
{
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);

namespace
{
ConfigurableValue<bool>
sdsFqFlTaxNeeded("FAREDISPLAY_SVC", "SDS_FQ_FL_TAX_NEEDED", false);
ConfigurableValue<uint32_t>
maxFaresSDSCfg("FAREDISPLAY_SVC", "MAX_FARES_SDS", 650);
ConfigurableValue<uint32_t>
maxResponseSize("FAREDISPLAY_SVC", "MAX_RESPONSE_SIZE", 335000);
Logger
logger("atseintl.Xform.FareDisplayResponseFormatter");
}

const Indicator ERROR_TYPE = 'E';
const Indicator MSG_TYPE = 'M';
const Indicator TEXT_TYPE = 'T';

void
FareDisplayResponseFormatter::formatResponse(const ErrorResponseException& ere,
                                             std::string& response)
{
  XMLConstruct construct;
  construct.openElement("FareDisplayResponse");

  ResponseFormatter::addMessageLine(ere.message(), construct, "E", Message::errCode(ere.code()));

  construct.closeElement();

  response = construct.getXMLData();
}

std::string
FareDisplayResponseFormatter::formatResponse(FareDisplayTrx& fareDisplayTrx)
{
  _recNum = 2;

  std::string tmpResponse = fareDisplayTrx.response().str();
  std::string msgType;
  if (isErrorCondition(fareDisplayTrx))
  {
    msgType = "E"; // Error msg
  }
  else
  {
    msgType = "X"; // General msg
  }

  if (fareDisplayTrx.isDiagnosticRequest())
  {
    if (fareDisplayTrx.isFQ() && fareDisplayTrx.getRequest()->diagnosticNumber() == DIAG_999_ID)
      mainDiagResponse(fareDisplayTrx);

    tmpResponse = fareDisplayTrx.diagnostic().toString();
    if (tmpResponse.empty())
    {
      std::ostringstream emptyDiagMessage;
      emptyDiagMessage << "DIAGNOSTIC " << fareDisplayTrx.getRequest()->diagnosticNumber()
                       << " RETURNED NO DATA";
      tmpResponse = emptyDiagMessage.str();
      msgType = "E"; // Error msg
    }
    else
      msgType = "X"; // General msg
  }

  std::string errorResponse = fareDisplayTrx.errorResponse().str();
  if (!errorResponse.empty())
  {
    tmpResponse = tmpResponse + errorResponse;
  }

  const int AVAILABLE_SIZE = 63;

  bool errorForSDS = (fareDisplayTrx.isSDSOutputType() && fareDisplayTrx.allPaxTypeFare().empty() &&
                      tmpResponse.length() <= 63);

  XMLConstruct construct;
  construct.openElement("FareDisplayResponse");

  if (fareDisplayTrx.getRequest()->requestType() == "FQ" ||
      !fareDisplayTrx.getRequest()->inclusionCode().empty())
  {
    construct.addAttribute("BI0", fareDisplayTrx.getRequest()->inclusionCode().c_str());
  }

  // add host/db diag info
  std::vector<std::string> hostInfo;
  std::vector<std::string> buildInfo;
  std::vector<std::string> dbInfo;
  std::vector<std::string> configInfo;

  // Check if host and db info needed before the display
  if (FareDisplayUtil::isHostDBInfoDiagRequest(fareDisplayTrx))
  {
    if (hostDiagString(hostInfo))
    {
      for (const auto& elem : hostInfo)
        addMessageLine(elem, construct, msgType);
    }

    buildDiagString(buildInfo);
    for (const auto& elem : buildInfo)
      addMessageLine(elem, construct, msgType);

    dbDiagString(dbInfo);
    for (const auto& elem : dbInfo)
      addMessageLine(elem, construct, msgType);

    if (configDiagString(configInfo, fareDisplayTrx))
    {
      for (auto& elem : configInfo)
      {
        if (elem.size() > (unsigned int)AVAILABLE_SIZE)
        {
          elem = elem.substr(0, AVAILABLE_SIZE - 1);
        }
        addMessageLine(elem, construct, msgType);
      }
    }
    // extra empty line after diag
    addMessageLine(" ", construct, msgType);
  } // endif - host/db diag info

  // If nothing for general display, don't do detail information
  if (tmpResponse.length() == 0 && !fareDisplayTrx.isERDFromSWS())
  {
    construct.openElement("MSG");
    construct.addAttribute("N06", "E");

    sprintf(_tmpBuf, "%06d", _recNum + 1);
    construct.addAttribute("Q0K", _tmpBuf);
    if (errorResponse.empty())
      construct.addAttribute("S18", "REQUEST RECEIVED - NO RESPONSE DATA");
    else
      addResponseLine(errorResponse, construct, ERROR_TYPE);
    construct.closeElement();
  }
  else
  {
    if ((!fareDisplayTrx.isSDSOutputType() &&
         (fareDisplayTrx.isDiagnosticRequest() || !fareDisplayTrx.isERDFromSWS())) ||
        errorForSDS)
    {
      addResponseLine(tmpResponse, construct, MSG_TYPE, msgType);
    }

    if (fareDisplayTrx.getRequest()->requestType() != "RB" && !errorForSDS)
    {
      if (!fareDisplayTrx.isDiagnosticRequest())
      {
        // Do the detail information
        addDetailResponse(fareDisplayTrx, construct);
      }
    }
  }

  construct.closeElement();

  LOG4CXX_DEBUG(logger, "XML Response: " << construct.getXMLData());
  return construct.getXMLData();
} // lint !e550

void
FareDisplayResponseFormatter::mainDiagResponse(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << "***************************************************************"
                            << std::endl;
  fareDisplayTrx.response() << "FARE DISPLAY DIAGNOSTICS MASTER LIST " << std::endl;
  fareDisplayTrx.response() << "***************************************************************"
                            << std::endl;
  fareDisplayTrx.response() << "TO ACCESS PRICING MASTER LIST, ADD " << CROSS_LORRAINE
                            << "Q/*999 TO PRICING ENTRY " << std::endl;
  fareDisplayTrx.response() << "***************************************************************"
                            << std::endl;
  fareDisplayTrx.response() << "ADD CROSS OF LORRAINE BEFORE EACH FQ DIAGNOSTIC." << std::endl;
  fareDisplayTrx.response() << "ENTER A SLASH AFTER THE NUMBER TO ADD APPLICABLE QUALIFIERS "
                            << std::endl;
  fareDisplayTrx.response() << "  /DDALLFARES        ALL VALID/INVALID FARES IN MARKET "
                            << std::endl;
  fareDisplayTrx.response() << "  /FCVLXAP6M         SPECIFIC FARE CLASS " << std::endl;
  fareDisplayTrx.response() << "  /FBBAPOW           SPECIFIC FARE BASIS CODE " << std::endl;
  fareDisplayTrx.response() << "  /RU                RULE NUMBER " << std::endl;
  fareDisplayTrx.response() << "  /DDINFO            DISPLAY RECORD 1 INFORMATION " << std::endl;
  fareDisplayTrx.response() << "  /DDRTGREQ          DISPLAY XML REQUEST  TO   RTG  /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << "  /DDRTGRES          DISPLAY XML RESPONSE FROM RTG  /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << "  /DDDSSREQ          DISPLAY XML REQUEST  TO   DSS  /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << "  /DDDSSRES          DISPLAY XML RESPONSE FROM DSS  /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << "  /DDABCCREQ         DISPLAY XML REQUEST  TO   ABCC /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << "  /DDABCCRES         DISPLAY XML RESPONSE FROM ABCC /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << "  /DDS8BRANDREQ      DISPLAY XML REQUEST  TO  S8BRAND /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << "  /DDS8BRANDRES      DISPLAY XML RESPONSE TO  S8BRAND /WITH 195"
                            << std::endl;
  fareDisplayTrx.response() << " " << std::endl;
  fareDisplayTrx.response() << " " << std::endl;
  fareDisplayTrx.response() << "195                  EXTERNAL SERVICES XML REQUEST/RESPONSE "
                            << std::endl;
  fareDisplayTrx.response() << "196                  FQ XML REQUEST TO ATSE " << std::endl;
  fareDisplayTrx.response() << "197                  FQ XML RESPONSE FROM ATSE " << std::endl;
  fareDisplayTrx.response() << "200                  FARE COLLECTOR VALID FARES " << std::endl;
  fareDisplayTrx.response() << "201                  TAXES " << std::endl;
  fareDisplayTrx.response() << "202                  SURCHARGES - VALID FARES " << std::endl;
  fareDisplayTrx.response() << "203                  PASSENGER,FARE AND DISPLAY TYPE-ALL FARES"
                            << std::endl;
  fareDisplayTrx.response() << "204                  RECORD 1 - SUPPRESSED FARES " << std::endl;
  fareDisplayTrx.response() << "205                  CURRENCY VALID FARES " << std::endl;
  fareDisplayTrx.response() << "206                  PASSENGER,FARE AND DISPLAY TYPE-VALID FARE"
                            << std::endl;
  fareDisplayTrx.response() << "207                  INCLUSION CODE TABLE " << std::endl;
  fareDisplayTrx.response() << "208                  CAT 25 RECORD 8 PROCESSING " << std::endl;
  fareDisplayTrx.response() << "209                  WEB FARES TABLE " << std::endl;
  fareDisplayTrx.response() << "210                  AVAILABLE " << std::endl;
  fareDisplayTrx.response() << "211                  HEADER MESSAGE TABLE " << std::endl;
  fareDisplayTrx.response() << "212                  SUPRESSION TABLE " << std::endl;
  fareDisplayTrx.response() << "213                  SORTING TABLE " << std::endl;
  fareDisplayTrx.response() << "214                  BYPASS GROUPING " << std::endl;
  fareDisplayTrx.response() << "215                  FBR ROUTING " << std::endl;
  fareDisplayTrx.response() << "216                  USER PREFERENCE TABLE " << std::endl;
  fareDisplayTrx.response() << "217                  MULTI TRANSPORT FARE GROUPING " << std::endl;
  fareDisplayTrx.response() << "225                  CAT 25 RECORD 2 PROCESSING" << std::endl;
  fareDisplayTrx.response() << "291                  RULE DISPLAY, FARE BASIS NOT FOUND"
                            << std::endl;
  fareDisplayTrx.response() << "301                  ELIGIBILITY RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "302                  DAY OF WEEK RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "303                  SEASON RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "305                  ADVANCE RES/TKT RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "306                  MINIMUM STAY RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "307                  MAXIMUM STAY RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "311                  BLACKOUTS RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "312                  SURCHARGES RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "314                  TRAVEL RESTRICTIONS RULE VALIDATION "
                            << std::endl;
  fareDisplayTrx.response() << "315                  SALES RESTRICTIONS RULE VALIDATION "
                            << std::endl;
  fareDisplayTrx.response() << "316                  PENALITIES RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "319                  CHILD/INFANT DISCOUNTS RULE VALIDATION"
                            << std::endl;
  fareDisplayTrx.response() << "320                  TOUR DISCOUNTS RULE VALIDATION" << std::endl;
  fareDisplayTrx.response() << "321                  AGENTS DISCOUNTS RULE VALIDATION "
                            << std::endl;
  fareDisplayTrx.response() << "322                  OTHER DISCOUNTS RULE VALIDATION " << std::endl;
  fareDisplayTrx.response() << "323                  MISCELLANEOUS PROVISIONS RULE VALIDATION "
                            << std::endl;
  fareDisplayTrx.response() << "325                  FARE BY RULE CAT 25 RULE VALIDATION "
                            << std::endl;
  fareDisplayTrx.response() << "335                  NEGOTIATED FARES RULE VALIDATION" << std::endl;
  fareDisplayTrx.response() << "854                  DATABASE SERVER AND PORT NUMBER" << std::endl;
  fareDisplayTrx.response() << "***************************************************************"
                            << std::endl;
  fareDisplayTrx.diagnostic().insertDiagMsg(fareDisplayTrx.response().str());
  fareDisplayTrx.response().clear();
}

void
FareDisplayResponseFormatter::addMessageLine(const std::string& line,
                                             XMLConstruct& construct,
                                             const std::string& msgType)
{
  construct.openElement("MSG");
  construct.addAttribute("N06", msgType);
  sprintf(_tmpBuf, "%06d", _recNum++);
  construct.addAttribute("Q0K", _tmpBuf);
  construct.addAttribute("S18", line);
  construct.closeElement();
}

//----------------------------------------------------------------------------
// FareDisplayResponseFormatter::addDetailResponse
//----------------------------------------------------------------------------
void
FareDisplayResponseFormatter::addDetailResponse(FareDisplayTrx& fareDisplayTrx,
                                                XMLConstruct& construct)
{
  addODCType(fareDisplayTrx, construct); // ATTEMP

  if (fareDisplayTrx.isSDSOutputType())
  {
    // Each type done as a section
    // ATTEMP addODCType(fareDisplayTrx, construct);

    if (fareDisplayTrx.getRequest()->requestType() == FARE_TAX_REQUEST)
    {
      addFTDType(fareDisplayTrx, construct);
      return;
    }

    // if (fareDisplayTrx.isShopperRequest())
    if (fareDisplayTrx.isScheduleCountRequested())
    {
      addMACType(fareDisplayTrx, construct);
      addNSFType(fareDisplayTrx, construct);
      addDFCType(fareDisplayTrx, construct);
      addCFCType(fareDisplayTrx, construct);
    }
    else
    {
      addOCMType(fareDisplayTrx, construct);
    }

    addHDMType(fareDisplayTrx, construct);
    addYYMType(fareDisplayTrx, construct);
    addRTGType(fareDisplayTrx, construct);
  }

  if (!fareDisplayTrx.allFDAddOnFare().empty())
  {
    addFQDTypeForAddon(fareDisplayTrx, construct);
  }

  addFQDType(fareDisplayTrx, construct);
}

void
FareDisplayResponseFormatter::addODCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Add Origin/Destination and Currency information
  construct.openElement("ODC");

  // Board point
  construct.addAttribute("A01", fareDisplayTrx.boardMultiCity().c_str());

  // Off point
  construct.addAttribute("A02", fareDisplayTrx.offMultiCity().c_str());

  // Display currency code
  construct.addAttribute("C46", fareDisplayTrx.itin().front()->calculationCurrency().c_str());

  construct.closeElement();
}

void
FareDisplayResponseFormatter::addMACType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Add Marketed Airline Codes information... needed from Schedules
  if (fareDisplayTrx.hasScheduleCountInfo())
  {
    std::vector<FlightCount*>::iterator iter =
        fareDisplayTrx.fdResponse()->scheduleCounts().begin();
    std::vector<FlightCount*>::iterator end = fareDisplayTrx.fdResponse()->scheduleCounts().end();

    for (; iter != end; iter++)
    {
      construct.openElement("MAC");
      construct.addAttribute("B06", (*iter)->_carrier);
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::addNSFType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Add Non-stop flight information... needed from Schedules
  if (fareDisplayTrx.hasScheduleCountInfo())
  {
    std::vector<FlightCount*>::iterator iter =
        fareDisplayTrx.fdResponse()->scheduleCounts().begin();
    std::vector<FlightCount*>::iterator end = fareDisplayTrx.fdResponse()->scheduleCounts().end();

    for (; iter != end; iter++)
    {
      construct.openElement("NSF");
      std::ostringstream s;
      s << (*iter)->_nonStop;
      construct.addAttribute("Q42", s.str());
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::addDFCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  if (fareDisplayTrx.hasScheduleCountInfo())
  {
    std::vector<FlightCount*>::iterator iter =
        fareDisplayTrx.fdResponse()->scheduleCounts().begin();
    std::vector<FlightCount*>::iterator end = fareDisplayTrx.fdResponse()->scheduleCounts().end();

    for (; iter != end; iter++)
    {
      construct.openElement("DFC");
      std::ostringstream s;
      s << (*iter)->_direct;
      construct.addAttribute("Q43", s.str());
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::addCFCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Add direct flight count information... needed from Schedules
  if (fareDisplayTrx.hasScheduleCountInfo())
  {
    std::vector<FlightCount*>::iterator iter =
        fareDisplayTrx.fdResponse()->scheduleCounts().begin();
    std::vector<FlightCount*>::iterator end = fareDisplayTrx.fdResponse()->scheduleCounts().end();

    for (; iter != end; iter++)
    {
      construct.openElement("CFC");
      construct.addAttribute("Q44",
                             FareDisplayUtil::getConnectingFlightCount(fareDisplayTrx, **iter));
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::addHDMType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Add alternate carriers header message information
  if (reqHasAltCxrs)
  {
    construct.openElement("HDM");
    construct.addAttribute("S46",
                           FareDisplayUtil::getAltCxrMsg(fareDisplayTrx.origin()->loc(),
                                                         fareDisplayTrx.destination()->loc()));
    construct.closeElement();
  }

  if (reqHasYYCxr)
  {
    // Add YY fare message information
    construct.openElement("HDM");
    construct.addAttribute("S46", FareDisplayUtil::ALT_CXR_YY_MSG);
    construct.closeElement();
  }

  std::vector<std::string> hdrMsgs;
  // Get DB header message information
  _xmlTags.hdrMsgs(fareDisplayTrx, hdrMsgs);
  // Get Currency Conversion header message information
  _xmlTags.currencyConversionHdrMsgs(fareDisplayTrx, hdrMsgs);
  std::vector<std::string>::const_iterator iter = hdrMsgs.begin();
  std::vector<std::string>::const_iterator iterEnd = hdrMsgs.end();
  for (; iter != iterEnd; iter++)
  {
    if ((*iter).length() > 0)
    {
      construct.openElement("HDM");
      construct.addAttribute("S46", *iter);
      construct.closeElement();
    }
  }

  std::ostringstream s;
  // If there's a no fares message, get it without whitespace padding
  if (FareDisplayUtil::getCarrierFareHeaderMsg(fareDisplayTrx, s, false))
  {
    construct.openElement("HDM");
    construct.addAttribute("S46", s.str());
    construct.closeElement();
  }
}

void
FareDisplayResponseFormatter::addYYMType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  std::string msg = _xmlTags.yyFareMsg(fareDisplayTrx);
  if (msg != "")
  {
    // Add YY fare message information
    construct.openElement("YYM");
    construct.addAttribute("S47", msg);
    construct.closeElement();
  }
}

void
FareDisplayResponseFormatter::addOCMType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Add other carriers to response
  std::set<CarrierCode>::const_iterator cxrSetIter = fareDisplayTrx.preferredCarriers().begin();
  std::set<CarrierCode>::const_iterator cxrSetIterEnd = fareDisplayTrx.preferredCarriers().end();
  for (; cxrSetIter != cxrSetIterEnd; cxrSetIter++)
  {
    // Skip requested carrier
    if ((*cxrSetIter) == fareDisplayTrx.requestedCarrier())
      continue;

    reqHasAltCxrs = true;

    if ((*cxrSetIter) == INDUSTRY_CARRIER)
      reqHasYYCxr = true;

    construct.openElement("OCM");
    construct.addAttribute("B00", *cxrSetIter);
    construct.closeElement();
  }
}

void
FareDisplayResponseFormatter::addRTGType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Don't populate these tags for shopper requests
  if (fareDisplayTrx.isShopperRequest())
    return;

  FareDisplayResponseRoutingXMLTags routingTags;
  routingTags.buildTags(fareDisplayTrx, construct);
}

void
FareDisplayResponseFormatter::addFQDType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Add routing message information
  std::vector<PaxTypeFare*>::iterator iter = fareDisplayTrx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator iterEnd = fareDisplayTrx.allPaxTypeFare().end();

  uint32_t fareLevelCount = 0;
  uint16_t count = 0;
  MoneyAmount fareAmt = 0;

  uint32_t numFareLevelRequest = fareDisplayTrx.getRequest()->numberOfFareLevels();
  uint32_t maxFares = FareDisplayUtil::getMaxFares();

  if (fareDisplayTrx.isSDSOutputType())
  {
    maxFares = maxFaresSDSCfg.getValue();
  }

  const bool limitResponseSize = !TrxUtil::libPoAtsePath(fareDisplayTrx);

  int index = -1;

  for (; iter != iterEnd; iter++)
  {
    PaxTypeFare*& paxTypeFare = *iter;
    index++;

    if (!paxTypeFare->isValid())
    {
      continue;
    }

    // Make sure we don't exceed the max number of fares allowed
    if (count >= maxFares)
      break;

    // Make sure we don't exceed the max size allowed
    if (limitResponseSize && construct.getXMLData().size() > maxResponseSize.getValue())
    {
      if (fareDisplayTrx.isSDSOutputType())
        break;

      // For non-SDS responses we cannot continue when the max size is exceeded
      uint32_t nbrOfFares = fareDisplayTrx.allPaxTypeFare().size();

      if (nbrOfFares > maxFares)
        nbrOfFares = maxFares;

      LOG4CXX_ERROR(logger,
                    "Max response size exceeded - " << construct.getXMLData().size() << " bytes.");
      LOG4CXX_ERROR(logger, "  Nbr of fares on display = " << nbrOfFares);
      LOG4CXX_ERROR(logger, "  Nbr of FQDs processed   = " << count);
      LOG4CXX_ERROR(logger, "  Entry: " << TrxUtil::getLineEntry(fareDisplayTrx));

      //          fareDisplayTrx.response().clear();
      fareDisplayTrx.errorResponse() << "OUTPUT SIZE TOO BIG - RETRY WITH ADDITIONAL QUALIFIERS"
                                     << std::endl;

      throw NonFatalErrorResponseException(ErrorResponseException::OUTPUT_SIZE_TOO_BIG,
                                           fareDisplayTrx.errorResponse().str().c_str());
    }

    if (numFareLevelRequest > 0)
    {
      MoneyAmount totalFareAmt = paxTypeFare->convertedFareAmount();
      if (paxTypeFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
        totalFareAmt *= 2;
      else if (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED)
        totalFareAmt = paxTypeFare->convertedFareAmountRT();

      if (fareAmt != totalFareAmt)
      {
        fareAmt = totalFareAmt;
        fareLevelCount++;
      }
      if (fareLevelCount > numFareLevelRequest)
        break;
    }

    FareDisplayInfo* fareDisplayInfo = paxTypeFare->fareDisplayInfo();
    if (fareDisplayInfo != nullptr)
    {
      const DateTime& ticketingDate = fareDisplayTrx.ticketingDate();

      count++;
      construct.openElement("FQD");

      const Fare* fare = paxTypeFare->fare();
      const PaxTypeFare* baseFare = paxTypeFare;
      try
      {
        baseFare = paxTypeFare->fareWithoutBase();
      }
      catch (TSEException& exc)
      {
        LOG4CXX_ERROR(logger, "No base fare for: " << paxTypeFare->fareClass());
        // something is really wrong, but attempt to complete
        // using the wrong (yet available) data from paxTypeFare
      }

      // Fare basis code
      construct.addAttributeNoNull("B50", paxTypeFare->createFareBasisCodeFD(fareDisplayTrx));

      // Carrier of fare
      construct.addAttribute("B00", fare->carrier().c_str());

      // Governing Carrier
      construct.addAttribute("B01", paxTypeFare->fareMarket()->governingCarrier());

      // FareType
      construct.addAttribute("S53", paxTypeFare->fcaFareType());

      // Booking Code
      BookingCode bookingCode = paxTypeFare->bookingCode();
      if (bookingCode.empty())
      {
        FareDisplayBookingCode fdbc;
        fdbc.getBookingCode(fareDisplayTrx, *paxTypeFare, bookingCode);
      }

      construct.addAttribute("B30", bookingCode.c_str());

      // Routing number/sequence
      construct.addAttribute("S49", _xmlTags.routing(*paxTypeFare, fareDisplayInfo));
      construct.addAttributeNoNull("S86", paxTypeFare->routingNumber().c_str());

      // Passenger type
      if (paxTypeFare->actualPaxType() && !paxTypeFare->actualPaxType()->paxType().empty())
        construct.addAttribute("B70", paxTypeFare->actualPaxType()->paxType().c_str());
      else
        construct.addAttribute("B70", "ADT");

      // Original fare amount
      construct.addAttribute("C50", _xmlTags.originalFareAmount(fare, ticketingDate));

      // Calculated fare amount
      std::ostringstream str;
      str.precision(9);
      str << fare->fareAmount();
      construct.addAttribute("C5A", str.str());

      // Currency code
      construct.addAttribute("C46", fare->currency().c_str());

      // Vendor code
      construct.addAttributeNoNull("S37", baseFare->vendor());

      construct.addAttribute("PAS", _xmlTags.constructedFareIndicator(*paxTypeFare));

      construct.addAttributeNoNull("AK0", FareDisplayUtil::getFareOrigin(paxTypeFare));
      construct.addAttributeNoNull("AL0", FareDisplayUtil::getFareDestination(paxTypeFare));

      construct.addAttributeNoNull("BJ0", baseFare->fareClass().c_str());
      construct.addAttributeInteger("Q3W", paxTypeFare->fareTariff());

      construct.addAttributeInteger("Q46", baseFare->linkNumber());
      construct.addAttributeInteger("Q1K", baseFare->sequenceNumber());

      if (BrandingUtil::isPopulateFareDataMapVec(fareDisplayTrx))
      {
        addAttributesForBrandInFQD(fareDisplayInfo, construct);
        addAttribute(fareDisplayTrx, index, construct);
      }

      // matched Account Code for multi AccCode/CorpID processing
      if (fareDisplayTrx.getRequest()->isMultiAccCorpId())
      // to be changed (SM6 also for single processing) when PSS is ready
      {
        bool tagProcessed = false;
        if (paxTypeFare->isFareByRule()) // cat 25 AccCode
        {
          const FBRPaxTypeFareRuleData* fbrData =
              paxTypeFare->getFbrRuleData(RuleConst::FARE_BY_RULE);
          if (fbrData != nullptr && !fbrData->fbrApp()->accountCode().empty())
          {
            construct.addAttribute("SM6", fbrData->fbrApp()->accountCode());
            tagProcessed = true;
          }
        }
        if (!paxTypeFare->matchedAccCode().empty() && !tagProcessed) // cat 1 AccCode
        {
          construct.addAttribute("SM6", paxTypeFare->matchedAccCode());
        }
      }

      // Base Fare Create date
      if (baseFare->createDate().isValid())
      {
        construct.addAttribute("D12", baseFare->createDate().dateToString(YYYYMMDD, "-").c_str());

        // Create time
        //                construct.addAttribute("D55",
        //                    baseFare->createDate().timeToSimpleString().c_str());
        std::string createTime = baseFare->createDate().timeToSimpleString();
        createTime[2] = '-';
        createTime[5] = '-';
        construct.addAttribute("D55", createTime.c_str());
      }

      // Private Indicator PCG
      std::string pi;
      bool isFQ = true;
      PrivateIndicator::privateIndicatorOld(*paxTypeFare, pi, true, isFQ);

      if (!pi.empty())
        construct.addAttribute("PCG", pi);

      construct.addAttributeNoNull("S90", paxTypeFare->ruleNumber());

      // cat35Type is also valid for base fares
      if (paxTypeFare->fareDisplayCat35Type() != ' ')
        construct.addAttributeChar("N1P", paxTypeFare->fareDisplayCat35Type());

      // Constructed fare information
      if (!paxTypeFare->fare()->isConstructedFareInfoMissing())
      {
        construct.addAttributeNoNull("AM0", paxTypeFare->gateway1());
        construct.addAttributeNoNull("AN0", paxTypeFare->gateway2());
        construct.addAttributeInteger("N1J", paxTypeFare->constructionType());

        sprintf(_tmpBuf, "%-.*f", 2, paxTypeFare->specifiedFareAmount());
        construct.addAttribute("C66", _tmpBuf);

        std::ostringstream str;
        str.precision(9);
        str << paxTypeFare->constructedNucAmount();
        construct.addAttribute("C6K", str.str());
      } // endif - has constructed data

      if (paxTypeFare->hasCat35Filed())
      {
        const NegPaxTypeFareRuleData* negRuleData = paxTypeFare->getNegRuleData();
        if (negRuleData && negRuleData->fareRetailerRuleId())
          construct.addAttributeNoNull("S38", negRuleData->sourcePseudoCity());

        if (!fallback::fallbackFRRProcessingRetailerCode(&fareDisplayTrx))
        {
          if (negRuleData && !negRuleData->sourcePseudoCity().empty() && !negRuleData->fareRetailerCode().empty())
            construct.addAttributeNoNull("RC1", negRuleData->fareRetailerCode());
        }
      }

      if (paxTypeFare->getAdjustedSellingCalcData())
      {
        construct.addAttributeNoNull("S39",
                                     paxTypeFare->getAdjustedSellingCalcData()->getSourcePcc());

        if (!fallback::fallbackFRRProcessingRetailerCode(&fareDisplayTrx))
        {
          const AdjustedSellingCalcData* adjSellingCalcData  = paxTypeFare->getAdjustedSellingCalcData();
          if (adjSellingCalcData->getFareRetailerRuleInfo() && 
              !adjSellingCalcData->getSourcePcc().empty() && 
              !adjSellingCalcData->getFareRetailerRuleInfo()->fareRetailerCode().empty())
            construct.addAttributeNoNull("RC2", adjSellingCalcData->getFareRetailerRuleInfo()->fareRetailerCode());
        }
      }

      std::string indicatorOrigAdjusted;
      if (paxTypeFare->getAdjustedSellingCalcData() &&
          !fareDisplayTrx.getOptions()->isXRSForFRRule())
        indicatorOrigAdjusted = "S";
      else if (fareDisplayTrx.getOptions()->isPDOForFRRule() &&
               paxTypeFare->isAdjustedSellingBaseFare())
        indicatorOrigAdjusted = "O";
      if (!indicatorOrigAdjusted.empty())
        construct.addAttribute("S40", indicatorOrigAdjusted);

      // Populate XML tags needed for SDS only
      if (fareDisplayTrx.isSDSOutputType())
        addAttributesForSDSInFQD(
            fareDisplayTrx, fareDisplayInfo, *paxTypeFare, baseFare, fare, construct);

      if (fareDisplayTrx.isERDFromSWS())
      {
        buildAdditionalRuleInfoForSWS(fareDisplayTrx, fareDisplayInfo, *paxTypeFare, construct);
      }

      // Fare by rule (Cat 25) fare information
      if (paxTypeFare->isFareByRule())
      {
        LOG4CXX_DEBUG(logger, "GOT FARE BY RULE FARE");
        addC25Type(*paxTypeFare, construct);
      }

      // Negotiated (Cat 35) fare information
      if (paxTypeFare->isNegotiated())
      {
        LOG4CXX_DEBUG(logger, "GOT FARE WITH CAT35 FILED (may be base fare)");
        addC35Type(*paxTypeFare, construct);
      }

      // Discounted (Cats 19-22) fare information
      if (paxTypeFare->isDiscounted())
      {
        LOG4CXX_DEBUG(logger, "GOT DISCOUNTED FARE");
        addDFIType(*paxTypeFare, construct);
      }
      // Constructed fare information
      // This logic was separated from the previous Constructed fare information build
      // due too issues with the xml tags built after OAO and DAO Types.
      if (!paxTypeFare->fare()->isConstructedFareInfoMissing())
      {
        addOAOType(fareDisplayTrx, *paxTypeFare, construct);
        addDAOType(fareDisplayTrx, *paxTypeFare, construct);
      } // endif - has constructed data

      if (fareDisplayTrx.isERDFromSWS())
      {
        buildRuleCategoryInfoTypesForSWS(fareDisplayTrx, fareDisplayInfo, *paxTypeFare, construct);
      }

    } // endif - FQD

    construct.closeElement();
  }
}

void
FareDisplayResponseFormatter::addAttributesForSDSInFQD(FareDisplayTrx& fareDisplayTrx,
                                                       FareDisplayInfo* fareDisplayInfo,
                                                       PaxTypeFare& paxTypeFare,
                                                       const PaxTypeFare* baseFare,
                                                       const Fare* fare,
                                                       XMLConstruct& construct)
{
  const DateTime& ticketingDate = fareDisplayTrx.ticketingDate();
  // Insdustry fare indicator
  construct.addAttribute("PAK", _xmlTags.industryFareIndicator(paxTypeFare));

  // First sales date (Quote date)
  construct.addAttribute("D14", _xmlTags.firstSalesDate(fareDisplayInfo));

  // Effective date
  construct.addAttribute("D05", _xmlTags.effectiveDate(fareDisplayInfo));

  // Expiration date
  construct.addAttribute("D06", _xmlTags.expirationDate(fareDisplayInfo));

  // Last ticket date (Ticket original issue date)
  construct.addAttribute("D07", _xmlTags.lastTicketDate(fareDisplayInfo));

  // Cabin and Premium Cabin
  construct.addAttribute("N00", _xmlTags.allCabin(paxTypeFare, fareDisplayTrx));

  // OW/RT indicator
  construct.addAttribute("P04", _xmlTags.journeyType(paxTypeFare, fareDisplayTrx).c_str());

  // Same day indicator
  construct.addAttribute("PAL", _xmlTags.sameDayChange(fareDisplayInfo).c_str());

  // Type of fare
  construct.addAttribute("N1H", _xmlTags.typeOfFare(paxTypeFare, fareDisplayTrx));

  // Advance purchase
  construct.addAttribute("S50", _xmlTags.advancePurchase(fareDisplayInfo).c_str());

  // Minimum stay
  construct.addAttribute("S51", _xmlTags.minStay(fareDisplayInfo).c_str());

  // Maximum stay
  construct.addAttribute("S52", _xmlTags.maxStay(fareDisplayInfo).c_str());

  // Vendor code indicator
  construct.addAttribute("PAV", _xmlTags.vendorCode(paxTypeFare, fareDisplayTrx).c_str());

  // Private fare indicator
  construct.addAttribute("PAR", _xmlTags.privateFareIndicator(fareDisplayInfo, fareDisplayTrx));

  // Corporate id
  construct.addAttribute("AC0", _xmlTags.corporateId(paxTypeFare, fareDisplayTrx));

  // Cat 22 indicator
  construct.addAttribute("PBR", _xmlTags.cat22Indicator(fareDisplayTrx, fareDisplayInfo));

  // Base fare amount
  construct.addAttribute("C51", _xmlTags.baseFareAmount(baseFare, fare, ticketingDate));

  // Base fare currency code
  construct.addAttribute("C40", baseFare->currency().c_str());

  // Base selling amount
  construct.addAttribute("C6N", _xmlTags.baseSellingAmount(paxTypeFare, ticketingDate));

  // Base selling currency code
  construct.addAttribute("C6M", _xmlTags.baseSellingCurrency(paxTypeFare));

  // Tags for cat 35 fares only
  if (paxTypeFare.isNegotiated())
  {
    // Cat 35 fare amount
    construct.addAttribute("C62", _xmlTags.cat35FareAmount(fareDisplayTrx, paxTypeFare));

    // Net fare indicator
    construct.addAttribute("N1I", _xmlTags.netFareIndicator(paxTypeFare).c_str());

    // Pricing/Ticketing restriction (cat 35 fares)
    construct.addAttribute("PAU", _xmlTags.pricingTicketingRestriction(paxTypeFare));
  }

  // Passenger type code
  addPTCType(fareDisplayInfo, paxTypeFare, construct);

  // Fare amount
  addFAMType(fareDisplayTrx, paxTypeFare, construct);

  // Seasonal application
  addSEAType(fareDisplayInfo, construct);

  // Rule category numbers
  addRULType(fareDisplayTrx, fareDisplayInfo, construct);
  // Tax amount
  if (sdsFqFlTaxNeeded.getValue() &&
      fareDisplayTrx.getRequest()->requestType() == FARE_DISPLAY_REQUEST &&
      fareDisplayTrx.getOptions()->displayBaseTaxTotalAmounts() == TRUE_INDICATOR)
    addFTDTypeForFQDType(fareDisplayTrx, construct, paxTypeFare);
}

void
FareDisplayResponseFormatter::addFTDType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  PaxTypeFare* paxTypeFare = fareDisplayTrx.allPaxTypeFare().front();
  if (paxTypeFare == nullptr)
    return; // add log4cxx error
  FareDisplayInfo* fareDisplayInfo = paxTypeFare->fareDisplayInfo();
  if (fareDisplayInfo == nullptr)
    return; // add log4cxx error
  const Fare* fare = paxTypeFare->fare();
  const DateTime& ticketingDate = fareDisplayTrx.ticketingDate();

  construct.openElement("FTD");
  // Carrier Code
  construct.addAttribute("B00", fare->carrier());
  // Fare Basis Code
  construct.addAttribute("B50", paxTypeFare->createFareBasisCodeFD(fareDisplayTrx));
  // OW/RT Indicator
  construct.addAttribute("P04", _xmlTags.journeyType(*paxTypeFare, fareDisplayTrx));
  // Base Fare currency code
  construct.addAttribute("C6P", fare->currency().c_str());
  // Base Fare amount
  construct.addAttribute("C6O", _xmlTags.originalFareAmount(fare, ticketingDate));

  CurrencyCode ccode = fareDisplayTrx.itin().front()->calculationCurrency();
  if (ccode != fare->currency().c_str())
  {
    // Equivalent currency code
    construct.addAttribute("C6Q", ccode);
    std::string equivalentFareTag;
    // Equivalent fare amount paid
    equivalentFareTag = _xmlTags.oneWayFare(fareDisplayTrx, *paxTypeFare);
    if (!equivalentFareTag.empty())
    {
      construct.addAttribute("C6R", equivalentFareTag);
    }
    else
    {
      equivalentFareTag = _xmlTags.roundTripFare(fareDisplayTrx, *paxTypeFare);
      construct.addAttribute("C6R", equivalentFareTag);
    }
  }
  // Get all the tax Info
  const std::vector<FareDisplaySDSTaxInfo*>& taxInfo =
      _xmlTags.getSDSTaxInfo(fareDisplayTrx, fareDisplayInfo, *paxTypeFare);
  // Total Amount
  construct.addAttribute("C6S", _xmlTags.totalAmount(taxInfo, fareDisplayTrx, *paxTypeFare));

  if (taxInfo.empty())
  {
    //********TAXTXT (Tax Info)*****************************
    // Free form text
    construct.addAttribute("S04", "*NO TAX*");
  }
  else
  {
    // Total tax currency code
    construct.addAttribute("C42", ccode);

    //*********TAXTTL (Total Tax Info)**********************
    // Total tax amount
    MoneyAmount totalTax = _xmlTags.getTotalTax(taxInfo);
    construct.addAttribute("C65", _xmlTags.formatMoneyAmount(totalTax, ccode, ticketingDate));
  }

  //********FQCONV (Currency Conversion Infos)***********
  addFTCType(fareDisplayTrx, construct);

  //********TAXDAT (Tax Infos)****************************
  if (!taxInfo.empty())
    addFTIType(taxInfo, ccode, construct, ticketingDate);

  construct.closeElement();
}

void
FareDisplayResponseFormatter::addRULType(FareDisplayTrx& fareDisplayTrx,
                                         FareDisplayInfo*& fareDisplayInfo,
                                         XMLConstruct& construct)
{
  // Get agent duty code
  Agent* agent = fareDisplayTrx.getRequest()->ticketingAgent();
  bool dutyCode78 = false;
  if ((agent->agentDuty() == "7") || (agent->agentDuty() == "8") || (agent->agentDuty() == "$"))
  {
    dutyCode78 = true;
  }

  // iterate over options ruleCategories
  std::vector<CatNumber>::const_iterator iter =
      fareDisplayTrx.getOptions()->ruleCategories().begin();
  std::vector<CatNumber>::const_iterator iterEnd =
      fareDisplayTrx.getOptions()->ruleCategories().end();

  for (; iter != iterEnd; iter++)
  {
    CatNumber catNumber = *iter;
    // get rule Category applicability for each category number for this fare
    char displayCode = fareDisplayInfo->getCategoryApplicability(dutyCode78, catNumber);
    if (displayCode == '*' || displayCode == '$' || displayCode == '-' || displayCode == '/')
    {
      std::ostringstream cnTag;
      if (catNumber == IC_RULE_CATEGORY)
        cnTag << INTL_CONST_CODE;
      else if (catNumber == RETAILER_CATEGORY)
        cnTag << RETAILER_CODE;
      else
        cnTag << catNumber;
      // Add rule information
      construct.openElement("RUL");
      construct.addAttribute("Q3V", cnTag.str());
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::addC25Type(PaxTypeFare& paxTypeFare, XMLConstruct& construct)
{
  try
  {
    const FareByRuleItemInfo& itemInfo = paxTypeFare.fareByRuleInfo();
    LOG4CXX_DEBUG(logger, "HAVE CAT25 RULE INFO");

    // Add CAT25 fare information
    construct.openElement("C25");
    construct.addAttributeNoNull("S37", itemInfo.vendor());
    construct.addAttributeInteger("Q41", itemInfo.itemNo());
    construct.closeElement();
  }
  catch (TSEException& exc)
  {
    LOG4CXX_ERROR(logger, "NO CAT25 RULE INFO");
  }
}

void
FareDisplayResponseFormatter::addC35Type(PaxTypeFare& paxTypeFare, XMLConstruct& construct)
{
  // Add C35 fare information
  construct.openElement("C35");

  try
  {
    const NegFareRest& itemInfo = paxTypeFare.negotiatedInfo();
    LOG4CXX_DEBUG(logger, "HAVE NEGOTIATED RULE INFO");

    construct.addAttributeNoNull("S37", itemInfo.vendor());
    construct.addAttributeInteger("Q41", itemInfo.itemNo());
  }
  catch (TSEException& exc)
  {
    LOG4CXX_ERROR(logger, "NO NEGOTIATED RULE INFO");
  }

  construct.closeElement();
}

void
FareDisplayResponseFormatter::addDFIType(PaxTypeFare& paxTypeFare, XMLConstruct& construct)
{
  try
  {
    const DiscountInfo& itemInfo = paxTypeFare.discountInfo();
    LOG4CXX_DEBUG(logger, "HAVE DISCOUNT RULE INFO");

    // Add Discounted fare information
    construct.openElement("DFI");
    construct.addAttributeNoNull("S37", itemInfo.vendor());
    construct.closeElement();
  }
  catch (TSEException& exc)
  {
    LOG4CXX_ERROR(logger, "NO DISCOUNT RULE INFO");
  }
}

void
FareDisplayResponseFormatter::addOAOType(FareDisplayTrx& fareDisplayTrx,
                                         PaxTypeFare& paxTypeFare,
                                         XMLConstruct& construct)
{
  // Add Constructed fare information for originating side
  construct.openElement("OAO");
  addAddonAttributes(fareDisplayTrx,
                     paxTypeFare,
                     paxTypeFare.origAddonFootNote1(),
                     paxTypeFare.origAddonFootNote2(),
                     paxTypeFare.origAddonFareClass(),
                     paxTypeFare.origAddonTariff(),
                     paxTypeFare.origAddonRouting(),
                     paxTypeFare.origAddonAmount(),
                     paxTypeFare.origAddonCurrency(),
                     paxTypeFare.origAddonOWRT(),
                     paxTypeFare.gateway1(),
                     paxTypeFare.market1(),
                     construct);
  construct.closeElement();
}

void
FareDisplayResponseFormatter::addDAOType(FareDisplayTrx& fareDisplayTrx,
                                         PaxTypeFare& paxTypeFare,
                                         XMLConstruct& construct)
{
  // Add Constructed fare information for destination side
  construct.openElement("DAO");
  addAddonAttributes(fareDisplayTrx,
                     paxTypeFare,
                     paxTypeFare.destAddonFootNote1(),
                     paxTypeFare.destAddonFootNote2(),
                     paxTypeFare.destAddonFareClass(),
                     paxTypeFare.destAddonTariff(),
                     paxTypeFare.destAddonRouting(),
                     paxTypeFare.destAddonAmount(),
                     paxTypeFare.destAddonCurrency(),
                     paxTypeFare.destAddonOWRT(),
                     paxTypeFare.gateway2(),
                     paxTypeFare.market2(),
                     construct);
  construct.closeElement();
}

void
FareDisplayResponseFormatter::addAddonAttributes(FareDisplayTrx& fareDisplayTrx,
                                                 PaxTypeFare& paxTypeFare,
                                                 const Footnote& addonFootnote1,
                                                 const Footnote& addonFootnote2,
                                                 const FareClassCode& addonFareClass,
                                                 const TariffNumber& addonTariff,
                                                 const RoutingNumber& addonRouting,
                                                 const MoneyAmount& addonAmount,
                                                 const CurrencyCode& addonCurrency,
                                                 const Indicator& addonOWRT,
                                                 const LocCode& gateWay,
                                                 const LocCode& market,
                                                 XMLConstruct& construct)
{
  construct.addAttributeNoNull("S55", addonFootnote1);
  construct.addAttributeNoNull("S64", addonFootnote2);
  construct.addAttributeNoNull("BJ0", addonFareClass.c_str());
  construct.addAttributeInteger("Q3W", addonTariff);
  construct.addAttributeNoNull("S65", addonRouting);

  sprintf(_tmpBuf, "%-.*f", 2, addonAmount);
  construct.addAttribute("C50", _tmpBuf);

  construct.addAttributeNoNull("C40", addonCurrency);
  construct.addAttributeChar("P04", addonOWRT);

  if (fareDisplayTrx.isERDFromSWS() && !addonFareClass.empty())
  {
    addAddonAttributesForERDFromSWS(fareDisplayTrx,
                                    paxTypeFare,
                                    addonTariff,
                                    addonAmount,
                                    addonCurrency,
                                    gateWay,
                                    market,
                                    construct);
  }
}

void
FareDisplayResponseFormatter::addAddonAttributesForERDFromSWS(FareDisplayTrx& fareDisplayTrx,
                                                              PaxTypeFare& paxTypeFare,
                                                              const TariffNumber& addonTariff,
                                                              const MoneyAmount& addonAmount,
                                                              const CurrencyCode& addonCurrency,
                                                              const LocCode& gateWay,
                                                              const LocCode& market,
                                                              XMLConstruct& construct)
{
  construct.addAttributeNoNull("AM0", gateWay);
  construct.addAttributeNoNull("AM1", market);

  if (addonCurrency != paxTypeFare.currency())
  {
    MoneyAmount convertedAmount = addonAmount;
    NUCCollectionResults nucResults;
    FareDisplayResponseUtil::convertFare(
        fareDisplayTrx, paxTypeFare, convertedAmount, addonCurrency, nucResults);
    construct.addAttribute("C53",
                           _xmlTags.formatMoneyAmount(convertedAmount,
                                                      const_cast<CurrencyCode&>(addonCurrency),
                                                      fareDisplayTrx.ticketingDate()));
  }

  TariffCode addonTariffCode = "";
  FareDisplayResponseUtil::getAddonTariffCode(
      fareDisplayTrx, paxTypeFare.vendor(), paxTypeFare.carrier(), addonTariff, addonTariffCode);
  construct.addAttributeNoNull("AOT", addonTariffCode);
  construct.addAttributeInteger("AOZ", paxTypeFare.origAddonZone());
}

void
FareDisplayResponseFormatter::addPTCType(FareDisplayInfo*& fareDisplayInfo,
                                         PaxTypeFare& paxTypeFare,
                                         XMLConstruct& construct)
{
  if (paxTypeFare.actualPaxType())
  {
    // At least one pax type
    construct.openElement("PTC");
    if (!paxTypeFare.actualPaxType()->paxType().empty())
      construct.addAttribute("B70", paxTypeFare.actualPaxType()->paxType().c_str());
    else
      construct.addAttribute("B70", "ADT");

    construct.closeElement();

    // Check for additional pax types
    if (!fareDisplayInfo->passengerTypes().empty())
    {
      std::set<PaxTypeCode>::const_iterator iter = fareDisplayInfo->passengerTypes().begin();
      std::set<PaxTypeCode>::const_iterator iterEnd = fareDisplayInfo->passengerTypes().end();

      for (uint16_t i = 0; iter != iterEnd && i < 3; iter++, i++)
      {
        // Limit to 3 additional pax Types
        if (paxTypeFare.actualPaxType()->paxType() != *iter)
        {
          construct.openElement("PTC");
          construct.addAttribute("B70", (*iter).c_str());
          construct.closeElement();
        }
      }
    }
  }
}

void
FareDisplayResponseFormatter::addFAMType(FareDisplayTrx& fareDisplayTrx,
                                         PaxTypeFare& paxTypeFare,
                                         XMLConstruct& construct)
{
  std::string fare;
  fare = _xmlTags.oneWayFare(fareDisplayTrx, paxTypeFare);
  if (!fare.empty())
  {
    // add one-way trip amount
    construct.openElement("FAM");
    construct.addAttribute("C6O", fare);
    construct.closeElement();
  }
  fare = _xmlTags.roundTripFare(fareDisplayTrx, paxTypeFare);
  if (!fare.empty())
  {
    // add round trip amount
    construct.openElement("FAM");
    construct.addAttribute("C6O", fare);
    construct.closeElement();
  }
}

void
FareDisplayResponseFormatter::addSEAType(FareDisplayInfo*& fareDisplayInfo, XMLConstruct& construct)
{
  if (!fareDisplayInfo->isAutoPriceable())
  {
    construct.openElement("SEA");
    construct.addAttribute("S61", "NP"); // SEASONS_NP
    construct.closeElement();
  }
  // iterate over seasonInfo std::vector and create for each season info
  else if (!fareDisplayInfo->seasons().empty())
  {
    std::vector<SeasonsInfo*>::reverse_iterator seasonsInfoIter =
        fareDisplayInfo->seasons().rbegin();
    std::vector<SeasonsInfo*>::reverse_iterator seasonsInfoEnd = fareDisplayInfo->seasons().rend();
    for (; seasonsInfoIter != seasonsInfoEnd; seasonsInfoIter++)
    {
      construct.openElement("SEA");
      construct.addAttribute("S61", _xmlTags.season(*seasonsInfoIter));
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::addFTCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  // Get Currency Converion Info
  CurrencyCode sourceCurrency;
  CurrencyCode targetCurrency;
  CurrencyCode intermediateCurrency;
  ExchRate exchangeRate1;
  ExchRate exchangeRate2;

  if (!_xmlTags.getCurrencyConversionInfo(fareDisplayTrx,
                                          sourceCurrency,
                                          targetCurrency,
                                          intermediateCurrency,
                                          exchangeRate1,
                                          exchangeRate2))
  {
    return;
  }
  else
  { // get first conversion
    std::ostringstream rateTag;
    rateTag.setf(std::ios::fixed);
    rateTag.precision(6);
    construct.openElement("FTC");
    // from currency code
    construct.addAttribute("C46", sourceCurrency);

    // to currency code
    if (!intermediateCurrency.empty())
      construct.addAttribute("C42", intermediateCurrency);
    else
      construct.addAttribute("C42", targetCurrency);
    // conversion rate
    rateTag << exchangeRate1;
    construct.addAttribute("C6T", rateTag.str());
    rateTag.str("");
    construct.closeElement();
    // get second conversion
    if (!intermediateCurrency.empty())
    {
      construct.openElement("FTC");
      // from currency code
      construct.addAttribute("C46", intermediateCurrency);
      // to currency code
      construct.addAttribute("C42", targetCurrency);
      // conversion rate
      rateTag << exchangeRate2;
      construct.addAttribute("C6T", rateTag.str());
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::addFTIType(const std::vector<FareDisplaySDSTaxInfo*>& taxInfo,
                                         CurrencyCode& ccode,
                                         XMLConstruct& construct,
                                         const DateTime& ticketingDate)
{
  if (!taxInfo.empty())
  {
    std::vector<FareDisplaySDSTaxInfo*>::const_iterator taxInfoIter = taxInfo.begin();
    std::vector<FareDisplaySDSTaxInfo*>::const_iterator taxInfoIterEnd = taxInfo.end();
    for (; taxInfoIter != taxInfoIterEnd; taxInfoIter++)
    {
      construct.openElement("FTI");
      // Tax Code
      construct.addAttribute("BC0", (*taxInfoIter)->taxCode());
      // Tax code for ticketing
      construct.addAttribute("BC1", ((*taxInfoIter)->ticketingTaxCode()).substr(0, 2));
      // Individual tax amount
      MoneyAmount taxAmount = (*taxInfoIter)->taxAmount();
      construct.addAttribute("C6B", _xmlTags.formatMoneyAmount(taxAmount, ccode, ticketingDate));
      // Tax Description
      construct.addAttribute("S04", (*taxInfoIter)->taxDescription());
      construct.closeElement();
    }
  }
}

//********************************ADDONSection***************************//
void
FareDisplayResponseFormatter::addFQDTypeForAddon(FareDisplayTrx& fareDisplayTrx,
                                                 XMLConstruct& construct)
{
  uint16_t count(0);
  // Add routing message information
  std::vector<FDAddOnFareInfo*>::iterator iter = fareDisplayTrx.allFDAddOnFare().begin();
  std::vector<FDAddOnFareInfo*>::iterator iterEnd = fareDisplayTrx.allFDAddOnFare().end();

  uint32_t maxFares = FareDisplayUtil::getMaxFares();
  int index = -1;

  for (; iter != iterEnd; iter++)
  {
    FDAddOnFareInfo*& addonFare = *iter;
    index++;

    if (count >= maxFares)
      break;

    count++;
    construct.openElement("FQD");

    construct.addAttribute("B00", addonFare->carrier().c_str());

    sprintf(_tmpBuf, "%-.*f", addonFare->noDec(), addonFare->fareAmt());
    construct.addAttribute("C50", _tmpBuf);

    construct.addAttribute("PAK", "F");

    if (addonFare->effDate().isValid())
      construct.addAttribute("D05", addonFare->effDate().dateToString(YYYYMMDD, "-").c_str());

    if (addonFare->expireDate().isValid())
      construct.addAttribute("D06", addonFare->expireDate().dateToString(YYYYMMDD, "-").c_str());

    construct.addAttributeChar("B30", addonFare->fareClass()[0]);
    construct.addAttributeChar("P04", addonFare->owrt());
    construct.addAttributeNoNull("S49", addonFare->routing());
    construct.addAttributeNoNull("B50", addonFare->fareClass().c_str());
    construct.addAttribute("C46", addonFare->cur());
    construct.addAttribute("S37", addonFare->vendor());
    // Market 1
    construct.addAttributeNoNull("AK0", addonFare->gatewayMarket());
    // Market 2
    construct.addAttributeNoNull("AL0", addonFare->interiorMarket());

    construct.addAttributeInteger("Q3W", addonFare->addonTariff());
    construct.addAttributeInteger("Q46", addonFare->linkNo());
    construct.addAttributeInteger("Q1K", addonFare->seqNo());

    construct.addAttribute("D12", addonFare->createDate().dateToString(YYYYMMDD, "-").c_str());
    std::string createTime = addonFare->createDate().timeToSimpleString();
    createTime[2] = '-';
    createTime[5] = '-';
    construct.addAttribute("D55", createTime.c_str());
    if (BrandingUtil::isPopulateFareDataMapVec(fareDisplayTrx))
      addAttribute(fareDisplayTrx, index, construct, false);
    construct.closeElement();
  } // endfor - all addon fares
}

void
FareDisplayResponseFormatter::addFTDTypeForFQDType(FareDisplayTrx& fareDisplayTrx,
                                                   XMLConstruct& construct,
                                                   PaxTypeFare& paxTypeFare)
{
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();
  if (fareDisplayInfo == nullptr)
  {
    LOG4CXX_ERROR(logger, "BYPASSING Tax Information due to NULL FareDisplayInfo");
    return;
  }
  std::string onewayRoundTrip;
  std::vector<FareDisplaySDSTaxInfo*> sdsTaxInfoVec;
  std::string owrt = "";
  if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
    owrt = "X";
  else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    owrt = "R";
  else if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    owrt = "O";

  MoneyAmount fareAmount = paxTypeFare.convertedFareAmount();
  std::vector<TaxRecord*> taxRecordVec;
  std::vector<TaxItem*> taxItemVec;
  if (owrt == "X")
  {
    // first is for Oneway tax information.
    taxRecordVec = fareDisplayInfo->taxRecordOWRTFareTaxVector();
    taxItemVec = fareDisplayInfo->taxItemOWRTFareTaxVector();
    onewayRoundTrip = "O";
    buildFTDTypeForFQDType(fareDisplayTrx,
                           construct,
                           paxTypeFare,
                           taxRecordVec,
                           taxItemVec,
                           onewayRoundTrip,
                           fareAmount);

    // second is for Roundtrip tax information.

    fareAmount = paxTypeFare.convertedFareAmountRT();

    taxRecordVec = fareDisplayInfo->taxRecordVector();
    taxItemVec = fareDisplayInfo->taxItemVector();
    onewayRoundTrip = "R";
    buildFTDTypeForFQDType(fareDisplayTrx,
                           construct,
                           paxTypeFare,
                           taxRecordVec,
                           taxItemVec,
                           onewayRoundTrip,
                           fareAmount);
  }
  else if (!owrt.empty())
  {
    taxRecordVec = fareDisplayInfo->taxRecordVector();
    taxItemVec = fareDisplayInfo->taxItemVector();
    buildFTDTypeForFQDType(
        fareDisplayTrx, construct, paxTypeFare, taxRecordVec, taxItemVec, owrt, fareAmount);
  }
}

void
FareDisplayResponseFormatter::buildFTDTypeForFQDType(FareDisplayTrx& fareDisplayTrx,
                                                     XMLConstruct& construct,
                                                     PaxTypeFare& paxTypeFare,
                                                     std::vector<TaxRecord*> taxRecordVec,
                                                     std::vector<TaxItem*> taxItemVec,
                                                     std::string owrt,
                                                     MoneyAmount fareAmount)
{
  const std::vector<FareDisplaySDSTaxInfo*>& taxInfo =
      _xmlTags.buildSDSTaxInfo(fareDisplayTrx, paxTypeFare, taxRecordVec, taxItemVec, owrt);

  const DateTime& ticketingDate = fareDisplayTrx.ticketingDate();

  construct.openElement("FTD");
  // OW/RT Indicator
  construct.addAttribute("P04", owrt);
  // Tax amount
  CurrencyCode ccode = fareDisplayTrx.itin().front()->calculationCurrency();
  MoneyAmount totalTax = _xmlTags.getTotalTax(taxInfo);
  construct.addAttribute("C65", _xmlTags.formatMoneyAmount(totalTax, ccode, ticketingDate));
  // Tax Amount plus fare amount
  construct.addAttribute("C6S",
                         _xmlTags.formatMoneyAmount((fareAmount + totalTax), ccode, ticketingDate));
  construct.closeElement();
}

bool
FareDisplayResponseFormatter::isErrorCondition(FareDisplayTrx& fareDisplayTrx)
{
  std::string response = fareDisplayTrx.response().str();

  if (fareDisplayTrx.getRequest()->requestType() == MP_REQUEST &&
      FareDisplayUtil::determineMPType(fareDisplayTrx) == NO_MARKET_MP)
  {
    if (response.empty() || response.length() <= 63)
      return true;
    else
      return false;
  }

  if (fareDisplayTrx.response().str().empty() ||
      (fareDisplayTrx.allPaxTypeFare().empty() && fareDisplayTrx.allFDAddOnFare().empty()))
  {
    return true;
  }

  return false;
}

void
FareDisplayResponseFormatter::buildRuleCategoryInfoTypesForSWS(FareDisplayTrx& fareDisplayTrx,
                                                               FareDisplayInfo* fareDisplayInfo,
                                                               PaxTypeFare& paxTypeFare,
                                                               XMLConstruct& construct)
{
  FareDisplayOptions& fdo = *(fareDisplayTrx.getOptions());
  if (fdo.ruleMenuDisplay() == TRUE_INDICATOR)
    addMNUType(fareDisplayTrx, fareDisplayInfo, construct);
  else if (fdo.routingDisplay() == TRUE_INDICATOR)
    addRSDType(fareDisplayTrx, construct);
  else if (fdo.FBDisplay() == TRUE_INDICATOR)
    addCATTypeForFB(fareDisplayTrx, fareDisplayInfo, paxTypeFare, construct);
  else if (fdo.headerDisplay() == TRUE_INDICATOR ||
           (fdo.IntlConstructionDisplay() == TRUE_INDICATOR && fdo.ruleCategories().size() == 1))
    return;
  else
    addCATType(fareDisplayTrx, fareDisplayInfo, construct);
}

void
FareDisplayResponseFormatter::addMNUType(FareDisplayTrx& fareDisplayTrx,
                                         FareDisplayInfo* fareDisplayInfo,
                                         XMLConstruct& construct)
{
  // Get agent duty code
  Agent* agent = fareDisplayTrx.getRequest()->ticketingAgent();
  bool dutyCode78 =
      ((agent->agentDuty() == "7") || (agent->agentDuty() == "8") || (agent->agentDuty() == "$"));

  // Categories for menu
  std::vector<CatNumber>::const_iterator cat =
                                             fareDisplayTrx.getOptions()->ruleCategories().begin(),
                                         catEnd =
                                             fareDisplayTrx.getOptions()->ruleCategories().end();

  for (; cat != catEnd; ++cat)
  {
    const RuleCategoryDescInfo* descriptionInfo =
        fareDisplayTrx.dataHandle().getRuleCategoryDesc(*cat);

    if (descriptionInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "No description for cat: " << *cat);
      continue;
    }

    construct.openElement("MNU");
    construct.addAttributeShort("NUM", *cat);
    construct.addAttribute("DSC", (descriptionInfo->shortDescription()).c_str());

    if (fareDisplayInfo->isRuleCategoryIncomplete(*cat))
      construct.addAttributeChar("IND", INCOMPLETE[0]);
    else if (fareDisplayInfo->isRuleCategoryUnavailable(*cat))
      construct.addAttributeChar("IND", UNAVAILABLE[0]);
    else
      construct.addAttributeChar("IND",
                                 fareDisplayInfo->getCategoryApplicability(dutyCode78, *cat));
    construct.closeElement();
  }
}

void
FareDisplayResponseFormatter::addRSDType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct)
{
  if (FareDisplayResponseUtil::buildRTGDisplay(fareDisplayTrx,
                                               *fareDisplayTrx.allPaxTypeFare().front()))
  {
    construct.openElement("RSD");
    std::string tempResponse = fareDisplayTrx.response().str();
    addResponseLine(tempResponse, construct, TEXT_TYPE);
    construct.closeElement();
  }
}

void
FareDisplayResponseFormatter::addCATType(FareDisplayTrx& fareDisplayTrx,
                                         FareDisplayInfo* fareDisplayInfo,
                                         XMLConstruct& construct)
{
  // Categories and text
  std::vector<CatNumber>::const_iterator iter =
      fareDisplayTrx.getOptions()->ruleCategories().begin();
  std::vector<CatNumber>::const_iterator iterEnd =
      fareDisplayTrx.getOptions()->ruleCategories().end();
  std::vector<CatNumber>::const_iterator iterLast = iterEnd;
  iterLast--;

  const FareDisplayOptions& fdo = *fareDisplayTrx.getOptions();
  bool showMX = fdo.isCombScoreboardDisplay();
  bool needMX = true;
  CatNumber crc = COMBINABILITY_RULE_CATEGORY;

  // Special case first
  if ((fareDisplayTrx.getOptions()->ruleCategories().empty()) && showMX)
  {
    addMXTextForCATType(fareDisplayTrx, fareDisplayInfo, construct, crc);
  }
  else
  {
    for (; iter != iterEnd; iter++)
    {
      CatNumber catNumber = (*iter);

      if (catNumber == IC_RULE_CATEGORY || catNumber == RETAILER_CATEGORY)
        continue;

      if (needMX && showMX)
      {
        // need to be looking for proper place in sequence to insert MX text.
        if (catNumber == COMBINABILITY_RULE_CATEGORY)
        {
          LOG4CXX_DEBUG(logger, "MX found, putting in now... with: " << catNumber);
          needMX = false;
          addMXAndCatTextForCATType(fareDisplayTrx, fareDisplayInfo, construct, catNumber);
          continue;
        }
        else if (catNumber > COMBINABILITY_RULE_CATEGORY && catNumber != 50)
        {
          LOG4CXX_DEBUG(logger, "MX found, putting in now... before: " << catNumber);
          needMX = false;
          addMXTextForCATType(fareDisplayTrx, fareDisplayInfo, construct, crc);
          addCatTextForCATType(fareDisplayTrx, fareDisplayInfo, construct, catNumber);
          continue;
        }
        else if (iter == iterLast)
        {
          LOG4CXX_DEBUG(logger, "MX found, putting in now... after: " << catNumber);
          needMX = false;
          addCatTextForCATType(fareDisplayTrx, fareDisplayInfo, construct, catNumber);
          addMXTextForCATType(fareDisplayTrx, fareDisplayInfo, construct, crc);
          continue;
        }
      }
      addCatTextForCATType(fareDisplayTrx, fareDisplayInfo, construct, catNumber);
    }
  }
}

void
FareDisplayResponseFormatter::addMXTextForCATType(FareDisplayTrx& fareDisplayTrx,
                                                  FareDisplayInfo* fareDisplayInfo,
                                                  XMLConstruct& construct,
                                                  CatNumber& catNumber)
{
  const RuleCategoryDescInfo* descriptionInfo =
      fareDisplayTrx.dataHandle().getRuleCategoryDesc(catNumber);
  if (descriptionInfo == nullptr)
  {
    LOG4CXX_WARN(logger, "No description for cat: " << catNumber);
    return;
  }
  std::ostringstream oss;
  construct.openElement("CAT");
  construct.addAttributeShort("NUM", catNumber);
  construct.addAttribute("DSC", (descriptionInfo->description()).c_str());
  FareDisplayResponseUtil::addMXText(*fareDisplayInfo, &oss);
  std::string tempResponse = oss.str();
  addResponseLine(tempResponse, construct, TEXT_TYPE);
  construct.closeElement();
}

void
FareDisplayResponseFormatter::addCatTextForCATType(FareDisplayTrx& fareDisplayTrx,
                                                   FareDisplayInfo* fareDisplayInfo,
                                                   XMLConstruct& construct,
                                                   CatNumber& catNumber)
{
  const RuleCategoryDescInfo* descriptionInfo =
      fareDisplayTrx.dataHandle().getRuleCategoryDesc(catNumber);
  if (descriptionInfo == nullptr)
  {
    LOG4CXX_WARN(logger, "No description for cat: " << catNumber);
    return;
  }
  std::ostringstream oss;
  construct.openElement("CAT");
  construct.addAttributeShort("NUM", catNumber);
  construct.addAttribute("DSC", (descriptionInfo->description()).c_str());
  FareDisplayResponseUtil::addCatText(fareDisplayTrx, catNumber, *fareDisplayInfo, &oss);
  std::string tempResponse = oss.str();
  addResponseLine(tempResponse, construct, TEXT_TYPE);
  construct.closeElement();
}

void
FareDisplayResponseFormatter::addMXAndCatTextForCATType(FareDisplayTrx& fareDisplayTrx,
                                                        FareDisplayInfo* fareDisplayInfo,
                                                        XMLConstruct& construct,
                                                        CatNumber& catNumber)
{
  const RuleCategoryDescInfo* descriptionInfo =
      fareDisplayTrx.dataHandle().getRuleCategoryDesc(catNumber);
  if (descriptionInfo == nullptr)
  {
    LOG4CXX_WARN(logger, "No description for cat: " << catNumber);
    return;
  }
  std::ostringstream oss;
  construct.openElement("CAT");
  construct.addAttributeShort("NUM", catNumber);
  construct.addAttribute("DSC", (descriptionInfo->description()).c_str());
  FareDisplayResponseUtil::addMXText(*fareDisplayInfo, &oss);
  FareDisplayResponseUtil::addCatText(fareDisplayTrx, catNumber, *fareDisplayInfo, &oss);
  std::string tempResponse = oss.str();
  addResponseLine(tempResponse, construct, TEXT_TYPE);
  construct.closeElement();
}

void
FareDisplayResponseFormatter::addCATTypeForFB(FareDisplayTrx& fareDisplayTrx,
                                              FareDisplayInfo* fareDisplayInfo,
                                              PaxTypeFare& paxTypeFare,
                                              XMLConstruct& construct)
{
  std::vector<CatNumber>::const_iterator cat =
                                             fareDisplayTrx.getOptions()->ruleCategories().begin(),
                                         catEnd =
                                             fareDisplayTrx.getOptions()->ruleCategories().end();

  for (; cat != catEnd; ++cat)
  {
    if (*cat == IC_RULE_CATEGORY || *cat == RETAILER_CATEGORY)
      continue;
    const RuleCategoryDescInfo* descriptionInfo =
        fareDisplayTrx.dataHandle().getRuleCategoryDesc(*cat);
    if (descriptionInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "No description for cat: " << *cat);
      continue;
    }

    fareDisplayTrx.response().str("");
    FareDisplayResponseUtil fdru;
    if (fdru.buildFBDisplay(*cat, fareDisplayTrx, fareDisplayInfo, paxTypeFare))
    {
      construct.openElement("CAT");
      construct.addAttributeShort("NUM", *cat);
      construct.addAttribute("DSC", (descriptionInfo->shortDescription()).c_str());
      std::string tempResponse = fareDisplayTrx.response().str();
      addResponseLine(tempResponse, construct, TEXT_TYPE);
      construct.closeElement();
    }
  }
}

void
FareDisplayResponseFormatter::buildAdditionalRuleInfoForSWS(FareDisplayTrx& fareDisplayTrx,
                                                            FareDisplayInfo* fareDisplayInfo,
                                                            PaxTypeFare& paxTypeFare,
                                                            XMLConstruct& construct)
{
  // Converted fare amount
  CurrencyCode currencyCode = fareDisplayTrx.itin().front()->calculationCurrency();
  construct.addAttribute("C5B",
                         _xmlTags.formatMoneyAmount(paxTypeFare.convertedFareAmount(),
                                                    currencyCode,
                                                    fareDisplayTrx.ticketingDate()));

  // Rule Tariff Code
  std::string ruleTariffCode =
      FareDisplayResponseUtil::getRuleTariffCode(fareDisplayTrx, paxTypeFare);
  if (!ruleTariffCode.empty())
    construct.addAttribute("S91", ruleTariffCode);

  // Pricing Cat
  construct.addAttributeChar("S92", paxTypeFare.fcaPricingCatType());

  // Display Cat
  construct.addAttributeChar("S93", paxTypeFare.fcaDisplayCatType());

  // OW/RT indicator
  construct.addAttributeChar("P04", paxTypeFare.owrt());

  // Fare Description
  std::string fareDescription =
      FareDisplayResponseUtil::getFareDescription(fareDisplayTrx, paxTypeFare);
  if (!fareDescription.empty())
    construct.addAttribute("S94", fareDescription);

  // Effective date
  DateTime effDate;
  FareDisplayResponseUtil::getEffDate(fareDisplayTrx, paxTypeFare, *fareDisplayInfo, effDate);

  if (effDate.isValid())
    construct.addAttribute("D05", effDate.dateToString(YYYYMMDD, "-").c_str());

  // Expiration date/time
  if (paxTypeFare.expirationDate().isValid())
  {
    construct.addAttribute("D06", paxTypeFare.expirationDate().dateToString(YYYYMMDD, "-"));
    construct.addAttribute("D56", paxTypeFare.expirationDate().timeToString(HHMM, ""));
  }

  // Discontinue date
  DateTime discDate;
  FareDisplayResponseUtil::getDiscDate(fareDisplayTrx, paxTypeFare, *fareDisplayInfo, discDate);

  if (discDate.isValid())
    construct.addAttribute("D08", discDate.dateToString(YYYYMMDD, "-").c_str());

  // Footnote1
  if (!paxTypeFare.footNote1().empty())
    construct.addAttribute("S56", paxTypeFare.footNote1());

  // Footnote2
  if (!paxTypeFare.footNote2().empty())
    construct.addAttribute("S57", paxTypeFare.footNote2());

  // Passenger type code
  addPTCType(fareDisplayInfo, paxTypeFare, construct);
}

void
FareDisplayResponseFormatter::addResponseLine(std::string& tmpResponse,
                                              XMLConstruct& construct,
                                              const Indicator& responseType,
                                              const std::string& msgType)
{
  const int AVAILABLE_SIZE = 63;
  const char* separator = "\n~";
  if (responseType == ERROR_TYPE)
    separator = "\n";

  char* pHolder = nullptr;
  int tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), separator, &pHolder); token != nullptr;
       token = strtok_r(nullptr, separator, &pHolder), _recNum++)
  {
    tokenLen = strlen(token);

    if (tokenLen == 0)
      continue;
    else if (tokenLen > AVAILABLE_SIZE)
    {
      LOG4CXX_WARN(logger, "Line: [" << token << "] too long!");
      continue;
    }
    else
    {
      if (responseType == ERROR_TYPE)
        construct.addAttribute("S18", token);
      else if (responseType == MSG_TYPE)
        addMessageLine(token, construct, msgType);
      else if (responseType == TEXT_TYPE)
      {
        construct.openElement("TXT");
        construct.addAttribute("LIN", token);
        construct.closeElement();
      }
    }
  }
}

void
FareDisplayResponseFormatter::addAttributesForBrandInFQD(const FareDisplayInfo* fareDisplayInfo,
                                                         XMLConstruct& construct)
{
  if (!fareDisplayInfo)
  {
    LOG4CXX_ERROR(logger, "No FareDisplayInfo for paxTypeFare");
    return;
  }

  QualifiedBrand programBrandPair = fareDisplayInfo->getBrandProgramPair();
  BrandProgram* brandProgram = programBrandPair.first;
  BrandInfo* brandInfo = programBrandPair.second;
  if (brandProgram && brandInfo)
  {
    // Assumption here is that all the clients are green screen based, so we need to convert brand
    // program data to uppercase
    std::string tmp = brandInfo->brandCode();
    boost::to_upper(tmp);
    construct.addAttribute(xmlAttr::BrandCode, tmp);
    tmp = brandInfo->brandName();
    boost::to_upper(tmp);
    construct.addAttribute(xmlAttr::BrandName, tmp);
    tmp = brandProgram->programCode();
    boost::to_upper(tmp);
    construct.addAttribute(xmlAttr::ProgramCode, tmp);
    tmp = brandProgram->programName();
    boost::to_upper(tmp);
    construct.addAttribute(xmlAttr::ProgramName, tmp);
  }
}

void
FareDisplayResponseFormatter::addAttribute(const FareDisplayTrx& fareDisplayTrx,
                                           int index,
                                           XMLConstruct& construct,
                                           bool addJourneyInd)
{
  if (fareDisplayTrx.getFareDataMapVec().empty())
    return;

  std::map<FieldColumnElement, std::string>* fareDataMap = nullptr;
  try
  {
    fareDataMap = fareDisplayTrx.getFareDataMapVec().at(index);
  }
  catch (std::out_of_range& outOfRange)
  {
    LOG4CXX_ERROR(logger, "FareDisplayResponseFormatter::addAttribute index out of range");
    return;
  }

  if (!fareDataMap)
  {
    LOG4CXX_ERROR(logger, "FareDisplayResponseFormatter::addAttribute fareDataMap is NULL");
    return;
  }

  if (fareDataMap->empty())
    return;

  std::string value;
  getFareDataMapValue(fareDataMap, SAME_DAY_IND, value);
  construct.addAttribute("PAL", value);

  if (addJourneyInd)
  {
    getFareDataMapValue(fareDataMap, JOURNEY_IND, value);
    construct.addAttribute("P04", value);
  }

  getFareDataMapValue(fareDataMap, NET_FARE_IND, value);
  construct.addAttribute("N1I", value);

  getFareDataMapValue(fareDataMap, ADVANCE_PURCHASE, value);
  construct.addAttribute("S50", value);

  getFareDataMapValue(fareDataMap, MIN_MAX_STAY, value, false);

  using tokenizer = boost::tokenizer<boost::char_separator<char>>;
  boost::char_separator<char> separator("/");
  tokenizer tokens(value, separator);

  uint16_t count = 1;

  for (std::string tmp : tokens)
  {
    if (count == 1)
    {
      formatValue(tmp);
      construct.addAttribute("S51", tmp);
    }
    else if (count == 2)
    {
      formatValue(tmp);
      construct.addAttribute("S52", tmp);
      break;
    }

    count++;
  }
}

void
FareDisplayResponseFormatter::getFareDataMapValue(
    const std::map<FieldColumnElement, std::string>* fareDataMap,
    FieldColumnElement key,
    std::string& value,
    bool format)
{
  value = EMPTY_STRING();
  if (fareDataMap->empty())
    return;

  std::map<FieldColumnElement, std::string>::const_iterator i = fareDataMap->find(key);

  if (i == fareDataMap->end())
    return;

  value = i->second;
  if (format)
    formatValue(value);
}

void
FareDisplayResponseFormatter::formatValue(std::string& value)
{
  if (value.empty())
    return;
  boost::trim(value);
  if (value == "-")
    value = EMPTY_STRING();
}
}
