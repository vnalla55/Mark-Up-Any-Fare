/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#include "Xform/StructuredRulesResponseFormatter.h"

#include "Xform/FareComponentDataFormatter.h"
#include "Xform/FarePathSFRDataFormatter.h"
#include "Xform/PricingUnitSFRDataFormatter.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareCalcConfig.h"

namespace tse
{
FIXEDFALLBACK_DECL(checkArunkForSfr);
namespace
{
void
formatPUDTag(PricingTrx& pricingTrx, const FarePath& farePath, XMLConstruct& construct)
{
  uint16_t pricingUnitNumber = 1;
  PricingUnitSFRDataFormatter puFormatter(construct);
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    if (pu->hasMostRestrictivePricingUnitSFRData())
      puFormatter.format(pricingUnitNumber, pu->getMostRestrictivePricingUnitSFRData());
    else
      puFormatter.addEmptyPUDTag(pricingUnitNumber);
    ++pricingUnitNumber;
  }
}
Logger
logger("atseintl.Xform.StructuredRulesResponseFormatter");
}

std::string
StructuredRulesResponseFormatter::formatResponse(
    const std::string& responseString,
    bool displayOnly,
    PricingTrx& pricingTrx,
    FareCalcCollector* fareCalcCollector,
    ErrorResponseException::ErrorResponseCode tktErrCode)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);

  _ticketingDate = pricingTrx.ticketingDate();
  _itin = nullptr;

  XMLConstruct construct;

  construct.openElement("StructuredRuleResponse");

  formatProperResponseWhetherMTActive(
      tktErrCode, responseString, displayOnly, pricingTrx, construct, fareCalcCollector);

  appendCDataToResponse(pricingTrx, construct);
  construct.closeElement();
  return logXmlData(construct);
}

void
StructuredRulesResponseFormatter::formatFareComponentLevelData(
    const std::vector<FareCompInfo*>& fcContainer,
    const FarePath* farePath,
    FareComponentDataFormatter& formater)
{
  for (FareCompInfo* fareCompInfo : fcContainer)
  {
    auto structuredFareInfo = farePath->findFUWithPUNumberWithFirstTravelSeg(
        fareCompInfo->fareMarket()->travelSeg().front());
    const FareUsage* fareUsage = structuredFareInfo.first;
    const uint16_t pricingUnitNumber = structuredFareInfo.second;
    TSE_ASSERT(fareUsage);
    if (fareUsage->hasStructuredRuleData())
      formater.format(
          fareUsage->getStructuredRuleData(), fareCompInfo->fareCompNumber(), pricingUnitNumber);
    else
      formater.addEmptyFCDTag(fareCompInfo->fareCompNumber(), pricingUnitNumber);
  }
}

void
StructuredRulesResponseFormatter::formatResponse(PricingTrx& pricingTrx,
                                                 CalcTotals& calcTotals,
                                                 XMLConstruct& construct)
{
  const FarePath* farePath = calcTotals.farePath;

  FareComponentDataFormatter formater(construct);
  if (pricingTrx.isMultiPassengerSFRRequestType())
  {
    TSE_ASSERT(pricingTrx.getMultiPassengerFCMapping());
    auto itFindResult = pricingTrx.getMultiPassengerFCMapping()->find(farePath->paxType());
    if (itFindResult != pricingTrx.getMultiPassengerFCMapping()->end())
    {
      formatFareComponentLevelData(itFindResult->second, farePath, formater);
    }
    else
    {
      return;
    }
  }
  else
  {
    formatFareComponentLevelData(farePath->itin()->fareComponent(), farePath, formater);
  }
  formatPUDTag(pricingTrx, *farePath, construct);
  if (farePath->hasMostRestrictiveJourneySFRData() &&
      farePath->getMostRestrictiveJourneySFRData().isAnyMRJData())
  {
    FarePathSFRDataFormatter journeyFormatter(construct);
    journeyFormatter.format(farePath->getMostRestrictiveJourneySFRData());
  }
}

void
StructuredRulesResponseFormatter::formatResponse(const ErrorResponseException& ere,
                                                 std::string& response)
{
  XMLConstruct construct;
  construct.openElement("StructuredRuleResponse");
  if (fallback::fixed::checkArunkForSfr())
  {
    ResponseFormatter::addMessageLine(ere.message(), construct, "E", Message::errCode(ere.code()));
  }
  else
  {
    ResponseFormatter::addMessageLine(ere.message(), construct, "E", static_cast<int>(ere.code()));
  }
  construct.closeElement();
  response = construct.getXMLData();
}
}
