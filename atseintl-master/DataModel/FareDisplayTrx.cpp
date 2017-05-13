//-------------------------------------------------------------------
//
//  File:        FareDisplayTrx.cpp
//
//  Description: Fare Display Transaction object
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "DataModel/FareDisplayTrx.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingDetailTrx.h"
#include "DBAccess/FareDispTemplateSeg.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/TemplateEnums.h"
#include "Xform/FareDisplayResponseFormatter.h"
#include "Xform/FareDisplayResponseXMLTags.h"
#include "Xform/XMLConvertUtils.h"

#include <algorithm>
#include <sstream>
#include <vector>

using namespace std;

namespace tse
{
static Logger
logger("atseintl.DataModel.FareDisplayTrx");

FareDisplayTrx::FareDisplayTrx()
{
  dataHandle().setIsFareDisplay(true);
  dataHandle().get(_fdResponse);
  setTrxType(FAREDISPLAY_TRX);
}

//-----------------------------------------------------------------
// FareDisplayTrx::initializeTemplate()
//  read Template Segments from DB and and find season column
//-----------------------------------------------------------------

void
FareDisplayTrx::initializeTemplate()
{
  _seasonTemplate = false;
  _twoColumnTemplate = false;

  bool isShopping = isShopperRequest();
  std::vector<FareDispTemplate*> templateRecs;
  std::vector<FareDispTemplateSeg*> templateSegRecs;
  int32_t templateNumber = 0;
  const FareDisplayPref* prefs = getOptions()->fareDisplayPref();

  TemplateType templateType = getOptions()->templateType() == 'S' ? SINGLE_CARRIER : MULTI_CARRIER;

  if (templateType == MULTI_CARRIER)
  {
    isShopping = true;
  }
  else
  {
    if (isShopping)
      templateType = MULTI_CARRIER;
  }

  bool isFL = getOptions()->displayBaseTaxTotalAmounts() == TRUE_INDICATOR;
  int32_t templateOverride = getOptions()->templateOverride();
  if (templateOverride != 0 && getRequest()->inclusionCode() != ADDON_FARES && !isFL)
  {
    // Pass on diagnostic requests
    if (templateOverride < MIN_DIAG_NBR)
      templateNumber = templateOverride;
  }
  else
  {
    if (getRequest()->inclusionCode() == ADDON_FARES)
    {
      templateNumber = atoi(prefs->addOnTemplateId().c_str());
      templateType = ADDON;
    }
    else if (isFL)
    { // FL Display - Template 19
      templateNumber = atoi(prefs->taxTemplateId().c_str());
      templateType = TAX;
    }
    else if (isShopping)
    {
      templateNumber = atoi(prefs->multiCxrTemplateId().c_str());
      templateType = MULTI_CARRIER;
    }
    else
      templateNumber = atoi(prefs->singleCxrTemplateId().c_str());
  }

  if (templateNumber < MIN_DIAG_NBR)
  {
    templateRecs = getFareDispTemplate(templateNumber, templateType);
    if (templateRecs.empty())
    {

      if (getRequest()->inclusionCode() == ADDON_FARES)
        templateNumber = DEFAULT_ADDON_TEMPLATE;
      else
        templateNumber = DEFAULT_TEMPLATE;

      templateRecs = getFareDispTemplate(templateNumber, templateType);
    }
    if (templateRecs.empty())
    {
      return;
    }
    templateSegRecs = getFareDispTemplateSeg(templateNumber, templateType);
  }

  if (!templateSegRecs.empty())
  {
    std::vector<FareDispTemplateSeg*>::const_iterator iter = templateSegRecs.begin();
    std::vector<FareDispTemplateSeg*>::const_iterator iterEnd = templateSegRecs.end();
    int16_t columnElement;
    for (; iter != iterEnd; iter++)
    {
      columnElement = (*iter)->columnElement();
      if (columnElement == SEASONS)
      {
        _seasonTemplate = true;
      }
      else if (columnElement == OW_FARE_AMOUNT || columnElement == RT_FARE_AMOUNT)
      {
        _twoColumnTemplate = true;
      }
    }
  }
  return;
}

bool
FareDisplayTrx::isSameCityPairRqst() const
{
  if (this->travelSeg().empty())
  {
    return false;
  }
  else
  {
    /**Pre Condition : board multi city and off multi city is populated in ItinAnalyzer*/
    return (this->boardMultiCity() == this->offMultiCity());
  }
}

bool
FareDisplayTrx::isDomestic() const
{
  const GeoTravelType itinTravelType = itin().front()->geoTravelType();
  return ((itinTravelType == GeoTravelType::Domestic) || (itinTravelType == GeoTravelType::Transborder));
}

bool
FareDisplayTrx::isForeignDomestic() const
{
  const GeoTravelType itinTravelType = itin().front()->geoTravelType();
  return (itinTravelType == GeoTravelType::ForeignDomestic);
}

bool
FareDisplayTrx::isInternational() const
{
  const GeoTravelType itinTravelType = itin().front()->geoTravelType();
  return ((itinTravelType == GeoTravelType::International) || (itinTravelType == GeoTravelType::ForeignDomestic));
}

bool
FareDisplayTrx::isFQ()
{
  return (this->getRequest()->requestType() == "FQ");
}

bool
FareDisplayTrx::isRD()
{
  return (this->getRequest()->requestType() == FARE_RULES_REQUEST ||
          this->getRequest()->requestType() == ENHANCED_RD_REQUEST);
}

bool
FareDisplayTrx::isERD() const
{
  return this->getRequest()->requestType() == ENHANCED_RD_REQUEST;
}

bool
FareDisplayTrx::isERDFromSWS() const
{
  return (isERD() && this->billing()->requestPath() == SWS_PO_ATSE_PATH);
}

bool
FareDisplayTrx::isFT()
{
  if (this->getRequest()->requestType() == "FT")
    return true;
  return false;
}

bool
FareDisplayTrx::isShortRD()
{
  if (this->getRequest()->requestType() == ENHANCED_RD_REQUEST)
  {
    return true;
  }
  if (this->getRequest()->requestType() == FARE_RULES_REQUEST)
  {
    return this->getOptions()->lineNumber() != 0;
  }
  return false;
}

bool
FareDisplayTrx::isLongRD()
{
  if (isRD())
  {
    return !isShortRD();
  }
  return false;
}

bool
FareDisplayTrx::isShortRequest()
{
  if (this->getRequest()->requestType() == ENHANCED_RD_REQUEST)
  {
    return true;
  }
  if (this->getRequest()->requestType() == FARE_RULES_REQUEST ||
      this->getRequest()->requestType() == "RB" || this->getRequest()->requestType() == "FT")
  {
    return this->getOptions()->lineNumber() != 0;
  }
  return false;
}

bool
FareDisplayTrx::isDiagnosticRequest()
{
  return this->diagnostic().isActive();
}

bool
FareDisplayTrx::isFDDiagnosticRequest()
{
  // diagnostic 207, 209, 212 are handled in ItinAnalyzer, and are in
  // common diagnostic directory (Diag207CollectorFD, Diag212CollectorFD
  int16_t diagNum = this->getRequest()->diagnosticNumber();
  return (diagNum == 195 || diagNum == 200 || diagNum == 201 || diagNum == 202 || diagNum == 203 ||
          diagNum == 205 || diagNum == 206 || diagNum == 211 || diagNum == 213 || diagNum == 214 ||
          diagNum == 215 || diagNum == 216 || diagNum == 217 || diagNum == 218 || diagNum == 220 ||
          diagNum == 854 || diagNum == 889);
}

bool
FareDisplayTrx::isTravelAgent()
{
  return !getRequest()->ticketingAgent()->tvlAgencyPCC().empty();
}

bool
FareDisplayTrx::isShopperRequest() const
{
  return (multipleCarriersEntered() || getOptions()->isAllCarriers());
}

//-----------------------------------------------------------------------------
// Return true if there are valid fares in multiple currencies
//-----------------------------------------------------------------------------
bool
FareDisplayTrx::multipleCurrencies()
{

  CurrencyCode firstValidCurrency = "ZZZ";

  Itin* itn = itin().front();
  std::vector<FareMarket*>::const_iterator fmItr = itn->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itn->fareMarket().end();

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket& fareMarket = **fmItr;

    std::vector<PaxTypeFare*>::const_iterator ptfItr = fareMarket.allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator ptfEnd = fareMarket.allPaxTypeFare().end();

    for (; ptfItr != ptfEnd; ptfItr++)
    {
      PaxTypeFare* paxTypeFare = (*ptfItr);

      if (paxTypeFare == nullptr || !(paxTypeFare->isValidForPricing()))
      {
        continue;
      }

      if (firstValidCurrency == "ZZZ")
      {
        firstValidCurrency = paxTypeFare->currency();
      }

      if (paxTypeFare->currency() != firstValidCurrency)
      {
        return true;
      }
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
// Return true if there are valid fares in multiple currencies
//-----------------------------------------------------------------------------
void
FareDisplayTrx::getAlternateCurrencies(std::set<CurrencyCode>& alternateCurrencies)
{
  Itin* itn = itin().front();
  std::vector<FareMarket*>::const_iterator fmItr = itn->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itn->fareMarket().end();

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket& fareMarket = **fmItr;

    std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
    std::vector<PaxTypeBucket>::iterator paxTypeCortI = paxTypeCortegeVec.begin();
    std::vector<PaxTypeBucket>::iterator paxTypeCortEnd = paxTypeCortegeVec.end();

    for (; paxTypeCortI < paxTypeCortEnd; paxTypeCortI++)
    {
      PaxTypeBucket& paxTypeCortege = (*paxTypeCortI);
      PaxTypeFarePtrVec& paxTypeFareVec = paxTypeCortege.paxTypeFare();
      std::vector<tse::PaxTypeFare*>::const_iterator paxTypeFareI = paxTypeFareVec.begin();
      std::vector<tse::PaxTypeFare*>::const_iterator paxTypeFareEnd = paxTypeFareVec.end();

      for (; paxTypeFareI < paxTypeFareEnd; paxTypeFareI++)
      {
        PaxTypeFare& paxTypeFare = **paxTypeFareI;

        if (paxTypeFare.carrier() == INDUSTRY_CARRIER)
        {
          continue;
        }

        if (paxTypeFare.actualPaxType()->paxType() == PFA ||
            paxTypeFare.actualPaxType()->paxType() == JCB)
        {
          continue;
        }

        if (paxTypeFare.notValidForDirection())
        {
          continue;
        }

        if (paxTypeFare.notValidForOutboundCurrency())
        {
          alternateCurrencies.insert(paxTypeFare.currency());
        }
      }
    }
  }
  return;
}

// -------------------------------------------------------------------
//
// @MethodName  FareDisplayUtil::errorResponse()
//
// Returns Error Response when there is no fares on the market
//
// @param   trx        - the transaction to use
//
// @return
//
// -------------------------------------------------------------------------
void
FareDisplayTrx::errResponse()
{
  response().clear();
  response() << std::endl;

  response() << "*** THERE ARE NO ";

  if (getRequest()->inclusionCode() == ALL_INCLUSION_CODE ||
      FareDisplayUtil::isQualifiedRequest(*this) || FareDisplayUtil::isAxessUser(*this))
  {
    response() << "REQUESTED ";
  }
  else
  {
    response() << getRequest()->inclusionCode() << " ";
  }

  if ((getOptions() != nullptr) && (getOptions()->isPrivateFares()))
  {
    response() << "PRIVATE ";
  }

  response() << "FARES PUBLISHED ";

  if ((!fareMarket().empty()) || (!travelSeg().empty()))
  {
    response() << boardMultiCity() << "-" << offMultiCity();
  }

  response() << " ***" << std::endl;
}

const InclusionCode
FareDisplayTrx::getRequestedInclusionCode() const
{
  return getRequest()->requestedInclusionCode();
}

bool
FareDisplayTrx::hasScheduleCountInfo() const
{
  return (this->fdResponse()->scheduleCounts().empty() == false);
}

// -------------------------------------------------------------------
//
// @MethodName  FareDisplayUtil::isRecordScopeDomestic()
//
// determines record scope type
//
// @param   trx        - the transaction to use
//
// @return  Boolean value. true if record scope is Domestinc.
//          false otherwise.
//
// -------------------------------------------------------------------------

bool
FareDisplayTrx::isRecordScopeDomestic()
{
  GeoTravelType recordScope;

  if (!fareMarket().empty())
    recordScope = fareMarket().front()->geoTravelType();
  else
    recordScope = itin().front()->geoTravelType();

  return !(recordScope == GeoTravelType::International || recordScope == GeoTravelType::ForeignDomestic);
}

bool
FareDisplayTrx::isSDSOutputType()
{
  if (this->getRequest()->outputType() == FD_OUTPUT_SDS)
    return true;
  return false;
}

bool
FareDisplayTrx::isShortRequestForPublished()
{
  if (isShortRequest() && (this->getOptions()->isPublishedFare()))
    return true;
  return false;
}

bool
FareDisplayTrx::needFbrFareCtrl()
{
  return !isShortRequest() || getOptions()->cat25Values().isNonPublishedFare();
}

bool
FareDisplayTrx::needDiscFareCtrl()
{
  return !isShortRequest() || getOptions()->discountedValues().isNonPublishedFare();
}

// even if pub fare, may still need to process cat35 rules for markup data
bool
FareDisplayTrx::needNegFareCtrl()
{
  return !isShortRequest() || getOptions()->FDcat35Type() != '\0';
}

//--------------------------------------------------------------------------
// FareDisplayTrx::isScheduleCountRequested()
//--------------------------------------------------------------------------

bool
FareDisplayTrx::isScheduleCountRequested() const
{
  if (this->isSameCityPairRqst())
    return false;

  if (this->getOptions() != nullptr)
  {
    bool isShopper(this->isShopperRequest());
    if (this->getOptions()->fareDisplayPref() == nullptr)
      return isShopper;
    else if (isShopper)
    {
      return (this->getOptions()->fareDisplayPref()->multiCarrierSvcSched() == DISPLAY_SCHEDULES);
    }
    else
      return (this->getOptions()->fareDisplayPref()->singleCarrierSvcSched() == DISPLAY_SCHEDULES);
  }
  return false;
}

void
FareDisplayTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  if (!ere.message().empty())
  {
    response = ere.message();
  }
  else
  {
    response = _response.str();
  }

  if (response.empty())
    response = "UNFORMATTED EXCEPTION";

  std::string msgType = "E"; // Error message

  std::string xmlResponse;
  XMLConvertUtils::formatResponse(response, xmlResponse, msgType);
  response = "<FareDisplayResponse>" + xmlResponse + "</FareDisplayResponse>";
}

bool
FareDisplayTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }
  LOG4CXX_DEBUG(logger, "Doing FareDisplayTrx response");
  FareDisplayResponseXMLTags fdXML;
  FareDisplayResponseFormatter formatter(fdXML);
  response = formatter.formatResponse(*this);
  LOG4CXX_DEBUG(logger, "response: " << response);

  return true;
}
} // tse namespace
