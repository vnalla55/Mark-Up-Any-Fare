//----------------------------------------------------------------------------
//
//  File:  AltPricingResponseFormatter.cpp
//  Description: See AltPricingResponseFormatter.h file
//  Created:  February 17, 2005
//  Authors:  Mike Carroll
//
//  Copyright Sabre 2005
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

#include "Xform/AltPricingResponseFormatter.h"

#include "Common/BaggageStringFormatter.h"
#include "Common/BaggageTripType.h"
#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/OBFeesUtils.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FarePath.h"
#include "DataModel/TNBrandsTypes.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalculation.h"
#include "Util/Base64.h"
#include "Util/CompressUtil.h"
#include "Xform/BrandingResponseBuilder.h"
#include "Xform/BrandsOptionsFilterForDisplay.h"
#include "Xform/CommonFormattingUtils.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/XformUtil.h"

#include "Common/TNBrands/ItinBranding.h"

#include <boost/algorithm/string/join.hpp>

#include <map>

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackOCFeesInSearchForBrandsPricing);
FALLBACK_DECL(fallbackPriceByCabinActivation)

static Logger
logger("atseintl.Xform.AltPricingResponseFormatter");

static Logger
uncompressedLogger("atseintl.Xform.AltPricingResponseFormatterUncompressed");

using QualifiedBrandSizeType = std::vector<QualifiedBrand>::size_type;

void
AltPricingResponseFormatter::formatResponse(const ErrorResponseException& ere,
                                            std::string& response)
{
  XMLConstruct construct;
  construct.openElement("AltPricingResponse");
  ResponseFormatter::addMessageLine(ere.message(), construct, "E", Message::errCode(ere.code()));
  construct.closeElement();

  response = construct.getXMLData();
}

namespace
{
ConfigurableValue<uint16_t>
maxNumberOfFees("SERVICE_FEES_SVC", "MAX_NUMBER_OF_FEES", 0);
}

std::string
AltPricingResponseFormatter::formatResponse(const std::string& responseString,
                                            bool displayOnly,
                                            PricingTrx& pricingTrx,
                                            FareCalcCollector* fareCalcCollector,
                                            ErrorResponseException::ErrorResponseCode tktErrCode)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);

  XMLConstruct construct;

  std::string responseStringWithoutOB = responseString;
  if (!OBFeesUtils::fallbackObFeesWPA(&pricingTrx))
  {
    responseStringWithoutOB = removeObFromResponse(responseString);
  }
  // check for diagnostic or normal response
  if (fareCalcCollector)
  {
    // normal response has a PricingResponse compressed and embedded in an AltPricingResponse
    construct.openElement("PricingResponse");
    prepareAgent(pricingTrx, construct);
    prepareBilling(pricingTrx, *fareCalcCollector, construct);
    if(pricingTrx.activationFlags().isSearchForBrandsPricing())
    {
      if (fallback::fallbackOCFeesInSearchForBrandsPricing(&pricingTrx))
      {
        preparePassengersWithBrands(pricingTrx, *fareCalcCollector, construct);
        formatOCFeesResponse(construct, pricingTrx);
      }
      else
      {
        ServiceFeeUtil::OcFeesUsagesMerger ocFeesMerger(pricingTrx.paxType(), maxNumberOfFees.getValue());
        XMLConstruct ocFeesXmlData;
        formatOCFeesResponse(ocFeesXmlData, pricingTrx, &ocFeesMerger);
        preparePassengersWithBrands(pricingTrx, *fareCalcCollector, construct, &ocFeesMerger);
        construct.addElementData(ocFeesXmlData);
      }
    }
    else
    {
      preparePassengers(pricingTrx, *fareCalcCollector, construct);
    }
    if (!OBFeesUtils::fallbackObFeesWPA(&pricingTrx))
    {
      prepareOBFeesInfo(pricingTrx, construct);
    }

  }
  else
  {
    // diagnostics are an AltPricingResponse with MSG elements
    construct.openElement("AltPricingResponse");
  }

  if (pricingTrx.diagnostic().diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(pricingTrx, construct);
  }

  Diagnostic& diag = pricingTrx.diagnostic();
  bool unlimitedResponse = diag.diagnosticType() != DiagnosticNone &&
                           diag.diagParamIsSet(Diagnostic::NO_LIMIT, Diagnostic::UNLIMITED_RESPONSE);

  // create MSG elements the same for diagnostics or normal responses
  if (tktErrCode > 0)
  {
    bool pbcDiag = (!fallback::fallbackPriceByCabinActivation(&pricingTrx) &&
                    !pricingTrx.getOptions()->cabin().isUndefinedClass() &&
                    diag.diagnosticType() == Diagnostic185 && !diag.toString().empty());
    if(pbcDiag)
      prepareResponseText(diag.toString(), construct, unlimitedResponse);

    // Pricing failed, return error message
    prepareErrorMessage(pricingTrx, construct, tktErrCode, responseString);

    if (!pbcDiag && diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct, unlimitedResponse);
    }
  }
  else
  {
    // Attaching MSG elements
    if (fareCalcCollector)
      formatGreenScreenMsg(construct, responseStringWithoutOB, pricingTrx, fareCalcCollector);
    else
      prepareResponseText(responseStringWithoutOB, construct, unlimitedResponse);
  }

  if (pricingTrx.activationFlags().isSearchForBrandsPricing())
  {
    if (checkForStructuredData(pricingTrx,
                               TrxUtil::isBaggageActivationXmlResponse(pricingTrx),
                               TrxUtil::isBaggage302ExchangeActivated(pricingTrx),
                               TrxUtil::isBaggage302GlobalDisclosureActivated(pricingTrx)))
      formatBaggageResponse(pricingTrx, construct);
  }

  appendCDataToResponse(pricingTrx, construct);
  construct.closeElement();

  LOG4CXX_INFO(uncompressedLogger,
               "Uncompressed PricingResponse in XML " << construct.getXMLData().size()
                                                      << " bytes:\n" << construct.getXMLData());
  LOG4CXX_INFO(logger,
               "Uncompressed PricingResponse size " << construct.getXMLData().size() << " bytes");

  // do compression and embedding only for non-diagnostics
  if (fareCalcCollector)
  {
    std::string pricingResponse = construct.getXMLData();;
    if(!pricingTrx.activationFlags().isSearchForBrandsPricing())
    {
      CompressUtil::compressBz2(pricingResponse);
      pricingResponse = Base64::encode(pricingResponse);
    }

    XMLConstruct altConstruct;
    altConstruct.openElement("AltPricingResponse");
    altConstruct.openElement("DTS");

    altConstruct.addAttributeInteger("Q0S", numOfValidCalcTotals(pricingTrx, *fareCalcCollector));

    bool isLowFare = pricingTrx.getRequest()->isLowFareRequested();
    altConstruct.addAttributeBoolean("PBV", isLowFare);
    altConstruct.addElementData(pricingResponse.c_str());
    altConstruct.closeElement(); // DTS

    std::string responseStringWithoutObTags = responseString;
    if (!OBFeesUtils::fallbackObFeesWPA(&pricingTrx))
    {
      responseStringWithoutObTags = removeObTagsFromResponse(responseString);
    }

    std::string responseCopy = responseStringWithoutObTags;
    prepareUncompressedResponseText(pricingTrx, responseCopy);
    prepareResponseText(responseCopy, altConstruct);

    getPsgOptions(pricingTrx, fareCalcCollector, altConstruct);
    prepareLatencyDataInResponse(pricingTrx, altConstruct);

    altConstruct.closeElement(); // AltPricingResponse

    LOG4CXX_INFO(logger,
                 "Compressed PricingResponse in XML " << altConstruct.getXMLData().size()
                                                      << " bytes:\n" << altConstruct.getXMLData());

    return altConstruct.getXMLData();
  }
  else
  {
    return construct.getXMLData();
  }
}

void
AltPricingResponseFormatter::scanTotalsItin(CalcTotals& calcTotals,
                                            bool& fpFound,
                                            bool& infantMessage,
                                            char& nonRefundable,
                                            MoneyAmount& moneyAmountAbsorbtion)
{
  fpFound = true;
  scanFarePath(*calcTotals.farePath, infantMessage, nonRefundable, moneyAmountAbsorbtion);
}

void
AltPricingResponseFormatter::getPsgOptions(PricingTrx& trx,
                                           FareCalcCollector* fareCalcCollector,
                                           XMLConstruct& altConstruct)
{
  std::map<PaxTypeCode, std::map<unsigned int, bool>> optionsMap;
  std::map<const PaxType*, PaxTypeCode> requestedPaxMap;

  std::vector<CalcTotals*>::const_iterator vit = fareCalcCollector->passengerCalcTotals().begin();
  for (; vit != fareCalcCollector->passengerCalcTotals().end(); vit++)
  {
    requestedPaxMap[(*vit)->farePath->paxType()] = (*vit)->requestedPaxType;

    bool noMatchOption = (*vit)->farePath->noMatchOption();
    if (trx.altTrxType() == AltPricingTrx::WP_NOMATCH)
      noMatchOption = true;

    if ((*vit)->wpaInfo.psgDetailRefNo && !(*vit)->wpaInfo.wpnDetailResponse.empty())
      optionsMap[(*vit)->requestedPaxType].insert(
          std::make_pair((*vit)->wpaInfo.psgDetailRefNo, noMatchOption));
  }

  altConstruct.openElement(xml2::WPAOptionInformation); // PXX

  std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(trx.paxType().begin(),
                                                         trx.paxType().end());
  std::set<PaxType*, PaxType::InputOrder>::const_iterator sit = inOrderPaxType.begin();
  for (; sit != inOrderPaxType.end(); sit++)
  {
    PaxTypeCode requestedPaxType = requestedPaxMap[*sit];

    if (requestedPaxType.length())
    {
      altConstruct.openElement(xml2::PassengerInfo); // PXI
      altConstruct.addAttribute(xml2::PassengerType, requestedPaxType);

      const std::map<PaxTypeCode, std::map<unsigned int, bool>>::const_iterator wpaOptionsIter =
          optionsMap.find(requestedPaxType);
      if (wpaOptionsIter == optionsMap.end())
      {
        // there is no fare for the pax type
        altConstruct.addAttributeUInteger(xml2::WPALastOption, 0);
      }
      else
      {
        const std::map<unsigned int, bool>& wpaOptions = wpaOptionsIter->second;
        altConstruct.addAttributeUInteger(xml2::WPALastOption, wpaOptions.rbegin()->first);

        std::vector<unsigned int> noMatch;
        std::map<unsigned int, bool>::const_iterator it = wpaOptions.begin();
        for (; it != wpaOptions.end(); it++)
          if (it->second)
            noMatch.push_back(it->first);

        size_t noMatchSize = noMatch.size();

        if (noMatchSize == wpaOptions.size())
          altConstruct.addAttribute("S81", "T");
        else if (noMatchSize)
        {
          std::stringstream s;
          s << noMatch[0];

          bool consecutive = false;
          for (unsigned int i = 1; i < noMatchSize; i++)
          {
            if ((noMatch[i] - noMatch[i - 1] == 1) &&
                ((i != noMatchSize - 1) && (noMatch[i + 1] - noMatch[i] == 1)))
            {
              if (!consecutive)
                s << "-";
              consecutive = true;
            }
            else
            {
              if (!consecutive)
                s << "/";
              s << noMatch[i];
              consecutive = false;
            }
          }

          altConstruct.addAttribute(xml2::WPANoMatchOptions, s.str());
        }
      }

      altConstruct.closeElement(); // PXI (PassengerInfo)
    }
  }

  altConstruct.closeElement(); // PXX
}

size_t
AltPricingResponseFormatter::numValidCalcTotals(FareCalcCollector& fareCalcCollector)
{
  size_t count = 0;

  FareCalcCollector::CalcTotalsMap::const_iterator totalsIter =
      fareCalcCollector.calcTotalsMap().begin();
  FareCalcCollector::CalcTotalsMap::const_iterator totalsIterEnd =
      fareCalcCollector.calcTotalsMap().end();
  for (; totalsIter != totalsIterEnd; totalsIter++)
  {
    CalcTotals& calcTotals = *totalsIter->second;
    if (calcTotals.wpaInfo.psgDetailRefNo != 0 && !calcTotals.wpaInfo.wpnDetailResponse.empty())
      count++;
  }

  return count;
}

void
AltPricingResponseFormatter::preparePassengersWithBrands(PricingTrx& pricingTrx,
                                                         FareCalcCollector& fareCalcCollector,
                                                         XMLConstruct& construct,
                                                         ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger)
{
  TSE_ASSERT(pricingTrx.itin().size() == 1);
  Itin& itin = *pricingTrx.itin().front();

  BrandsOptionsFilterForDisplay brandOptionFilter(pricingTrx);
  BrandsOptionsFilterForDisplay::BrandingSpacesWithSoldoutVector filteredOptionsVector =
    brandOptionFilter.collectOptionsForDisplay(itin);

  const FareCalcConfig& fcConfig = *(FareCalcUtil::getFareCalcConfig(pricingTrx));
  int paxNumber = 0;
  for (const PaxType* paxType : pricingTrx.paxType())
  {
    for(const BrandsOptionsFilterForDisplay::BrandingOptionWithSoldout& option
        : filteredOptionsVector)
    {
      BrandsOptionsFilterForDisplay::BrandingOptionSpaceIndex spaceIndex = option.first;
      const BrandsOptionsFilterForDisplay::SoldoutStatus& soldout = option.second;

      if(soldout)
      {
        addSoldOutInformation(construct, pricingTrx, itin, spaceIndex, soldout, paxType);
      }
      else
      {
        std::pair<const FarePath*, CalcTotals*> paxTypeFareAndCalcTotals
          = BrandingResponseUtils::findFarePathAndCalcTotalsForPaxTypeAndBrand(
              itin, fareCalcCollector, paxType, spaceIndex, pricingTrx);

        CalcTotals* totals = paxTypeFareAndCalcTotals.second;
        if (totals != nullptr)
        {
          addCalcTotalsToResponse(pricingTrx, fareCalcCollector, *totals, itin, fcConfig,
                                  paxNumber, construct, ocFeesMerger);
        }
        else
        {
          LOG4CXX_ERROR(logger, "totals == 0");
          //TODO:
          //uncomment the code below when soldout logic will be ready
          //throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED,
        }
      }
      ++paxNumber;
    }
  }
}

void
AltPricingResponseFormatter::addSoldOutInformation(XMLConstruct& construct,
                                                  PricingTrx& pricingTrx,
                                                  Itin& itin,
                                                  BrandsOptionsFilterForDisplay::BrandingOptionSpaceIndex spaceIndex,
                                                  const BrandsOptionsFilterForDisplay::SoldoutStatus& soldout,
                                                  const PaxType* paxType) const
{
  construct.openElement(xml2::SummaryInfo);

  construct.addAttributeChar(xml2::BrandSoldout,
      ShoppingUtil::getIbfErrorCode(
          IbfAvailabilityTools::translateForOutput(
              itin.getItinBranding().getItinSoldoutStatusForSpace(spaceIndex))));

  construct.openElement(xml2::PassengerInfo);
  construct.addAttribute(xml2::PassengerType, paxType->paxType());

  for (const TravelSeg* seg : itin.travelSeg())
  {
    const AirSeg* airSeg = seg->toAirSeg();
    if (airSeg == nullptr)
      continue;

    construct.openElement(xml2::FareCalcInformation);

    construct.addAttribute(xml2::FareCalcDepartureCity, airSeg->boardMultiCity());
    construct.addAttribute(xml2::FareCalcDepartureAirport, airSeg->origAirport());
    construct.addAttribute(xml2::FareCalcArrivalCity, airSeg->offMultiCity());
    construct.addAttribute(xml2::FareCalcArrivalAirport, airSeg->destAirport());

    const CarrierCode& cxr = airSeg->carrier();
    const skipper::CarrierBrandPairs& cbpairs =
      itin.getItinBranding().getCarriersBrandsForSegment(spaceIndex, airSeg);

    //TODO(andrzej.fediuk)any idea how to calculate directionality with soldouts? :)
    Direction direction = Direction::BOTHWAYS;
    skipper::CarrierBrandPairs::const_iterator cbp =
        cbpairs.find(skipper::CarrierDirection(cxr, direction));

    if (cbp != cbpairs.end() && cbp->second != NO_BRAND)
    {
      const skipper::QualifiedBrandIndices& indices =
          itin.getItinBranding().getQualifiedBrandIndicesForCarriersBrand(
              cxr, cbp->second);
      TSE_ASSERT(!indices.empty());

      int brandProgramIndex = *indices.begin();
      TSE_ASSERT(static_cast<QualifiedBrandSizeType>(brandProgramIndex) <
                                                         pricingTrx.brandProgramVec().size());
      const QualifiedBrand& qb = pricingTrx.brandProgramVec()[brandProgramIndex];
      if (xform::formatBrandProgramData(construct, qb))
      {
        //TODO(karol.jurek): remove this attribute (SC0)
        construct.addAttribute(xml2::ProgramCode,
                               boost::to_upper_copy(std::string(qb.first->programCode())));
      }
      else
      {
        LOG4CXX_ERROR(logger, "Could not format brand program data");
      }
    }
    construct.openElement(xml2::SegmentInformation);
    construct.addAttribute(xml2::SegmentDepartureCity, airSeg->boardMultiCity());
    construct.addAttribute(xml2::SegmentDepartureAirport, airSeg->origAirport());
    construct.addAttribute(xml2::SegmentArrivalCity, airSeg->offMultiCity());
    construct.addAttribute(xml2::SegmentArrivalAirport, airSeg->destAirport());
    construct.addAttribute(xml2::MarketingCarrier, airSeg->marketingCarrierCode());
    construct.addAttribute(xml2::OperatingCarrier, airSeg->operatingCarrierCode());

    if (!airSeg->bookedCabin().isUndefinedClass())
    {
      if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(pricingTrx) &&
         TrxUtil::isRbdByCabinValueInPriceResponse(pricingTrx)         )
        construct.addAttributeChar(xml2::SegmentCabinCode, airSeg->bookedCabinCharAnswer());
      else
        construct.addAttributeChar(xml2::SegmentCabinCode, airSeg->bookedCabinChar());
    }

    if (!airSeg->equipmentType().empty())
      construct.addAttribute(xml2::SEGEquipmentCode, airSeg->equipmentType());

    construct.closeElement(); // SegmentInformation
    construct.closeElement(); // FareCalcInformation
  }
  construct.closeElement(); // PassengerInfo
  construct.closeElement(); // SummaryInfo
}

void
AltPricingResponseFormatter::addCalcTotalsToResponse(PricingTrx& pricingTrx,
                                                    FareCalcCollector& fareCalcCollector,
                                                    CalcTotals& calcTotals,
                                                    const Itin& itin,
                                                    const FareCalcConfig& fcConfig,
                                                    int paxNumber,
                                                    XMLConstruct& construct,
                                                    ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger)
{
  TSE_ASSERT(calcTotals.farePath);
  const FarePath& farePath = *calcTotals.farePath;
  construct.openElement(xml2::SummaryInfo);

  Money money(calcTotals.convertedBaseFareCurrencyCode);
  construct.addAttributeDouble(
      xml2::TotalPriceAll, calcTotals.convertedBaseFare, money.noDec(pricingTrx.ticketingDate()));
  construct.addAttribute(xml2::TotalCurrencyCode, calcTotals.convertedBaseFareCurrencyCode);
  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.addAttributeDouble(xml2::NetRemitBaseFareAmount,
                                 calcTotals.netRemitCalcTotals->convertedBaseFare,
                                 money.noDec(pricingTrx.ticketingDate()));
  }

  if (itin.isPlusUpPricing())
  {
    Money fareCalcCurrency(farePath.calculationCurrency());

    construct.addAttribute(xml2::ConsolidatorPlusUpCurrencyCode,
                           itin.consolidatorPlusUp()->currencyCode());
    construct.addAttributeDouble(xml2::ConsolidatorPlusUpAmount,
                                 itin.consolidatorPlusUp()->fareCalcAmount(),
                                 fareCalcCurrency.noDec());
  }

  prepareCommonSummaryAttrs(pricingTrx, fareCalcCollector, construct, &calcTotals);
  prepareNoBrandsInfo(pricingTrx, calcTotals, construct);

  if (pricingTrx.isValidatingCxrGsaApplicable())
  {
    if (!fallback::fallbackValidatingCxrMultiSp(&pricingTrx) || pricingTrx.overrideFallbackValidationCXRMultiSP())
    {
      if (farePath.defaultValCxrPerSp().empty())
        prepareValidatingCarrierLists(pricingTrx, construct, farePath);
      else
        buildValidatingCarrierList(pricingTrx, construct, farePath);
    }
    else
      buildValidatingCarrierList(pricingTrx, construct, farePath);
  }

  preparePassengerInfo(pricingTrx, fcConfig, calcTotals, paxNumber, construct);

  if (pricingTrx.activationFlags().isSearchForBrandsPricing() && ocFeesMerger &&
      !fallback::fallbackOCFeesInSearchForBrandsPricing(&pricingTrx))
  {
    addOCFeesReferences(ocFeesMerger->getGroupedOCFeeIdsForFarePath(calcTotals.farePath), construct);
  }

  construct.closeElement();
}

void
AltPricingResponseFormatter::preparePassengers(PricingTrx& pricingTrx,
                                               FareCalcCollector& fareCalcCollector,
                                               XMLConstruct& construct)
{
  const FareCalcConfig& fcConfig = *(FareCalcUtil::getFareCalcConfig(pricingTrx));
  const DateTime& ticketingDate = pricingTrx.ticketingDate();

  CalcTotals* calcTotalsTemp = nullptr;

  uint16_t numPassengers = 0;

  numPassengers = numOfValidCalcTotals(pricingTrx, fareCalcCollector);

  for (uint16_t paxNumber = 1; paxNumber <= numPassengers; ++paxNumber)
  {
    bool paxFound = false;
    bool useAdjustedCalcTotal = false;

    FareCalcCollector::CalcTotalsMap::const_iterator totalsIter =
        fareCalcCollector.calcTotalsMap().begin();
    FareCalcCollector::CalcTotalsMap::const_iterator totalsIterEnd =
        fareCalcCollector.calcTotalsMap().end();
    for (; totalsIter != totalsIterEnd; totalsIter++)
    {
      if (pricingTrx.getOptions() && !pricingTrx.getOptions()->isPDOForFRRule() &&
          totalsIter->second->adjustedCalcTotal != nullptr &&
          !totalsIter->second->adjustedCalcTotal->wpaInfo.wpnDetailResponse.empty() &&
          totalsIter->second->adjustedCalcTotal->wpaInfo.psgDetailRefNo == paxNumber)
      {
        calcTotalsTemp = totalsIter->second;
        calcTotalsTemp->wpaInfo.psgDetailRefNo =
           calcTotalsTemp->adjustedCalcTotal->wpaInfo.psgDetailRefNo;
        paxFound = true;
        useAdjustedCalcTotal = true;
        break;
      }
      else
      {
        if (totalsIter->second != nullptr && !totalsIter->second->wpaInfo.wpnDetailResponse.empty() &&
            totalsIter->second->wpaInfo.psgDetailRefNo == paxNumber)
        {
          calcTotalsTemp = totalsIter->second;
          paxFound = true;
          break;
        }
      }
    }
    if (!paxFound)
      continue;

    CalcTotals& calcTotals = *calcTotalsTemp;

    construct.openElement(xml2::SummaryInfo);

    construct.addAttributeBoolean(xml2::WpaWpNoMatch, calcTotals.farePath->noMatchOption());

    Money money(calcTotals.convertedBaseFareCurrencyCode);
    construct.addAttributeDouble(xml2::TotalPriceAll,
                                 (useAdjustedCalcTotal ?
                                    calcTotals.adjustedCalcTotal->convertedBaseFare :
                                    calcTotals.convertedBaseFare),
                                 money.noDec(ticketingDate));

    construct.addAttribute(xml2::TotalCurrencyCode, calcTotals.convertedBaseFareCurrencyCode);
    if (calcTotals.netRemitCalcTotals != nullptr)
    {
      construct.addAttributeDouble(xml2::NetRemitBaseFareAmount,
                                   calcTotals.netRemitCalcTotals->convertedBaseFare,
                                   money.noDec(ticketingDate));
    }

    prepareCommonSummaryAttrs(pricingTrx, fareCalcCollector, construct, totalsIter->second);
    prepareNoBrandsInfo(pricingTrx, calcTotals, construct);

    const Itin* itin = calcTotals.farePath->itin();
    if (itin && pricingTrx.isValidatingCxrGsaApplicable())
    {
      if (!fallback::fallbackValidatingCxrMultiSp(&pricingTrx) || pricingTrx.overrideFallbackValidationCXRMultiSP())
      {
        if (calcTotals.farePath && !calcTotals.farePath->defaultValCxrPerSp().empty())
          prepareValidatingCarrierLists(pricingTrx, construct, *(calcTotals.farePath));
        else
          buildValidatingCarrierList(pricingTrx, construct, *(calcTotals.farePath));
      }
      else
        buildValidatingCarrierList(pricingTrx, construct, *(calcTotals.farePath));
    }

    preparePassengerInfo(pricingTrx, fcConfig, calcTotals, paxNumber, construct);

    MAFUtil::checkElementONV(pricingTrx, &calcTotals, construct);

    construct.closeElement();
  }
}

void
AltPricingResponseFormatter::addAdditionalPaxInfo(PricingTrx& pricingTrx,
                                                  CalcTotals& calcTotals,
                                                  uint16_t paxNumber,
                                                  XMLConstruct& construct)
{
  const Itin* itin = !pricingTrx.itin().empty() ? pricingTrx.itin().front() : nullptr;
  const bool isUsDot = itin && itin->getBaggageTripType().isUsDot();
  const bool isSchemaValid = validSchemaVersion(pricingTrx);
  const bool useAdjustedCalcTotal =  pricingTrx.getOptions() &&
                                     !pricingTrx.getOptions()->isPDOForFRRule() &&
                                     calcTotals.adjustedCalcTotal != nullptr &&
                                     !calcTotals.adjustedCalcTotal->wpaInfo.wpnDetailResponse.empty();

  addUsDotItinIndicator(construct, itin, pricingTrx);

  construct.addAttribute(xml2::RequestedPassengerType, calcTotals.requestedPaxType);
  construct.addAttributeInteger(xml2::WpnOptionNumber, calcTotals.wpaInfo.psgDetailRefNo);

  addAdditionalPaxInfoFareCalc(construct,
                               pricingTrx,
                               (useAdjustedCalcTotal ? *calcTotals.adjustedCalcTotal : calcTotals),
                               isUsDot,
                               isSchemaValid,
                               TrxUtil::isBaggage302GlobalDisclosureActivated(pricingTrx));

  construct.addAttributeBoolean(xml2::SurfaceRestrictedInd,
                                calcTotals.farePath->intlSurfaceTvlLimit());

  if (!OBFeesUtils::fallbackObFeesWPA(&pricingTrx))
    construct.addAttributeBoolean(xml2::ForbidCreditCard,
                                  calcTotals.farePath->forbidCreditCardFOP());
}

void
AltPricingResponseFormatter::addAdditionalPaxInfoFareCalc(XMLConstruct& construct,
                                                          PricingTrx& pricingTrx,
                                                          const CalcTotals& calcTotals,
                                                          const bool isUsDot,
                                                          const bool isSchemaValid,
                                                          const bool isDisclosureActivate) const
{
  std::string baggageResponse;
  BaggageResponseBuilder baggageBuilder(pricingTrx, calcTotals);
  baggageBuilder.getWpaGsResponse(baggageResponse);

  std::string wpnDetails(calcTotals.wpaInfo.wpnDetailResponse);
  if (!isUsDot && isSchemaValid && isDisclosureActivate)
  {
    XformMsgFormatter::formatFc(
        FreeBaggageUtil::BaggageTagHead, FreeBaggageUtil::BaggageTagTotalSize + 1, "", wpnDetails);
    construct.addAttribute(xml2::WpnDetails, wpnDetails);
    construct.addAttribute(xml2::BaggageResponse, baggageResponse);
  }
  else
  {
    XformMsgFormatter::formatFc(FreeBaggageUtil::BaggageTagHead,
                                FreeBaggageUtil::BaggageTagTotalSize + 1,
                                baggageResponse,
                                wpnDetails);
    construct.addAttribute(xml2::WpnDetails, wpnDetails);
  }
}

bool
AltPricingResponseFormatter::validSchemaVersion(const PricingTrx& pricingTrx) const
{
  return ((pricingTrx.altTrxType() == PricingTrx::WPA ||
           pricingTrx.altTrxType() == PricingTrx::WPA_NOMATCH)
              ? pricingTrx.getRequest()->checkSchemaVersion(1, 0, 1)
              : pricingTrx.getRequest()->checkSchemaVersion(1, 1, 1));
}

void
AltPricingResponseFormatter::prepareFarePath(PricingTrx& pricingTrx,
                                             CalcTotals& calcTotals,
                                             const CurrencyNoDec& noDec1,
                                             const CurrencyNoDec& noDec2,
                                             uint16_t paxNumber,
                                             bool stopoverFlag,
                                             const FuFcIdMap& fuFcIdCol,
                                             XMLConstruct& construct)
{
  traverseTravelSegs(pricingTrx,
                     calcTotals,
                     noDec1,
                     noDec2,
                     *calcTotals.farePath,
                     paxNumber,
                     stopoverFlag,
                     fuFcIdCol,
                     construct);
}

void
AltPricingResponseFormatter::prepareOBFeesInfo(const PricingTrx& pricingTrx,
                                               XMLConstruct& construct) const
{
  construct.openElement(xml2::ServiceFeeDetailedInfo);

  const PricingRequest* request = pricingTrx.getRequest();
  if (OBFeesUtils::isBinCorrect(request->formOfPayment()))
  {
    setFopBinNumber(request->formOfPayment(), construct, xml2::FormOfPayment);
    setFopBinNumber(request->secondFormOfPayment(), construct, xml2::SecondFormOfPayment);
  }
  construct.addAttributeDouble(xml2::CardChargeAmount, request->paymentAmountFop());
  construct.addAttributeBoolean(xml2::ResidualSpecifiedChargeAmount, request->chargeResidualInd());

  std::vector<std::string> formsOfPayment;

  if (request->isFormOfPaymentCash())
    formsOfPayment.push_back(CASH);
  if (request->isFormOfPaymentCard())
    formsOfPayment.push_back(CREDIT_CARD);
  if (request->isFormOfPaymentCheck())
    formsOfPayment.push_back(CHECK);
  if (request->isFormOfPaymentGTR())
    formsOfPayment.push_back(GTR);

  if (!pricingTrx.itin().empty())
  {
    if (!request->isCollectOBFee())
      construct.addAttributeChar(xml2::ObFeesFailStatus, OBFeesUtils::OB_FEES_NOT_REQUESTED);
    else if (ItinUtil::getFirstValidTravelSeg(pricingTrx.itin().front()) == nullptr)
      construct.addAttributeChar(xml2::ObFeesFailStatus, OBFeesUtils::ALL_SEGMENTS_OPEN);
    else if (!ItinUtil::atleastOneSegmentConfirm(pricingTrx.itin().front()))
      construct.addAttributeChar(xml2::ObFeesFailStatus, OBFeesUtils::NOT_ALL_SEGS_CONFIRM);
  }

  construct.addAttribute(xml2::FormsOfPayment, boost::algorithm::join(formsOfPayment, DELIMITER));

  std::set<std::string> accCodeSet(request->accCodeVec().begin(), request->accCodeVec().end());
  if (!request->accountCode().empty())
    accCodeSet.insert(request->accountCode());
  construct.addAttribute(xml2::AccountCodes, boost::algorithm::join(accCodeSet, DELIMITER));

  std::set<std::string> corpIdSet(request->corpIdVec().begin(), request->corpIdVec().end());
  if (!request->corporateID().empty())
    corpIdSet.insert(request->corporateID());
  construct.addAttribute(xml2::CorporateIDs, boost::algorithm::join(corpIdSet, DELIMITER));

  construct.closeElement();
}

void
AltPricingResponseFormatter::prepareUncompressedResponseText(const PricingTrx& pricingTrx,
                                                             std::string& str)
{
  size_t bagTextPos = 0;
  const size_t bagTextLen = FreeBaggageUtil::BaggageTagTotalSize + 1;

  while (1)
  {
    bagTextPos = str.find(FreeBaggageUtil::BaggageTagHead, bagTextPos);

    if (bagTextPos == std::string::npos || bagTextPos + bagTextLen > str.size())
      break;

    str.erase(bagTextPos, bagTextLen);
  }

  if (pricingTrx.getOptions()->isServiceFeesTemplateRequested())
  {
    size_t sftTextPos = str.find(FareCalcConsts::SERVICE_FEE_AMOUNT_LINE);
    size_t sftTotalPos = str.find(FareCalcConsts::SERVICE_FEE_TOTAL_LINE);
    if (sftTextPos != std::string::npos && sftTotalPos != std::string::npos &&
        sftTextPos < sftTotalPos)
    {
      str.erase(sftTextPos,
                sftTotalPos - sftTextPos + FareCalcConsts::SERVICE_FEE_TOTAL_LINE.length() + 1);
    }
  }
}

std::string
AltPricingResponseFormatter::removeObFromResponse(const std::string& responseString) const
{
  std::string str = responseString;
  size_t obStart, obEnd;
  while ((obStart = str.find(OBFeesUtils::ObfSectionStart)) != std::string::npos &&
        ((obEnd = str.find(OBFeesUtils::ObfSectionEnd)) != std::string::npos))
  {
    str.erase(obStart, (obEnd - obStart) + OBFeesUtils::ObfSectionEnd.length());
  }
  return str;
}

std::string
AltPricingResponseFormatter::removeObTagsFromResponse(const std::string& responseString) const
{
  std::string str = responseString;
  size_t obStart, obEnd;
  while ((obStart = str.find(OBFeesUtils::ObfSectionStart)) != std::string::npos)
  {
    str.erase(obStart, OBFeesUtils::ObfSectionStart.length());
  }
  while ((obEnd = str.find(OBFeesUtils::ObfSectionEnd)) != std::string::npos)
  {
    str.erase(obEnd, OBFeesUtils::ObfSectionEnd.length());
  }
  return str;
}

void AltPricingResponseFormatter::prepareNoBrandsInfo(const PricingTrx& pricingTrx,
                                                      CalcTotals& calcTotals,
                                                      XMLConstruct& construct) const
{
  if ( pricingTrx.activationFlags().isSearchForBrandsPricing() )
  {
    if ( BrandingUtil::isNoBrandsOffered(pricingTrx, calcTotals) )
      construct.addAttributeBoolean(xml2::NoBrandsOffered, true);
  }
}

void AltPricingResponseFormatter::addOCFeesReferences(const std::map<std::string, std::set<int>>& sfgToOCFeesMapping,
                                                      XMLConstruct& construct)
{
  for(auto sfg : sfgToOCFeesMapping)
  {
    construct.openElement(xml2::GroupedOCFeesReferences);
    construct.addAttribute(xml2::ServiceGroup, sfg.first.c_str());
    for (auto ocfId : sfg.second)
    {
      construct.openElement(xml2::OCFeesReference);
      construct.addAttributeInteger(xml2::OCFeesReferenceNumber, ocfId);
      construct.closeElement(); // xml2::OCFeesReference
    }
    construct.closeElement(); // xml2::GroupedOCFeesReferences
  }
}

} // namespace tse
