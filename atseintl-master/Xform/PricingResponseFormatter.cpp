//----------------------------------------------------------------------------
//
//  File:  PricingResponseFormatter.cpp
//  Description: See PricingResponseFormatter.h file
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
#include "Xform/PricingResponseFormatter.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/BaggageStringFormatter.h"
#include "Common/CommissionKeys.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareMarketUtil.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/ObFeeDescriptors.h"
#include "Common/OBFeesUtils.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RtwUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TaxRound.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/MileageTypeData.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PrivateIndicator.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RefundPenalty.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/StructuredRuleData.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/NoPNROptions.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "Diagnostic/Diagnostic.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcHelper.h"
#include "FareCalc/FareCalculation.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/TicketingEndorsement.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Xform/CommonFormattingUtils.h"
#include "Xform/CommonParserUtils.h"
#include "Xform/ComparablePenaltyFee.h"
#include "Xform/DataModelMap.h"
#include "Xform/ERDSectionFormatter.h"
#include "Xform/OCFeesPrice.h"
#include "Xform/PreviousTaxInformationFormatter.h"
#include "Xform/PreviousTaxInformationModel.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/ReissueExchangeFormatter.h"
#include "Xform/StructuredRulesResponseFormatter.h"
#include "Xform/TaxFormatter.h"
#include "Xform/XformUtil.h"
#include "Xform/XMLFareCalcFormatter.h"
#include "Xform/XMLTaxSplitFormatter.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>

#include <unistd.h>

namespace tse
{
FALLBACK_DECL(fallbackFixFRRHpuForNet)
FALLBACK_DECL(fallbackFsc1155MoreThan147);
FALLBACK_DECL(fallbackCATElementMark);
FALLBACK_DECL(fallbackBrandedFaresPricing);
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(endorsementExpansion);
FIXEDFALLBACK_DECL(noNraAttrInShoppingResponse);
FALLBACK_DECL(fallbackCommissionManagement);
FALLBACK_DECL(fallbackDebugOverrideBrandingServiceResponses);
FIXEDFALLBACK_DECL(validateAllCat16Records);
FALLBACK_DECL(sswvt22412ObFeeTCurConv);
FALLBACK_DECL(fallbackOCFeesInSearchForBrandsPricing);
FALLBACK_DECL(fallbackAMCPhase2);
FALLBACK_DECL(extractReissueExchangeFormatter);
FALLBACK_DECL(fallbackSPInHierarchyOrder);
FALLBACK_DECL(fallbackAMC2Cat35CommInfo);
FALLBACK_DECL(virtualFOPMaxOBCalculation);
FALLBACK_DECL(fixSSDSP1780);
FALLBACK_DECL(fallbackJira1908NoMarkup);
FALLBACK_DECL(taxRexPricingRefundableInd);
FALLBACK_DECL(fallbackXMLVCCOrderChange);
FALLBACK_DECL(fallbackEndorsementsRefactoring)
FALLBACK_DECL(serviceFeeTimeInResponse)
FALLBACK_DECL(serviceFeeOpenSeg)
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(taxRexPricingTxType);
FALLBACK_DECL(fallbackFRROBFeesFixAsl);
FALLBACK_DECL(fallbackTagPY9matchITBTCCPayment);
FALLBACK_DECL(fallbackFixPQNRedistributedWholesale);

namespace
{
struct NonZeroAmount
{
  bool operator()(const TicketingFeesInfo* feeInfo) const
  {
    if (feeInfo->feeAmount() > EPSILON || feeInfo->feePercent() > EPSILON)
      return true;
    return false;
  }
};

// config params
const std::string INFANT_MESSAGE = "REQUIRES ACCOMPANYING ADT PASSENGER";

constexpr int32_t DEFAULT_MAX_PSS_OUTPUT_SIZE = 290000;

const size_t TAX_ON_OC_BUFF_SIZE = 15;
const Indicator TRUE = 'T';
const Indicator FALSE = 'F';
ConfigurableValue<uint32_t>
maxTotalBuffSize("OUTPUT_LIMITS", "MAX_PSS_OUTPUT", DEFAULT_MAX_PSS_OUTPUT_SIZE);
ConfigurableValue<uint16_t>
maxNumberOfFees("SERVICE_FEES_SVC", "MAX_NUMBER_OF_FEES", 0);
}
static const unsigned int AVAILABLE_SIZE_FOR_MSG = 204;
static const char MAY_NOT_APPLY_TO_ALL_PASSENGERS = 'P';

namespace
{
Logger
logger("atseintl.Xform.PricingResponseFormatter");
}

PricingResponseFormatter::PricingResponseFormatter()
  : _maxTotalBuffSize(maxTotalBuffSize.getValue())
{
}

void
PricingResponseFormatter::prepareRexSummary(RexPricingTrx* rexTrx,
                                            XMLConstruct& construct,
                                            FareCalcCollector& originalFCC)
{
  _ticketingDate = rexTrx->ticketingDate();

  bool haveAsBooked(rexTrx->isBookedSolutionValid());
  bool haveRebooked(rexTrx->isRebookedSolutionValid());

  if (!haveAsBooked)
    prepareMessage(construct,
                   Message::TYPE_ERROR,
                   Message::errCode(rexTrx->reissuePricingErrorCode()),
                   rexTrx->reissuePricingErrorMsg());

  if (haveAsBooked || haveRebooked)
  {
    std::vector<FarePath*> farePathV(rexTrx->itin()[0]->farePath());

    const FareCalcConfig& fcConfig = *(FareCalcUtil::getFareCalcConfig(*rexTrx));
    rexTrx->itin()[0]->farePath().resize(1);

    if (haveAsBooked)
      callPrepareSummaryForRex(
          rexTrx, *rexTrx->lowestBookedFarePath(), fcConfig, originalFCC, construct, "0");

    if (haveRebooked)
      callPrepareSummaryForRex(
          rexTrx, *rexTrx->lowestRebookedFarePath(), fcConfig, originalFCC, construct, "1");

    rexTrx->itin()[0]->farePath() = farePathV;
  }
}

void
PricingResponseFormatter::callPrepareSummaryForRex(RexPricingTrx* rexTrx,
                                                   FarePath& farePath,
                                                   const FareCalcConfig& fcConfig,
                                                   FareCalcCollector& originalFCC,
                                                   XMLConstruct& construct,
                                                   const char* itinNumber)
{
  rexTrx->itin()[0]->farePath()[0] = &farePath;
  if (originalFCC.passengerCalcTotals().size() > 1) // Has AsBooked and AsRebooked
  {
    FareCalcCollector fareCalcCollector;
    CollectedNegFareData* cnfd = farePath.collectedNegFareData();
    farePath.collectedNegFareData() = nullptr;
    fareCalcCollector.initialize(*rexTrx, rexTrx->itin()[0], &fcConfig, true);
    farePath.collectedNegFareData() = cnfd;

    CalcTotals* newCT = fareCalcCollector.findCalcTotals(&farePath);
    CalcTotals* orgCT = originalFCC.findCalcTotals(&farePath);
    if (newCT && orgCT)
    {
      newCT->fareCalculationLine = orgCT->fareCalculationLine;
      newCT->netCalcTotals = orgCT->netCalcTotals;
    }

    DateTime latestTktDT;
    DateTime earliestTktDT;
    farePath.determineMostRestrTktDT(
        *rexTrx, latestTktDT, earliestTktDT, fareCalcCollector.simultaneousResTkt());
    if (latestTktDT.isValid() && (latestTktDT > rexTrx->currentTicketingDT()))
    {
      fareCalcCollector.lastTicketDay() = latestTktDT.dateToSqlString();
      fareCalcCollector.lastTicketTime() = latestTktDT.timeToString(HHMM, ":");
    }

    prepareSummary(*rexTrx, fareCalcCollector, construct, itinNumber);
  }
  else
    prepareSummary(*rexTrx, originalFCC, construct, itinNumber);
}

void
PricingResponseFormatter::formatPricingResponse(
    XMLConstruct& construct,
    const std::string& responseString,
    bool displayOnly,
    PricingTrx& pricingTrx,
    FareCalcCollector* fareCalcCollector,
    ErrorResponseException::ErrorResponseCode tktErrCode,
    bool prepareAgentAndBilling /* = true*/)
{
  RexPricingTrx* rexTrx =
      (pricingTrx.excTrxType() == PricingTrx::AR_EXC_TRX ? static_cast<RexPricingTrx*>(&pricingTrx)
                                                         : nullptr);
  bool mtOfferedWithDiag854 =
      pricingTrx.diagnostic().diagnosticType() == Diagnostic854 &&
      pricingTrx.getRequest()->multiTicketActive() &&
      (MultiTicketUtil::getTicketSolution(pricingTrx) ==
           MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT ||
       MultiTicketUtil::getTicketSolution(pricingTrx) == MultiTicketUtil::SINGLETKT_NOT_APPLICABLE);

  if (!displayOnly && !mtOfferedWithDiag854 && (fareCalcCollector ||
       (pricingTrx.excTrxType() == PricingTrx::TAX_INFO_TRX  &&
        !fallback::taxRexPricingTxType(&pricingTrx))) )
  {
    if (prepareAgentAndBilling && pricingTrx.excTrxType() != PricingTrx::EXC1_WITHIN_ME &&
        pricingTrx.excTrxType() != PricingTrx::EXC2_WITHIN_ME && !pricingTrx.getRequest()->isSFR())
    {
      prepareAgent(pricingTrx, construct);
      prepareBilling(pricingTrx, *fareCalcCollector, construct);
    }

    if (rexTrx)
      prepareRexSummary(rexTrx, construct, *fareCalcCollector);
    else
      prepareSummary(pricingTrx, *fareCalcCollector, construct);
  }

  if (pricingTrx.diagnostic().diagnosticType() == Diagnostic854 && !mtOfferedWithDiag854)
  {
    prepareHostPortInfo(pricingTrx, construct);
    if (!pricingTrx.fareCalcCollector().empty())
      fareCalcCollector = pricingTrx.fareCalcCollector().front();
  }

  Diagnostic& diag = pricingTrx.diagnostic();

  bool unlimitedResponse =
      diag.diagParamIsSet(Diagnostic::NO_LIMIT, Diagnostic::UNLIMITED_RESPONSE);

  if (diag.diagParamMapItemPresent(Diagnostic::RESPONSE_SIZE))
  {
    adjustDiagnosticResponseSize(diag);
  }

  if (tktErrCode > 0)
  {
    if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct, unlimitedResponse);
    }

    // Pricing failed, return error message
    prepareErrorMessage(pricingTrx, construct, tktErrCode, responseString);
  }
  else if (pricingTrx.getRequest()->isSFR() && diag.diagnosticType() != DiagnosticNone)
  {
    prepareResponseText(responseString, construct, unlimitedResponse);
  }
  else
  {
    bool isRex = rexTrx || pricingTrx.excTrxType() == PricingTrx::AF_EXC_TRX;

    // Attaching MSG elements
    if ((diag.diagnosticType() != DiagnosticNone ||
         (!isRex && pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)) &&
        !pricingTrx.getRequest()->isSFR())
    {
      if (fareCalcCollector)
      {
        formatGreenScreenMsg(construct, responseString, pricingTrx, fareCalcCollector);
      }
      else
      {
        prepareResponseText(responseString, construct, unlimitedResponse);
      }
    }
  }

  if (rexTrx)
    prepareUnflownItinPriceInfo(*rexTrx, construct);

  prepareLatencyDataInResponse(pricingTrx, construct);
}

//remove with fallback taxRexPricingTxType
void
PricingResponseFormatter::formatResponseForFullRefund(
    XMLConstruct& construct,
    const std::string& responseString,
    RefundPricingTrx& trx,
    ErrorResponseException::ErrorResponseCode tktErrCode)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(trx, construct);
  }

  if (tktErrCode > 0)
  {
    if (trx.diagnostic().diagnosticType() != DiagnosticNone && !trx.diagnostic().toString().empty())
      prepareResponseText(trx.diagnostic().toString(), construct);

    prepareErrorMessage(trx, construct, tktErrCode, responseString);
  }
  else
  {
    if (trx.diagnostic().diagnosticType() != DiagnosticNone)
      prepareResponseText(responseString, construct);
    else
    {
      prepareAgentForFullRefund(trx, construct);
      prepareBillingForFullRefund(trx, construct);
      prepareSummaryForFullRefund(trx, construct);
    }
  }
}

void
PricingResponseFormatter::formatResponseIfRedirected(
    XMLConstruct& construct,
    const std::string& responseString,
    RexBaseTrx& rexTrx,
    FareCalcCollector* fareCalcCollector,
    ErrorResponseException::ErrorResponseCode tktErrCode)
{
  bool isSuccessfullyRedirected =
      !fareCalcCollector && rexTrx.redirectResult() &&
      !rexTrx.exchangePricingTrxForRedirect()->fareCalcCollector().empty();

  fareCalcCollector = (isSuccessfullyRedirected
                           ? rexTrx.exchangePricingTrxForRedirect()->fareCalcCollector().front()
                           : fareCalcCollector);

  if (fareCalcCollector)
  {
    prepareAgent(rexTrx, construct);
    prepareBilling(rexTrx, *fareCalcCollector, construct);

    if (isSuccessfullyRedirected)
    {
      if (rexTrx.diagnostic().diagnosticType() == DiagnosticNone)
        prepareErrorMessage(rexTrx,
                            construct,
                            rexTrx.redirectReasonError().code(),
                            rexTrx.redirectReasonError().message());
      prepareSummary(*rexTrx.exchangePricingTrxForRedirect(), *fareCalcCollector, construct);
    }
  }

  if (rexTrx.diagnostic().diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(rexTrx, construct);
  }

  Diagnostic& diag = rexTrx.diagnostic();
  if (tktErrCode > 0)
  {
    if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct);
    }

    if (!isSuccessfullyRedirected)
    {
      prepareErrorMessage(rexTrx,
                          construct,
                          rexTrx.redirectReasonError().code(),
                          rexTrx.redirectReasonError().message());
      prepareErrorMessage(rexTrx, construct, tktErrCode, responseString);
    }
  }
  else
  {
    if (diag.diagnosticType() != DiagnosticNone)
      prepareResponseText(responseString, construct);
  }
}

inline const std::string&
PricingResponseFormatter::logXmlData(const XMLConstruct& construct) const
{
  LOG4CXX_INFO(logger, "Response in XML:\n" << construct.getXMLData());
  return construct.getXMLData();
}

void
PricingResponseFormatter::formatResponse(const ErrorResponseException& ere,
                                         bool displayOnly,
                                         std::string& response)
{
  XMLConstruct construct;

  if (displayOnly)
    construct.openElement("PricingDisplayResponse");
  else
    construct.openElement("PricingResponse");

  ResponseFormatter::addMessageLine(ere.message(), construct, "E", Message::errCode(ere.code()));

  construct.closeElement();

  response = construct.getXMLData();
}

void
PricingResponseFormatter::formatRexResponse(const ErrorResponseException& ere,
                                            std::string& response)
{
  XMLConstruct construct;

  construct.openElement("RexPricingResponse");

  ResponseFormatter::addMessageLine(ere.message(), construct, "E", Message::errCode(ere.code()));

  construct.closeElement();

  response = construct.getXMLData();
}

void
PricingResponseFormatter::formatPricingResponseMTInactive(
    const std::string& responseString,
    bool displayOnly,
    ErrorResponseException::ErrorResponseCode tktErrCode,
    XMLConstruct& construct,
    PricingTrx& pricingTrx,
    FareCalcCollector* fareCalcCollector)

{
  formatPricingResponse(
      construct, responseString, displayOnly, pricingTrx, fareCalcCollector, tktErrCode);
  if (pricingTrx.getRequest()->isCollectOCFees() && fareCalcCollector)
    formatOCFeesResponse(construct, pricingTrx);

  if (checkForStructuredData(pricingTrx,
                             TrxUtil::isBaggageActivationXmlResponse(pricingTrx),
                             TrxUtil::isBaggage302ExchangeActivated(pricingTrx),
                             TrxUtil::isBaggage302GlobalDisclosureActivated(pricingTrx)))
    formatBaggageResponse(pricingTrx, construct);
}

void
PricingResponseFormatter::formatProperResponseWhetherMTActive(
    ErrorResponseException::ErrorResponseCode tktErrCode,
    const std::string& responseString,
    bool displayOnly,
    PricingTrx& pricingTrx,
    XMLConstruct& construct,
    FareCalcCollector* fareCalcCollector)
{
  if (pricingTrx.getRequest()->multiTicketActive() && tktErrCode == 0 &&
      (pricingTrx.diagnostic().diagnosticType() == DiagnosticNone ||
       pricingTrx.diagnostic().diagnosticType() == Diagnostic854))
  {
    formatPricingResponseMTActive(
        construct, responseString, displayOnly, pricingTrx, fareCalcCollector, tktErrCode);
  }
  else
  {
    formatPricingResponseMTInactive(
        responseString, displayOnly, tktErrCode, construct, pricingTrx, fareCalcCollector);
  }
}

std::string
PricingResponseFormatter::formatResponse(const std::string& responseString,
                                         bool displayOnly,
                                         PricingTrx& pricingTrx,
                                         FareCalcCollector* fareCalcCollector,
                                         ErrorResponseException::ErrorResponseCode tktErrCode)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << responseString);

  _ticketingDate = pricingTrx.ticketingDate();
  _itin = nullptr;

  if ((pricingTrx.excTrxType() == PricingTrx::ME_DIAG_TRX) ||
      (pricingTrx.excTrxType() == PricingTrx::NEW_WITHIN_ME) ||
      (pricingTrx.excTrxType() == PricingTrx::EXC1_WITHIN_ME) ||
      (pricingTrx.excTrxType() == PricingTrx::EXC2_WITHIN_ME))
  {
    XMLConstruct construct;
    appendCDataToResponse(pricingTrx, construct);
    formatPricingResponse(
        construct, responseString, displayOnly, pricingTrx, fareCalcCollector, tktErrCode);

    if (!pricingTrx.itin().empty() && TrxUtil::isBaggage302ExchangeActivated(pricingTrx) &&
        TrxUtil::isBaggageActivationXmlResponse(pricingTrx))
      formatBaggageResponse(pricingTrx, construct);

    return logXmlData(construct);
  }

  if (pricingTrx.excTrxType() == PricingTrx::AR_EXC_TRX ||
      (pricingTrx.excTrxType() == PricingTrx::TAX_INFO_TRX &&
        !fallback::taxRexPricingTxType(&pricingTrx)) ||
      pricingTrx.excTrxType() == PricingTrx::AF_EXC_TRX)
    return formatRexPricingResponse(
        responseString, static_cast<RexBaseTrx&>(pricingTrx), fareCalcCollector, tktErrCode);

  XMLConstruct construct;

  BaseExchangeTrx* anyRexTrx = dynamic_cast<BaseExchangeTrx*>(&pricingTrx);
  if (anyRexTrx)
  {
    construct.openElement("RexPricingResponse");
    construct.addAttribute(xml2::RequestType, anyRexTrx->reqType());
  }
  else
    construct.openElement(displayOnly ? "PricingDisplayResponse" : "PricingResponse");

  formatProperResponseWhetherMTActive(
      tktErrCode, responseString, displayOnly, pricingTrx, construct, fareCalcCollector);

  appendCDataToResponse(pricingTrx, construct);
  construct.closeElement();
  return logXmlData(construct);
}

void
PricingResponseFormatter::formatResponseForFullRefund(XMLConstruct& construct,
  const std::string& responseString,
  RexBaseTrx& rexTrx,
  ErrorResponseException::ErrorResponseCode tktErrCode,
  bool taxInfoRequest)
{
  if (rexTrx.diagnostic().diagnosticType() == Diagnostic854)
    prepareHostPortInfo(rexTrx, construct);

  if (tktErrCode > 0)
  {
    if (rexTrx.diagnostic().diagnosticType() != DiagnosticNone &&
        !rexTrx.diagnostic().toString().empty())
      prepareResponseText(rexTrx.diagnostic().toString(), construct);

    prepareErrorMessage(rexTrx, construct, tktErrCode, responseString);
  }
  else
  {
    if (rexTrx.diagnostic().diagnosticType() != DiagnosticNone)
      prepareResponseText(responseString, construct);
    else
    {
      prepareAgentForFullRefund(rexTrx, construct);
      prepareBillingForFullRefund(rexTrx, construct);

      if (taxInfoRequest)
        prepareSummaryForTaxInfo(rexTrx, construct);
      else
        prepareSummaryForFullRefund(static_cast<RefundPricingTrx&>(rexTrx), construct);
    }
  }
}

void
PricingResponseFormatter::prepareSummaryForTaxInfo(RexBaseTrx& trx, XMLConstruct& construct) const
{
  construct.openElement(xml2::SummaryInfo);
  construct.addAttribute(xml2::TicketingDate, trx.originalTktIssueDT().isValid() ?
    trx.originalTktIssueDT().dateToSqlString() : trx.currentTicketingDT().dateToSqlString());

  const uint16_t HOSTNAME_SIZE = 256;
  char hostname[HOSTNAME_SIZE];
  if (!gethostname(hostname, HOSTNAME_SIZE))
    construct.addAttribute(xml2::ServerHostname, hostname);

  construct.openElement(xml2::PassengerInfo);
  PreviousTaxInformationFormatter(construct).formatPTI(PreviousTaxInformationModel(trx));
  construct.closeElement();

  construct.closeElement();
}

std::string
PricingResponseFormatter::formatRexPricingResponse(
    const std::string& responseString,
    RexBaseTrx& trx,
    FareCalcCollector* fareCalcCollector,
    ErrorResponseException::ErrorResponseCode tktErrCode)
{
  XMLConstruct construct;
  construct.openElement("RexPricingResponse");
  construct.addAttribute(xml2::RequestType, trx.reqType());

  if (trx.redirected())
  {
    construct.addAttribute(xml2::SecondaryRequestType, trx.secondaryExcReqType());
    formatResponseIfRedirected(construct, responseString, trx, fareCalcCollector, tktErrCode);
  }
  else if (trx.excTrxType() == PricingTrx::AF_EXC_TRX &&
           static_cast<RefundPricingTrx&>(trx).fullRefund())
  {
    //with fallback remove unused taxRexPricingTxType method
    if(!fallback::taxRexPricingTxType(&trx))
      formatResponseForFullRefund(
        construct, responseString, trx, tktErrCode, false);
    else
      formatResponseForFullRefund(
        construct, responseString, static_cast<RefundPricingTrx&>(trx), tktErrCode);
  }
  else if (trx.excTrxType() == PricingTrx::TAX_INFO_TRX)
  {
    formatResponseForFullRefund(
        construct, responseString, trx, tktErrCode, true);
  }
  else
  {
    formatPricingResponse(construct, responseString, false, trx, fareCalcCollector, tktErrCode);
  }

  if (!trx.itin().empty() && TrxUtil::isBaggage302ExchangeActivated(trx) &&
      TrxUtil::isBaggageActivationXmlResponse(trx))
    formatBaggageResponse(trx, construct);

  appendCDataToResponse(trx, construct);

  construct.closeElement();
  return logXmlData(construct);
}

void
PricingResponseFormatter::prepareUnflownItinPriceInfo(RexPricingTrx& rexTrx,
                                                      XMLConstruct& construct)
{
  if (!rexTrx.isCSOSolutionValid() || rexTrx.pricingTrxForCSO()->fareCalcCollector().empty())
    return;

  CalcTotals* calcTotal = rexTrx.pricingTrxForCSO()->fareCalcCollector().front()->findCalcTotals(
      rexTrx.lowestCSOFarePath());
  if (!calcTotal)
    return;

  Money moneyEquiv(calcTotal->equivCurrencyCode);
  construct.openElement(xml2::UnflownItinPriceInfo);
  construct.addAttribute(xml2::PaxFareCurrencyCode, calcTotal->equivCurrencyCode);

  if (rexTrx.segmentFeeApplied())
  {
    construct.addAttributeDouble(xml2::TotalPerPassengerPlusImposed,
                                 (calcTotal->equivFareAmount + calcTotal->taxAmount()),
                                 moneyEquiv.noDec(rexTrx.ticketingDate()));
    construct.addAttributeDouble(xml2::TotalTaxesPlusImposed,
                                 calcTotal->taxAmount(),
                                 moneyEquiv.noDec(rexTrx.ticketingDate()));
  }

  calcTotal->getMutableFcTaxInfo().subtractSupplementalFeeFromTaxAmount();
  construct.addAttributeDouble(xml2::TotalPerPassenger,
                               (calcTotal->equivFareAmount + calcTotal->taxAmount()),
                               moneyEquiv.noDec(rexTrx.ticketingDate()));

  construct.closeElement();
}

void
PricingResponseFormatter::prepareErrorMessage(PricingTrx& pricingTrx,
                                              XMLConstruct& construct,
                                              ErrorResponseException::ErrorResponseCode errCode,
                                              const std::string& msgText)
{
  if (errCode > 0)
  {
    if (errCode == ErrorResponseException::VALIDATING_CXR_ERROR &&
        0 == strncmp(msgText.c_str(), "MULTIPLE VALIDATING CARRIER OPTIONS - ", 38))
    {
      prepareErrorMessageWithErrorCode(pricingTrx, construct, errCode, msgText);
      std::string noFareMsg = "SELECT ONE AND RETRY USING $A";

      prepareErrorMessageWithErrorCode(pricingTrx, construct, errCode, noFareMsg);
      return;
    }
    if (typeid(pricingTrx) == typeid(NoPNRPricingTrx) &&
        (errCode == ErrorResponseException::NO_FARE_FOR_CLASS_USED ||
         errCode == ErrorResponseException::NO_RULES_FOR_PSGR_TYPE_OR_CLASS ||
         errCode == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS ||
         errCode == ErrorResponseException::NO_FARE_VALID_FOR_PSGR_TYPE ||
         errCode == ErrorResponseException::NO_FARE_FOR_CLASS ||
         errCode == ErrorResponseException::NO_FARES_RBD_CARRIER ||
         errCode == ErrorResponseException::NO_APPLICABLE_YY_FARES ||
         errCode == ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT))
    {
      // WQ specific
      const NoPNROptions* noPnrOptions = nullptr;

      NoPNRPricingTrx* noPnrTrx = static_cast<NoPNRPricingTrx*>(&pricingTrx);
      noPnrOptions = noPnrTrx->noPNROptions();

      if (noPnrOptions != nullptr)
      {
        std::string noFareMsg = "";

        if (noPnrOptions->wqNotPermitted() == 'Y')
        {
          noFareMsg = "INVALID INPUT FORMAT";
        }
        else if (pricingTrx.getOptions() && pricingTrx.getOptions()->isCat35Net())
        {
          noFareMsg = "NO NET FARE AMOUNT";
        }
        else if (noPnrOptions->noMatchNoFaresErrorMsg() == '1')
        {
          noFareMsg = "*NO FARES FOR CLASS USED";
        }
        else // noPnrOptions->noMatchNoFaresErrorMsg() == '2'
        {
          noFareMsg = "*NO FARES/RBD/CARRIER";
        }

        // Axess user - additional 'VE' indicator
        if (pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent() &&
            pricingTrx.getRequest()->ticketingAgent()->axessUser())
          noFareMsg = std::string("VE\n") + noFareMsg;

        prepareResponseText(noFareMsg, construct);
        return;
      }
      // fallback to WPA processing
    }
    // Abacus specific error message
    if (pricingTrx.isNotExchangeTrx() && pricingTrx.getRequest() &&
        pricingTrx.getRequest()->ticketingAgent() &&
        (pricingTrx.getRequest()->ticketingAgent()->abacusUser() ||
         pricingTrx.getRequest()->ticketingAgent()->axessUser() ||
         pricingTrx.getRequest()->ticketingAgent()->infiniUser() ||
         (pricingTrx.getRequest()->ticketingAgent()->sabre1SUser() &&
          (pricingTrx.altTrxType() == AltPricingTrx::WPA))) &&
        (errCode == ErrorResponseException::NO_FARE_FOR_CLASS_USED ||
         errCode == ErrorResponseException::NO_RULES_FOR_PSGR_TYPE_OR_CLASS ||
         errCode == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS ||
         errCode == ErrorResponseException::NO_FARE_VALID_FOR_PSGR_TYPE ||
         errCode == ErrorResponseException::NO_FARE_FOR_CLASS ||
         errCode == ErrorResponseException::NO_FARES_RBD_CARRIER ||
         errCode == ErrorResponseException::NO_APPLICABLE_YY_FARES ||
         errCode == ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT ||
         errCode == ErrorResponseException::SFB_RULE_VALIDATION_FAILED ||
         errCode == ErrorResponseException::FARE_BASIS_NOT_AVAIL))
    {
      AltPricingTrx* altTrx = dynamic_cast<AltPricingTrx*>(&pricingTrx);
      std::string noFareMsg;
      if (altTrx && (altTrx->altTrxType() == AltPricingTrx::WPA) && pricingTrx.getOptions() &&
          pricingTrx.getOptions()->isCat35Net())
      {
        noFareMsg = "NO NET FARE AMOUNT";
      }
      else if (pricingTrx.isRequestedFareBasisInfoUseful())
      {
        const bool ruleFail = pricingTrx.areFareMarketsFailingOnRules();
        const char* msgNoFareRbd = "NO FARES/RBD/CARRIER";
        const char* msgFareNotAvail = "$FORMAT$ FARE BASIS NOT AVAILABLE";
        const char* msgFareValidFail = "ATTN* RULE VALIDATION FAILED, TRY COMMAND PRICING WPQ";

        // Abacus Specific Fare Basis
        if (errCode == ErrorResponseException::NO_FARE_FOR_CLASS_USED)
        {
          if (ruleFail)
          {
            noFareMsg = msgFareValidFail;
          }
          else
          {
            noFareMsg = msgNoFareRbd;
          }
        }
        else if (errCode == ErrorResponseException::NO_RULES_FOR_PSGR_TYPE_OR_CLASS ||
                 errCode == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS ||
                 errCode == ErrorResponseException::NO_FARE_VALID_FOR_PSGR_TYPE ||
                 errCode == ErrorResponseException::NO_FARE_FOR_CLASS ||
                 errCode == ErrorResponseException::NO_FARES_RBD_CARRIER ||
                 errCode == ErrorResponseException::NO_APPLICABLE_YY_FARES ||
                 errCode == ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT)
        {
          noFareMsg = msgNoFareRbd;
        }
        else if (errCode == ErrorResponseException::FARE_BASIS_NOT_AVAIL)
        {
          noFareMsg = msgFareNotAvail;
        }
        else if (errCode == ErrorResponseException::SFB_RULE_VALIDATION_FAILED || ruleFail)
        {
          noFareMsg = msgFareValidFail;
        }
        else
        {
          prepareErrorMessageWithErrorCode(pricingTrx, construct, errCode, noFareMsg);
        }
      }
      else
      {
        noFareMsg = "NO FARES/RBD/CARRIER";
        FareCalculation::getMsgAppl(
            FareCalcConfigText::WPA_NO_MATCH_NO_FARE, noFareMsg, pricingTrx);
      }

      if ((pricingTrx.getOptions() && pricingTrx.getOptions()->returnAllData() == GDS) ||
          (pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent() &&
           pricingTrx.getRequest()->ticketingAgent()->axessUser()))
      {
        prepareErrorMessageWithErrorCode(
            pricingTrx, construct, ErrorResponseException::NO_FARES_RBD_CARRIER, noFareMsg);
      }
      else
      {
        if (pricingTrx.billing()->requestPath() == SWS_PO_ATSE_PATH)
          prepareErrorMessageWithErrorCode(pricingTrx, construct, errCode, noFareMsg);
        else
          prepareResponseText(noFareMsg, construct);
      }
    }
    else
    {
      prepareErrorMessageWithErrorCode(pricingTrx, construct, errCode, msgText);
    }
  }
  else
  {
    prepareMessage(construct, Message::TYPE_GENERAL, Message::errCode(errCode), msgText);
  }
}

void
PricingResponseFormatter::prepareErrorMessageWithErrorCode(
    PricingTrx& pricingTrx,
    XMLConstruct& construct,
    ErrorResponseException::ErrorResponseCode errCode,
    const std::string& msgText)
{
  uint16_t msgErrCode = Message::errCode(errCode);

  if (pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent() &&
      (pricingTrx.getRequest()->ticketingAgent()->abacusUser() ||
       pricingTrx.getRequest()->ticketingAgent()->infiniUser()) &&
      errCode == ErrorResponseException::LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR)
  {
    bool is370Diag = (pricingTrx.diagnostic().diagnosticType() == Diagnostic370) &&
                     (pricingTrx.diagnostic().isActive());
    bool is372Diag = (pricingTrx.diagnostic().diagnosticType() == Diagnostic372) &&
                     (pricingTrx.diagnostic().isActive());

    if (!is370Diag && !is372Diag)
    {
      std::vector<std::string> msgTextVector;
      msgTextVector.push_back(msgText);
      msgTextVector.push_back("*SEE OTHER FARES-USE WPA");

      prepareMessage(construct, Message::TYPE_ERROR, msgTextVector);
      return;
    }
  }

  if (msgErrCode == 0 && pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent() &&
      pricingTrx.getRequest()->ticketingAgent()->axessUser() && pricingTrx.getOptions() &&
      pricingTrx.getOptions()->returnAllData() != GDS)
    prepareMessage(construct, Message::TYPE_ERROR, msgErrCode, "VE");

  prepareMessage(construct, Message::TYPE_ERROR, msgErrCode, msgText);
}

void
PricingResponseFormatter::prepareSimpleMessage(XMLConstruct& construct,
                                               const char msgType,
                                               const uint16_t msgCode,
                                               const std::string& msgText) const
{
  construct.openElement(xml2::MessageInformation);
  construct.addAttribute(xml2::MessageText, msgText);
  construct.closeElement();
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareMessage
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareMessage(XMLConstruct& construct,
                                         const char msgType,
                                         const uint16_t msgCode,
                                         const std::string& msgText) const
{
  construct.openElement(xml2::MessageInformation);
  construct.addAttributeChar(xml2::MessageType, msgType);
  construct.addAttributeShort(xml2::MessageFailCode, msgCode);
  construct.addAttribute(xml2::MessageText, msgText);
  construct.closeElement();
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareMessage
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareMessage(XMLConstruct& construct,
                                         const char msgType,
                                         const std::vector<std::string>& msgTextVec)
{
  for (const std::string& msg : msgTextVec)
  {
    construct.openElement(xml2::MessageInformation);
    construct.addAttributeChar(xml2::MessageType, msgType);
    construct.addAttributeShort(xml2::MessageFailCode, 0);
    construct.addAttribute(xml2::MessageText, msg);
    construct.closeElement();
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareMessage
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareMessage(XMLConstruct& construct,
                                         const char msgType,
                                         const uint16_t msgCode,
                                         const std::string& airlineCode,
                                         const std::string& msgText)
{
  construct.openElement(xml2::MessageInformation);
  construct.addAttributeChar(xml2::MessageType, msgType);
  construct.addAttributeShort(xml2::MessageFailCode, msgCode);
  if (!airlineCode.empty())
    construct.addAttribute(xml2::MessageAirlineCode, airlineCode);
  construct.addAttribute(xml2::MessageText, msgText);
  construct.closeElement();
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareMessage
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareMessage(PricingTrx& pricingTrx,
                                         XMLConstruct& construct,
                                         const FcMessage& message)
{
  if (TrxUtil::swsPoAtsePath(pricingTrx) &&
      message.messageType() == FcMessage::NOPNR_RULE_WARNING && message.messageText().size() > 3)
  {
    std::string messageText = message.messageText().substr(3);
    prepareMessage(construct,
                   message.messageType(),
                   message.messageCode(),
                   message.airlineCode(),
                   messageText);
  }
  else
  {
    prepareMessage(construct,
                   message.messageType(),
                   message.messageCode(),
                   message.airlineCode(),
                   message.messageText());
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareAgent
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareAgent(PricingTrx& pricingTrx, XMLConstruct& construct)
{
  if (!pricingTrx.getRequest() || !pricingTrx.getRequest()->ticketingAgent())
    return;

  const Agent& agent = *pricingTrx.getRequest()->ticketingAgent();

  construct.openElement(xml2::TicketingAgentInfo);

  construct.addAttribute(xml2::AgentCity, agent.agentCity());
  construct.addAttribute(xml2::TvlAgencyPCC, agent.tvlAgencyPCC());
  construct.addAttribute(xml2::MainTvlAgencyPCC, agent.mainTvlAgencyPCC());
  construct.addAttribute(xml2::TvlAgencyIATA, agent.tvlAgencyIATA());
  construct.addAttribute(xml2::HomeAgencyIATA, agent.homeAgencyIATA());
  construct.addAttribute(xml2::AgentFunctions, agent.agentFunctions());
  construct.addAttribute(xml2::AgentDuty, agent.agentDuty());
  construct.addAttribute(xml2::AirlineDept, agent.airlineDept());
  construct.addAttribute(xml2::CxrCode, MCPCarrierUtil::swapToPseudo(&pricingTrx, agent.cxrCode()));
  construct.addAttribute(xml2::CurrencyCodeAgent, agent.currencyCodeAgent());
  construct.addAttributeUShort(xml2::CoHostID, agent.coHostID());
  construct.addAttribute(xml2::AgentCommissionType, agent.agentCommissionType());
  construct.addAttribute(xml2::VendorCrsCode,
                         MCPCarrierUtil::swapToPseudo(&pricingTrx, agent.vendorCrsCode()));

  if (!agent.agentCommissionType().empty())
  {
    construct.addAttributeUInteger(xml2::AgentCommissionAmount, agent.agentCommissionAmount());
  }

  if (!agent.officeDesignator().empty()) // optional parameter
  {
    construct.addAttribute(xml2::OfficeDesignator, agent.officeDesignator()); // AE1
  }

  if (!agent.officeStationCode().empty()) // optional parameter
  {
    construct.addAttribute(xml2::OfficeCode, agent.officeStationCode()); // AE2
  }

  if (!agent.airlineChannelCode().empty()) // optional parameter
  {
    construct.addAttribute(xml2::AirlineChannelId, agent.airlineChannelCode()); // AE4
  }

  construct.closeElement();
}

void
PricingResponseFormatter::prepareAgentForFullRefund(RexBaseTrx& trx, XMLConstruct& construct)
{
  construct.openElement(xml2::TicketingAgentInfo);

  const Agent& agent = *static_cast<RexBaseRequest&>(*trx.getRequest()).currentTicketingAgent();
  construct.addAttribute(xml2::AgentCity, agent.agentCity());
  construct.addAttribute(xml2::MainTvlAgencyPCC, agent.mainTvlAgencyPCC());
  construct.addAttribute(xml2::AgentFunctions, agent.agentFunctions());
  construct.addAttribute(xml2::AgentDuty, agent.agentDuty());
  construct.addAttribute(xml2::AirlineDept, agent.airlineDept());
  construct.addAttribute(xml2::CxrCode, MCPCarrierUtil::swapToPseudo(&trx, agent.cxrCode()));
  construct.addAttribute(xml2::CurrencyCodeAgent, agent.currencyCodeAgent());
  construct.addAttribute(xml2::VendorCrsCode,
                         MCPCarrierUtil::swapToPseudo(&trx, agent.vendorCrsCode()));

  construct.closeElement();
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareBilling
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareBilling(PricingTrx& pricingTrx,
                                         FareCalcCollector& fareCalcCollector,
                                         XMLConstruct& construct)
{
  const Billing& billing = *pricingTrx.billing();
  construct.openElement(xml2::BillingInformation);
  addBillingElements(pricingTrx, billing, construct);
  construct.closeElement();
}

void
PricingResponseFormatter::prepareBillingForFullRefund(RexBaseTrx& trx,
                                                      XMLConstruct& construct)
{
  const Billing& billing = *trx.billing();
  construct.openElement(xml2::BillingInformation);
  addBillingElements(static_cast<PricingTrx&>(trx), billing, construct);
  construct.addAttributeULong(xml2::ClientTrxID, billing.clientTransactionID());
  construct.closeElement();
}

void
PricingResponseFormatter::addBillingElements(PricingTrx& pricingTrx,
                                             const Billing& billing,
                                             XMLConstruct& construct)
{
  construct.addAttribute(xml2::UserPseudoCityCode, billing.userPseudoCityCode());
  construct.addAttribute(xml2::UserStation, billing.userStation());
  construct.addAttribute(xml2::UserBranch, billing.userBranch());
  construct.addAttribute(xml2::PartitionID,
                         MCPCarrierUtil::swapToPseudo(&pricingTrx, billing.partitionID()));
  construct.addAttribute(xml2::UserSetAddress, billing.userSetAddress());
  construct.addAttribute(xml2::ServiceName, billing.serviceName());
  construct.addAttribute(xml2::AaaCity, billing.aaaCity());
  construct.addAttribute(xml2::AaaSine, billing.aaaSine());
  construct.addAttribute(xml2::ActionCode, billing.actionCode());
}

void
PricingResponseFormatter::prepareValidatingCarrierAttr(const PricingTrx& pricingTrx,
                                                       XMLConstruct& construct)
{
  CarrierCode carrier = getValidatingCarrier(pricingTrx);

  if (carrier != BAD_CARRIER)
    construct.addAttribute(xml2::ValidatingCarrier, carrier);
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareCommonSummaryAttrs
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareCommonSummaryAttrs(PricingTrx& pricingTrx,
                                                    FareCalcCollector& fareCalcCollector,
                                                    XMLConstruct& construct,
                                                    CalcTotals* calcTotals)
{
  if (!pricingTrx.getRequest()->isSFR())
  {
    construct.addAttribute(xml2::TicketingDate, pricingTrx.ticketingDate().dateToSqlString());
    construct.addAttributeInteger(xml2::TicketingTime, pricingTrx.ticketingDate().totalMinutes());
  }
  construct.addAttribute(xml2::IATASalesCode, fareCalcCollector.IATASalesCode());

  LocCode locCode;
  getSaleLoc(pricingTrx, locCode);
  construct.addAttribute(xml2::SalesLocation, locCode);
  if (!pricingTrx.getRequest()->isSFR())
  {
    getTicketLoc(pricingTrx, locCode);
    construct.addAttribute(xml2::TicketPointOverride, locCode);
  }

  prepareValidatingCarrierAttr(pricingTrx, construct);

  if (pricingTrx.excTrxType() != PricingTrx::AF_EXC_TRX && !pricingTrx.getRequest()->isSFR())
  {
    if (pricingTrx.altTrxType() == PricingTrx::WP)
    {
      construct.addAttribute(xml2::LastTicketDay, fareCalcCollector.lastTicketDay()); // d00
      construct.addAttribute(xml2::AdvancedPurchaseDate, fareCalcCollector.lastTicketDay()); // d14
      construct.addAttribute(xml2::PurchaseByDate, fareCalcCollector.lastTicketDay()); // d16

      std::ostringstream temp;
      temp << fareCalcCollector.lastTicketTime();
      construct.addAttribute(xml2::LastTicketTime, temp.str());
    }
    else
    {
      construct.addAttribute(xml2::LastTicketDay, calcTotals->lastTicketDay); // d00
      construct.addAttribute(xml2::AdvancedPurchaseDate, calcTotals->lastTicketDay); // d14
      construct.addAttribute(xml2::PurchaseByDate, calcTotals->lastTicketDay); // d16

      std::ostringstream temp;
      temp << calcTotals->lastTicketTime;
      construct.addAttribute(xml2::LastTicketTime, temp.str());
    }
  }

  const FarePath* farePath = fareCalcCollector.passengerCalcTotals().front()->farePath;

  if (farePath->exchangeReissue() != BLANK)
  {
    construct.addAttributeChar(xml2::ExchangeReissueIndicator, farePath->exchangeReissue());
    BaseExchangeTrx& excTrx = static_cast<BaseExchangeTrx&>(pricingTrx);
    if (farePath->exchangeReissue() == EXCHANGE)
    {
      construct.addAttribute(xml2::RoeBSRDate, excTrx.currentTicketingDT().dateToSqlString());
      if (excTrx.currentTicketingDT().historicalIncludesTime())
        construct.addAttribute(xml2::RoeBSRTime,
                               excTrx.currentTicketingDT().timeToString(HHMM, ":"));
    }
    else if (farePath->exchangeReissue() == REISSUE)
    {
      if (excTrx.excTrxType() == PricingTrx::AR_EXC_TRX)
      {
        const RexPricingTrx& rtrx = static_cast<const RexPricingTrx&>(excTrx);

        const DateTime adjustedPreviousDate = rtrx.adjustToCurrentUtcZone();
        if (adjustedPreviousDate == DateTime::emptyDate())
          LOG4CXX_ERROR(logger, "D95 cat31 date utc adjustement failed !!!");

        construct.addAttribute(xml2::RoeBSRDate, adjustedPreviousDate.dateToSqlString());
        if (adjustedPreviousDate.historicalIncludesTime())
          construct.addAttribute(xml2::RoeBSRTime, adjustedPreviousDate.timeToString(HHMM, ":"));
      }
      else
      {
        if (excTrx.previousExchangeDT().isEmptyDate())
        {
          construct.addAttribute(xml2::RoeBSRDate, excTrx.originalTktIssueDT().dateToSqlString());
          if (excTrx.originalTktIssueDT().historicalIncludesTime())
            construct.addAttribute(xml2::RoeBSRTime,
                                   excTrx.originalTktIssueDT().timeToString(HHMM, ":"));
        }
        else
        {
          construct.addAttribute(xml2::RoeBSRDate, excTrx.previousExchangeDT().dateToSqlString());
          if (excTrx.previousExchangeDT().historicalIncludesTime())
            construct.addAttribute(xml2::RoeBSRTime,
                                   excTrx.previousExchangeDT().timeToString(HHMM, ":"));
        }
      }
    }
  }

  std::string today = DateTime::fromMinutes(0).dateToSqlString();
  construct.addAttributeBoolean(xml2::SimultaneousRes, fareCalcCollector.simultaneousResTkt());

  const PricingOptions* options = pricingTrx.getOptions();
  if (options != nullptr)
  {
    construct.addAttributeBoolean(xml2::PrivateFareIndication, options->isPrivateFares());
  }

  if (fareCalcCollector.isFareCalcTooLong())
  {
    construct.addAttributeBoolean(xml2::FareCalcTooLong, true);
  }

  if (fareCalcCollector.passengerCalcTotals().front()->dispSegmentFeeMsg() ||
      pricingTrx.segmentFeeApplied())
    construct.addAttributeBoolean(xml2::SegmentFeeMessage, true);

  const uint16_t HOSTNAME_SIZE = 256;
  char hostname[HOSTNAME_SIZE];
  if (gethostname(hostname, HOSTNAME_SIZE) == 0)
  {
    construct.addAttribute(xml2::ServerHostname, hostname);
  }

  if (isAncillaryNonGuarantee(pricingTrx, farePath))
  {
    construct.addAttributeBoolean(xml2::AncillariesNonGuarantee, true);
    _ancNonGuarantee = true;
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareSummary
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareSummary(PricingTrx& pricingTrx,
                                         FareCalcCollector& fareCalcCollector,
                                         XMLConstruct& construct,
                                         const char* itinNumber)
{
  Itin* itin = getItin(pricingTrx);
  construct.openElement(xml2::SummaryInfo);

  if (pricingTrx.excTrxType() == PricingTrx::NEW_WITHIN_ME)
    construct.addAttribute(xml2::ItinNumber, "0");
  else if (pricingTrx.excTrxType() == PricingTrx::EXC1_WITHIN_ME)
    construct.addAttribute(xml2::ItinNumber, "1");
  else if (pricingTrx.excTrxType() == PricingTrx::EXC2_WITHIN_ME)
    construct.addAttribute(xml2::ItinNumber, "2");

  // Get total amount and currency
  CurrencyCode totalPriceCurrency;
  CurrencyNoDec totalPriceNoDec;
  MoneyAmount totalPrice =
      fareCalcCollector.getBaseFareTotal(pricingTrx, totalPriceCurrency, totalPriceNoDec, false);

  construct.addAttributeDouble(xml2::TotalPriceAll, totalPrice, totalPriceNoDec);
  construct.addAttribute(xml2::TotalCurrencyCode, fareCalcCollector.getTotalPriceCurrency());

  MoneyAmount netRemitTotalPrice =
      fareCalcCollector.getBaseFareTotal(pricingTrx, totalPriceCurrency, totalPriceNoDec, true);
  if (netRemitTotalPrice != 0)
    construct.addAttributeDouble(xml2::NetRemitTotalPriceAll, netRemitTotalPrice, totalPriceNoDec);

  if (itin->isPlusUpPricing())
  {
    const FarePath* farePath = fareCalcCollector.passengerCalcTotals().front()->farePath;

    Money fareCalcCurrency(farePath->calculationCurrency());

    construct.addAttribute(xml2::ConsolidatorPlusUpCurrencyCode,
                           itin->consolidatorPlusUp()->currencyCode());
    construct.addAttributeDouble(xml2::ConsolidatorPlusUpAmount,
                                 itin->consolidatorPlusUp()->fareCalcAmount(),
                                 fareCalcCurrency.noDec());
  }

  if (itinNumber)
    construct.addAttribute(xml2::ItinNumber, itinNumber);

  prepareCommonSummaryAttrs(pricingTrx, fareCalcCollector, construct);

  if (itin && pricingTrx.isValidatingCxrGsaApplicable())
  {
    for (const CalcTotals* calc : fareCalcCollector.passengerCalcTotals())
    {
      if (calc->farePath->processed())
      {
        if (!fallback::fallbackValidatingCxrMultiSp(&pricingTrx) || pricingTrx.overrideFallbackValidationCXRMultiSP())
        {
          if (calc->farePath && !calc->farePath->defaultValCxrPerSp().empty())
            prepareValidatingCarrierLists(pricingTrx, construct, *(calc->farePath));
          else
            buildValidatingCarrierList(pricingTrx, construct, *(calc->farePath));
        }
        else
          buildValidatingCarrierList(pricingTrx, construct, *(calc->farePath));

        break;
      }
    }
  }

  preparePassengers(pricingTrx, fareCalcCollector, construct);

  MAFUtil::checkElementONV(pricingTrx, fareCalcCollector, construct);

  construct.closeElement();
}

bool
PricingResponseFormatter::isAnyDefaultVcxrEmpty(const FarePath& farePath) const
{
  for (const auto& spCxrPair : farePath.defaultValCxrPerSp())
    if (spCxrPair.second.empty())
      return true;
  return false;
}

// Builds all VCL elements.
void
PricingResponseFormatter::prepareValidatingCarrierLists(PricingTrx& pricingTrx,
                                                        XMLConstruct& construct,
                                                        const FarePath& fp)
{
  const std::string inhibitPq = isAnyDefaultVcxrEmpty(fp) ? "T" : "F";
  if (!fallback::fallbackSPInHierarchyOrder(&pricingTrx))
  {
    uint16_t spCount = 0;
    for (SettlementPlanType sp : vcx::SP_HIERARCHY)
    {
      if (sp == "IPC")
        continue;
      if (spCount >= fp.defaultValCxrPerSp().size())
        break;

      auto spIt = fp.defaultValCxrPerSp().find(sp);
      if (spIt != fp.defaultValCxrPerSp().end())
      {
        buildValidatingCarrierList(construct, fp, spIt->first, spIt->second, inhibitPq);
        ++spCount;
      }
    }
  }
  else
  {
    if (fp.validatingCarriers().empty() && fp.defaultValCxrPerSp().empty())
      return;

    for (const auto& spCxrPair : fp.defaultValCxrPerSp())
      buildValidatingCarrierList(construct, fp, spCxrPair.first, spCxrPair.second, inhibitPq);
  }
}

// Builds VCL element and corresponding DCX/ACX elements (default and alternate carriers).
void
PricingResponseFormatter::buildValidatingCarrierList(XMLConstruct& construct,
                                                     const FarePath& fp,
                                                     const SettlementPlanType& sp,
                                                     const CarrierCode& defaultValCxr,
                                                     const std::string& inhibitPq)
{
  if (!fp.itin() || !fp.itin()->spValidatingCxrGsaDataMap())
    return;

  auto it = fp.itin()->spValidatingCxrGsaDataMap()->find(sp);
  if (it == fp.itin()->spValidatingCxrGsaDataMap()->end() || !it->second)
    return;

  // VCL per SP
  const ValidatingCxrGSAData& valCxrGsaData = *(it->second);
  construct.openElement(xml2::VcxrInfo);
  construct.addAttribute(xml2::InhibitPq, inhibitPq);
  construct.addAttribute(xml2::SettlementMethod, sp);
  construct.addAttribute(xml2::AtseProcess, "T"); // Always true

  // Indicator set to 'T' when we have no Default Validating Carriers and Multiple Neutral
  // Validating Carriers
  if (defaultValCxr.empty() && fp.itin()->hasNeutralValidatingCarrier(sp))
    construct.addAttribute(xml2::MultiNeutralValCxr, "T");

  // DCX
  if (!defaultValCxr.empty())
  {
    auto it = valCxrGsaData.validatingCarriersData().find(defaultValCxr);
    if (it != valCxrGsaData.validatingCarriersData().end())
      buildValidatingCarrier(construct, defaultValCxr, it->second, true);
  }

  // ACX
  for (const CarrierCode& vcxr : fp.validatingCarriers())
  {
    if (defaultValCxr == vcxr)
      continue;

    auto it = valCxrGsaData.validatingCarriersData().find(vcxr);
    if (it == valCxrGsaData.validatingCarriersData().end())
      continue;
    buildValidatingCarrier(construct, vcxr, it->second, false);
  }
  construct.closeElement();
}

// Builds VCL element and corresponding DCX element.
void
PricingResponseFormatter::buildValidatingCarrierList(const PricingTrx& pricingTrx,
                                                     XMLConstruct& construct,
                                                     const FarePath& farePath)
{
  const CarrierCode validatingCxr = farePath.defaultValidatingCarrier();
  if (validatingCxr.empty() && farePath.validatingCarriers().empty())
    return;

  construct.openElement(xml2::VcxrInfo);

  if (validatingCxr.empty() && farePath.validatingCarriers().size() > 1)
    construct.addAttribute(xml2::InhibitPq, "T");
  else
    construct.addAttribute(xml2::InhibitPq, "F");

  construct.addAttribute(xml2::SettlementMethod,
                         pricingTrx.countrySettlementPlanInfo()->getSettlementPlanTypeCode());
  construct.addAttribute(xml2::AtseProcess, "T"); // Silly, but it was requested; always true.

  // Indicator set to 'T' when we have no Default Validating Carriers and Multiple Neutral
  // Validating Carriers
  if (validatingCxr.empty() && farePath.itin()->hasNeutralValidatingCarrier())
    construct.addAttribute(xml2::MultiNeutralValCxr, "T");

  if (!validatingCxr.empty())
  {
    buildValidatingCarrier(construct, *(farePath.itin()), validatingCxr, true);
  }

  if (!farePath.validatingCarriers().empty())
  {
    for (const CarrierCode& validatingCxr : farePath.validatingCarriers())
    {
      buildValidatingCarrier(construct, *(farePath.itin()), validatingCxr);
    }
  }

  construct.closeElement();
}

void
PricingResponseFormatter::buildValidatingCarrier(XMLConstruct& construct,
                                                 const Itin& itin,
                                                 const CarrierCode& vcxr,
                                                 bool isDefaultValidatingCxr)
{
  if (isDefaultValidatingCxr)
    construct.openElement(xml2::DefaultValidatingCxr);
  else
    construct.openElement(xml2::AlternateValidatingCxr);

  construct.addAttribute(xml2::ValidatingCxrCode, vcxr);

  ValidatingCxrDataMap::const_iterator it =
      itin.validatingCxrGsaData()->validatingCarriersData().find(vcxr);
  if (it == itin.validatingCxrGsaData()->validatingCarriersData().end())
  {
    construct.closeElement();
    return;
  }
  const vcx::ValidatingCxrData& vcxrData = it->second;

  construct.addAttribute(xml2::TicketType, getTicketTypeText(vcxrData.ticketType));

  uint16_t pcxCount = 0;
  for (const vcx::ParticipatingCxr& pcx : vcxrData.participatingCxrs)
  {
    if (pcx.cxrName != vcxr)
    {
      buildPartcipatingCarrier(construct, pcx);
      if (++pcxCount >= vcx::MAX_PARTICIPATING_CARRIERS)
        break;
    }
  }

  construct.closeElement();
}

void
PricingResponseFormatter::buildValidatingCarrier(XMLConstruct& construct,
                                                 const CarrierCode& vcxr,
                                                 const vcx::ValidatingCxrData& vcxrData,
                                                 bool isDefaultValidatingCxr)
{
  if (isDefaultValidatingCxr)
    construct.openElement(xml2::DefaultValidatingCxr);
  else
    construct.openElement(xml2::AlternateValidatingCxr);

  construct.addAttribute(xml2::ValidatingCxrCode, vcxr);
  construct.addAttribute(xml2::TicketType, getTicketTypeText(vcxrData.ticketType));

  uint16_t pcxCount = 0;
  for (const vcx::ParticipatingCxr& pcx : vcxrData.participatingCxrs)
  {
    if (pcx.cxrName != vcxr)
    {
      buildPartcipatingCarrier(construct, pcx);
      if (++pcxCount >= vcx::MAX_PARTICIPATING_CARRIERS)
        break;
    }
  }

  construct.closeElement();
}

void
PricingResponseFormatter::buildPartcipatingCarrier(XMLConstruct& construct,
                                                   const vcx::ParticipatingCxr& pcx)
{
  if (pcx.cxrName.empty())
    return;

  construct.openElement(xml2::ParticipatingCxr);

  construct.addAttribute(xml2::ParticipatingCxrCode, pcx.cxrName);
  construct.addAttribute(xml2::AgreementTypeCode, getITAgreementTypeCodeText(pcx.agmtType));

  construct.closeElement();
}

void
PricingResponseFormatter::prepareSummaryForFullRefund(RefundPricingTrx& trx,
                                                      XMLConstruct& construct)
{
  construct.openElement(xml2::SummaryInfo);

  construct.addAttribute(xml2::TicketingDate, trx.currentTicketingDT().dateToSqlString());
  construct.addAttributeInteger(xml2::TicketingTime, trx.currentTicketingDT().totalMinutes());

  LocCode locCode;
  getSaleLoc(trx, locCode);
  construct.addAttribute(xml2::SalesLocation, locCode);
  getTicketLoc(trx, locCode);
  construct.addAttribute(xml2::TicketPointOverride, locCode);
  construct.addAttribute(xml2::ValidatingCarrier, getValidatingCarrier(trx));

  const uint16_t HOSTNAME_SIZE = 256;
  char hostname[HOSTNAME_SIZE];
  if (!gethostname(hostname, HOSTNAME_SIZE))
    construct.addAttribute(xml2::ServerHostname, hostname);

  preparePassengerForFullRefund(trx, construct);

  construct.closeElement();
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::preparePassengers
//----------------------------------------------------------------------------
void
PricingResponseFormatter::preparePassengers(PricingTrx& pricingTrx,
                                            FareCalcCollector& fareCalcCollector,
                                            XMLConstruct& construct)
{
  const FareCalcConfig& fcConfig = *(FareCalcUtil::getFareCalcConfig(pricingTrx));

  checkLimitOBFees(pricingTrx, fareCalcCollector);

  uint16_t paxNumber = 1;
  for (CalcTotals* calcTotals : fareCalcCollector.passengerCalcTotals())
  {
    if (calcTotals->farePath->processed())
      preparePassengerInfo(pricingTrx, fcConfig, *calcTotals, paxNumber++, construct);
  }
}

void
PricingResponseFormatter::prepareEndorsementMessages(PricingTrx& pricingTrx,
                                                     const FarePath* farePath,
                                                     XMLConstruct& construct)
{
  if (!farePath)
  {
    LOG4CXX_DEBUG(logger, "FarePath pointer = 0\n");
    return;
  }

  TicketingEndorsement::TicketEndoLines messages;
  TicketingEndorsement tktEndo;

  if (!fallback::fallbackEndorsementsRefactoring(&pricingTrx))
  {
    // this refactoring strongly depends on endorsementExpansion
    // both need to be OFF (value 'Y') to return to Endorse Cutter Limited
    tktEndo.collectEndorsements(pricingTrx, *farePath, messages, EndorseCutter());
  }
  else
  {
    std::shared_ptr<EndorseCutter> endoCutter(
          new EndorseCutterLimited(TicketingEndorsement::maxEndorsementMsgLen(pricingTrx)));

    // ToDo: when removing fallback change to std::unique_ptr
    if (!fallback::endorsementExpansion(&pricingTrx))
    {
      endoCutter = std::make_shared<EndorseCutterUnlimited>();
    }

    if (!fallback::fallbackFsc1155MoreThan147(&pricingTrx))
    {
      const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(pricingTrx);
      bool moreThan147 = !pricingTrx.getRequest()->isTicketEntry() && fcConfig->endorsements() == 'Y';

      if (moreThan147 && pricingTrx.getRequest()->ticketingAgent() &&
          pricingTrx.getRequest()->ticketingAgent()->infiniUser())
      {
        endoCutter = std::make_shared<EndorseCutterUnlimited>();
      }
    }

    tktEndo.collectEndorsements(pricingTrx, *farePath, messages, *endoCutter);
  }
  tktEndo.sortLinesByPrio(pricingTrx, *farePath, messages);

  for (TicketEndorseLine* line : messages)
    prepareMessage(construct,
                   Message::TYPE_ENDORSE,
                   0,
                   MCPCarrierUtil::swapToPseudo(&pricingTrx, line->carrier),
                   line->endorseMessage);

  // Cat35 trailer message/ Cat35 commission msg
  const CollectedNegFareData* cNegFareData = farePath->collectedNegFareData();
  if (cNegFareData != nullptr)
  {
    if (!cNegFareData->trailerMsg().empty() && !TrxUtil::cat35LtypeEnabled(pricingTrx))
    {
      prepareMessage(
          construct,
          Message::TYPE_ENDORSE,
          0,
          TicketingEndorsement::trimEndorsementMsg(pricingTrx, cNegFareData->trailerMsg()));
    }
    if (!cNegFareData->commissionMsg().empty())
    {
      prepareMessage(
          construct,
          Message::TYPE_ENDORSE,
          0,
          TicketingEndorsement::trimEndorsementMsg(pricingTrx, cNegFareData->commissionMsg()));
    }
  }
}

void
PricingResponseFormatter::prepareFOPwarningMessages(PricingTrx& pricingTrx,
                                                    CalcTotals& calcTotals,
                                                    XMLConstruct& construct)
{
  for (const auto& message : calcTotals.warningFopMsgs)
    prepareMessage(construct, Message::TYPE_WARNING, 0, getValidatingCarrier(pricingTrx), message);
}

void
PricingResponseFormatter::prepareGIMessage(PricingTrx& pricingTrx, XMLConstruct& construct)
{
  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&pricingTrx);
  if (noPNRTrx == nullptr)
    return;
  std::map<const TravelSeg*, const std::string>& globalWarnMap = noPNRTrx->GIWarningMap();

  std::vector<const std::string*> sortedMessages(noPNRTrx->itin().front()->travelSeg().size(),
                                                 nullptr);

  // sort the messages by travel segment order
  for (std::map<const TravelSeg*, const std::string>::const_iterator iter = globalWarnMap.begin(),
                                                                     end = globalWarnMap.end();
       iter != end;
       ++iter)
    sortedMessages.at(iter->first->segmentOrder() - 1) = &(iter->second);

  // display the messages in correct order
  for (const auto sortedMessage : sortedMessages)
    if (sortedMessage != nullptr)
    {
      FcMessage giMessage(FcMessage::WARNING, 0, *sortedMessage);
      prepareMessage(pricingTrx, construct, giMessage);
    }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareMessages
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareMessages(PricingTrx& pricingTrx,
                                          CalcTotals& calcTotals,
                                          XMLConstruct& construct)
{
  std::string msg = AdjustedSellingUtil::getADJSellingLevelMessage(pricingTrx, calcTotals);
  if (!msg.empty())
    prepareMessage(construct, FcMessage::GENERAL, 0, msg);

  msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(pricingTrx, calcTotals);

  if (!msg.empty())
    prepareMessage(construct, FcMessage::GENERAL, 0, msg);

  // Add currency endorsement message
  for (const std::string message : calcTotals.farePath->itin()->csTextMessages())
    prepareMessage(construct, Message::TYPE_CURRENCY, 0, message);

  prepareEndorsementMessages(pricingTrx, calcTotals.farePath, construct);
  prepareFOPwarningMessages(pricingTrx, calcTotals, construct);

  for (const FcMessage& message : calcTotals.fcMessage)
  {
    if (message.messageContent() == FcMessage::PVT_INDICATOR && !TrxUtil::swsPoAtsePath(pricingTrx))
      continue; // do not prepare "PVT" msg in detailed PXI

    if (message.messageContent() == FcMessage::SERVICE_FEE_TEMPLATE)
      continue;

    if (!fallback::fallbackFRRProcessingRetailerCode(&pricingTrx) &&
        (message.messageContent() == FcMessage::RETAILER_RULE_MESSAGE))
    {
      std::set<std::string> retailerCodes;
      AdjustedSellingUtil::getAllRetailerCodeFromFRR(calcTotals, retailerCodes);

      for (const std::string& retailerCode : retailerCodes)
        prepareMessage(construct, FcMessage::WARNING, 0,
                       "AGENCY RETAILER RULE QUALIFIER USED: " + retailerCode);

      continue;
    }

    if (TrxUtil::isBaggage302DotActivated(pricingTrx) &&
        std::string::npos != message.messageText().find(FreeBaggageUtil::BaggageTagHead))
    {
      std::vector<std::string> lines;
      BaggageResponseBuilder baggageBuilder(pricingTrx, calcTotals);
      baggageBuilder.getPqResponse(lines, &pricingTrx);

      for (const std::string& line : lines)
        prepareMessage(construct, FcMessage::BAGGAGE, 0, line);
    }
    else
      prepareMessage(pricingTrx, construct, message);
  }

  if (pricingTrx.getOptions() && pricingTrx.getRequest() &&
      pricingTrx.getRequest()->ticketingAgent() &&
      pricingTrx.getOptions()->getSpanishLargeFamilyDiscountLevel()
          != SLFUtil::DiscountLevel::NO_DISCOUNT)
  {
    std::string dfnMessage;
    if (LocUtil::isSpain(*(pricingTrx.getRequest()->ticketingAgent()->agentLocation())) &&
        LocUtil::isWholeTravelInSpain(pricingTrx.travelSeg()))
    {
      dfnMessage = "DFN APPLIED - PASSENGER DATA REQUIRED AT TICKETING";
    }
    else
    {
      dfnMessage = "DFN DOES NOT APPLY - VERIFY RESTRICTIONS";
    }
    prepareMessage(construct, FcMessage::WARNING, 0, dfnMessage);
  }

  if (TrxUtil::swsPoAtsePath(pricingTrx))
    prepareGIMessage(pricingTrx, construct);
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareTaxes
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareTaxes(PricingTrx& pricingTrx,
                                       CalcTotals& calcTotals,
                                       XMLConstruct& construct,
                                       bool& taxExemptProcessed)
{
  const DateTime& ticketingDate = pricingTrx.ticketingDate();

  for (const TaxRecord* taxRecordPtr : calcTotals.taxRecords())
  {
    const TaxRecord& taxRecord = *taxRecordPtr;

    if (taxRecord.taxOnChangeFee())
      continue;

    if (FareDisplayTax::isSupplementalFee(taxRecord.specialProcessNo()))
      continue;

    if (taxRecord.isTaxFeeExempt())
    {
      taxExemptProcessed = true;
      construct.openElement(xml2::TaxInformation);
      construct.addAttribute(xml2::ATaxCode, taxRecord.taxCode());
      construct.addAttribute(xml2::TaxAmount, "0");
      construct.addAttribute(xml2::StationCode, "TE");
      construct.addAttribute(xml2::PublishedCurrency, "TE");
      construct.closeElement();
    }
    else
    {
      if (taxRecord.getTaxAmount() < EPSILON) // 0 tax amount for non-exempted tax
        continue;

      construct.openElement(xml2::TaxInformation);

      construct.addAttribute(xml2::ATaxCode, taxRecord.taxCode());
      construct.addAttributeDouble(xml2::TaxAmount, taxRecord.getTaxAmount(), taxRecord.taxNoDec());
      construct.addAttribute(xml2::TaxCurrencyCode, taxRecord.taxCurrencyCode());
      if (taxRecord.taxCode().equalToConst("XF"))
      {
        bool bAirPort = false;
        for (PfcItem* pfcItem : calcTotals.pfcItems())
        {
          if (pfcItem->pfcAmount() == taxRecord.getTaxAmount())
          {
            bAirPort = true;
            construct.addAttribute(xml2::StationCode, pfcItem->pfcAirportCode());

            break;
          }
        }

        if (!bAirPort && !calcTotals.pfcItems().empty())
          construct.addAttribute(xml2::StationCode,
                                 calcTotals.pfcItems().front()->pfcAirportCode());
      }
      else
      {
        construct.addAttribute(xml2::StationCode, taxRecord.localBoard());
      }
      construct.addAttribute(xml2::ATaxDescription, taxRecord.taxDescription());

      Money moneyPub(taxRecord.publishedCurrencyCode());

      // 4 digits should cover all percentage taxes, before this fix precision was taken from
      // currency settings
      CurrencyNoDec noDec = taxRecord.taxType() != 'P' ? moneyPub.noDec(ticketingDate) : 4;

      construct.addAttributeDouble(xml2::AmountPublished, taxRecord.publishedAmount(), noDec);
      construct.addAttribute(xml2::PublishedCurrency, taxRecord.publishedCurrencyCode());

      if (taxRecord.gstTaxInd())
      {
        construct.addAttributeBoolean(xml2::GSTTax, taxRecord.gstTaxInd());
      }
      construct.addAttribute(xml2::TaxCountryCode, taxRecord.taxNation());

      construct.closeElement();
    }
  }

  if (!taxExemptProcessed)
    prepareExemptTaxes(pricingTrx, calcTotals, construct);

  if (!pricingTrx.getRequest())
    return;

  // Tax Breakdown elements
  std::string taxCodes = EMPTY_STRING();

  for (const TaxItem* taxItemPtr : calcTotals.taxItems())
  {
    const TaxItem& taxItem = *taxItemPtr;

    if (taxItem.failCode() || taxItem.taxAmount() < 0.01)
      continue;

    if (taxItem.taxOnChangeFee())
      continue;

    std::string taxCode = taxItem.taxCode();

    std::string::size_type pos = taxCodes.find(taxCode);

    if (pos != std::string::npos)
      continue;

    MoneyAmount moneyAmount = taxItem.taxAmount();

    if (taxItem.multioccconvrndInd() == YES)
    {
      taxCodes += taxCode;

      for (TaxRecord* taxRecord : calcTotals.taxRecords())
      {
        if (taxRecord->taxCode() == taxCode)
        {
          moneyAmount = taxRecord->getTaxAmount();
          break;
        }
      }
    }

    MoneyAmount amountPublished = taxItem.taxAmt();
    Money target(taxItem.taxCur());
    CurrencyNoDec amountPublishedNoDec = target.noDec(ticketingDate);
    if (taxItem.taxType() == Tax::PERCENTAGE)
    {
      Money source(taxItem.taxAmount(), taxItem.paymentCurrency());
      TaxUtil::convertTaxCurrency(pricingTrx, source, target);

      amountPublished = target.value();
    }

    construct.openElement(xml2::TaxBreakdown);
    construct.addAttribute(xml2::ATaxCode, taxItem.taxCode());
    construct.addAttributeDouble(xml2::TaxAmount, moneyAmount, taxItem.paymentCurrencyNoDec());
    construct.addAttribute(xml2::PublishedCurrency, taxItem.taxCur());
    construct.addAttributeDouble(xml2::AmountPublished, amountPublished, amountPublishedNoDec);
    construct.addAttribute(xml2::TaxAirlineCode,
                           MCPCarrierUtil::swapToPseudo(&pricingTrx, taxItem.carrierCode()));
    construct.addAttribute(xml2::StationCode, taxItem.taxLocalBoard());
    construct.addAttribute(xml2::TaxCurrencyCode, taxItem.paymentCurrency());
    construct.addAttributeChar(xml2::TbdTaxType, taxItem.taxType());

    if (taxItem.gstTax())
    {
      construct.addAttributeBoolean(xml2::GSTTax, taxItem.gstTax());
    }
    construct.addAttribute(xml2::TaxCountryCode, taxItem.nation());
    construct.addAttribute(xml2::ATaxDescription, taxItem.taxDescription());

    bool isExcTrx = (pricingTrx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
                     pricingTrx.excTrxType() == PricingTrx::AR_EXC_TRX ||
                     pricingTrx.excTrxType() == PricingTrx::NEW_WITHIN_ME ||
                     pricingTrx.excTrxType() == PricingTrx::EXC1_WITHIN_ME ||
                     pricingTrx.excTrxType() == PricingTrx::EXC2_WITHIN_ME);
    if (isExcTrx)
    {
      // construct.addAttributeChar(xml2::ReissueTaxType, taxItem.reissueTaxType());
      construct.addAttributeBoolean(xml2::ReissueRestrictionApply,
                                    taxItem.reissueTaxInfo().reissueRestrictionApply);
      construct.addAttributeBoolean(xml2::TaxApplyToReissue,
                                    taxItem.reissueTaxInfo().taxApplyToReissue);
      construct.addAttributeBoolean(xml2::ReissueTaxRefundable,
                                    taxItem.reissueTaxInfo().reissueTaxRefundable);
    }

    if (isExcTrx || pricingTrx.excTrxType() == PricingTrx::AF_EXC_TRX)
    {
      if (taxItem.reissueTaxInfo().reissueTaxAmount != 0)
      {
        construct.addAttribute(xml2::ReissueTaxCurrency,
                               taxItem.reissueTaxInfo().reissueTaxCurrency);
        construct.addAttributeDouble(xml2::ReissueTaxAmount,
                                     taxItem.reissueTaxInfo().reissueTaxAmount,
                                     calcTotals.taxNoDec());
      }
      if (taxItem.partialTax() || taxItem.mixedTax())
        construct.addAttributeChar(xml2::TaxType, 'M');
      else
        construct.addAttributeChar(xml2::TaxType, taxItem.taxType());

      prepareMinMaxTaxInfo(construct, taxItem);

      if (TrxUtil::isAutomatedRefundCat33Enabled(pricingTrx))
      {
        construct.addAttributeChar(xml2::RefundableTaxTag, taxItem.getRefundableTaxTag());
      }
    }

    construct.closeElement();
  }
  for (const TaxOverride* taxOverride : pricingTrx.getRequest()->taxOverride())
  {
    construct.openElement(xml2::TaxBreakdown);
    construct.addAttribute(xml2::ATaxCode, taxOverride->taxCode().substr(0, 2));
    construct.addAttributeDouble(xml2::TaxAmount, taxOverride->taxAmt(), calcTotals.taxNoDec());
    construct.addAttribute(xml2::TaxCurrencyCode, calcTotals.taxCurrencyCode());
    construct.addAttribute(xml2::PublishedCurrency, "OV");
    construct.closeElement();
  }

  TaxRound taxRound;

  for (PfcItem* pfcItemPtr : calcTotals.pfcItems())
  {
    PfcItem& pfcItem = *pfcItemPtr;
    MoneyAmount amount = pfcItem.pfcAmount();
    CurrencyNoDec amountNoDec = pfcItem.pfcDecimals();
    if (pfcItem.pfcCurrencyCode() != calcTotals.taxCurrencyCode())
    {
      CurrencyConversionFacade ccFacade;
      Money targetMoney(calcTotals.taxCurrencyCode());
      Money sourceMoney(pfcItem.pfcAmount(), pfcItem.pfcCurrencyCode());

      if (!ccFacade.convert(
              targetMoney, sourceMoney, pricingTrx, false, CurrencyConversionRequest::TAXES))
      {
        LOG4CXX_WARN(logger, "Currency Convertion Error");
      }
      amount = targetMoney.value();
      amountNoDec = targetMoney.noDec(ticketingDate);

      CurrencyConverter currencyConverter;
      RoundingFactor roundingUnit = 0.1;
      RoundingRule roundingRule = NONE;

      taxRound.retrieveNationRoundingSpecifications(
          pricingTrx, roundingUnit, amountNoDec, roundingRule);

      Money targetMoney2(amount, calcTotals.taxCurrencyCode());
      amountNoDec = targetMoney2.noDec(ticketingDate);

      if (currencyConverter.round(targetMoney2, roundingUnit, roundingRule))
        amount = targetMoney2.value();
    }
    construct.openElement(xml2::TaxBreakdown);
    construct.addAttribute(xml2::ATaxCode, "XF");
    construct.addAttributeDouble(xml2::TaxAmount, amount, amountNoDec);
    construct.addAttribute(xml2::TaxCurrencyCode, calcTotals.equivCurrencyCode);
    construct.addAttributeDouble(xml2::AmountPublished, pfcItem.pfcAmount(), pfcItem.pfcDecimals());
    construct.addAttribute(xml2::StationCode, pfcItem.pfcAirportCode());
    construct.addAttribute(xml2::TaxCountryCode, "US");
    construct.addAttribute(xml2::ATaxDescription, "PASSENGER FACILITY CHARGES");
    construct.addAttribute(xml2::PublishedCurrency, pfcItem.pfcCurrencyCode());
    construct.closeElement();
  }

  for (const TaxItem* taxItemPtr : calcTotals.taxItems())
  {
    const TaxItem& taxItem = *taxItemPtr;

    if (taxItem.taxOnChangeFee())
      continue;

    if ((taxItem.failCode() != TaxItem::EXEMPT_ALL_TAXES) &&
        (taxItem.failCode() != TaxItem::EXEMPT_SPECIFIED_TAXES))
      continue;

    if (FareDisplayTax::isSupplementalFee(taxItem.specialProcessNo()))
      continue;

    construct.openElement(xml2::TaxExempt);
    construct.addAttribute(xml2::ATaxCode, taxItem.taxCode());
    construct.addAttributeDouble(
        xml2::TaxAmount, taxItem.taxExemptAmount(), taxItem.paymentCurrencyNoDec());
    construct.addAttribute(xml2::TaxAirlineCode,
                           MCPCarrierUtil::swapToPseudo(&pricingTrx, taxItem.carrierCode()));
    construct.addAttribute(xml2::StationCode, taxItem.taxLocalBoard());
    construct.addAttribute(xml2::TaxCurrencyCode, taxItem.paymentCurrency());
    construct.addAttributeChar(xml2::TbdTaxType, taxItem.taxType());

    if (pricingTrx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
        pricingTrx.excTrxType() == PricingTrx::AR_EXC_TRX ||
        pricingTrx.excTrxType() == PricingTrx::NEW_WITHIN_ME ||
        pricingTrx.excTrxType() == PricingTrx::EXC1_WITHIN_ME ||
        pricingTrx.excTrxType() == PricingTrx::EXC2_WITHIN_ME ||
        pricingTrx.excTrxType() == PricingTrx::AF_EXC_TRX)
    {
      prepareMinMaxTaxInfo(construct, taxItem);
    }

    construct.closeElement();
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareExemptTaxes
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareExemptTaxes(PricingTrx& pricingTrx,
                                             CalcTotals& calcTotals,
                                             XMLConstruct& construct)
{
  std::set<TaxCode>::const_iterator taxCodeIter;
  for (taxCodeIter = calcTotals.getTaxExemptCodes().begin();
       taxCodeIter != calcTotals.getTaxExemptCodes().end();
       ++taxCodeIter)
  {
    construct.openElement(xml2::TaxInformation);

    construct.addAttribute(xml2::ATaxCode, (*taxCodeIter));
    construct.addAttribute(xml2::TaxAmount, "0");
    construct.addAttribute(xml2::StationCode, "TE");
    construct.addAttribute(xml2::PublishedCurrency, "TE");

    construct.closeElement();
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareTaxesOnChangeFee
//----------------------------------------------------------------------------

// depricated, should be removed with fallback EXTRACT_REISSUE_EXCHANGE_FORMATTER
void
PricingResponseFormatter::prepareTaxesOnChangeFee(PricingTrx& pricingTrx,
                                                  CalcTotals& calcTotals,
                                                  XMLConstruct& construct)
{
  if (!pricingTrx.getRequest())
    return;

  for (const TaxItem* taxItemPtr : calcTotals.changeFeeTaxItems())
  {
    const TaxItem& taxItem = *taxItemPtr;

    if (taxItem.failCode())
      continue;

    MoneyAmount amountPublished = taxItem.taxAmt();

    Money target(taxItem.taxCur());
    CurrencyNoDec amountPublishedNoDec = target.noDec(pricingTrx.ticketingDate());
    if (taxItem.taxType() == Tax::PERCENTAGE)
    {
      Money source(taxItem.taxAmount(), taxItem.paymentCurrency());
      TaxUtil::convertTaxCurrency(pricingTrx, source, target);

      amountPublished = target.value();
    }

    construct.openElement(xml2::TaxBreakdown);
    construct.addAttribute(xml2::ATaxCode, taxItem.taxCode());
    construct.addAttributeDouble(
        xml2::TaxAmount, taxItem.taxAmount(), taxItem.paymentCurrencyNoDec());
    construct.addAttribute(xml2::PublishedCurrency, taxItem.taxCur());
    construct.addAttributeDouble(xml2::AmountPublished, amountPublished, amountPublishedNoDec);
    construct.addAttribute(xml2::StationCode, taxItem.taxLocalBoard());
    construct.addAttribute(xml2::TaxCurrencyCode, taxItem.paymentCurrency());
    construct.addAttributeChar(xml2::TbdTaxType, taxItem.taxType());
    construct.addAttribute(xml2::TaxCountryCode, taxItem.nation());
    construct.addAttribute(xml2::ATaxDescription, taxItem.taxDescription());
    construct.addAttribute(xml2::MinMaxTaxCurrency, taxItem.minMaxTaxCurrency());
    construct.addAttributeDouble(xml2::TaxRateUsed, taxItem.taxAmt(), taxItem.taxNodec());

    construct.closeElement();
  }

  for (const TaxItem* taxItem : calcTotals.changeFeeTaxItems())
  {
    if ((taxItem->failCode() != TaxItem::EXEMPT_ALL_TAXES) &&
        (taxItem->failCode() != TaxItem::EXEMPT_SPECIFIED_TAXES))
      continue;

    construct.openElement(xml2::TaxExempt);
    construct.addAttribute(xml2::ATaxCode, taxItem->taxCode());
    construct.addAttributeDouble(
        xml2::TaxAmount, taxItem->taxExemptAmount(), taxItem->paymentCurrencyNoDec());
    construct.addAttribute(xml2::TaxAirlineCode,
                           MCPCarrierUtil::swapToPseudo(&pricingTrx, taxItem->carrierCode()));
    construct.addAttribute(xml2::StationCode, taxItem->taxLocalBoard());
    construct.addAttribute(xml2::TaxCurrencyCode, taxItem->paymentCurrency());
    construct.addAttributeChar(xml2::TbdTaxType, taxItem->taxType());
    construct.addAttribute(xml2::MinMaxTaxCurrency, taxItem->minMaxTaxCurrency());
    construct.addAttributeDouble(xml2::TaxRateUsed, taxItem->taxAmt(), taxItem->taxNodec());

    construct.closeElement();
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareTaxBSR
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareTaxBSR(PricingTrx& pricingTrx,
                                        CalcTotals& calcTotals,
                                        const CurrencyNoDec& noDec,
                                        XMLConstruct& construct)
{
  std::set<std::string> exchangeSet;

  for (const TaxItem* taxItem : calcTotals.taxItems())
  {
    const std::string& base = taxItem->taxCur();
    const std::string& target = taxItem->paymentCurrency();
    const std::string key(base + target);

    if ((base != target) && (exchangeSet.count(key) == 0) &&
        ((taxItem->exchangeRate1() > 0.0) || (taxItem->exchangeRate2() > 0.0)))
    {
      construct.openElement(xml2::TaxBankerSellRate);
      construct.openElement(xml2::CurrencyConversionInformation);

      construct.addAttribute(xml2::From, base);
      construct.addAttribute(xml2::To, target);

      if (TrxUtil::isIcerActivated(pricingTrx))
      {
        construct.addAttributeDouble(
            xml2::ExchangeRateOne, (1 / taxItem->exchangeRate1()), taxItem->exchangeRate1NoDec());
        construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne,
                                      taxItem->exchangeRate1NoDec());
      }
      else
      {
        if (taxItem->intermediateCurrency().empty() && taxItem->taxType() == Tax::FIXED &&
            taxItem->taxAmt() > taxItem->taxAmount())
        {
          construct.addAttributeDouble(
              xml2::ExchangeRateOne, (1 / taxItem->exchangeRate1()), exchangeRatePrecision + 1);
        }
        else
        {
          construct.addAttributeDouble(
              xml2::ExchangeRateOne, taxItem->exchangeRate1(), exchangeRatePrecision + 1);
        }
        construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne,
                                      exchangeRatePrecision);
        if (taxItem->exchangeRate2() > 0.0 && !taxItem->intermediateCurrency().empty())
        {
          construct.addAttribute(xml2::IntermediateCurrency, taxItem->intermediateCurrency());
          if (taxItem->taxAmount() < taxItem->intermediateAmount())
          {
            construct.addAttributeDouble(
                xml2::ExchangeRateTwo, (1 / taxItem->exchangeRate2()), exchangeRatePrecision + 1);
          }
          else
          {
            construct.addAttributeDouble(
                xml2::ExchangeRateTwo, taxItem->exchangeRate2(), exchangeRatePrecision + 1);
          }
          construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateTwo,
                                        exchangeRatePrecision);
        }
      }

      construct.closeElement();
      construct.closeElement();

      exchangeSet.insert(key);
    }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareFareIATARate
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareFareIATARate(PricingTrx& pricingTrx,
                                              CalcTotals& calcTotals,
                                              XMLConstruct& construct)
{
  if (pricingTrx.getOptions() && pricingTrx.getOptions()->returnAllData() == GDS)
  {
    std::map<uint16_t, TravelSeg*, std::less<uint16_t>>::const_iterator j =
        calcTotals.travelSegs.begin();

    TravelSeg* travelSeg = (*j).second;
    const Loc* agentLocation = nullptr;
    if (pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent())
      agentLocation = pricingTrx.getRequest()->ticketingAgent()->agentLocation();

    if (agentLocation && LocUtil::isUS(*agentLocation) && LocUtil::isUS(*travelSeg->origin()))
      return;
  }

  construct.openElement(xml2::FareIATARate);

  construct.openElement(xml2::CurrencyConversionInformation);

  construct.addAttribute(xml2::From, calcTotals.convertedBaseFareCurrencyCode);

  // This is hard coded in here to fix a WPDF especific ROE display problem
  // Infini project PL 22740
  int roeNumberDecimalPlaces = 10;
  construct.addAttributeDouble(
      xml2::ExchangeRateOne, calcTotals.roeRate, roeNumberDecimalPlaces + 1);
  construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne, roeNumberDecimalPlaces);

  if (TrxUtil::swsPoAtsePath(pricingTrx))
  {
    addDateAttr(calcTotals.effectiveDate, xml2::EffectiveDate, construct);
    addDateAttr(calcTotals.discontinueDate, xml2::DiscontinueDate, construct);
  }
  else
  {
    if (calcTotals.effectiveDate.isValid())
      construct.addAttribute(xml2::EffectiveDate,
                             calcTotals.effectiveDate.dateToString(DDMMMYY, nullptr));

    if (calcTotals.discontinueDate.dateToString(DDMMMYY, nullptr) == NO_DATE_STRING)
      construct.addAttribute(xml2::DiscontinueDate, "INDEF");
    else
      construct.addAttribute(xml2::DiscontinueDate,
                             calcTotals.discontinueDate.dateToString(DDMMMYY, nullptr));
  }

  construct.closeElement();
  construct.closeElement();
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareFareBSR
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareFareBSR(PricingTrx& pricingTrx,
                                         CalcTotals& calcTotals,
                                         XMLConstruct& construct)
{
  const char OVERRIDE_CONVERSION = 'O';

  if (pricingTrx.getOptions() && pricingTrx.getOptions()->returnAllData() == GDS)
  {
    std::map<uint16_t, TravelSeg*, std::less<uint16_t>>::const_iterator j =
        calcTotals.travelSegs.begin();

    TravelSeg* travelSeg = (*j).second;
    const Loc* agentLocation = nullptr;
    if (pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent())
      agentLocation = pricingTrx.getRequest()->ticketingAgent()->agentLocation();

    if (agentLocation && LocUtil::isUS(*agentLocation) && LocUtil::isUS(*travelSeg->origin()))
      return;
  }

  if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    return;

  construct.openElement(xml2::FareBankerSellRate);

  construct.openElement(xml2::CurrencyConversionInformation);

  construct.addAttribute(xml2::From, calcTotals.convertedBaseFareCurrencyCode);
  construct.addAttribute(xml2::To, calcTotals.equivCurrencyCode);

  if (TrxUtil::isIcerActivated(pricingTrx))
  {
    if (pricingTrx.getRequest()->rateAmountOverride() > 0 ||
        pricingTrx.getRequest()->secondRateAmountOverride() > 0)
    {
      construct.addAttributeDouble(
          xml2::ExchangeRateOne, calcTotals.bsrRate1, exchangeRatePrecision + 1);
      construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne,
                                    exchangeRatePrecision);
    }
    else
    {
      construct.addAttributeDouble(
          xml2::ExchangeRateOne, calcTotals.bsrRate1, calcTotals.bsrRate1NoDec);
      construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne,
                                    calcTotals.bsrRate1NoDec);
    }
  }
  // Always display FROM currency based on Agent currency
  else if ((pricingTrx.getOptions() &&
            calcTotals.equivCurrencyCode == pricingTrx.getOptions()->currencyOverride()) ||
           (pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent() &&
            calcTotals.equivCurrencyCode ==
                pricingTrx.getRequest()->ticketingAgent()->currencyCodeAgent()))
  {
    construct.addAttributeDouble(xml2::ExchangeRateOne,
                                 calcTotals.bsrRate1 == 0 ? 999 : 1 / calcTotals.bsrRate1,
                                 exchangeRatePrecision + 1);
    construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne, exchangeRatePrecision);

    if (calcTotals.bsrRate2 > 0.0)
    {
      construct.addAttribute(xml2::IntermediateCurrency, calcTotals.interCurrencyCode);
      construct.addAttributeDouble(xml2::ExchangeRateTwo,
                                   calcTotals.bsrRate2 == 0 ? 999 : 1 / calcTotals.bsrRate2,
                                   exchangeRatePrecision + 1);
      construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateTwo,
                                    exchangeRatePrecision);
    }
    construct.addAttributeChar(xml2::ConversionType, OVERRIDE_CONVERSION);
  }
  else
  {
    construct.addAttributeDouble(
        xml2::ExchangeRateOne, calcTotals.bsrRate1, exchangeRatePrecision + 1);
    construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne, exchangeRatePrecision);

    if (calcTotals.bsrRate2 > 0.0)
    {
      construct.addAttribute(xml2::IntermediateCurrency, calcTotals.interCurrencyCode);
      construct.addAttributeDouble(
          xml2::ExchangeRateTwo, calcTotals.bsrRate2, exchangeRatePrecision + 1);
      construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateTwo,
                                    exchangeRatePrecision);
    }
  }
  construct.closeElement();

  construct.closeElement();
}

/// Creates CAL element in PXI. Each CAL maps to FareComponent
void
PricingResponseFormatter::prepareFareCalcs(PricingTrx& pricingTrx,
                                           CalcTotals& calcTotals,
                                           const FareUsage& fareUsage,
                                           const PricingUnit& pricingUnit,
                                           const CurrencyNoDec& noDecCalc,
                                           const FarePath& farePath,
                                           bool stopoverFlag,
                                           uint16_t puCount,
                                           uint16_t fcCount,
                                           uint16_t& segmentOrder,
                                           XMLConstruct& construct,
                                           std::vector<const FareUsage*>& fuPlusUpsShown,
                                           std::vector<MoneyAmount>& faeValues,
                                           std::vector<MoneyAmount>& ftsValues,
                                           const FuFcIdMap& fuFcIdCol)
{
  FareCalcModel fareCalcModel(pricingTrx,
                              fareUsage,
                              calcTotals,
                              farePath,
                              pricingUnit,
                              stopoverFlag,
                              puCount,
                              fcCount,
                              faeValues,
                              ftsValues);
  if (fareCalcModel.validate())
  {
    uint16_t fcId = 0;
    if (!fallback::fallbackAMCPhase2(&pricingTrx))
    {
      const auto it = fuFcIdCol.find(const_cast<FareUsage*>(&fareUsage));
      if (it != fuFcIdCol.end())
        fcId = it->second;
    }

    XMLFareCalcFormatter fareCalcFormatter(fareCalcModel, construct, *this);
    fareCalcFormatter.formatCAL(pricingTrx,
                                calcTotals,
                                fareUsage,
                                pricingUnit,
                                noDecCalc,
                                farePath,
                                segmentOrder,
                                construct,
                                fuPlusUpsShown,
                                fcId);
  }
}

void
PricingResponseFormatter::addSearchForBrandsPricingFareBrandDetails(PricingTrx& pricingTrx,
                                                                    CalcTotals& calcTotals,
                                                                    const FareUsage& fareUsage,
                                                                    XMLConstruct& construct) const
{
  TSE_ASSERT(fareUsage.paxTypeFare() != nullptr);
  const PaxTypeFare& paxTypeFare = *(fareUsage.paxTypeFare());

  TSE_ASSERT(calcTotals.farePath != nullptr);
  TSE_ASSERT(calcTotals.farePath->itin() != nullptr);

  const Itin& itin = *(calcTotals.farePath->itin());

  TSE_ASSERT(!fareUsage.travelSeg().empty());
  const int16_t firstFuSegOrder = itin.segmentOrder(fareUsage.travelSeg().front());

  // in multiple brand mode we need to know brand context we're in
  // in case a fare is valid for more than one brand we have to display one actually used.
  const uint16_t brandIndex = calcTotals.farePath->brandIndex();

  if (brandIndex == INVALID_BRAND_INDEX)
    return;

  const uint16_t segmentIndex = firstFuSegOrder - 1;
  const CarrierCode& carrierCode = paxTypeFare.fareMarket()->governingCarrier();

  const skipper::ItinBranding& itinBranding = itin.getItinBranding();

  Direction direction = Direction::BOTHWAYS;
  if (BrandingUtil::isDirectionalityToBeUsed(pricingTrx))
    direction = fareUsage.paxTypeFare()->getDirection();
  BrandCode brandCode;
  brandCode = itinBranding.getBrandCode(
    brandIndex, segmentIndex, carrierCode, direction, fareUsage.getFareUsageDirection());

  // If fare that is used is not valid for brandCombination it is used in, something is seriously
  // wrong:)
  TSE_ASSERT(paxTypeFare.isValidForBrand(pricingTrx, &brandCode));

  // In NO_BRAND ( the cheapest scenario ) we still want to display brands if they are defined for a
  // given fare
  const BrandCode fareBrandCode = (brandCode != NO_BRAND) ?
    brandCode : paxTypeFare.getFirstValidBrand(pricingTrx, fareUsage.getFareUsageDirection());

  if (fareBrandCode.empty())
    return;

  const unsigned int index = paxTypeFare.getValidBrandIndex(pricingTrx, &fareBrandCode,
                                                            fareUsage.getFareUsageDirection());

  if (index == INVALID_BRAND_INDEX)
    return;

  const QualifiedBrand& qb = pricingTrx.brandProgramVec().at(index);
  if (xform::formatBrandProgramData(construct, qb, fareBrandCode))
  {
    // TODO(karol.jurek): remove this attribute (SC0)
    construct.addAttribute(xml2::ProgramCode,
                           boost::to_upper_copy(std::string(qb.first->programCode())));
  }
  else
  {
    LOG4CXX_ERROR(logger, "Could not format brand program data");
  }
}

void
PricingResponseFormatter::formatElementsInCAL(PricingTrx& pricingTrx,
                                              CalcTotals& calcTotals,
                                              const FareUsage& fareUsage,
                                              const PricingUnit& pricingUnit,
                                              const CurrencyNoDec& noDecCalc,
                                              const FarePath& farePath,
                                              uint16_t& segmentOrder,
                                              XMLConstruct& construct,
                                              std::vector<const FareUsage*>& fuPlusUpsShown,
                                              CalcTotals::FareCompInfo& fareCompInfo)
{
  if (!fallback::fallbackFRRProcessingRetailerCode(&pricingTrx))
  {
    std::string fareRetailerCode = AdjustedSellingUtil::getFareRetailerCodeForNet(fareUsage);
    if (!fareRetailerCode.empty())
      construct.addAttribute(xml2::FareRetailerCodeNet, fareRetailerCode);

    std::string fareRetailerCodeAdj = AdjustedSellingUtil::getFareRetailerCodeForAdjusted(fareUsage);
    if (!fareRetailerCodeAdj.empty())
      construct.addAttribute(xml2::FareRetailerCodeAdjusted, fareRetailerCodeAdj);
  }

  if (pricingTrx.activationFlags().isSearchForBrandsPricing())
    addSearchForBrandsPricingFareBrandDetails(pricingTrx, calcTotals, fareUsage, construct);
  else if (!fallback::fallbackBrandedFaresPricing(&pricingTrx))
    addFareBrandDetails(pricingTrx, *(fareUsage.paxTypeFare()), construct, fareUsage.getFareUsageDirection());

  formatXMLTaxSplit(pricingTrx, calcTotals, fareUsage, construct);

  if (_buildPlusUp)
  {
    prepareFareUsagePlusUps(fareUsage, noDecCalc, construct, fuPlusUpsShown);
  }

  // Prepare plus up from Net Remit
  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    FareUsage* netRemitFu1 = nullptr;
    FareUsage* netRemitFu2 = nullptr;
    CalcTotals::getNetRemitFareUsage(&farePath, &fareUsage, netRemitFu1, netRemitFu2);
    if (netRemitFu1 != nullptr)
    {
      construct.openElement(xml2::NetRemitInfo);

      prepareFareUsagePlusUps(*netRemitFu1, noDecCalc, construct, fuPlusUpsShown);

      if (netRemitFu2 != nullptr)
        prepareFareUsagePlusUps(*netRemitFu2, noDecCalc, construct, fuPlusUpsShown);

      construct.closeElement();
    }
  }

  if (isTrxInProperVersion(pricingTrx, PricingTrx::WP, 1, 1, 3) || isAltPricingTrxInVer(pricingTrx))
  {
    construct.addAttributeShort(xml2::PublicPrivateFare,
                                fareUsage.paxTypeFare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF
                                    ? RuleConst::PRIVATE_TARIFF
                                    : RuleConst::PUBLIC_TARIFF);
  }

  prepareRuleCategoryIndicator(pricingTrx, fareUsage, construct);

  prepareDifferential(pricingTrx, fareUsage, noDecCalc, construct);

  if (pricingTrx.excTrxType() == PricingTrx::NOT_EXC_TRX)
  {
    // Add ERD tags in  Pricing Response
    ERDSectionFormatter erd(pricingTrx, calcTotals, fareUsage, fareCompInfo, construct);
    erd.prepareERD();
  }

  prepareSegments(
      pricingTrx, calcTotals, fareUsage, farePath, pricingUnit, segmentOrder, noDecCalc, construct);

  if (!fallback::fallbackFRRProcessingRetailerCode(&pricingTrx))
  {
    prepareHPUForNet(pricingTrx, fareUsage, construct);

    prepareHPUForAdjusted(pricingTrx, fareUsage, construct);
  }
}

void
PricingResponseFormatter::formatXMLTaxSplit(PricingTrx& pricingTrx,
                                            CalcTotals& calcTotals,
                                            const FareUsage& fareUsage,
                                            XMLConstruct& construct) const
{
  if (TrxUtil::isSplitTaxesByFareComponentEnabled(pricingTrx) &&
      pricingTrx.getOptions()->isSplitTaxesByFareComponent())
  {
    XMLTaxSplitFormatter formatter(construct, pricingTrx);
    TaxSplitModel model(pricingTrx, calcTotals);

    if (!model.contains(&fareUsage))
      return;

    for (const std::shared_ptr<AbstractTaxSummaryInfo>& each : model.getTaxSummaryInfo(fareUsage))
      formatter.formatTAX(*each);

    for (const std::shared_ptr<AbstractTaxSplitData>& each : model.getTaxBreakdown(fareUsage))
      formatter.formatTBD(*each);
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::addBrandDetails
//----------------------------------------------------------------------------
void
PricingResponseFormatter::addFareBrandDetails(const PricingTrx& pricingTrx,
                                              const PaxTypeFare& pax,
                                              XMLConstruct& construct,
                                              Direction fareUsageDirection) const
{
  if (!pricingTrx.itin().empty() && pricingTrx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS)
  {
    std::vector<BrandCode> brands;
    pax.getValidBrands(pricingTrx, brands);

    if (!brands.empty())
    {
      BrandCode bc = brands.front();
      const size_t index = pax.getValidBrandIndex(pricingTrx, &bc, fareUsageDirection);
      if (index < pricingTrx.brandProgramVec().size())
      {
        const QualifiedBrand& qb = pricingTrx.brandProgramVec()[index];
        if (xform::formatBrandProgramData(construct, qb, bc))
        {
          // TODO(karol.jurek): remove this attribute (SC0)
          construct.addAttribute(xml2::ProgramCode,
                                 boost::to_upper_copy(std::string(qb.first->programCode())));
        }
        else
        {
          LOG4CXX_ERROR(logger, "Could not format brand program data");
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareSegments
//----------------------------------------------------------------------------
void
PricingResponseFormatter::addOAndDTags(PricingTrx& pricingTrx,
                                       const FarePath& farePath,
                                       const Itin* itin,
                                       const TravelSeg& tvlSeg,
                                       const FareUsage& fareUsage,
                                       const bool rebooked,
                                       XMLConstruct& construct)
{
  if (rebooked && pricingTrx.getRequest()->isLowFareRequested())
  {
    const OAndDMarket* od = JourneyUtil::getOAndDMarketFromSegment(&tvlSeg, itin, &farePath);
    if (od && od->validAfterRebook())
    {
      char tmpBuf[10];
      sprintf(tmpBuf, "%d", od->fareMarket()->travelSeg().front()->pnrSegment());
      construct.addAttribute(xml2::OAndDMarketBeginSegOrder, tmpBuf);

      sprintf(tmpBuf, "%d", od->fareMarket()->travelSeg().back()->pnrSegment());
      construct.addAttribute(xml2::OAndDMarketEndSegOrder, tmpBuf);
    }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareSegments
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareSegmentsCity(PricingTrx& pricingTrx,
                                              const FarePath& farePath,
                                              const Itin* itin,
                                              const TravelSeg& tvlSeg,
                                              const FareUsage& fareUsage,
                                              const bool rebooked,
                                              XMLConstruct& construct)
{
  construct.openElement(xml2::SegmentInformation);

  const LocCode& orgLoc = (itin == nullptr || itin->geoTravelType() == GeoTravelType::Domestic)
                              ? tvlSeg.boardMultiCity()
                              : pricingTrx.dataHandle().getMultiTransportCityCode(
                                    tvlSeg.origAirport(),
                                    fareUsage.paxTypeFare()->fareMarket()->governingCarrier(),
                                    itin ? itin->geoTravelType() : tvlSeg.geoTravelType(),
                                    pricingTrx.ticketingDate());

  const LocCode& dstLoc = (itin == nullptr || itin->geoTravelType() == GeoTravelType::Domestic)
                              ? tvlSeg.offMultiCity()
                              : pricingTrx.dataHandle().getMultiTransportCityCode(
                                    tvlSeg.destAirport(),
                                    fareUsage.paxTypeFare()->fareMarket()->governingCarrier(),
                                    itin ? itin->geoTravelType() : tvlSeg.geoTravelType(),
                                    pricingTrx.ticketingDate());

  if (orgLoc.empty())
    construct.addAttribute(xml2::SegmentDepartureCity, tvlSeg.boardMultiCity());
  else
    construct.addAttribute(xml2::SegmentDepartureCity, orgLoc);

  construct.addAttribute(xml2::SegmentDepartureAirport, tvlSeg.origAirport());

  if (dstLoc.empty())
    construct.addAttribute(xml2::SegmentArrivalCity, tvlSeg.offMultiCity());
  else
    construct.addAttribute(xml2::SegmentArrivalCity, dstLoc);

  construct.addAttribute(xml2::SegmentArrivalAirport, tvlSeg.destAirport());

  addOAndDTags(pricingTrx, farePath, itin, tvlSeg, fareUsage, rebooked, construct);
}

void
PricingResponseFormatter::prepareSegments(PricingTrx& pricingTrx,
                                          CalcTotals& calcTotals,
                                          const FareUsage& fareUsage,
                                          const FarePath& farePath,
                                          const PricingUnit& pricingUnit,
                                          uint16_t& segmentOrder,
                                          const CurrencyNoDec& noDecCalc,
                                          XMLConstruct& construct)
{
  if (fareUsage.travelSeg().empty())
    return;

  bool displayTfdpsc = displayTfdPsc(pricingTrx, fareUsage);

  std::vector<TravelSeg*>::const_iterator tvlSegIter = fareUsage.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegIterEnd = fareUsage.travelSeg().end();
  std::vector<TravelSeg*>::const_iterator tvlSegIterLast = fareUsage.travelSeg().end() - 1;

  bool cat5RebookRequired = false;
  uint16_t segmentNumber = 0;
  const Itin* itin = calcTotals.farePath->itin();

  if (RtwUtil::isRtwArunk(pricingTrx, *tvlSegIterLast))
  {
    --tvlSegIterEnd;
    --tvlSegIterLast;
  }

  for (; tvlSegIter != tvlSegIterEnd; tvlSegIter++) // segmentOrder++ ?
  {
    if (itin->segmentOrder(*tvlSegIter) == segmentOrder)
      break;
  }

  for (; tvlSegIter != tvlSegIterEnd; tvlSegIter++, segmentOrder++) // segmentOrder++ ?
  {
    cat5RebookRequired = false;
    TravelSeg& tvlSeg = **tvlSegIter;

    bool isFareBreak = (tvlSegIter == tvlSegIterLast);

    if (itin->segmentOrder(*tvlSegIter) != segmentOrder)
      return;

    std::string bookingCode = tvlSeg.getBookingCode();

    segmentNumber = itin->segmentOrder((*tvlSegIter)) - 1;

    bool rebooked = false;
    if (segmentNumber < calcTotals.bookingCodeRebook.size())
    {
      if (!calcTotals.bookingCodeRebook[segmentNumber].empty())
      {
        bookingCode = calcTotals.bookingCodeRebook[segmentNumber];
        if (pricingTrx.getRequest() && pricingTrx.getRequest()->isLowFareRequested()) // WPNC entry
        {
          rebooked = true;

          if (tvlSeg.getBookingCode() == calcTotals.bookingCodeRebook[segmentNumber])
            cat5RebookRequired = true;
        }
      }
    }

    prepareSegmentsCity(pricingTrx, farePath, itin, tvlSeg, fareUsage, rebooked, construct);

    if (isTrxInProperVersion(pricingTrx, PricingTrx::WP, 1, 1, 5) ||
        isAltPricingTrxInVer(pricingTrx))
      addDateAttr(tvlSeg.departureDT(), xml2::SegmentDepartureDate, construct);

    if (!fallback::serviceFeeTimeInResponse(&pricingTrx))
    {
      if (pricingTrx.getOptions()->isServiceFeesTemplateRequested() &&
          (isPricingTrxInVer(pricingTrx, 1, 3, 5) ||
           isTrxInProperVersion(pricingTrx, PricingTrx::WPA, 1, 1, 4)))
      {
        if (fallback::serviceFeeOpenSeg(&pricingTrx) ||
            tvlSeg.segmentType() != Open)
        {
          construct.addAttribute(xml2::SegmentDepartureTime,
                                 tvlSeg.departureDT().timeToString(HHMM, ""));
          addDateAttr(tvlSeg.arrivalDT(), xml2::SegmentArrivalDate, construct);
          construct.addAttribute(xml2::SegmentArrivalTime,
                                 tvlSeg.arrivalDT().timeToString(HHMM, ""));
        }
      }
    }

    if (isFareBreak || TrxUtil::swsPoAtsePath(pricingTrx))
    {
      const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();
      construct.addAttribute(xml2::FareVendor, paxTypeFare->vendor());
      construct.addAttribute(xml2::FareCarrier,
                             MCPCarrierUtil::swapToPseudo(&pricingTrx, paxTypeFare->carrier()));

      char buf[128];
      sprintf(buf, "%d", paxTypeFare->fareTariff());
      std::string tariff(buf);
      construct.addAttribute(xml2::FareTariff, tariff);
      construct.addAttribute(xml2::FareRule, paxTypeFare->ruleNumber());
      prepareNetRemitTrailerMsg(pricingTrx, farePath, fareUsage, construct, calcTotals);
    }

    construct.addAttribute(xml2::FareClass, bookingCode);

    char tmpBuf[10];
    sprintf(tmpBuf, "%02d", tvlSeg.pnrSegment());
    construct.addAttribute(xml2::ItinSegmentNumber, tmpBuf);

    // Global direction
    if (!fareUsage.paxTypeFare()->isDummyFare())
    {
      GlobalDirection gd = GlobalDirection::XX;
      std::vector<TravelSeg*> travelSegs;
      travelSegs.push_back(&tvlSeg);
      getGlobalDirection(&pricingTrx, calcTotals.farePath->itin()->travelDate(), travelSegs, gd);

      std::string dst = "";
      globalDirectionToStr(dst, gd);
      construct.addAttribute(xml2::RouteTravel, dst);
    }
    // Origin and destination country codes
    prepareCountryCodes(tvlSeg, construct);

    // Not Valid dates
    std::string notValidDateStr = calcTotals.tvlSegNVB[segmentOrder].dateToSqlString();
    if (notValidDateStr != NO_DATE_STRING)
    {
      construct.addAttribute(xml2::NotValidBeforeDate, notValidDateStr);
    }
    notValidDateStr = calcTotals.tvlSegNVA[segmentOrder].dateToSqlString();
    if (notValidDateStr != NO_DATE_STRING)
    {
      construct.addAttribute(xml2::NotValidAfterDate, notValidDateStr);
    }

    construct.addAttributeBoolean(xml2::FareBreakPoint, isFareBreak);

    if (!fareUsage.paxTypeFare()->isDummyFare())
    {
      construct.addAttributeBoolean(xml2::ExtraMileageAllowance,
                                    (calcTotals.extraMileageTravelSegs.find(&tvlSeg) !=
                                     calcTotals.extraMileageTravelSegs.end()) ||
                                        (calcTotals.extraMileageFareUsages.find(&fareUsage) !=
                                         calcTotals.extraMileageFareUsages.end()));
    }

    if (segmentNumber < calcTotals.bkgCodeSegStatus.size())
    {
      if (calcTotals.bkgCodeSegStatus[segmentNumber].isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
        construct.addAttributeChar(xml2::AvailabilityBreak, 'T');
      else
        construct.addAttributeChar(xml2::AvailabilityBreak, 'F');
    }
    else
      construct.addAttributeChar(xml2::AvailabilityBreak, 'T');

    prepareSegmentSideTrips(
        pricingTrx, farePath, pricingUnit, itin, tvlSeg, segmentOrder, construct);

    uint segmentNumber = calcTotals.farePath->itin()->segmentOrder(&tvlSeg) - 1;
    bool isRebooked = calcTotals.bkgCodeSegStatus[segmentNumber].isSet(PaxTypeFare::BKSS_REBOOKED);
    prepareScheduleInfo(pricingTrx, tvlSeg, construct, fareUsage, isRebooked);

    prepareStopoverSegment(pricingTrx, tvlSeg, calcTotals, construct);

    prepareTransferCharges(tvlSeg, calcTotals, fareUsage, noDecCalc, construct);

    const int cat35 = 35;
    bool cat35BaggageSeg = false;
    if (farePath.collectedNegFareData() != nullptr && fareUsage.paxTypeFare() != nullptr &&
        fareUsage.paxTypeFare()->paxTypeFareRuleData(cat35) != nullptr &&
        fareUsage.paxTypeFare()->paxTypeFareRuleData(cat35)->ruleItemInfo() != nullptr)
    {
      const CollectedNegFareData& negFareData = *(farePath.collectedNegFareData());
      const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(
          fareUsage.paxTypeFare()->paxTypeFareRuleData(cat35)->ruleItemInfo());
      if (negFareData.indicatorCat35() && negFareRest->bagTypeInd() != ' ' &&
          !TrxUtil::isIata302BaggageActivated(pricingTrx))
      {
        construct.addAttributeUShort(
            xml2::SegmentPassengerNumber,
            FareCalcUtil::getPtcRefNo(pricingTrx, calcTotals.farePath->paxType()));
        if (negFareRest->bagNo() != "00")
        {
          construct.addAttributeChar(xml2::BaggageIndicator, negFareRest->bagTypeInd());
          construct.addAttribute(xml2::BaggageValue, negFareRest->bagNo());
        }
        else
        {
          construct.addAttributeChar(xml2::BaggageIndicator, ' ');
          construct.addAttribute(xml2::BaggageValue, "NI");
        }
        construct.addAttributeChar(xml2::Cat35FareSegment, 'T');
        cat35BaggageSeg = true;
      }
    }
    if (!cat35BaggageSeg)
    {
      if (tvlSeg.isAir())
      {
        const AirSeg* airSeg = static_cast<const AirSeg*>(&tvlSeg);
        const auto baggageIter = farePath.baggageAllowance().find(airSeg);

        if (baggageIter != farePath.baggageAllowance().end())
        {
          const std::string& baggageAllowance = baggageIter->second;
          uint8_t baggageStringLength = baggageAllowance.length();
          if (baggageStringLength > 0)
          {
            construct.addAttributeUShort(
                xml2::SegmentPassengerNumber,
                FareCalcUtil::getPtcRefNo(pricingTrx, calcTotals.farePath->paxType()));
            construct.addAttribute(xml2::BaggageValue,
                                   baggageAllowance.substr(0, baggageStringLength - 1));
            if (baggageAllowance.substr(0, baggageStringLength - 1) == "NI")
              construct.addAttributeChar(xml2::BaggageIndicator, ' ');
            else
              construct.addAttributeChar(xml2::BaggageIndicator,
                                         baggageAllowance[baggageStringLength - 1]);
            construct.addAttributeChar(xml2::Cat35FareSegment, 'F');
          }
        }
        else
        {
          construct.addAttributeUShort(
              xml2::SegmentPassengerNumber,
              FareCalcUtil::getPtcRefNo(pricingTrx, calcTotals.farePath->paxType()));
          //                  construct.addAttributeChar(xml2::BaggageIndicator, ' ');
          construct.addAttributeChar(xml2::Cat35FareSegment, 'F');
        }
      }
    }

    if (pricingTrx.getRequest() && pricingTrx.getRequest()->isLowFareRequested()) // WPNC entry
    {
      if (cat5RebookRequired)
        construct.addAttributeChar(xml2::Cat5RequiresRebook, 'T');
      else
        construct.addAttributeChar(xml2::Cat5RequiresRebook, 'F');
    }

    if (displayTfdpsc)
      prepareTfdpsc(pricingTrx, &tvlSeg, fareUsage, construct);

    if (!fareUsage.paxTypeFare()->isDummyFare())
    {
      prepareMileage(tvlSeg, calcTotals, construct);
    }
    prepareSurcharges(tvlSeg, calcTotals, noDecCalc, construct);

    // Prepare surcharge from Net Remit Published fare
    if (calcTotals.netRemitCalcTotals != nullptr)
    {
      const TravelSeg* tktNetRemitTravelSeg = fareUsage.getTktNetRemitTravelSeg(&tvlSeg);
      if ((tktNetRemitTravelSeg != nullptr) &&
          (calcTotals.netRemitCalcTotals->surcharges.find(tktNetRemitTravelSeg) !=
           calcTotals.netRemitCalcTotals->surcharges.end()))
      {
        construct.openElement(xml2::NetRemitInfo);
        prepareSurcharges(
            *tktNetRemitTravelSeg, *(calcTotals.netRemitCalcTotals), noDecCalc, construct);
        construct.closeElement();
      }
    }

    construct.closeElement();
  }

  // Need to put ARUNK on bottom of Travel Segment for SDS
  prepareArunk(pricingTrx, farePath, fareUsage, pricingUnit, itin, segmentOrder, construct);
}

void
PricingResponseFormatter::getGlobalDirection(const PricingTrx* trx,
                                             DateTime travelDate,
                                             const std::vector<TravelSeg*>& tvlSegs,
                                             GlobalDirection& globalDir) const
{
  GlobalDirectionFinderV2Adapter::getGlobalDirection(trx, travelDate, tvlSegs, globalDir);
}

void
PricingResponseFormatter::prepareHPUForNet(PricingTrx& trx, 
                                           const FareUsage& fareUsage,
                                           XMLConstruct& construct)
{
  const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();

  if (!paxTypeFare->hasCat35Filed())
    return;

  const NegPaxTypeFareRuleData* negRuleData = paxTypeFare->getNegRuleData();
  if (!negRuleData)
    return;

  if (!fallback::fallbackFixFRRHpuForNet(&trx) &&
       !negRuleData->fareRetailerRuleId())
    return;

  const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(negRuleData->ruleItemInfo());

  if (!negFareRest)
    return;

  construct.openElement(xml2::MarkupDetail);

  construct.addAttribute(xml2::MarkupFeeAppId, "NT");
  //construct.addAttribute(xml2::MarkupTypeCode, "X"); // TO DO

  MoneyAmount netAmt = negRuleData->netAmount();
  MoneyAmount selAmt = paxTypeFare->fareAmount();

  if (paxTypeFare->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
      negFareRest->negFareCalcTblItemNo() == 0) // L w/o 979
  {
    netAmt = selAmt;
  }

  MoneyAmount markupAmt = selAmt - netAmt;
  MoneyAmount faAfterMarkUp = netAmt;

  if (paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)
  { 
    faAfterMarkUp = selAmt;
  }

  construct.addAttributeDouble(xml2::FareAmountAfterMarkup, faAfterMarkUp, paxTypeFare->numDecimal());
  construct.addAttributeDouble(xml2::MarkupAmount, markupAmt, paxTypeFare->numDecimal());
  construct.addAttribute(xml2::AmountCurrency, paxTypeFare->currency());
  construct.addAttribute(xml2::MarkupRuleSourcePCC, negRuleData->sourcePseudoCity());
  construct.addAttributeULong(xml2::MarkupRuleItemNumber, negRuleData->fareRetailerRuleId());

  construct.closeElement();
}

void
PricingResponseFormatter::prepareHPUForAdjusted(PricingTrx& trx, 
                                                const FareUsage& fareUsage,
                                                XMLConstruct& construct)
{
  const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();

  if (!paxTypeFare->getAdjustedSellingCalcData())
    return;
  
  const AdjustedSellingCalcData* adjSellingCalcData  = paxTypeFare->getAdjustedSellingCalcData();

  if (!adjSellingCalcData)
    return;

  construct.openElement(xml2::MarkupDetail);

  construct.addAttribute(xml2::MarkupFeeAppId, "AJ");
  //construct.addAttribute(xml2::MarkupTypeCode, "X"); // TO DO 
  construct.addAttributeDouble(xml2::FareAmountAfterMarkup, adjSellingCalcData->getCalculatedAmt(), paxTypeFare->numDecimal());
  construct.addAttributeDouble(xml2::MarkupAmount, adjSellingCalcData->getCalculatedAmt() - paxTypeFare->fareAmount(), paxTypeFare->numDecimal());
  construct.addAttribute(xml2::AmountCurrency, paxTypeFare->currency());
  construct.addAttribute(xml2::MarkupRuleSourcePCC, adjSellingCalcData->getSourcePcc());
  construct.addAttributeULong(xml2::MarkupRuleItemNumber, adjSellingCalcData->getFareRetailerRuleId());

  construct.closeElement();
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareArunk
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareArunk(PricingTrx& pricingTrx,
                                       const FarePath& farePath,
                                       const FareUsage& fareUsage,
                                       const PricingUnit& pricingUnit,
                                       const Itin* itin,
                                       uint16_t& segmentOrder,
                                       XMLConstruct& construct)
{
  for (TravelSeg* travelSeg : itin->travelSeg())
  {
    if (itin->segmentOrder(travelSeg) != segmentOrder)
      continue;

    if (travelSeg->toAirSeg())
      return;

    if (farePath.isTravelSegPartOfFarePath(travelSeg))
      return;

    prepareSegmentsCity(pricingTrx, farePath, itin, *travelSeg, fareUsage, false, construct);

    construct.addAttributeChar(xml2::PureSurfaceSegment, 'T');
    construct.addAttributeChar(xml2::StopoverSegment, 'T');

    if (pricingUnit.isSideTripPU() && !pricingUnit.fareUsage().empty())
    {
      bool startOfSideTrip = false;
      bool endOfSideTrip = false;

      isStartOrEndOfSideTrip(farePath, travelSeg, startOfSideTrip, endOfSideTrip);
      if (endOfSideTrip)
      {
        construct.addAttributeChar(xml2::SideTripStart, 'T');
        construct.addAttributeChar(xml2::SideTripEnd, 'T');
        construct.addAttributeChar(xml2::SideTripIndicator, 'T');
      }
    }

    bool unchargeable = true;
    construct.addAttributeBoolean(xml2::UnchargeableSurface, unchargeable);

    char tmpBuf[10];
    sprintf(tmpBuf, "%02d", travelSeg->pnrSegment());
    construct.addAttribute(xml2::ItinSegmentNumber, tmpBuf);

    construct.closeElement();
    segmentOrder++;
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareSegmentSideTrips
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareSegmentSideTrips(PricingTrx& pricingTrx,
                                                  const FarePath& farePath,
                                                  const PricingUnit& pricingUnit,
                                                  const Itin* itin,
                                                  const TravelSeg& tvlSeg,
                                                  uint16_t& segmentOrder,
                                                  XMLConstruct& construct)
{
  if (pricingUnit.isSideTripPU())
  {
    if (!pricingUnit.fareUsage().empty())
    {
      FareUsage* fareUsageSideTrip = pricingUnit.fareUsage().back();

      if (fareUsageSideTrip->travelSeg().empty())
      {
        return;
      }

      if (itin->segmentOrder(&tvlSeg) == segmentOrder)
      {
        construct.addAttributeChar(xml2::SideTripIndicator, 'T');
        generateSegmentSideTripAttributes(pricingTrx, farePath, &tvlSeg, construct);
      }
      else
      {
        for (const FareUsage* fu : pricingUnit.fareUsage())
        {
          for (const TravelSeg* tSeg : fu->travelSeg())
          {
            if (itin->segmentOrder(tSeg) == segmentOrder)
            {
              construct.addAttributeChar(xml2::SideTripIndicator, 'T');
              generateSegmentSideTripAttributes(pricingTrx, farePath, tSeg, construct);
              return;
            }
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareScheduleInfo
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareScheduleInfo(PricingTrx& pricingTrx,
                                              const TravelSeg& tvlSeg,
                                              XMLConstruct& construct,
                                              const FareUsage& fUsage,
                                              bool isRebooked)
{
  const AirSeg* airSeg = tvlSeg.toAirSeg();

  if (airSeg)
  {
    construct.addAttribute(xml2::MarketingCarrier, airSeg->marketingCarrierCode());
    construct.addAttribute(xml2::OperatingCarrier, airSeg->operatingCarrierCode());
    if (!airSeg->bookedCabin().isUndefinedClass())
    {
      // find the index of the tvlseg in the fareUsage
      uint16_t tvlSegIndex =
          (std::find(fUsage.travelSeg().begin(), fUsage.travelSeg().end(), &tvlSeg) -
           (fUsage.travelSeg().begin()));
      // APO-43827: set segment cabin code correctly if it was rebooked for WQ req.
      if ((pricingTrx.noPNRPricing()) && (isRebooked) &&
          (tvlSegIndex < fUsage.segmentStatus().size()))
      {
        PaxTypeFare::SegmentStatus segStat = (fUsage.segmentStatus())[tvlSegIndex];
        if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(pricingTrx) &&
            TrxUtil::isRbdByCabinValueInPriceResponse(pricingTrx))
          construct.addAttributeChar(xml2::SegmentCabinCode,
                                     segStat._reBookCabin.getClassAlphaNum(true));
        else
          construct.addAttributeChar(xml2::SegmentCabinCode,
                                     segStat._reBookCabin.getClassAlphaNum());
      }
      else
      {
        if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(pricingTrx) &&
            TrxUtil::isRbdByCabinValueInPriceResponse(pricingTrx))
          construct.addAttributeChar(xml2::SegmentCabinCode, airSeg->bookedCabinCharAnswer());
        else
          construct.addAttributeChar(xml2::SegmentCabinCode, airSeg->bookedCabinChar());
      }
    }
    if (airSeg->flowJourneyCarrier())
      construct.addAttributeChar(xml2::JourneyType, 'F');
    else if (airSeg->localJourneyCarrier())
      construct.addAttributeChar(xml2::JourneyType, 'L');
  }
  else
  {
    construct.addAttributeChar(xml2::PureSurfaceSegment, 'T');
    // TODO Currently no requirements temporary until requirments are discussed for
    // UnchargeableSurface
    construct.addAttributeChar(xml2::UnchargeableSurface, 'F');
  }

  if (!tvlSeg.equipmentType().empty())
    construct.addAttribute(xml2::SEGEquipmentCode, tvlSeg.equipmentType());
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::generateSegmentSideTripAttributes
//----------------------------------------------------------------------------
bool
PricingResponseFormatter::generateSegmentSideTripAttributes(PricingTrx& pricingTrx,
                                                            const FarePath& farePath,
                                                            const TravelSeg* travelSeg,
                                                            XMLConstruct& construct)
{
  bool startOfSideTrip = false;
  bool endOfSideTrip = false;

  isStartOrEndOfSideTrip(farePath, travelSeg, startOfSideTrip, endOfSideTrip);

  if (endOfSideTrip)
  {
    construct.addAttributeChar(xml2::SideTripStart, 'T');
    construct.addAttributeChar(xml2::SideTripEnd, 'T');
  }
  return startOfSideTrip || endOfSideTrip;
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareSurcharges
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareSurcharges(const TravelSeg& tvlSeg,
                                            CalcTotals& calcTotals,
                                            const CurrencyNoDec& noDecCalc,
                                            XMLConstruct& construct)
{
  // Surcharge Information
  auto surchargesIter = calcTotals.surcharges.find(&tvlSeg);
  if (surchargesIter != calcTotals.surcharges.end())
  {
    MoneyAmount tempSurcharge;
    for (SurchargeData* surchargeData : surchargesIter->second)
    {
      if (surchargeData->amountNuc() != 0 && surchargeData->selectedTkt() &&
          !surchargeData->isFromOverride())
      {
        tempSurcharge = surchargeData->amountNuc() * surchargeData->itinItemCount();
        construct.openElement(xml2::SurchargeInformation);
        construct.addAttributeChar(xml2::SurchargeType, surchargeData->surchargeType());
        construct.addAttributeDouble(xml2::SurchargeAmount, tempSurcharge, noDecCalc);
        construct.addAttribute(xml2::SurchargeCurrencyCode,
                               calcTotals.farePath->calculationCurrency());
        construct.addAttribute(xml2::PublishedCurrency, surchargeData->currSelected());
        construct.addAttribute(xml2::SurchargeDescription, surchargeData->surchargeDesc());
        construct.addAttribute(xml2::SurchargeDepartureCity, surchargeData->fcBrdCity());
        construct.addAttribute(xml2::SurchargeArrivalCity, surchargeData->fcOffCity());
        if (surchargeData->fcFpLevel())
          construct.addAttribute(xml2::SurchargeSurchargeType, "J");
        else if (surchargeData->singleSector())
          construct.addAttribute(xml2::SurchargeSurchargeType, "S");
        else
          construct.addAttribute(xml2::SurchargeSurchargeType, "C");
        construct.closeElement();
      }
    }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareMileage
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareMileage(const TravelSeg& tvlSeg,
                                         CalcTotals& calcTotals,
                                         XMLConstruct& construct)
{
  for (MileageTypeData* mileageTypeData : calcTotals.mileageTypeData())
  {
    if (mileageTypeData->travelSeg() == &tvlSeg)
    {
      construct.openElement(xml2::MileageDisplayInformation);
      if (mileageTypeData->type() == MileageTypeData::EQUALIZATION)
        construct.addAttributeChar(xml2::MileageDisplayType, 'B');
      else if (mileageTypeData->type() == MileageTypeData::TICKETEDPOINT)
        construct.addAttributeChar(xml2::MileageDisplayType, 'T');
      construct.addAttribute(xml2::MileageDisplayCity, mileageTypeData->city());
      construct.closeElement();
      break;
    }
  }
}

bool
PricingResponseFormatter::displayTfdPsc(const PricingTrx& pricingTrx, const FareUsage& fareUsage)
    const
{
  if (!TrxUtil::optimusNetRemitEnabled(pricingTrx) || !fareUsage.paxTypeFare()->isNegotiated() ||
      fareUsage.netRemitPscResults().empty())
  {
    return false;
  }
  return true;
}

void
PricingResponseFormatter::prepareTfdpsc(PricingTrx& pricingTrx,
                                        const TravelSeg* tvlSeg,
                                        const FareUsage& fareUsage,
                                        XMLConstruct& construct)
{
  FareUsage::TktNetRemitPscResultVec::const_iterator nrResults =
      FareCalculation::findNetRemitPscResults(fareUsage, tvlSeg);

  if (nrResults != fareUsage.netRemitPscResults().end())
  {
    const NegPaxTypeFareRuleData* ruleData = fareUsage.paxTypeFare()->getNegRuleData();
    TSE_ASSERT(ruleData);
    const NegFareRestExt* negFareRestExt = ruleData->negFareRestExt();
    Indicator fareBasisAmtInd = negFareRestExt->fareBasisAmtInd();

    std::string fareBasis;
    if (fareBasisAmtInd == RuleConst::NR_VALUE_A || fareBasisAmtInd == RuleConst::NR_VALUE_F)
    {
      if (nrResults->_resultFare)
        fareBasis = nrResults->_resultFare->createFareBasis(pricingTrx);
    }
    else
    {
      if (nrResults->_tfdpscSeqNumber)
      {
        const TravelSeg* lastTvlSeg = TravelSegUtil::lastAirSeg(fareUsage.travelSeg());
        if (lastTvlSeg != nullptr && lastTvlSeg->specifiedFbc().empty())
        {
          if (!nrResults->_tfdpscSeqNumber->uniqueFareBasis().empty())
          {
            const std::string uniqueFareBasis =
                nrResults->_tfdpscSeqNumber->uniqueFareBasis().c_str();
            fareBasis = fareUsage.paxTypeFare()->createFareBasis(pricingTrx, uniqueFareBasis);
          }
        }
      }
    }

    if (!fareBasis.empty())
    {
      construct.addAttribute(xml2::NetRemitPubFareBasisCode, fareBasis);
      construct.addAttributeInteger(xml2::NetRemitPubFareBasisCodeLength, fareBasis.length());
    }
  }
}

void
PricingResponseFormatter::prepareNetRemitTrailerMsg(PricingTrx& pricingTrx,
                                                    const FarePath& farePath,
                                                    const FareUsage& fareUsage,
                                                    XMLConstruct& construct,
                                                    CalcTotals& calcTotals)
{
  if (farePath.collectedNegFareData() != nullptr &&
      farePath.collectedNegFareData()->netRemitTicketInd())
  {
    const NegPaxTypeFareRuleData* ruleData = fareUsage.paxTypeFare()->getNegRuleData();
    if (ruleData && ruleData->negFareRestExt() &&
        ruleData->negFareRestExt()->tktFareDataSegExistInd() == 'Y')
    {
      Indicator fareBasisAmtInd = ruleData->negFareRestExt()->fareBasisAmtInd();
      if ((fareBasisAmtInd == RuleConst::NR_VALUE_B || fareBasisAmtInd == RuleConst::NR_VALUE_N ||
           fareBasisAmtInd == RuleConst::BLANK) &&
          (fareUsage.netRemitPscResults().empty() ||
           (fareUsage.netRemitPscResults().front()._tfdpscSeqNumber &&
            fareUsage.netRemitPscResults().front()._tfdpscSeqNumber->uniqueFareBasis().empty())))
        return;
    }
    prepareFareVendorSource(fareUsage, ruleData, construct);
    prepareNegotiatedFbc(pricingTrx, fareUsage, construct, calcTotals);
  }
}

void
PricingResponseFormatter::prepareFareVendorSource(const FareUsage& fareUsage,
                                                  const NegPaxTypeFareRuleData* ruleData,
                                                  XMLConstruct& construct)
{
  std::string fareVendorSource = fareUsage.paxTypeFare()->vendor();
  if (fareVendorSource == ATPCO_VENDOR_CODE) // ATP
    fareVendorSource += "C";
  else if (fareVendorSource == SMF_ABACUS_CARRIER_VENDOR_CODE || // SMFA
           fareVendorSource == SMF_CARRIER_VENDOR_CODE) // SMFC
  {
    if (ruleData && ruleData->fareProperties())
      fareVendorSource = ruleData->fareProperties()->fareSource();
  }
  construct.addAttribute(xml2::FareVendorSource, fareVendorSource);
}

void
PricingResponseFormatter::prepareNegotiatedFbc(PricingTrx& pricingTrx,
                                               const FareUsage& fareUsage,
                                               XMLConstruct& construct,
                                               CalcTotals& calcTotals)
{
  const TravelSeg* tvlSeg = TravelSegUtil::lastAirSeg(fareUsage.travelSeg());
  if (tvlSeg != nullptr && !tvlSeg->specifiedFbc().empty())
  {
    std::string fareBasis = fareUsage.paxTypeFare()->createFareBasis(pricingTrx);
    if (!fareBasis.empty())
    {
      SpanishFamilyDiscountDesignator appender = spanishFamilyDiscountDesignatorBuilder(
          pricingTrx, calcTotals, FareCalcConsts::MAX_FARE_BASIS_SIZE);
      appender(fareBasis);

      construct.addAttribute(xml2::FareBasisCode, fareBasis.c_str());
      construct.addAttributeInteger(xml2::FareBasisCodeLength, fareBasis.length());
    }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareCountryCodes
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareCountryCodes(const TravelSeg& tvlSeg, XMLConstruct& construct)
{
  if (tvlSeg.origin() != nullptr)
    construct.addAttribute(xml2::SegDepartureCountry, tvlSeg.origin()->nation());
  if (tvlSeg.destination() != nullptr)
    construct.addAttribute(xml2::SegArrivalCountry, tvlSeg.destination()->nation());
  switch (tvlSeg.geoTravelType())
  {
  case GeoTravelType::Domestic:
  {
    construct.addAttributeChar(xml2::LocationType, 'D');
    break;
  }
  case GeoTravelType::International:
  {
    construct.addAttributeChar(xml2::LocationType, 'I');
    break;
  }
  case GeoTravelType::Transborder:
  {
    construct.addAttributeChar(xml2::LocationType, 'T');
    break;
  }
  case GeoTravelType::ForeignDomestic:
  {
    construct.addAttributeChar(xml2::LocationType, 'F');
    break;
  }
  default:
  {
    construct.addAttributeChar(xml2::LocationType, 'U');
  }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareTransferCharges
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareTransferCharges(const TravelSeg& tvlS,
                                                 CalcTotals& calcTotals,
                                                 const FareUsage& fareUsage,
                                                 const CurrencyNoDec& noDecCalc,
                                                 XMLConstruct& construct)
{
  auto transferSurcharges = calcTotals.transferSurcharges.find(&tvlS);
  if (transferSurcharges == calcTotals.transferSurcharges.end())
    return;

  for (const auto transferSurcharge : transferSurcharges->second)
  {
    if (!transferSurcharge)
      continue;
    if (transferSurcharge->amount() != 0 && transferSurcharge->isSegmentSpecific())
    {
      construct.addAttributeBoolean(xml2::TransferSegment, true);
      construct.addAttributeDouble(xml2::TransferCharge, transferSurcharge->amount(), noDecCalc);
      construct.addAttribute(xml2::TransferPublishedCurrencyCode,
                             transferSurcharge->unconvertedCurrencyCode());
    }
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareFarePathPlusUps
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareFarePathPlusUps(const FarePath& farePath,
                                                 const CurrencyNoDec& noDecCalc,
                                                 XMLConstruct& construct)
{
  // For OSC
  for (auto oscPlusUp : farePath.oscPlusUp())
    preparePupElement("OSC", *oscPlusUp, noDecCalc, "", construct);

  // For RSC
  for (auto rscPlusUp : farePath.rscPlusUp())
    preparePupElement("RSC", *rscPlusUp, noDecCalc, "", construct);
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::preparePricingUnitPlusUps
//----------------------------------------------------------------------------
void
PricingResponseFormatter::preparePricingUnitPlusUps(const PricingUnit& pricingUnit,
                                                    const CurrencyNoDec& noDecCalc,
                                                    PricingTrx& pricingTrx,
                                                    XMLConstruct& construct)
{
  if (pricingUnit.minFarePlusUp().empty())
    return;

  MinFarePlusUp::const_iterator pupIterEnd = pricingUnit.minFarePlusUp().end();

  MinFarePlusUp::const_iterator pupIter = pricingUnit.minFarePlusUp().find(CTM);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("CTM", *(pupIter->second), noDecCalc, "", construct);
  }
  pupIter = pricingUnit.minFarePlusUp().find(COP);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    const Loc* saleLoc = TrxUtil::saleLoc(pricingTrx);
    if (saleLoc != nullptr)
    {
      preparePupElement("LCM", *(pupIter->second), noDecCalc, saleLoc->nation(), construct);
    }
  }
  pupIter = pricingUnit.minFarePlusUp().find(OJM);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("OJM", *(pupIter->second), noDecCalc, "", construct);
  }
  pupIter = pricingUnit.minFarePlusUp().find(CPM);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("CPM", *(pupIter->second), noDecCalc, "", construct);
  }
  pupIter = pricingUnit.minFarePlusUp().find(HRT);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("HRTC", *(pupIter->second), noDecCalc, "", construct);
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareFareUsagePlusUps
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareFareUsagePlusUps(
    const FareUsage& fareUsage,
    const CurrencyNoDec& noDecCalc,
    XMLConstruct& construct,
    std::vector<const FareUsage*>& fusPlusUpsAlreadyShown)
{
  // check whether PlusUp info for this fare usage has already been shown
  if (std::find(fusPlusUpsAlreadyShown.begin(), fusPlusUpsAlreadyShown.end(), &fareUsage) !=
      fusPlusUpsAlreadyShown.end())
    return;
  fusPlusUpsAlreadyShown.push_back(&fareUsage);

  if (fareUsage.minFarePlusUp().empty())
    return;

  MinFarePlusUp::const_iterator pupIterEnd = fareUsage.minFarePlusUp().end();

  MinFarePlusUp::const_iterator pupIter = fareUsage.minFarePlusUp().find(HIP);
  if (pupIter != pupIterEnd)
  {
    const MinFarePlusUpItem& plusUp = *(pupIter->second);
    construct.addAttribute(xml2::HIPOrigCity, plusUp.boardPoint);
    construct.addAttribute(xml2::HIPDestCity, plusUp.offPoint);
    construct.addAttribute(xml2::ConstructedHIPCity, plusUp.constructPoint);
  }
  pupIter = fareUsage.minFarePlusUp().find(DMC);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("DMC", *(pupIter->second), noDecCalc, "", construct);
  }
  pupIter = fareUsage.minFarePlusUp().find(COM);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("COM", *(pupIter->second), noDecCalc, "", construct);
  }
  pupIter = fareUsage.minFarePlusUp().find(BHC);
  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("BHC", *(pupIter->second), noDecCalc, "", construct);
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::preparePupElement
//----------------------------------------------------------------------------
void
PricingResponseFormatter::preparePupElement(const std::string& payload,
                                            const MinFarePlusUpItem& plusUp,
                                            const CurrencyNoDec& noDecCalc,
                                            const NationCode& countryOfPmt,
                                            XMLConstruct& construct)
{
  construct.openElement(xml2::PlusUps);

  construct.addAttributeDouble(xml2::PlusUpAmount, plusUp.plusUpAmount, noDecCalc);
  construct.addAttribute(xml2::PlusUpOrigCity, plusUp.boardPoint);
  construct.addAttribute(xml2::PlusUpDestCity, plusUp.offPoint);
  if (payload == "BHC")
  {
    const BhcPlusUpItem& bhcPlusUp = dynamic_cast<const BhcPlusUpItem&>(plusUp);
    construct.addAttribute(xml2::PlusUpFareOrigCity, bhcPlusUp.fareBoardPoint);
    construct.addAttribute(xml2::PlusUpFareDestCity, bhcPlusUp.fareOffPoint);
    construct.addAttribute(xml2::PlusUpCountryOfPmt, plusUp.currency);
  }
  else if (payload == "LCM") // COP
  {
    construct.addAttribute(xml2::PlusUpCountryOfPmt, countryOfPmt);
  }
  else if (!plusUp.currency.empty())
  {
    construct.addAttribute(xml2::PlusUpCountryOfPmt, plusUp.currency);
  }

  construct.addAttribute(xml2::PlusUpViaCity, plusUp.constructPoint);
  construct.addAttribute(xml2::PlusUpMessage, payload);

  construct.closeElement();
}

bool
PricingResponseFormatter::isCategory25Applies(uint16_t category, const PaxTypeFare& ptFare) const
{
  const PaxTypeFareRuleData* ptfrd = ptFare.paxTypeFareRuleData(FARE_BY_RULE);

  if (ptfrd != nullptr && ptfrd->ruleItemInfo() != nullptr)
  {
    bool checkFBRCategory = false;
    bool checkBaseFareCategory = false;
    RuleUtil::determineRuleChecks(category,
                                  *(static_cast<const FareByRuleItemInfo*>(ptfrd->ruleItemInfo())),
                                  checkFBRCategory,
                                  checkBaseFareCategory);

    if (checkBaseFareCategory && ptfrd->baseFare())
    {
      if (category == COMBINABILITY_RULE)
        return ptfrd->baseFare()->rec2Cat10();
      else
      {
        ptfrd = ptfrd->baseFare()->paxTypeFareRuleData(category);
        return (ptfrd && (ptfrd->categoryRuleInfo() || ptfrd->ruleItemInfo() ||
                          PTFRuleData::toFBRPaxTypeFare(ptfrd)));
      }
    }
  }
  return false;
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::prepareRuleCategoryIndicator
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareRuleCategoryIndicator(const PricingTrx& pricingTrx,
                                                       const FareUsage& fareUsage,
                                                       XMLConstruct& construct)
{
  const uint16_t CATEGORY_BEGIN = 1;
  const uint16_t CATEGORY_END = 24;
  const uint16_t CAT_10 = 10;
  const uint16_t CAT_25 = 25;
  const uint16_t CAT_35 = 35;
  const uint16_t CAT_27 = 27;
  const PaxTypeFare& ptFare = *fareUsage.paxTypeFare();
  std::ostringstream dataStream;
  bool categoryFound = false;
  bool categoryApplies = false;
  PaxTypeFareRuleData* ptfrd = nullptr;

  for (uint16_t category = CATEGORY_BEGIN; category < CATEGORY_END; ++category)
  {
    if (category == CAT_10)
    {
      categoryApplies = (fareUsage.rec2Cat10() != nullptr);
    }
    else
    {
      ptfrd = ptFare.paxTypeFareRuleData(category);

      if (category == 19 && ptfrd && ptfrd->categoryRuleInfo())
      {
        if (categoryFound)
        {
          dataStream << " ";
        }
        dataStream << ptfrd->categoryRuleInfo()->categoryNumber();
        categoryFound = true;
        continue;
      }

      categoryApplies = (ptfrd && (ptfrd->categoryRuleInfo() || ptfrd->ruleItemInfo() ||
                                   PTFRuleData::toFBRPaxTypeFare(ptfrd)));
    }

    if (!fallback::fallbackCATElementMark(&pricingTrx) && !categoryApplies &&
        ptFare.isFareByRule() && !ptFare.isSpecifiedFare())
    {
      categoryApplies = isCategory25Applies(category, ptFare);
    }

    if (categoryApplies)
    {
      if (categoryFound)
      {
        dataStream << " ";
      }
      dataStream << category;
      categoryFound = true;
    }
  }

  if (ptFare.isFareByRule())
  {
    if (categoryFound)
    {
      dataStream << " ";
    }
    dataStream << CAT_25;
    categoryFound = true;
  }

  if (fallback::fallbackCATElementMark(&pricingTrx))
  {
    ptfrd = ptFare.paxTypeFareRuleData(CAT_35);
    if (ptfrd != nullptr &&
        (ptfrd->categoryRuleInfo() != nullptr || ptfrd->ruleItemInfo() != nullptr))
    {
      if (categoryFound)
      {
        dataStream << " ";
      }
      dataStream << CAT_35;
      categoryFound = true;
    }
  }
  else
  {
    processRuleCategoryIndicator(ptFare, CAT_27, categoryFound, dataStream);
    processRuleCategoryIndicator(ptFare, CAT_35, categoryFound, dataStream);
  }

  if (categoryFound)
  {
    construct.openElement(xml2::CategoryList);
    construct.addElementData(dataStream.str().c_str());
    construct.closeElement();
  }
}

void
PricingResponseFormatter::processRuleCategoryIndicator(const PaxTypeFare& ptFare,
                                                       const uint16_t category,
                                                       bool& categoryFound,
                                                       std::ostringstream& dataStream)
{
  PaxTypeFareRuleData* ptfrd = nullptr;

  ptfrd = ptFare.paxTypeFareRuleData(category);
  if (ptfrd != nullptr &&
      (ptfrd->categoryRuleInfo() != nullptr || ptfrd->ruleItemInfo() != nullptr))
  {
    if (categoryFound)
    {
      dataStream << " ";
    }
    dataStream << category;
    categoryFound = true;
  }
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::prepareDifferential
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareDifferential(PricingTrx& pricingTrx,
                                              const FareUsage& fareUsage,
                                              const uint16_t noDecCalc,
                                              XMLConstruct& construct)
{
  MoneyAmount moneyAmount = 0.0;

  for (DifferentialData* differentialPtr : fareUsage.differentialPlusUp())
  {
    if (!differentialPtr)
      continue;

    DifferentialData& di = *differentialPtr;
    DifferentialData::STATUS_TYPE aStatus = di.status();

    if (aStatus != DifferentialData::SC_PASSED && aStatus != DifferentialData::SC_CONSOLIDATED_PASS)
      continue;

    if (di.hipAmount() == 0.0)
      moneyAmount = di.amount();
    else
      moneyAmount = di.hipAmount();

    std::string tag = xml2::HigherIntermediatePoint;
    if (di.amount() == 0)
      tag = xml2::ZeroDifferentialItem;
    construct.openElement(tag);

    construct.addAttributeDouble(xml2::AmountHIP, moneyAmount, noDecCalc);

    const LocCode& orgLoc =
        pricingTrx.dataHandle().getMultiTransportCity((*di.travelSeg().front()).origAirport());
    const LocCode& dstLoc =
        pricingTrx.dataHandle().getMultiTransportCity((*di.travelSeg().back()).destAirport());
    if (fareUsage.isInbound() && !fareUsage.dirChangeFromOutbound())
    {
      if (dstLoc.empty())
        construct.addAttribute(xml2::OrigCityHIP, (*di.fareMarket().begin())->offMultiCity());
      else
      {
        construct.addAttribute(xml2::OrigCityHIP, dstLoc);
        if (dstLoc != (*di.fareMarket().begin())->offMultiCity())
          construct.addAttribute(xml2::OrigCityHIPWPDF, (*di.fareMarket().begin())->offMultiCity());
      }
      if (orgLoc.empty())
        construct.addAttribute(xml2::DestCityHIP, (*di.fareMarket().begin())->boardMultiCity());
      else
      {
        construct.addAttribute(xml2::DestCityHIP, orgLoc);
        if (orgLoc != (*di.fareMarket().begin())->boardMultiCity())
          construct.addAttribute(xml2::DestCityHIPWPDF,
                                 (*di.fareMarket().begin())->boardMultiCity());
      }
    }
    else
    {
      if (orgLoc.empty())
        construct.addAttribute(xml2::OrigCityHIP, (*di.fareMarket().begin())->boardMultiCity());
      else
      {
        construct.addAttribute(xml2::OrigCityHIP, orgLoc);
        if (orgLoc != (*di.fareMarket().begin())->boardMultiCity())
          construct.addAttribute(xml2::OrigCityHIPWPDF,
                                 (*di.fareMarket().begin())->boardMultiCity());
      }
      if (dstLoc.empty())
        construct.addAttribute(xml2::DestCityHIP, (*di.fareMarket().begin())->offMultiCity());
      else
      {
        construct.addAttribute(xml2::DestCityHIP, dstLoc);
        if (dstLoc != (*di.fareMarket().begin())->offMultiCity())
          construct.addAttribute(xml2::DestCityHIPWPDF, (*di.fareMarket().begin())->offMultiCity());
      }
    }

    if (!di.fareClassLow().empty())
      construct.addAttribute(xml2::FareClassLow, di.fareClassLow().c_str());

    if (!di.fareClassHigh().empty())
    {
      construct.addAttribute(xml2::FareClassHigh, di.fareClassHigh().c_str());
      if (di.fareHigh()->mileageSurchargePctg() > 0)
        construct.addAttributeUShort(xml2::MileageSurchargePctg,
                                     di.fareHigh()->mileageSurchargePctg());
    }

    bool cabinHighSet = false;
    if (!di.hipLowOrigin().empty() || !di.hipLowDestination().empty() ||
        !di.hipHighOrigin().empty() || !di.hipHighDestination().empty()) // HIP
    {
      //
      // Legacy requires both Low and Hight city pairs but might not require Cabin???
      //
      construct.addAttribute(xml2::LowOrigHIP, di.hipLowOrigin());
      construct.addAttribute(xml2::LowDestHIP, di.hipLowDestination());
      construct.addAttribute(xml2::HighOrigHIP, di.hipHighOrigin());
      construct.addAttribute(xml2::HighDestHIP, di.hipHighDestination());

      if ((di.hipLowOrigin().empty() && di.hipLowDestination().empty()) || // 4.
          (di.hipHighOrigin().empty() && di.hipHighDestination().empty()))
      {
        if (di.hipHighOrigin().empty())
        {
          if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(pricingTrx) &&
              TrxUtil::isRbdByCabinValueInPriceResponse(pricingTrx))
          {
            construct.addAttributeChar(xml2::CabinLowHIP, di.hipCabinLow());
          }
          else
          {
            Indicator cabin = di.hipCabinLow();
            construct.addAttributeChar(xml2::CabinLowHIP, cabinChar(cabin));
          }
        }
        else
        {
          if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(pricingTrx) &&
              TrxUtil::isRbdByCabinValueInPriceResponse(pricingTrx))
          {
            construct.addAttributeChar(xml2::CabinHighHIP, di.hipCabinHigh());
          }
          else
          {
            Indicator cabin = di.hipCabinHigh();
            construct.addAttributeChar(xml2::CabinHighHIP, cabinChar(cabin));
          }
          cabinHighSet = true;
        }
      }
      else // 5.
      {
        if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(pricingTrx) &&
            TrxUtil::isRbdByCabinValueInPriceResponse(pricingTrx))
        {
          construct.addAttributeChar(xml2::CabinLowHIP, di.hipCabinLow());
          construct.addAttributeChar(xml2::CabinHighHIP, di.hipCabinHigh());
        }
        else
        {
          Indicator cabin = di.hipCabinLow();
          construct.addAttributeChar(xml2::CabinLowHIP, cabin);
          cabin = di.hipCabinHigh();
          construct.addAttributeChar(xml2::CabinHighHIP, cabin);
        }
        cabinHighSet = true;
      }
    }
    if (!cabinHighSet && !di.fareClassHigh().empty())
    {
      construct.addAttributeChar(xml2::CabinHighHIP, di.fareClassHigh()[0]);
    }

    char tmpBuf[10];
    sprintf(tmpBuf, "%02d", di.fareMarket().front()->travelSeg().front()->pnrSegment());
    construct.addAttribute(xml2::OrigSegOrder, tmpBuf);
    sprintf(tmpBuf, "%02d", di.fareMarket().front()->travelSeg().back()->pnrSegment());
    construct.addAttribute(xml2::DestSegOrder, tmpBuf);

    const PaxTypeFare* diffPtf = di.fareHigh();
    if (diffPtf)
    {
      CarrierCode diffCxr = diffPtf->carrier();
      if (diffPtf->fare()->isIndustry() || diffPtf->carrier() == INDUSTRY_CARRIER)
        diffCxr = diffPtf->fareMarket()->governingCarrier();

      construct.addAttribute(xml2::FareSourceCarrier, diffCxr.c_str());
    }

    construct.closeElement();
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::prepareStopoverSegment
//----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareStopoverSegment(const PricingTrx& pricingTrx,
                                                 const TravelSeg& tvlS,
                                                 CalcTotals& calcTotals,
                                                 XMLConstruct& construct)
{
  std::map<const TravelSeg*, std::vector<const FareUsage::StopoverSurcharge*>>::const_iterator
  surchargeVectorIter = calcTotals.stopoverSurcharges.find(&tvlS);
  if (surchargeVectorIter != calcTotals.stopoverSurcharges.end())
  {
    std::vector<const FareUsage::StopoverSurcharge*>::const_iterator surchargeIter =
        surchargeVectorIter->second.begin();
    std::vector<const FareUsage::StopoverSurcharge*>::const_iterator surchargeIterEnd =
        surchargeVectorIter->second.end();
    for (; surchargeIter != surchargeIterEnd; surchargeIter++)
    {
      const FareUsage::StopoverSurcharge& stopoverSurcharge = **surchargeIter;
      if (stopoverSurcharge.amount() > 0 && !stopoverSurcharge.isFromOverride() &&
          stopoverSurcharge.isSegmentSpecific())
      {
        construct.addAttributeDouble(
            xml2::CityStopoverCharge, stopoverSurcharge.amount(), stopoverSurcharge.noDecimals());
        construct.addAttribute(xml2::StopOverPublishedCurrencyCode,
                               stopoverSurcharge.unconvertedCurrencyCode());
      }
    }
  }

  Indicator connectionInd = calcTotals.fcConfig->wpConnectionInd();
  if (calcTotals.fcConfig->itinDisplayInd() != FareCalcConsts::FC_YES ||
      calcTotals.fcConfig->wpConnectionInd() == ' ')
  {
    connectionInd = calcTotals.fcConfig->fcConnectionInd();
  }

  bool connectionPoint = calcTotals.getDispConnectionInd(pricingTrx, &tvlS, connectionInd);
  if (connectionPoint)
  {
    construct.addAttributeBoolean(xml2::StopoverSegment, false);
    construct.addAttributeBoolean(xml2::ConnectionSegment, true);
  }
  else
  {
    construct.addAttributeBoolean(xml2::StopoverSegment, true);
    construct.addAttributeBoolean(xml2::ConnectionSegment, false);
  }
}

//----------------------------------------------------------------------------
// PricingResponseFormatter::getDirectionality
//----------------------------------------------------------------------------

std::string
PricingResponseFormatter::getDirectionality(const PaxTypeFareRuleData* ptfRuleData)
{
  std::string directionality = "";

  if (ptfRuleData != nullptr && ptfRuleData->categoryRuleItemInfo() != nullptr)
  {
    if (ptfRuleData->categoryRuleItemInfo()->directionality() == RuleConst::FROM_LOC1_TO_LOC2)
      directionality = "FR";
    else if (ptfRuleData->categoryRuleItemInfo()->directionality() == RuleConst::TO_LOC1_FROM_LOC2)
      directionality = "TO";
  }

  return directionality;
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::prepareCat27TourCode
// ----------------------------------------------------------------------------
bool
PricingResponseFormatter::prepareCat27TourCode(const FarePath& farePath,
                                               uint16_t paxNumber,
                                               XMLConstruct& construct)
{
  const CollectedNegFareData* negFareData = farePath.collectedNegFareData();
  bool cat35 = negFareData && negFareData->indicatorCat35();
  if (!cat35 || negFareData->tourCode().empty())
  {
    if (!farePath.multipleTourCodeWarning())
    {
      const std::string& tourCode = farePath.cat27TourCode();

      if (!tourCode.empty())
      {
        construct.addAttribute(xml2::TourCodeDescription, tourCode);
        if (!cat35)
          construct.addAttributeUShort(xml2::PaxFarePassengerNumber, paxNumber);
        return true;
      }
    }
    else
    {
      if (!cat35)
      {
        const int ISSUE_SEPARATE_TICKET_MESSAGE = 163;
        construct.addAttributeUShort(xml2::Cat35Warning, ISSUE_SEPARATE_TICKET_MESSAGE);
      }
    }
  }
  return false;
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::prepareCat35Ticketing
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareCat35Ticketing(PricingTrx& pricingTrx,
                                                const FarePath& farePath,
                                                uint16_t paxNumber,
                                                const CurrencyNoDec& noDecCalc,
                                                const CurrencyNoDec& baseNoDecCalc,
                                                bool cat27TourCode,
                                                XMLConstruct& construct,
                                                bool netSellTkt,
                                                bool& isUnableToOverrideNegDataForComm)
{
  Agent* agent = (pricingTrx.getRequest()) ? pricingTrx.getRequest()->ticketingAgent() : nullptr;
  if (agent && !(agent->axessUser() || agent->tvlAgencyPCC().empty()))
    construct.addAttribute(xml2::TicketFareVendorSource, farePath.tktFareVendor(pricingTrx));

  if (farePath.collectedNegFareData() == nullptr)
    return;

  const CollectedNegFareData& negFareData = *(farePath.collectedNegFareData());

  if (!negFareData.indicatorCat35() || !cat27TourCode)
    construct.addAttributeBoolean(xml2::Cat35Used, negFareData.indicatorCat35());

  if (negFareData.indicatorCat35())
  {
    if (cat27TourCode)
      construct.addAttribute(xml2::Cat35Used, "A");

    construct.addAttribute(xml2::TourCodeDescription, negFareData.tourCode());
    construct.addAttribute(xml2::ValueCode, negFareData.valueCode()); // from cat18 or from
    // DAVC/static value code
    // (NetRemit Optimus)
    construct.addAttribute(xml2::TextBox, negFareData.fareBox());

    if (negFareData.indTypeTour() != BLANK)
    {
      construct.addAttributeChar(xml2::TourIndicator, negFareData.indTypeTour());
    }

    if (negFareData.indNetGross() != BLANK)
    {
      construct.addAttributeChar(xml2::NetGross, negFareData.indNetGross());
    }

    if (negFareData.bspMethod() != BLANK)
    {
      construct.addAttributeChar(xml2::BSPMethodType, negFareData.bspMethod());
    }

    if (negFareData.netRemitTicketInd())
    {
      construct.addAttributeBoolean(xml2::NetRemitTicketIndicator, true);
    }

    construct.addAttributeDouble(xml2::NetFareAmount, negFareData.netTotalAmt(), noDecCalc);
    construct.addAttributeDouble(
        xml2::NetFareAmountPlusCharges, negFareData.netTotalAmtCharges(), noDecCalc);
    construct.addAttributeDouble(
        xml2::NetFareBaseAmount, negFareData.getNetTotalBaseAmtCharges(), baseNoDecCalc);

    if (farePath.commissionPercent() <= HUNDRED)
    {
      const uint16_t Cat35CommissionPercentageNoDec = 2;
      construct.addAttributeDouble(xml2::Cat35CommissionPercentage,
                                   farePath.commissionPercent(),
                                   Cat35CommissionPercentageNoDec);
    }
    else
    {
      construct.addAttributeDouble(xml2::Cat35CommissionPercentage, farePath.commissionPercent());
    }

    construct.addAttributeDouble(
        xml2::Cat35CommissionAmount, farePath.commissionAmount(), noDecCalc);

    if (!fallback::fallbackCommissionManagement(&pricingTrx))
    {
      if (!fallback::fallbackJira1908NoMarkup(&pricingTrx))
        prepareMarkupAndCommissionAmount(pricingTrx, construct, farePath, noDecCalc);
      else
        prepareMarkupAndCommissionAmountOld(construct, farePath, noDecCalc);
    }

    if (negFareData.comPercent() == RuleConst::PERCENT_NO_APPL && negFareData.comAmount() == 0 &&
        pricingTrx.getRequest() &&
        pricingTrx.getRequest()->ticketingAgent()->agentCommissionType().empty())
    {
      construct.addAttributeBoolean(xml2::Cat35BlankCommission, true);
      construct.addAttributeDouble(
          xml2::CommissionBaseAmount, negFareData.commissionBaseAmount(), noDecCalc);
    }

    construct.addAttributeUShort(xml2::PaxFarePassengerNumber, paxNumber);
    construct.addAttributeUShort(xml2::Cat35Warning, negFareData.numberWarningMsg());

    if (!negFareData.trailerMsg().empty() && pricingTrx.getRequest() &&
        (pricingTrx.getRequest()->isTicketEntry() ||
         (pricingTrx.getRequest()->ticketingAgent() &&
          TrxUtil::isNetRemitEnabled(pricingTrx) /*checks for abacus or inifini users*/)))
    {
      isUnableToOverrideNegDataForComm = true;
      if (fallback::fallbackXMLVCCOrderChange(&pricingTrx))
      {
        prepareMessage(construct, Message::TYPE_TICKETING_WARNING, 0, negFareData.trailerMsg());
      }
    }
  }
}

void
PricingResponseFormatter::adjustDiagnosticResponseSize(const Diagnostic& diag)
{
  try
  {
    const uint32_t percentageSize =
        boost::lexical_cast<uint32_t>(diag.diagParamMapItem(Diagnostic::RESPONSE_SIZE));
    if (percentageSize < 100)
      return;

    _maxTotalBuffSize += _maxTotalBuffSize * (static_cast<double>(percentageSize - 100) / 100);
  }
  catch (const boost::bad_lexical_cast&)
  {
  }
}

void
PricingResponseFormatter::prepareResponseText(
    const std::string& responseString,
    XMLConstruct& construct,
    bool noSizeLImit,
    void (PricingResponseFormatter::*msgMhod)(XMLConstruct& construct,
                                              const char msgType,
                                              const uint16_t msgCode,
                                              const std::string& msgText) const,
    const char MessageType) const
{
  std::string tmpResponse = responseString;
  int recNum = 2;
  size_t lastPos = 0;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;

  while (1)
  {
    lastPos = tmpResponse.rfind("\n");
    if (lastPos != std::string::npos && lastPos > 0 && lastPos == (tmpResponse.length() - 1))
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
    else if (tokenLen > AVAILABLE_SIZE && !noSizeLImit)
    {
      LOG4CXX_WARN(logger, "Line: [" << token << "] too long!");
      continue;
    }
    (this->*msgMhod)(construct, MessageType, recNum + 1, token);

    // limit the size of the output returned
    if (construct.getXMLData().size() > _maxTotalBuffSize && !noSizeLImit)
    {
      const char* msg = "RESPONSE TOO LONG FOR CRT";
      (this->*msgMhod)(construct, MessageType, recNum + 2, msg);

      break;
    }
  }
}

void
PricingResponseFormatter::getSaleLoc(PricingTrx& pricingTrx, LocCode& locCode) const
{
  if (!pricingTrx.getRequest())
    return;

  const PricingRequest& request = *pricingTrx.getRequest();
  const Loc* saleLoc;
  if (request.salePointOverride().size() != 0)
  {
    saleLoc = pricingTrx.dataHandle().getLoc(request.salePointOverride(), time(nullptr));
  }
  else
  {
    saleLoc = request.ticketingAgent()->agentLocation();
  }
  locCode = saleLoc->loc();
}

void
PricingResponseFormatter::getTicketLoc(PricingTrx& pricingTrx, LocCode& locCode) const
{
  if (!pricingTrx.getRequest())
    return;

  const PricingRequest& request = *pricingTrx.getRequest();
  const Loc* ticketLoc;
  if (request.ticketPointOverride().size() != 0)
  {
    ticketLoc = pricingTrx.dataHandle().getLoc(request.ticketPointOverride(), time(nullptr));
  }
  else
  {
    ticketLoc = request.ticketingAgent()->agentLocation();
  }
  locCode = ticketLoc->loc();
}
namespace
{
void
getSegmentNumber(const BaggageTravel* baggageTravel, std::set<int>& segmentNumbers)
{
  BOOST_FOREACH (
      TravelSeg* travelSeg,
      std::make_pair(baggageTravel->getTravelSegBegin(), baggageTravel->getTravelSegEnd()))
  {
    if (travelSeg->pnrSegment() && travelSeg->pnrSegment() != ARUNK_PNR_SEGMENT_ORDER)
      segmentNumbers.insert(travelSeg->pnrSegment());
  }
}
}

static Money
selectNonRefundableAmount(PricingTrx& pricingTrx, const FarePath& farePath)
{
  switch (pricingTrx.excTrxType())
  {
  case PricingTrx::AR_EXC_TRX:
    return farePath.getHigherNonRefundableAmount();
  case PricingTrx::AF_EXC_TRX:
    return static_cast<const RexBaseTrx&>(pricingTrx).exchangeItin().front()->getNonRefAmount();
  default:
    ;
  }

  return ExchangeUtil::convertCurrency(pricingTrx,
                                       farePath.getNonrefundableAmount(pricingTrx),
                                       farePath.baseFareCurrency(),
                                       farePath.itin()->useInternationalRounding());
}

void
PricingResponseFormatter::addPassengerInfoTag(const FareCalcConfig& fcConfig,
                                              CalcTotals& calcTotals,
                                              XMLConstruct& construct) const
{
  PaxTypeCode usePaxType;

  {
    if (fcConfig.truePsgrTypeInd() == YES)
      usePaxType = calcTotals.truePaxType;
    else
      usePaxType = calcTotals.requestedPaxType;
    if (usePaxType == CHILD)
    {
      if (calcTotals.farePath->paxType()->age() != 0)
      {
        char cldBuf[10];
        sprintf(cldBuf, "C%02d", calcTotals.farePath->paxType()->age());
        usePaxType = cldBuf;
      }
    }

    construct.openElement(xml2::PassengerInfo);
    construct.addAttribute(xml2::PassengerType, usePaxType);
  }
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::preparePassengerInfo
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::preparePassengerInfo(PricingTrx& pricingTrx,
                                               const FareCalcConfig& fcConfig,
                                               CalcTotals& calcTotals,
                                               uint16_t paxNumber,
                                               XMLConstruct& construct)
{
  bool fpFound = false;
  bool infantMessage = false;
  char nonRefundable = FALSE;
  const bool netSellTkt = (TrxUtil::isCat35TFSFEnabled(pricingTrx) && // cat35 tfsf
                           nullptr != calcTotals.netCalcTotals);

  MoneyAmount moneyAmountAbsorbtion = 0.0;
  const DateTime& ticketingDate = pricingTrx.ticketingDate();

  scanTotalsItin(calcTotals, fpFound, infantMessage, nonRefundable, moneyAmountAbsorbtion);

  if (!fpFound && pricingTrx.getOptions() && pricingTrx.getOptions()->isXoFares())
    return;

  if (calcTotals.farePath->paxType()->number() == 0)
    return;

  addPassengerInfoTag(fcConfig, calcTotals, construct);

  if (calcTotals.netCalcTotals && calcTotals.netCalcTotals->farePath->regularNet())
  {
    PricingRequest* request = pricingTrx.getRequest();
    bool isDirectTicketing = pricingTrx.getRequest()->isTicketEntry();
    NegotiatedFareRuleUtil nfru;

    if(!fallback::fallbackTagPY9matchITBTCCPayment(&pricingTrx))
    {
      if  ( !isDirectTicketing ||
            !nfru.isNetTicketingWithItBtData(pricingTrx, const_cast<FarePath&>(*calcTotals.farePath)) 
            || !request->isFormOfPaymentCard())
      {
        construct.addAttributeBoolean(xml2::Cat35RegularNet, true); // PY9
      }
    }
    else
      construct.addAttributeBoolean(xml2::Cat35RegularNet, true); // PY9
  }
  // Set indicator for Net Remit
  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.addAttributeBoolean(xml2::NetRemitPublishedFareRetrieved, true);
  }

  Money moneyBase(calcTotals.convertedBaseFareCurrencyCode);
  Money moneyCalc(calcTotals.farePath->calculationCurrency());
  construct.addAttribute(xml2::ConstructionCurrencyCode,
                         calcTotals.farePath->calculationCurrency());
  construct.addAttributeDouble(xml2::ConstructionTotalAmount,
                               calcTotals.farePath->getTotalNUCAmount(),
                               moneyCalc.noDec(ticketingDate));

  if (netSellTkt) // Cat 35 TFSF
  {
    construct.addAttributeDouble(xml2::NetConstructionTotalAmount,
                                 calcTotals.netCalcTotals->farePath->getTotalNUCAmount(),
                                 moneyCalc.noDec(ticketingDate));

    if (!calcTotals.netCalcTotals || !calcTotals.netCalcTotals->farePath->regularNet())
    {
      construct.addAttributeBoolean(xml2::Cat35TFSFWithNet, true); // PY6 tag
    }
  }
  if (pricingTrx.hasPriceDynamicallyDeviated())
  {
    const Money& epd = calcTotals.getEffectivePriceDeviation();

    construct.addAttributeDouble(
        xml2::EffectivePriceDeviation, epd.value(), epd.noDec(ticketingDate));
  }

  // Data for Accompanied Travel Restriction validation
  if (!calcTotals.wpaInfo.accTvlData.empty() && !pricingTrx.getRequest()->isSFR())
  {
    construct.addAttribute(xml2::AccTvlData, calcTotals.wpaInfo.accTvlData);
  }

  if (!pricingTrx.getRequest()->isSFR())
  {
    construct.addAttributeBoolean(
        xml2::ReqAccTvl, (!calcTotals.wpaInfo.tktGuaranteed) && calcTotals.wpaInfo.reqAccTvl);
  }

  addAdditionalPaxInfo(pricingTrx, calcTotals, paxNumber, construct);

  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.addAttribute(xml2::NetRemitConstructionCurrencyCode,
                           calcTotals.netRemitCalcTotals->farePath->calculationCurrency());
    construct.addAttributeDouble(xml2::NetRemitConstructionTotalAmount,
                                 calcTotals.netRemitCalcTotals->farePath->getTotalNUCAmount(),
                                 moneyCalc.noDec(ticketingDate));
  }

  construct.addAttribute(xml2::BaseCurrencyCode, calcTotals.convertedBaseFareCurrencyCode);
  construct.addAttributeDouble(
      xml2::BaseFareAmount, calcTotals.convertedBaseFare, moneyBase.noDec(ticketingDate));

  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.addAttribute(xml2::NetRemitBaseCurrencyCode,
                           calcTotals.netRemitCalcTotals->convertedBaseFareCurrencyCode);
    Money moneyConvertBase(calcTotals.netRemitCalcTotals->convertedBaseFareCurrencyCode);
    construct.addAttributeDouble(xml2::NetRemitBaseFareAmount,
                                 calcTotals.netRemitCalcTotals->convertedBaseFare,
                                 moneyConvertBase.noDec(ticketingDate));
  }

  Money moneyEquiv(calcTotals.equivCurrencyCode);

  if (calcTotals.equivCurrencyCode != calcTotals.convertedBaseFareCurrencyCode)
  {
    construct.addAttribute(xml2::EquivalentCurrencyCode, calcTotals.equivCurrencyCode);
    construct.addAttributeDouble(
        xml2::EquivalentAmount, calcTotals.equivFareAmount, moneyEquiv.noDec(ticketingDate));
  }

  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.addAttributeDouble(xml2::NetRemitEquivalentAmount,
                                 calcTotals.netRemitCalcTotals->equivFareAmount,
                                 moneyEquiv.noDec(ticketingDate));
  }

  // Calculation ROE (SDS always needs ROE.)
  if (calcTotals.roeRate > 0.0)
  {
    construct.addAttributeDouble(
        xml2::ExchangeRateOne, calcTotals.roeRate, exchangeRatePrecision + 1);
    construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne, exchangeRatePrecision);
  }

  construct.addAttribute(xml2::PaxFareCurrencyCode, calcTotals.fareCurrencyCode);

  if (pricingTrx.segmentFeeApplied())
  {
    if (calcTotals.equivFareAmount == 0 &&
        calcTotals.convertedBaseFareCurrencyCode != calcTotals.equivCurrencyCode)
    {
      construct.addAttributeDouble(xml2::TotalPerPassengerPlusImposed,
                                   (calcTotals.taxAmount()),
                                   moneyEquiv.noDec(ticketingDate));
    }
    else if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    {
      construct.addAttributeDouble(xml2::TotalPerPassengerPlusImposed,
                                   (calcTotals.convertedBaseFare + calcTotals.taxAmount()),
                                   moneyEquiv.noDec(ticketingDate));
    }
    else
    {
      construct.addAttributeDouble(xml2::TotalPerPassengerPlusImposed,
                                   (calcTotals.equivFareAmount + calcTotals.taxAmount()),
                                   moneyEquiv.noDec(ticketingDate));
    }
    construct.addAttributeDouble(
        xml2::TotalTaxesPlusImposed, calcTotals.taxAmount(), moneyEquiv.noDec(ticketingDate));
  }
  calcTotals.getMutableFcTaxInfo().subtractSupplementalFeeFromTaxAmount();

  if (calcTotals.equivFareAmount == 0 &&
      calcTotals.convertedBaseFareCurrencyCode != calcTotals.equivCurrencyCode)
  {
    construct.addAttributeDouble(
        xml2::TotalPerPassenger, (calcTotals.taxAmount()), moneyEquiv.noDec(ticketingDate));
    if (netSellTkt) // NETSELL option for the direct ticketing entry
    {
      construct.addAttributeDouble(xml2::NetTotalPerPassenger,
                                   (calcTotals.netCalcTotals->taxAmount()),
                                   moneyEquiv.noDec(ticketingDate));
    }
  }
  else if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
  {
    construct.addAttributeDouble(xml2::TotalPerPassenger,
                                 (calcTotals.convertedBaseFare + calcTotals.taxAmount()),
                                 moneyEquiv.noDec(ticketingDate));
    if (netSellTkt) // NETSELL option for the direct ticketing entry
    {
      construct.addAttributeDouble(
          xml2::NetTotalPerPassenger,
          (calcTotals.netCalcTotals->convertedBaseFare + calcTotals.netCalcTotals->taxAmount()),
          moneyEquiv.noDec(ticketingDate));
    }
  }
  else
  {
    construct.addAttributeDouble(xml2::TotalPerPassenger,
                                 (calcTotals.equivFareAmount + calcTotals.taxAmount()),
                                 moneyEquiv.noDec(ticketingDate));
    if (netSellTkt) // NETSELL option for the direct ticketing entry
    {
      construct.addAttributeDouble(
          xml2::NetTotalPerPassenger,
          (calcTotals.netCalcTotals->equivFareAmount + calcTotals.netCalcTotals->taxAmount()),
          moneyEquiv.noDec(ticketingDate));
    }
  }

  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.addAttributeDouble(xml2::NetRemitTotalPerPassenger,
                                 (calcTotals.netRemitCalcTotals->equivFareAmount +
                                  calcTotals.netRemitCalcTotals->taxAmount()),
                                 moneyEquiv.noDec(ticketingDate));
  }

  if (moneyAmountAbsorbtion)
    construct.addAttributeDouble(xml2::TotalPerPassengerPlusAbsorbtion,
                                 (calcTotals.equivFareAmount + calcTotals.taxAmount()) +
                                     moneyAmountAbsorbtion,
                                 moneyEquiv.noDec(ticketingDate));
  construct.addAttributeDouble(
      xml2::TotalTaxes, calcTotals.taxAmount(), moneyEquiv.noDec(ticketingDate));

  if (netSellTkt) // NETSELL option for the direct ticketing entry
  {
    construct.addAttributeDouble(xml2::NetTotalTaxes,
                                 calcTotals.netCalcTotals->taxAmount(),
                                 moneyEquiv.noDec(ticketingDate));
  }

  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.addAttributeDouble(xml2::NetRemitTotalTaxes,
                                 calcTotals.netRemitCalcTotals->taxAmount(),
                                 moneyEquiv.noDec(ticketingDate));
  }

  if ((pricingTrx.getRequest() && pricingTrx.getRequest()->isTicketEntry()) ||
      ((pricingTrx.getRequest() && pricingTrx.getRequest()->ticketingAgent() &&
        (pricingTrx.getRequest()->ticketingAgent()->abacusUser() ||
         pricingTrx.getRequest()->ticketingAgent()->infiniUser())) ||
       !calcTotals.farePath->tktRestricted()))
  {
    if (calcTotals.farePathInfo.commissionPercent <= HUNDRED)
    {
      construct.addAttributeDouble(xml2::CommissionPercentage,
                                   calcTotals.farePathInfo.commissionPercent,
                                   moneyEquiv.noDec(ticketingDate));
    }
    else
    {
      construct.addAttributeDouble(xml2::CommissionPercentage,
                                   calcTotals.farePathInfo.commissionPercent);
    }

    construct.addAttributeDouble(xml2::CommissionAmount,
                                 calcTotals.farePathInfo.commissionAmount,
                                 moneyEquiv.noDec(ticketingDate));
  }
  if (!pricingTrx.getRequest()->isSFR())
  {
    construct.addAttributeDouble(
        xml2::TravelAgencyTax, calcTotals.taxAmount(), moneyEquiv.noDec(ticketingDate));
    construct.addAttribute(xml2::FareCalculation, calcTotals.fareCalculationLine);

    if (pricingTrx.excTrxType() != PricingTrx::AF_EXC_TRX)
    {
      construct.addAttributeChar(xml2::NonRefundable, nonRefundable);
    }

    if (fallback::fixed::noNraAttrInShoppingResponse())
    {
      // During fallback removal please remove also selectNonRefundableAmount() function
      if (fallback::fixed::validateAllCat16Records())
      {
        // During fallback removal please remove also all functions releated with old calculation
        // way
        Money nonRefundableAmt = selectNonRefundableAmount(pricingTrx, *calcTotals.farePath);
        construct.addAttributeDouble(xml2::NonRefundableAmount,
                                     nonRefundableAmt.value(),
                                     nonRefundableAmt.noDec(ticketingDate));
      }
      else
      {
        Money nonRefundableAmt(0, NUC);

        if (nonRefundable == FALSE)
        {
          nonRefundableAmt = calcTotals.farePath->getNonrefundableAmountFromCat16(pricingTrx);
        }
        else
        {
          nonRefundableAmt = selectNonRefundableAmount(pricingTrx, *calcTotals.farePath);
        }
        construct.addAttributeDouble(xml2::NonRefundableAmount,
                                     nonRefundableAmt.value(),
                                     nonRefundableAmt.noDec(ticketingDate));
      }
    }
    else
    {
      const Money nonRefundableAmt =
          CommonParserUtils::nonRefundableAmount(pricingTrx, calcTotals, nonRefundable == TRUE);
      construct.addAttributeDouble(xml2::NonRefundableAmount,
                                   nonRefundableAmt.value(),
                                   nonRefundableAmt.noDec(ticketingDate));
    }
  }

  if (calcTotals.farePath->tktRestricted())
  {
    construct.addAttributeBoolean(xml2::TicketingRestricted, true);
  }

  if (calcTotals.farePath->tfrRestricted())
  {
    construct.addAttributeBoolean(xml2::TfrRestricted, true);
  }

  Indicator privateInd = PrivateIndicator::privateFareIndicator(calcTotals.privateFareIndSeq);
  if (privateInd != ' ')
    construct.addAttributeChar(xml2::PrivateFareInd, privateInd);

  uint16_t stopoverCount = 0;
  MoneyAmount stopoverCharges = 0;
  CurrencyCode pubCurr = "NUC";
  bool stopoverFlag = calcTotals.getStopoverSummary(stopoverCount, stopoverCharges, pubCurr);
  if (stopoverFlag)
  {
    construct.addAttributeUShort(xml2::StopOverCount, stopoverCount);
    construct.addAttributeDouble(xml2::StopOverCharges, stopoverCharges, calcTotals.fclNoDec);
    construct.addAttribute(xml2::StopOverPublishedCurrencyCode, pubCurr);
  }
  // Get Transfer data
  uint16_t transferCount = 0;
  MoneyAmount transferCharges = 0;
  pubCurr = "NUC";
  bool transferFlag = calcTotals.getTransferSummary(transferCount, transferCharges, pubCurr);
  if (transferFlag)
  {
    construct.addAttributeUShort(xml2::TransferCount, transferCount);
    construct.addAttributeDouble(xml2::TransferCharges, transferCharges, calcTotals.fclNoDec);
    construct.addAttribute(xml2::TransferPublishedCurrencyCode, pubCurr);
  }

  bool cat27TourCode = prepareCat27TourCode(*calcTotals.farePath, paxNumber, construct);

  bool isUnableToOverrideNegDataForComm = false;
  prepareCat35Ticketing(pricingTrx,
                        *calcTotals.farePath,
                        paxNumber,
                        moneyEquiv.noDec(ticketingDate),
                        moneyBase.noDec(ticketingDate),
                        cat27TourCode,
                        construct,
                        netSellTkt,
                        isUnableToOverrideNegDataForComm);

  buildSpanishDiscountIndicator(pricingTrx, calcTotals, construct);

  if (pricingTrx.isObFeeApplied())
    setFopBinNumber(pricingTrx.getRequest()->formOfPayment(), construct, xml2::FopBINNumberApplied);

  buildTotalTTypeOBFeeAmount(pricingTrx, calcTotals, construct);

  // If farepath solution has non cat35 fares and JCB PTC then send BT in textbox
  if (!calcTotals.farePath->collectedNegFareData() ||
      !calcTotals.farePath->collectedNegFareData()->indicatorCat35() /*non cat35 fares*/)
  {
    PaxTypeCode ptc = pricingTrx.paxType().front()->paxType();
    if (JCB == ptc || JNN == ptc || JNF == ptc || JNS == ptc)
      construct.addAttribute(xml2::TextBox, "BT");
  }

  if (!fallback::fallbackFRRProcessingRetailerCode(&pricingTrx))
  {
    std::string retailerCode = AdjustedSellingUtil::getRetailerCodeFromFRR(calcTotals);
    if (!retailerCode.empty())
      construct.addAttribute(xml2::FareRetailerCodeNet, retailerCode);
  }

  FuFcIdMap fuFcIdCol;
  if (!fallback::fallbackAMCPhase2(&pricingTrx))
  {
    Indicator commSrcIndicator = getCommissionSourceIndicator(pricingTrx, calcTotals);
    if (commSrcIndicator != ' ')
    {
      construct.addAttributeChar(xml2::CommissionSourceIndicator, commSrcIndicator);
      if (commSrcIndicator == 'A')
        prepareCommissionForValidatingCarriers(pricingTrx,
                                               construct,
                                               *calcTotals.farePath,
                                               fuFcIdCol,
                                               moneyEquiv.noDec(ticketingDate));
      else if (commSrcIndicator == 'C' && !fallback::fallbackAMC2Cat35CommInfo(&pricingTrx))
        constructElementVCCForCat35(
            pricingTrx, construct, *calcTotals.farePath, moneyEquiv.noDec(ticketingDate));
    }
  }
  else
  {
    if (!fallback::fallbackCommissionManagement(&pricingTrx) &&
        (!calcTotals.farePath->collectedNegFareData() ||
         !calcTotals.farePath->collectedNegFareData()->indicatorCat35() /*non cat35 fares*/))
    {
      prepareCommissionForValidatingCarriers(
          pricingTrx, construct, *calcTotals.farePath, fuFcIdCol, moneyEquiv.noDec(ticketingDate));
    }
  }

  if (!fallback::fallbackXMLVCCOrderChange(&pricingTrx) &&
      isUnableToOverrideNegDataForComm &&
      calcTotals.farePath &&
      calcTotals.farePath->collectedNegFareData())
  {
    prepareMessage(construct,
                   Message::TYPE_TICKETING_WARNING,
                   0,
                   calcTotals.farePath->collectedNegFareData()->trailerMsg());
  }

  if (infantMessage)
  {
    prepareMessage(construct, Message::TYPE_WARNING, 0, INFANT_MESSAGE);
  }

  prepareMessages(pricingTrx, calcTotals, construct);

  bool taxExemptProcessed = false;

  if (!pricingTrx.getRequest()->isSFR())
  {
    prepareTaxes(pricingTrx, calcTotals, construct, taxExemptProcessed);


    if ((!fallback::taxRexPricingRefundableInd(&pricingTrx) &&
         !pricingTrx.getTaxInfoResponse().empty()) ||
        (TrxUtil::isAutomatedRefundCat33Enabled(pricingTrx) &&
         ItinSelector(pricingTrx).isRefundTrx()))
    {
      PreviousTaxInformationFormatter(construct).formatPTI(PreviousTaxInformationModel(pricingTrx));
    }

    prepareTaxBSR(pricingTrx, calcTotals, moneyEquiv.noDec(ticketingDate), construct);

    if (calcTotals.useNUC)
      prepareFareIATARate(pricingTrx, calcTotals, construct); // WPDF need this.

    prepareFareBSR(pricingTrx, calcTotals, construct);

    prepareFarePath(pricingTrx,
                    calcTotals,
                    moneyCalc.noDec(ticketingDate),
                    moneyEquiv.noDec(ticketingDate),
                    paxNumber,
                    stopoverFlag,
                    fuFcIdCol,
                    construct);
  }

  if (netSellTkt) // NETSELL option for the direct ticketing entry
  {
    construct.openElement(xml2::NetRemitInfo);
    construct.addAttribute(xml2::FareCalculation, calcTotals.netCalcTotals->fareCalculationLine);
    /* if cat35tfsf  add net commission element */
    if (pricingTrx.getRequest() && !pricingTrx.getRequest()->isTicketEntry())
    {
      if (calcTotals.farePath->collectedNegFareData()->netComPercent() <= HUNDRED)
      {
        const uint16_t Cat35CommissionPercentageNoDec = 2;
        construct.addAttributeDouble(xml2::Cat35CommissionPercentage,
                                     calcTotals.farePath->collectedNegFareData()->netComPercent(),
                                     Cat35CommissionPercentageNoDec);
      }
      else
      {
        construct.addAttributeDouble(xml2::Cat35CommissionPercentage,
                                     calcTotals.farePath->collectedNegFareData()->netComPercent());
      }
      construct.addAttributeDouble(xml2::Cat35CommissionAmount,
                                   calcTotals.farePath->collectedNegFareData()->netComAmount(),
                                   moneyEquiv.noDec(ticketingDate));
    }
    construct.closeElement();

    construct.openElement(xml2::NetRemitInfo);
    bool netTaxExemptProcessed = false;
    prepareTaxes(pricingTrx, *calcTotals.netCalcTotals, construct, netTaxExemptProcessed);
    prepareTaxBSR(
        pricingTrx, *calcTotals.netCalcTotals, moneyEquiv.noDec(ticketingDate), construct);
    construct.closeElement();
  }

  if (calcTotals.netRemitCalcTotals)
  {
    construct.openElement(xml2::NetRemitInfo);
    construct.addAttribute(xml2::FareCalculation,
                           calcTotals.netRemitCalcTotals->fareCalculationLine);
    construct.closeElement();

    construct.openElement(xml2::NetRemitInfo);
    bool netTaxExemptProcessed = false;
    prepareTaxes(pricingTrx, *calcTotals.netRemitCalcTotals, construct, netTaxExemptProcessed);
    prepareTaxBSR(
        pricingTrx, *calcTotals.netRemitCalcTotals, moneyEquiv.noDec(ticketingDate), construct);
    construct.closeElement();
  }
  else if (!calcTotals.netRemitFareCalcLine.empty())
  {
    construct.openElement(xml2::NetRemitInfo);
    construct.addAttribute(xml2::FareCalculation, calcTotals.netRemitFareCalcLine);

    construct.closeElement();
  }

  if (!pricingTrx.getRequest()->isSFR())
  {
    addBillingInfo(pricingTrx, calcTotals, construct);
  }

  if (fallback::extractReissueExchangeFormatter(&pricingTrx))
  {
    // Add <REX><CHG.../><CHG .../></REX> change fee(s)
    RexPricingTrx* rex = dynamic_cast<RexPricingTrx*>(&pricingTrx);
    if (rex)
    {
      construct.openElement(xml2::ReissueExchange);

      Indicator resPenIndicator = calcTotals.farePath->residualPenaltyIndicator(*rex);
      if (resPenIndicator != ' ')
        construct.addAttributeChar(xml2::ResidualIndicator, resPenIndicator);

      const ProcessTagPermutation& winnerPerm = *calcTotals.farePath->lowestFee31Perm();

      construct.addAttributeChar(xml2::FormOfRefundIndicator, winnerPerm.formOfRefundInd());

      const char isTag7Permutation = winnerPerm.hasTag7only() ? 'T' : 'F';
      construct.addAttributeChar(xml2::Tag7PricingSolution, isTag7Permutation);

      electronicTicketInd(winnerPerm.electronicTicket(), construct);

      prepareChangeFee(*rex, construct, calcTotals);

      // Add <TBD.../><TBD .../> taxes on change fee(s)
      prepareTaxesOnChangeFee(pricingTrx, calcTotals, construct);

      // close <REX>
      construct.closeElement();
    }
    else if (pricingTrx.excTrxType() == PricingTrx::AF_EXC_TRX)
    {
      const RefundPricingTrx& trx = static_cast<RefundPricingTrx&>(pricingTrx);

      addReissueExchangeSectionForRefund(
          pricingTrx,
          construct,
          *calcTotals.farePath->lowestFee33Perm(),
          trx.exchangeItin().front()->farePath().front()->pricingUnit());
    }
  }
  else
  {
    ReissueExchangeFormatter(pricingTrx, construct).formatReissueExchange(calcTotals);
  }


  if (calcTotals.adjustedCalcTotal && !fallback::fallbackFRROBFeesFixAsl(&pricingTrx))
    prepareOBFees(pricingTrx, *calcTotals.adjustedCalcTotal, construct);
  else
    prepareOBFees(pricingTrx, calcTotals, construct);


  bool addBDI = checkForStructuredData(pricingTrx,
                                       TrxUtil::isBaggageActivationXmlResponse(pricingTrx),
                                       TrxUtil::isBaggage302ExchangeActivated(pricingTrx),
                                       TrxUtil::isBaggage302GlobalDisclosureActivated(pricingTrx));

  if (addBDI)
  {
    // C  - Day of Check-in Charges
    for (const BaggageTravel* baggageTravel : calcTotals.farePath->baggageTravels())
    {
      if (baggageTravel->shouldAttachToDisclosure())
      {
        std::set<int> segmentNumbers;
        getSegmentNumber(baggageTravel, segmentNumbers);

        buildBDI(pricingTrx,
                 construct,
                 baggageTravel->_allowance,
                 BaggageProvisionType(BAGGAGE_ALLOWANCE),
                 segmentNumbers);

        for (const BaggageCharge* bc : baggageTravel->_charges)
          buildBDI(pricingTrx, construct, bc, BaggageProvisionType(BAGGAGE_CHARGE), segmentNumbers);
      }
    }

    buildCarryOnAllowanceBDI(pricingTrx, construct, calcTotals.farePath->baggageTravelsPerSector());
    buildCarryOnChargesBDI(pricingTrx, construct, calcTotals.farePath->baggageTravelsPerSector());
    buildEmbargoesBDI(pricingTrx, construct, calcTotals.farePath->baggageTravelsPerSector());
  }

  if (const MaxPenaltyResponse* maxPenaltyResponse = calcTotals.farePath->maxPenaltyResponse())
  {
    prepareMaxPenaltyResponse(pricingTrx,
                              *calcTotals.farePath,
                              *maxPenaltyResponse,
                              pricingTrx.ticketingDate(),
                              construct);
  }

  if (calcTotals.adjustedCalcTotal)
    prepareAdjustedCalcTotal(pricingTrx, construct, calcTotals);
  if (pricingTrx.getRequest()->isSFR())
  {
    StructuredRulesResponseFormatter::formatResponse(pricingTrx, calcTotals, construct);
  }
  construct.closeElement();
}




void
PricingResponseFormatter::prepareAdjustedCalcTotal(PricingTrx& trx,
                                                   XMLConstruct& xml,
                                                   CalcTotals& calcTotals)
{
  xml.openElement(xml2::ASLSellingFareData); // SFD

  if (trx.getOptions()->isMslRequest())
    xml.addAttribute(xml2::ASLLayerTypeName, "MSL"); // TYP
  else
    xml.addAttribute(xml2::ASLLayerTypeName, "ADS"); // TYP

  CalcTotals& act = *calcTotals.adjustedCalcTotal;
  if (trx.getOptions() && trx.getOptions()->isPDOForFRRule()) // check requested ORG tag
  {
    prepareHPSItems(trx, xml, act);

    xml.closeElement(); // close SFD
    return;
  }

  Money moneyCalcA(act.farePath->calculationCurrency());
  xml.addAttributeDouble(xml2::ConstructionTotalAmount, // C5E
                         act.farePath->getTotalNUCAmount(),
                         moneyCalcA.noDec(trx.ticketingDate()));

  Money moneyBaseA(act.convertedBaseFareCurrencyCode);
  xml.addAttributeDouble(xml2::BaseFareAmount, // C5A
                         act.convertedBaseFare,
                         moneyBaseA.noDec(trx.ticketingDate()));

  Money moneyEquivA(calcTotals.equivCurrencyCode);
  int moneyEquivANoDec = moneyEquivA.noDec(trx.ticketingDate());

  if (act.equivCurrencyCode != act.convertedBaseFareCurrencyCode)
    xml.addAttributeDouble(xml2::EquivalentAmount, act.equivFareAmount, moneyEquivANoDec); // C5F

  act.getMutableFcTaxInfo().subtractSupplementalFeeFromTaxAmount();
  MoneyAmount adjustedTotalPerPax = 0.0;

  if (act.equivFareAmount == 0 && act.convertedBaseFareCurrencyCode != act.equivCurrencyCode)
    adjustedTotalPerPax = act.taxAmount();
  else if (act.convertedBaseFareCurrencyCode == act.equivCurrencyCode)
    adjustedTotalPerPax = act.convertedBaseFare + act.taxAmount();
  else
    adjustedTotalPerPax = act.equivFareAmount + act.taxAmount();

  xml.addAttributeDouble(xml2::TotalPerPassenger, adjustedTotalPerPax, moneyEquivANoDec); // C66

  xml.addAttributeDouble(xml2::TotalTaxes, act.taxAmount(), moneyEquivANoDec); // C65

  if (!trx.getRequest()->isSFR())
    xml.addAttribute(xml2::FareCalculation, act.fareCalculationLine); // S66

  prepareHPSItems(trx, xml, act);

  bool taxExemptProcessedA = false;
  prepareTaxes(trx, act, xml, taxExemptProcessedA);

  xml.closeElement(); // close SFD
}

void
PricingResponseFormatter::prepareHPSItems(PricingTrx& trx,
                                          XMLConstruct& xml,
                                          CalcTotals& calcTotals) const
{
  for (const auto& item : calcTotals.adjustedSellingDiffInfo)
  {
    xml.openElement(xml2::ASLMarkupSummary); // HPS
    xml.addAttribute(xml2::ASLTypeCode, item.typeCode); // T52
    xml.addAttribute(xml2::ASLDescription, item.description); // N52

    xml.addAttribute(xml2::Amount, item.amount); // C52
    xml.closeElement(); // close HPS
  }
}

void
PricingResponseFormatter::prepareMaxPenaltyResponse(PricingTrx& pricingTrx,
                                                    const FarePath& farePath,
                                                    const MaxPenaltyResponse& maxPenRes,
                                                    const DateTime& ticketingDate,
                                                    XMLConstruct& xml)
{
  auto prepareMissingData = [&](const std::vector<const FareInfo*>& missingDataVec,
                                XMLConstruct& xml)
  {
    std::vector<const PaxTypeFare*> allPTFs;

    if (!fallback::fixSSDSP1780(&pricingTrx) && pricingTrx.isMultiPassengerSFRRequestType())
    {
      allPTFs = smp::getAllPaxTypeFaresForSfrMultipax(farePath,
                                                      *pricingTrx.getMultiPassengerFCMapping());
    }
    else
    {
      allPTFs = smp::getAllPaxTypeFares(farePath);
    }

    for (const FareInfo* fareInfo : missingDataVec)
    {
      const std::pair<const PaxTypeFare*, int16_t> ptfFcNumberPair =
          smp::grabMissingDataFareInformationAndCleanUp(*fareInfo, allPTFs);

      xml.openElement(xml2::PenaltyMissingData);
      xml.addAttribute(xml2::FareBasisCode, ptfFcNumberPair.first->createFareBasis(pricingTrx));
      xml.addAttributeShort(xml2::FareComponentNumber, ptfFcNumberPair.second);
      xml.closeElement();
    }
  };

  auto preparePenalty = [&](const MaxPenaltyResponse::Fee& fee, const bool cat16, XMLConstruct& xml)
  {
    if (cat16 && fee.isMissingData())
    {
      xml.addAttributeBoolean(xml2::IsCategory16, true);
      prepareMissingData(fee._missingDataVec, xml);
      return;
    }

    if (fee._fee)
    {
      xml.addAttributeDouble(
          xml2::MaximumPenaltyAmount, fee._fee->value(), fee._fee->noDec(ticketingDate));
      xml.addAttribute(xml2::MaximumPenaltyCurrency, fee._fee->code());
    }

    if (fee._non)
    {
      xml.addAttributeBoolean(xml2::NonChangeable, true);
    }

    if (cat16)
    {
      xml.addAttributeBoolean(xml2::IsCategory16, true);
    }
  };

  xml.openElement(xml2::PenaltyInformation);

  xml.openElement(xml2::ChangePenaltyBefore);
  preparePenalty(maxPenRes._changeFees._before, maxPenRes._changeFees._cat16 & smp::BEFORE, xml);
  xml.closeElement();

  xml.openElement(xml2::ChangePenaltyAfter);
  preparePenalty(maxPenRes._changeFees._after, maxPenRes._changeFees._cat16 & smp::AFTER, xml);
  xml.closeElement();

  xml.openElement(xml2::RefundPenaltyBefore);
  preparePenalty(maxPenRes._refundFees._before, maxPenRes._refundFees._cat16 & smp::BEFORE, xml);
  xml.closeElement();

  xml.openElement(xml2::RefundPenaltyAfter);
  preparePenalty(maxPenRes._refundFees._after, maxPenRes._refundFees._cat16 & smp::AFTER, xml);
  xml.closeElement();

  xml.closeElement();
}

namespace
{
class PffComparator
{
private:
  PricingTrx* _pricingTrx;
  DateTime _ticketingDate;

  void getEquivPrice(const OCFees* ocFees, Money& equivPrice, MoneyAmount& feeAmount)
  {
    ServiceFeeUtil util(*_pricingTrx);
    feeAmount = ocFees->feeAmount();
    if (ServiceFeeUtil::checkServiceGroupForAcs(ocFees->subCodeInfo()->serviceGroup()))
    {
      equivPrice = util.convertBaggageFeeCurrency(ocFees);
      if (ServiceFeeUtil::isFeeFarePercentage(*(ocFees->optFee())))
      {
        // first convert the currency, next round the original amount
        CurrencyRoundingUtil roundingUtil;
        roundingUtil.round(feeAmount, ocFees->feeCurrency(), *_pricingTrx);
      }
    }
    else
      equivPrice = util.convertOCFeeCurrency(ocFees);
  }

  inline bool compareN41(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->frequentFlyerMileageAppl() == s7second->frequentFlyerMileageAppl();
  }

  inline bool compareP01(const OCFees* ocFees1, const OCFees* ocFees2)
  {
    return ((ocFees1->softMatchS7Status().value() != 0 || (!ocFees1->isFeeGuaranteed())) ==
            (ocFees2->softMatchS7Status().value() != 0 || (!ocFees2->isFeeGuaranteed())));
  }

  inline bool compareN42(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->advPurchTktIssue() == s7second->advPurchTktIssue();
  }

  inline bool compareN43(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->notAvailNoChargeInd() == s7second->notAvailNoChargeInd();
  }

  inline bool compareN44(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->formOfFeeRefundInd() == s7second->formOfFeeRefundInd();
  }

  inline bool compareN45(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->refundReissueInd() == s7second->refundReissueInd();
  }

  inline bool compareP03(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->commissionInd() == s7second->commissionInd();
  }

  inline bool compareP04(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->interlineInd() == s7second->interlineInd();
  }

  inline bool compareB70(const OCFees* ocFees1, const OCFees* ocFees2)
  {
    if ((!ocFees1->farePath() && !ocFees2->farePath()) ||
        (!ocFees1->farePath()->paxType() && !ocFees2->farePath()->paxType()))
    {
      return true;
    }
    else if (ocFees1->farePath() && ocFees1->farePath()->paxType() && ocFees2->farePath() &&
             ocFees2->farePath()->paxType())
    {
      return ocFees1->farePath()->paxType()->paxType() == ocFees2->farePath()->paxType()->paxType();
    }
    else
      return false;
  }

  inline bool compareC50(const OCFees* ocFees1, const OCFees* ocFees2)
  {
    MoneyAmount feeAmount1 = ocFees1->feeAmount();
    MoneyAmount feeAmount2 = ocFees2->feeAmount();
    CurrencyCode currencyCode1 = ocFees1->feeCurrency();
    CurrencyCode currencyCode2 = ocFees2->feeCurrency();

    if (ocFees1->orginalFeeCurrency() != "" && currencyCode1 != ocFees1->orginalFeeCurrency())
    {
      currencyCode1 = ocFees1->orginalFeeCurrency();
      feeAmount1 = ocFees1->orginalFeeAmount();
    }

    if (ocFees2->orginalFeeCurrency() != "" && currencyCode2 != ocFees2->orginalFeeCurrency())
    {
      currencyCode2 = ocFees2->orginalFeeCurrency();
      feeAmount2 = ocFees2->orginalFeeAmount();
    }

    Money equivPrice1(feeAmount1, currencyCode1);
    Money equivPrice2(feeAmount2, currencyCode2);

    DataHandle dataHandle;
    OCFeesPrice* ocFeesPrice1 = OCFeesPrice::create(*ocFees1, *_pricingTrx, dataHandle);
    OCFeesPrice* ocFeesPrice2 = OCFeesPrice::create(*ocFees2, *_pricingTrx, dataHandle);

    const MoneyAmount& firstPrice =
        ocFeesPrice1->getFeeAmountInSellingCurrencyPlusTaxes(*ocFees1, equivPrice1);

    const MoneyAmount& secondPrice =
        ocFeesPrice2->getFeeAmountInSellingCurrencyPlusTaxes(*ocFees2, equivPrice2);

    return firstPrice == secondPrice &&
           equivPrice1.noDec(_ticketingDate) == equivPrice2.noDec(_ticketingDate);
  }

  inline bool compareC51(const OCFees* ocFees1,
                         const OCFees* ocFees2,
                         const Money& equivPrice1,
                         const Money& equivPrice2,
                         const MoneyAmount& feeAmount1,
                         const MoneyAmount& feeAmount2)
  {
    CurrencyCode currencyCode1 = ocFees1->feeCurrency();
    CurrencyCode currencyCode2 = ocFees2->feeCurrency();
    CurrencyNoDec currencyNoDec1 = ocFees1->feeNoDec();
    CurrencyNoDec currencyNoDec2 = ocFees2->feeNoDec();

    if (ocFees1->orginalFeeCurrency() != "" && currencyCode1 != ocFees1->orginalFeeCurrency())
    {
      currencyCode1 = ocFees1->orginalFeeCurrency();
      currencyNoDec1 = ocFees1->orginalFeeNoDec();
    }

    if (ocFees2->orginalFeeCurrency() != "" && currencyCode2 != ocFees2->orginalFeeCurrency())
    {
      currencyCode2 = ocFees2->orginalFeeCurrency();
      currencyNoDec2 = ocFees2->orginalFeeNoDec();
    }

    CurrencyNoDec noDec1 =
        currencyCode1.empty() ? equivPrice1.noDec(_ticketingDate) : currencyNoDec1;
    CurrencyNoDec noDec2 =
        currencyCode2.empty() ? equivPrice2.noDec(_ticketingDate) : currencyNoDec2;

    return feeAmount1 == feeAmount2 && noDec1 == noDec2;
  }

  inline bool compareC5A(const OCFees* ocFees1,
                         const OCFees* ocFees2,
                         const Money& equivPrice1,
                         const Money& equivPrice2)
  {
    CurrencyCode currencyCode1 = ocFees1->feeCurrency();
    CurrencyCode currencyCode2 = ocFees2->feeCurrency();

    if (ocFees1->orginalFeeCurrency() != "" && currencyCode1 != ocFees1->orginalFeeCurrency())
      currencyCode1 = ocFees1->orginalFeeCurrency();

    if (ocFees2->orginalFeeCurrency() != "" && currencyCode2 != ocFees2->orginalFeeCurrency())
      currencyCode2 = ocFees2->orginalFeeCurrency();

    bool isPresent1 = !currencyCode1.empty();
    bool isPresent2 = !currencyCode2.empty();

    if (isPresent1 && isPresent2)
      return currencyCode1 == currencyCode2;
    else
      return isPresent1 == isPresent2;
  }

  inline bool compareC52(const OCFees* ocFees1,
                         const OCFees* ocFees2,
                         const Money& equivPrice1,
                         const Money& equivPrice2)
  {
    bool isPresent1 = !ocFees1->feeCurrency().empty();
    bool isPresent2 = !ocFees2->feeCurrency().empty();

    if (isPresent1 && isPresent2)
      return equivPrice1.value() == equivPrice2.value() &&
             equivPrice1.noDec(_ticketingDate) == equivPrice2.noDec(_ticketingDate);
    else
      return isPresent1 == isPresent2;
  }

  inline bool compareC5B(const OCFees* ocFees1,
                         const OCFees* ocFees2,
                         const Money& equivPrice1,
                         const Money& equivPrice2)
  {
    bool isPresent1 = !ocFees1->feeCurrency().empty();
    bool isPresent2 = !ocFees2->feeCurrency().empty();

    if (isPresent1 && isPresent2)
      return equivPrice1.code() == equivPrice2.code();
    else
      return isPresent1 == isPresent2;
  }

  inline bool compareN21(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->taxInclInd() == s7second->taxInclInd();
  }

public:
  explicit PffComparator(PricingTrx* pricingTrx, const DateTime& ticketingDate)
    : _pricingTrx(pricingTrx), _ticketingDate(ticketingDate)
  {
  }

  bool compare(const OCFees* ocFees1, const OCFees* ocFees2)
  {
    if (!ocFees1 || !ocFees1->subCodeInfo() || !ocFees1->optFee() || !ocFees2 ||
        !ocFees2->subCodeInfo() || !ocFees2->optFee())
      return false;

    const OptionalServicesInfo* s7first = ocFees1->optFee();
    const OptionalServicesInfo* s7second = ocFees2->optFee();

    Money equivPrice1(""), equivPrice2("");
    MoneyAmount feeAmount1, feeAmount2;

    getEquivPrice(ocFees1, equivPrice1, feeAmount1);
    getEquivPrice(ocFees2, equivPrice2, feeAmount2);

    return compareN41(s7first, s7second) && compareP01(ocFees1, ocFees2) &&
           compareN42(s7first, s7second) && compareN43(s7first, s7second) &&
           compareN44(s7first, s7second) && compareN45(s7first, s7second) &&
           compareP03(s7first, s7second) && compareP04(s7first, s7second) &&
           compareB70(ocFees1, ocFees2) && compareC50(ocFees1, ocFees2) &&
           compareC51(ocFees1, ocFees2, equivPrice1, equivPrice2, feeAmount1, feeAmount2) &&
           compareC5A(ocFees1, ocFees2, equivPrice1, equivPrice2) &&
           compareC52(ocFees1, ocFees2, equivPrice1, equivPrice2) &&
           compareC5B(ocFees1, ocFees2, equivPrice1, equivPrice2) && compareN21(s7first, s7second);
  }
};

bool
compareITR(PricingTrx* pricingTrx, const OCFees* ocFees1, const OCFees* ocFees2)
{
  const OptionalServicesInfo* s7first = ocFees1->optFee();
  const OptionalServicesInfo* s7second = ocFees2->optFee();
  std::multimap<ServiceSubTypeCode, int> subCodesMap1;
  std::multimap<ServiceSubTypeCode, int> subCodesMap2;

  FreeBaggageUtil::getServiceSubCodes(*pricingTrx, s7first, subCodesMap1);
  FreeBaggageUtil::getServiceSubCodes(*pricingTrx, s7second, subCodesMap2);

  return s7first->carrier() == s7second->carrier() && subCodesMap1 == subCodesMap2;
}

class BdiComparator
{
private:
  inline bool compareSFK(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->carrier() == s7second->carrier();
  }

  inline bool compareSHK(const SubCodeInfo* s5first, const SubCodeInfo* s5second)
  {
    bool isPresent1 = s5first && !s5first->serviceSubTypeCode().empty();
    bool isPresent2 = s5second && !s5second->serviceSubTypeCode().empty();

    if (isPresent1 && isPresent2)
      return s5first->serviceSubTypeCode() == s5second->serviceSubTypeCode() &&
             s5first->vendor() == s5second->vendor() && s5first->carrier() == s5second->carrier();
    else
      return isPresent1 == isPresent2;
  }

  inline bool compareBPC(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->freeBaggagePcs() == s7second->freeBaggagePcs();
  }

  inline bool compareB20(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->baggageWeight() == s7second->baggageWeight();
  }

  inline bool compareN0D(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->baggageWeightUnit() == s7second->baggageWeightUnit();
  }

  inline bool compareOC1(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->baggageOccurrenceFirstPc() == s7second->baggageOccurrenceFirstPc();
  }

  inline bool compareOC2(const OptionalServicesInfo* s7first, const OptionalServicesInfo* s7second)
  {
    return s7first->baggageOccurrenceLastPc() == s7second->baggageOccurrenceLastPc();
  }

public:
  BdiComparator() {}

  bool compare(const OCFees* ocFees1, const OCFees* ocFees2)
  {
    if (!ocFees1 || !ocFees1->subCodeInfo() || !ocFees1->optFee() || !ocFees2 ||
        !ocFees2->subCodeInfo() || !ocFees2->optFee())
      return false;

    const OptionalServicesInfo* s7first = ocFees1->optFee();
    const OptionalServicesInfo* s7second = ocFees2->optFee();

    const SubCodeInfo* s5first = ocFees1->subCodeInfo();
    const SubCodeInfo* s5second = ocFees2->subCodeInfo();

    return compareSFK(s7first, s7second) && compareSHK(s5first, s5second) &&
           compareBPC(s7first, s7second) && compareB20(s7first, s7second) &&
           compareN0D(s7first, s7second) && compareOC1(s7first, s7second) &&
           compareOC2(s7first, s7second);
  }
};

struct AllowanceComparatorForBDI
    : std::unary_function<const std::pair<const OCFees*, std::set<int>>&, bool>
{
private:
  PricingTrx* _pricingTrx;
  const OCFees* _ocFees;

public:
  AllowanceComparatorForBDI(PricingTrx* pricingTrx, const OCFees* ocFees)
    : _pricingTrx(pricingTrx), _ocFees(ocFees)
  {
  }

  bool operator()(const std::pair<const OCFees*, std::set<int>>& baggageTravelPair) const
  {
    return BdiComparator().compare(_ocFees, baggageTravelPair.first) &&
           compareITR(_pricingTrx, _ocFees, baggageTravelPair.first);
  }
};

struct ChargesComparatorForBDI
    : std::unary_function<const std::pair<const OCFees*, std::set<int>>&, bool>
{
private:
  PricingTrx* _pricingTrx;
  DateTime _ticketingDate;
  const OCFees* _ocFees;

public:
  ChargesComparatorForBDI(PricingTrx* pricingTrx,
                          const OCFees* ocFees,
                          const DateTime& ticketingDate)
    : _pricingTrx(pricingTrx), _ticketingDate(ticketingDate), _ocFees(ocFees)
  {
  }

  bool operator()(const std::pair<const OCFees*, std::set<int>>& baggageTravelPair) const
  {
    return BdiComparator().compare(_ocFees, baggageTravelPair.first) &&
           PffComparator(_pricingTrx, _ticketingDate).compare(_ocFees, baggageTravelPair.first);
  }
};
}

void
PricingResponseFormatter::buildCarryOnAllowanceBDI(
    PricingTrx& pricingTrx,
    XMLConstruct& construct,
    const std::vector<const BaggageTravel*>& baggageTravels)
{
  typedef std::vector<std::pair<const OCFees*, std::set<int>>> OCFeesForAllowanceList;
  OCFeesForAllowanceList ocFeesForAllowance;
  ocFeesForAllowance.reserve(baggageTravels.size());
  for (const BaggageTravel* baggageTravel : baggageTravels)
  {
    if (baggageTravel->shouldAttachToDisclosure())
    {
      OCFeesForAllowanceList::iterator it =
          std::find_if(ocFeesForAllowance.begin(),
                       ocFeesForAllowance.end(),
                       AllowanceComparatorForBDI(&pricingTrx, baggageTravel->_allowance));

      std::set<int> segmentNumbers;
      getSegmentNumber(baggageTravel, segmentNumbers);

      if (it == ocFeesForAllowance.end())
        ocFeesForAllowance.push_back(std::make_pair(baggageTravel->_allowance, segmentNumbers));
      else
        it->second.insert(segmentNumbers.begin(), segmentNumbers.end());
    }
  }

  const OCFees* ocFees;
  std::set<int> segmentNumbers;
  BOOST_FOREACH (std::tie(ocFees, segmentNumbers), ocFeesForAllowance)
  {
    buildBDI(
        pricingTrx, construct, ocFees, BaggageProvisionType(CARRY_ON_ALLOWANCE), segmentNumbers);
  }
}

void
PricingResponseFormatter::buildCarryOnChargesBDI(
    PricingTrx& pricingTrx,
    XMLConstruct& construct,
    const std::vector<const BaggageTravel*>& baggageTravels)
{
  typedef std::vector<std::pair<const OCFees*, std::set<int>>> OCFeesForChargesList;
  OCFeesForChargesList ocFeesForCharges;
  ocFeesForCharges.reserve(baggageTravels.size());
  for (const BaggageTravel* baggageTravel : baggageTravels)
  {
    if (baggageTravel->shouldAttachToDisclosure())
    {
      for (const OCFees* ocFees : baggageTravel->_chargeVector)
      {
        OCFeesForChargesList::iterator it =
            std::find_if(ocFeesForCharges.begin(),
                         ocFeesForCharges.end(),
                         ChargesComparatorForBDI(&pricingTrx, ocFees, _ticketingDate));

        std::set<int> segmentNumbers;
        getSegmentNumber(baggageTravel, segmentNumbers);

        if (it == ocFeesForCharges.end())
          ocFeesForCharges.push_back(std::make_pair(ocFees, segmentNumbers));
        else
          it->second.insert(segmentNumbers.begin(), segmentNumbers.end());
      }
    }
  }

  const OCFees* ocFees;
  std::set<int> segmentNumbers;
  BOOST_FOREACH (std::tie(ocFees, segmentNumbers), ocFeesForCharges)
  {
    buildBDI(pricingTrx, construct, ocFees, CARRY_ON_CHARGE, segmentNumbers);
  }
}

void
PricingResponseFormatter::buildEmbargoesBDI(PricingTrx& pricingTrx,
                                            XMLConstruct& construct,
                                            const std::vector<const BaggageTravel*>& baggageTravels)
{
  typedef std::vector<std::pair<const OCFees*, std::set<int>>> OCFeesForEmbargoesList;
  OCFeesForEmbargoesList ocFeesForEmbargoe;
  ocFeesForEmbargoe.reserve(baggageTravels.size());
  for (const BaggageTravel* baggageTravel : baggageTravels)
  {
    if (baggageTravel->shouldAttachToDisclosure())
    {
      for (const OCFees* ocFees : baggageTravel->_embargoVector)
      {
        OCFeesForEmbargoesList::iterator it =
            std::find_if(ocFeesForEmbargoe.begin(),
                         ocFeesForEmbargoe.end(),
                         ChargesComparatorForBDI(&pricingTrx, ocFees, _ticketingDate));

        std::set<int> segmentNumbers;
        getSegmentNumber(baggageTravel, segmentNumbers);

        if (it == ocFeesForEmbargoe.end())
          ocFeesForEmbargoe.push_back(std::make_pair(ocFees, segmentNumbers));
        else
          it->second.insert(segmentNumbers.begin(), segmentNumbers.end());
      }
    }
  }

  const OCFees* ocFees;
  std::set<int> segmentNumbers;
  BOOST_FOREACH (std::tie(ocFees, segmentNumbers), ocFeesForEmbargoe)
  {
    buildBDI(pricingTrx, construct, ocFees, BaggageProvisionType(BAGGAGE_EMBARGO), segmentNumbers);
  }
}

void
PricingResponseFormatter::buildBDI(PricingTrx& pricingTrx,
                                   XMLConstruct& construct,
                                   const OCFees* ocFees,
                                   const BaggageProvisionType& baggageProvision,
                                   const std::set<int> segmentNumbers)
{
  if (!ocFees || !ocFees->optFee())
    return;

  const SubCodeInfo* subCode = ocFees->subCodeInfo();
  const OptionalServicesInfo* s7 = ocFees->optFee();

  if (subCode)
    _subCodesForOSC.insert(subCode);

  construct.openElement(xml2::BaggageBDI);
  // BPT - Provision Type (e.g. allowance, charges, embargo etc)
  construct.addAttribute(xml2::BaggageProvision, baggageProvision);

  // SFK - Carrier whose baggage provisions apply
  construct.addAttribute(xml2::ServiceOwningCxr, s7->carrier());

  // SHK
  if (subCode && !subCode->serviceSubTypeCode().empty())
  {
    if (CARRY_ON_CHARGE == baggageProvision)
      construct.addAttribute(
          xml2::ExtendedSubCodeKey,
          buildExtendedSubCodeKey(subCode, BaggageProvisionType(BAGGAGE_CHARGE)));
    else
      construct.addAttribute(xml2::ExtendedSubCodeKey,
                             buildExtendedSubCodeKey(subCode, baggageProvision));
  }

  if (baggageProvision == BAGGAGE_ALLOWANCE || baggageProvision == CARRY_ON_ALLOWANCE)
  {
    if (s7->freeBaggagePcs() > 0 || s7->baggageWeightUnit() == ' ')
      // BPC - Number of Pieces
      construct.addAttributeInteger(xml2::BaggageBPC, s7->freeBaggagePcs());

    if (s7->baggageWeightUnit() != ' ')
    {
      // B20 - Weight Limit
      construct.addAttributeInteger(xml2::BaggageSizeWeightLimit, s7->baggageWeight());

      // N0D - Units of the Weight Limit
      construct.addAttributeChar(xml2::BaggageSizeWeightUnitType,
                                 s7->baggageWeightUnit() == 'K' ? 'K' : 'L');
    }
  }

  if (s7->baggageOccurrenceFirstPc() > 0)
    // OC1 - First Occurrence
    construct.addAttributeInteger(xml2::BaggageOC1, s7->baggageOccurrenceFirstPc());

  if (s7->baggageOccurrenceLastPc() > 0)
    // OC2 - Last Occurrence
    construct.addAttributeInteger(xml2::BaggageOC2, s7->baggageOccurrenceLastPc());

  if (pricingTrx.getRequest()->checkSchemaVersion(1, 1, 1) &&
      (ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT) || !ocFees->isDisplayOnly() ||
       checkFFStatus(ocFees)) &&
      ocFees->optFee()->frequentFlyerStatus() != 0)
  { // Q7D - SHB - FQTV carrier filed tier level
    construct.addAttributeShort(xml2::FQTVCxrFiledTierLvl, ocFees->optFee()->frequentFlyerStatus());
  }

  if (TrxUtil::isFrequentFlyerTierActive(pricingTrx))
  {
    if (baggageProvision == BAGGAGE_CHARGE && s7->frequentFlyerStatus() != 0 &&
        !pricingTrx.getRequest()->isSpecificAgencyText() &&
        pricingTrx.getOptions()->isWPwithOutAE() && !PaxTypeUtil::isOnlyOnePassenger(pricingTrx))
    {
      construct.addAttributeChar(xml2::FrequentFlyerWarning, MAY_NOT_APPLY_TO_ALL_PASSENGERS);
    }
  }

  // build ITR or PFF element
  if (baggageProvision == BAGGAGE_ALLOWANCE || baggageProvision == CARRY_ON_ALLOWANCE)
  {
    buildITR(pricingTrx, construct, ocFees, baggageProvision);
  }
  else if (baggageProvision == BAGGAGE_CHARGE || baggageProvision == CARRY_ON_CHARGE ||
           baggageProvision == BAGGAGE_EMBARGO)
  {
    buildPFF(pricingTrx, construct, ocFees);
  }

  for (int segmentNumber : segmentNumbers)
    buildQ00(pricingTrx, construct, segmentNumber);

  construct.closeElement();
}

void
PricingResponseFormatter::buildITR(PricingTrx& pricingTrx,
                                   XMLConstruct& construct,
                                   const OCFees* ocFees,
                                   const BaggageProvisionType& baggageProvision)
{
  const OptionalServicesInfo* s7 = ocFees->optFee();

  if (baggageProvision == BAGGAGE_ALLOWANCE)
  {
    std::vector<ServiceSubTypeCode> subcodes;
    FreeBaggageUtil::getServiceSubCodes(pricingTrx, s7, subcodes);

    const SubCodeInfo* s5 = FreeBaggageUtil::getS5Record(s7->carrier(), subcodes, pricingTrx);
    if (s5)
    {
      _subCodesForOSC.insert(s5);

      construct.openElement(xml2::BaggageITR);
      // BPC - Number of Pieces
      construct.addAttributeInteger(xml2::BaggageBPC, s7->freeBaggagePcs());
      // SHK - Subcode from S5 record.
      construct.addAttribute(xml2::ExtendedSubCodeKey,
                             buildExtendedSubCodeKey(s5, BaggageProvisionType(BAGGAGE_CHARGE)));
      construct.closeElement();
    }
  }
  else if (baggageProvision == CARRY_ON_ALLOWANCE)
  {
    std::multimap<ServiceSubTypeCode, int> subCodesMap;
    FreeBaggageUtil::getServiceSubCodes(pricingTrx, s7, subCodesMap);

    ServiceSubTypeCode serviceSubTypeCode;
    int pieces;

    BOOST_FOREACH (std::tie(serviceSubTypeCode, pieces), subCodesMap)
    {
      const SubCodeInfo* s5 =
          FreeBaggageUtil::getS5RecordCarryOn(s7->carrier(), serviceSubTypeCode, pricingTrx);
      if (s5)
      {
        _subCodesForOSC.insert(s5);

        construct.openElement(xml2::BaggageITR);
        // BPC - Number of Pieces
        construct.addAttributeInteger(xml2::BaggageBPC, pieces);
        // SHK - Subcode from S5 record.
        construct.addAttribute(xml2::ServiceRFICSubCode,
                               buildExtendedSubCodeKey(s5, BaggageProvisionType(BAGGAGE_CHARGE)));
        construct.closeElement();
      }
    }
  }
}

void
PricingResponseFormatter::buildPFF(PricingTrx& pricingTrx,
                                   XMLConstruct& construct,
                                   const OCFees* ocFees)
{
  LOG4CXX_DEBUG(logger, "PricingResponseFormatter::buildPFF() - entered");

  ServiceFeeUtil util(pricingTrx);
  Money equivPrice("");
  MoneyAmount feeAmount = ocFees->feeAmount();
  CurrencyCode currencyCode = ocFees->feeCurrency();
  CurrencyNoDec currencyNoDec = ocFees->feeNoDec();

  if (ocFees->orginalFeeCurrency() != "" && currencyCode != ocFees->orginalFeeCurrency())
  {
    currencyCode = ocFees->orginalFeeCurrency();
    feeAmount = ocFees->orginalFeeAmount();
    currencyNoDec = ocFees->orginalFeeNoDec();
  }

  if (ServiceFeeUtil::checkServiceGroupForAcs(ocFees->subCodeInfo()->serviceGroup()))
  {
    equivPrice = util.convertBaggageFeeCurrency(ocFees);
    if (ServiceFeeUtil::isFeeFarePercentage(*(ocFees->optFee())))
    {
      // first convert the currency, next round the original amount
      CurrencyRoundingUtil roundingUtil;
      roundingUtil.round(feeAmount, ocFees->feeCurrency(), pricingTrx);
    }
  }
  else
  {
    equivPrice = util.convertOCFeeCurrency(ocFees);
  }
  const DateTime& ticketingDate = pricingTrx.ticketingDate();

  // Price and Fulfillment Information
  construct.openElement(xml2::PriceAndFulfillmentInf); // PFF Open

  // N41 - SHG - Fee Application Ind
  construct.addAttributeChar(xml2::FeeApplInd, ocFees->optFee()->frequentFlyerMileageAppl());

  // P01 - SFR - Fee is not Guaranteed
  if (ocFees->softMatchS7Status().value() != 0 || (!ocFees->isFeeGuaranteed()))
  {
    construct.addAttributeBoolean(xml2::FeeGuranInd, true);
  }

  if (ocFees->optFee()->advPurchTktIssue() != ' ')
  { // N42 - SFX - Simultaneous Ticket Indicator
    construct.addAttributeChar(xml2::SimulTktInd, ocFees->optFee()->advPurchTktIssue());
  }

  if (ocFees->optFee()->notAvailNoChargeInd() != ' ')
  { // N43 - SNN - No Charge or Not Available Indicator
    construct.addAttributeChar(xml2::NoChrgNotAvailInd, ocFees->optFee()->notAvailNoChargeInd());
  }

  if (ocFees->optFee()->formOfFeeRefundInd() != ' ')
  { // N44 - SFP - Form of Refund
    construct.addAttributeChar(xml2::FormOfRefund, ocFees->optFee()->formOfFeeRefundInd());
  }

  if (pricingTrx.getRequest()->checkSchemaVersion(1, 1, 0) &&
      !pricingTrx.getRequest()->checkSchemaVersion(1, 1, 1) &&
      (ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT) || !ocFees->isDisplayOnly() ||
       checkFFStatus(ocFees)) &&
      ocFees->optFee()->frequentFlyerStatus() != 0)
  { // Q7D - SHB - FQTV carrier filed tier level
    construct.addAttributeShort(xml2::FQTVCxrFiledTierLvl, ocFees->optFee()->frequentFlyerStatus());
  }

  if (ocFees->optFee()->refundReissueInd() != ' ')
  { // N45 - SFL - Refund Reissue Indicator
    construct.addAttributeChar(xml2::RefundReissueInd, ocFees->optFee()->refundReissueInd());
  }

  if (ocFees->optFee()->commissionInd() != ' ')
  { // P03 - SFL - Commision Indicator
    construct.addAttributeChar(xml2::CommisionInd, ocFees->optFee()->commissionInd());
  }

  if (ocFees->optFee()->interlineInd() != ' ')
  { // P04 - SFL - Interline Indicator
    construct.addAttributeChar(xml2::InterlineInd, ocFees->optFee()->interlineInd());
  }
  // B70 - SHF - Passenger Type Code
  if (ocFees->farePath() && ocFees->farePath()->paxType())
    construct.addAttribute(xml2::PsgrTypeCode, ocFees->farePath()->paxType()->paxType());

  DataHandle dataHandle;
  OCFeesPrice* ocFeesPrice = OCFeesPrice::create(*ocFees, pricingTrx, dataHandle);

  if (currencyCode != "")
  {
    // C50 - SFE - Total Price - Place Holder
    construct.addAttributeDouble(
        xml2::ServiceTotalPrice4OC,
        ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(*ocFees, equivPrice),
        equivPrice.noDec(ticketingDate));

    // C51 - SFA - Base Price
    construct.addAttributeDouble(
        xml2::SUMBasePrice, ocFeesPrice->getBasePrice(*ocFees, feeAmount), currencyNoDec);

    // C5A - SFB - Base Currency
    construct.addAttribute(xml2::SUMBaseCurrencyCode, currencyCode);
  }
  else // If base price currency is blank use equivalent currency code for base price decimal
  // calculation, C51 - SFA - Base Price
  {
    construct.addAttributeDouble(xml2::SUMBasePrice,
                                 ocFeesPrice->getBasePrice(*ocFees, feeAmount),
                                 equivPrice.noDec(ticketingDate));
    // SFC, SFH - Equivalent Price and Equivalent Currency
    // PNR needs this currency code when base price is 0
    construct.addAttribute(xml2::SUMEquiCurCode, equivPrice.code());
  }

  if (ocFees->feeCurrency() != "")
  {
    // C52 - SFC  - Equivalent Base Price
    construct.addAttributeDouble(xml2::SUMEquiBasePrice,
                                 ocFeesPrice->getEquivalentBasePrice(*ocFees, equivPrice.value()),
                                 equivPrice.noDec(ticketingDate));

    // C5B - SFC, SFH - Equivalent Price and Equivalent Currency
    construct.addAttribute(xml2::SUMEquiCurCode, equivPrice.code());
  }

  // N21 - SFI Tax Indicator
  if (ocFees->optFee()->taxInclInd() != ' ')
    construct.addAttributeChar(xml2::TaxInd, ocFees->optFee()->taxInclInd());

  // Close Price and Fulfillment Information
  construct.closeElement(); // PFF Open
}

bool
PricingResponseFormatter::checkFFStatus(const OCFees* ocFees)
{
  return (ocFees->optFee()->serviceSubTypeCode() == "0DG");
}

void
PricingResponseFormatter::buildQ00(PricingTrx& pricingTrx, XMLConstruct& construct, int id)
{
  // Q00 - Price and Fulfillment Information
  construct.openElement(xml2::TravelSegId);
  std::string s = boost::lexical_cast<std::string>(id);
  construct.addElementData(s.c_str());
  construct.closeElement();
}

void
PricingResponseFormatter::preparePassengerForFullRefund(RefundPricingTrx& trx,
                                                        XMLConstruct& construct)
{
  construct.openElement(xml2::PassengerInfo);

  construct.addAttribute(xml2::PassengerType, trx.paxType().front()->paxType());

  if (TrxUtil::isAutomatedRefundCat33Enabled(trx) && !trx.exchangeItin().empty())
  {
    TSE_ASSERT(trx.exchangeItin().front() != nullptr);
    ExcItin& excItin = *trx.exchangeItin().front();

    FareCalcCollector fcCollector;
    fcCollector.initialize(trx, &excItin, FareCalcUtil::getFareCalcConfig(trx), true);

    const CalcTotals* calcTotals = !excItin.farePath().empty()
                                       ? fcCollector.findCalcTotals(excItin.farePath().front())
                                       : nullptr;

    if (calcTotals && calcTotals->netCalcTotals)
    {
      const CalcTotals& netCalcTotals = *calcTotals->netCalcTotals;
      construct.addAttributeDouble(xml2::NetConstructionTotalAmount,
                                   netCalcTotals.farePath->getTotalNUCAmount(),
                                   Money(NUC).noDec(trx.originalTktIssueDT()));

      if (!netCalcTotals.farePath->regularNet())
      {
        construct.addAttributeBoolean(xml2::Cat35TFSFWithNet, true); // PY6 tag
      }
    }
  }

  construct.addAttributeInteger(xml2::WpnOptionNumber, 1);
  construct.addAttribute(xml2::SurchargeCurrencyCode, trx.exchangeItinCalculationCurrency());

  // During fallback removal please remove also selectNonRefundableAmount() function
  Money nonRefundableAmt =
      fallback::fixed::noNraAttrInShoppingResponse()
          ? selectNonRefundableAmount(trx, *trx.exchangeItin().front()->farePath().front())
          : CommonParserUtils::selectNonRefundableAmount(
                trx, *trx.exchangeItin().front()->farePath().front());
  construct.addAttributeDouble(xml2::NonRefundableAmount,
                               nonRefundableAmt.value(),
                               nonRefundableAmt.noDec(trx.ticketingDate()));

  addUsDotItinIndicator(construct, getItin(trx), trx);

  if ((!fallback::taxRexPricingRefundableInd(&trx) &&
       !trx.getTaxInfoResponse().empty()) ||
      TrxUtil::isAutomatedRefundCat33Enabled(trx))
  {
    PreviousTaxInformationFormatter(construct).formatPTI(PreviousTaxInformationModel(trx));
  }

  if (fallback::extractReissueExchangeFormatter(&trx))
  {
    addReissueExchangeSectionForRefund(
        trx,
        construct,
        *trx.fullRefundWinningPermutation(),
        trx.exchangeItin().front()->farePath().front()->pricingUnit());
  }
  else
  {
    ReissueExchangeFormatter(trx, construct)
        .formatReissueExchange(*ReissueExchangeModel::create(trx).get());
  }

  construct.closeElement();
}

void
PricingResponseFormatter::addReissueExchangeSectionForRefund(const PricingTrx& trx,
                                                             XMLConstruct& construct,
                                                             const RefundPermutation& winnerPerm,
                                                             const PricingUnits& pricingUnits)
{
  construct.openElement(xml2::ReissueExchange);
  construct.addAttributeChar(xml2::FormOfRefundIndicator, winnerPerm.formOfRefundInd());
  construct.addAttributeBoolean(xml2::ReissueTaxRefundable, winnerPerm.taxRefundable());

  bool isWinnerPermutationRefundable = winnerPerm.refundable(pricingUnits);

  construct.addAttributeBoolean(xml2::NonRefundable, !isWinnerPermutationRefundable);

  if (isWinnerPermutationRefundable)
    addChangeFeesForRefund(trx, construct, winnerPerm);

  construct.closeElement();
}

void
PricingResponseFormatter::addChangeFeesForRefund(const PricingTrx& trx,
                                                 XMLConstruct& construct,
                                                 const RefundPermutation& winnerPerm)
{
  if (winnerPerm.minimumPenalty().value() > EPSILON)
    addNonZeroFee(trx, construct, winnerPerm.minimumPenalty(), true, false);

  else if (winnerPerm.totalPenalty().value() < EPSILON)
    addZeroFee(trx, construct, winnerPerm);

  else
  {
    for (const auto& pF : winnerPerm.penaltyFees())
      for (const auto& f : pF.second->fee())
        addNonZeroFee(trx, construct, f.amount(), f.highest(), f.nonRefundable());
  }
}

void
PricingResponseFormatter::addZeroFee(const PricingTrx& trx,
                                     XMLConstruct& construct,
                                     const RefundPermutation& winnerPerm)
{
  construct.openElement(xml2::ChangeFee);

  construct.addAttributeDouble(
      xml2::ChangeFeeAmount, winnerPerm.totalPenalty().value(), tse::Money::NUC_DECIMALS);
  construct.addAttributeBoolean(xml2::ChangeFeeNotApplicable, !winnerPerm.waivedPenalty());
  construct.addAttributeBoolean(xml2::ChangeFeeWaived, winnerPerm.waivedPenalty());
  construct.addAttributeBoolean(xml2::HighestChangeFeeIndicator, true);

  construct.addAttributeBoolean(xml2::NonRefundableIndicator, false);

  construct.closeElement();
}

void
PricingResponseFormatter::addNonZeroFee(const PricingTrx& trx,
                                        XMLConstruct& construct,
                                        const Money& fee,
                                        bool highestFee,
                                        bool nonRefundable)
{
  if (fee.value() > EPSILON)
  {
    construct.openElement(xml2::ChangeFee);

    construct.addAttributeDouble(xml2::ChangeFeeAmount, fee.value(), fee.noDec());
    construct.addAttribute(xml2::ChangeFeeCurrency, fee.code());
    construct.addAttributeBoolean(xml2::HighestChangeFeeIndicator, highestFee);

    construct.addAttributeBoolean(xml2::NonRefundableIndicator, nonRefundable);

    construct.closeElement();
  }
}

// depricated, should be removed with fallback EXTRACT_REISSUE_EXCHANGE_FORMATTER
void
PricingResponseFormatter::prepareChangeFee(RexPricingTrx& trx,
                                           XMLConstruct& construct,
                                           CalcTotals& calcTotals)
{
  if (trx.newItin().size())
  {
    // Final Pricing Solution's FarePath
    Itin& itin(*trx.newItin()[0]);
    if (itin.farePath().size())
    {
      FarePath& farePath(*itin.farePath()[0]);

      if (farePath.lowestFee31Perm())
      {
        const ProcessTagPermutation* leastExpensivePermutation = farePath.lowestFee31Perm();

        RexPricingTrx::WaivedChangeRecord3Set& waived(trx.waivedChangeFeeRecord3());
        const RexPricingTrx::WaivedChangeRecord3Set::const_iterator NOT_WAIVED(waived.end());
        // Rated fees orders the fee for identifying the most expensive penalty.
        std::multiset<ComparablePenaltyFee> rated;

        std::vector<ProcessTagInfo*>::const_iterator pti =
            leastExpensivePermutation->processTags().begin();
        std::vector<ProcessTagInfo*>::const_iterator pte =
            leastExpensivePermutation->processTags().end();

        bool waiverCode(false);

        ReissueCharges* reissueCharges(farePath.reissueCharges());
        if (farePath.ignoreReissueCharges() || (reissueCharges && reissueCharges->changeFee == 0.0))
        {
          for (; pti != pte; pti++)
          {
            ProcessTagInfo* info(*pti);
            if (waived.find(info->record3()->orig()) != NOT_WAIVED)
            {
              waiverCode = true;
              break;
            }
          }

          PenaltyFee pf;
          pf.penaltyAmount = 0.0;
          pf.penaltyCurrency = (reissueCharges != nullptr ? reissueCharges->changeFeeCurrency
                                                          : farePath.baseFareCurrency());
          ComparablePenaltyFee fee(pf, trx);
          if (waiverCode)
            fee.waived = true;
          else
            fee.notApplicable = true;
          rated.insert(fee);
        }

        if (!farePath.ignoreReissueCharges() && reissueCharges)
        {
          if (reissueCharges->minAmtApplied)
          {
            PenaltyFee pf;
            pf.penaltyAmount = reissueCharges->changeFee;
            pf.penaltyCurrency = reissueCharges->changeFeeCurrency;
            ComparablePenaltyFee fee(pf, trx);
            rated.insert(fee);
          }
          else
          {
            std::map<const PaxTypeFare*, PenaltyFee*>& penaltyFees(reissueCharges->penaltyFees);
            std::map<const PaxTypeFare*, PenaltyFee*>::iterator pfi(penaltyFees.begin());
            std::map<const PaxTypeFare*, PenaltyFee*>::iterator pfe(penaltyFees.end());
            for (; pfi != pfe; pfi++)
            {
              ComparablePenaltyFee fee(*pfi->second, trx);
              rated.insert(fee);
            }
          }
        }

        std::multiset<ComparablePenaltyFee>::reverse_iterator highestChange(rated.rbegin());
        if (highestChange != rated.rend())
          highestChange->highest = true;

        CurrencyNoDec noDec = Money::NUC_DECIMALS;
        BOOST_REVERSE_FOREACH (const ComparablePenaltyFee& fee, rated)
        {
          if (fee.penaltyCurrency != NUC)
          {
            const Currency* currency = nullptr;
            currency = trx.dataHandle().getCurrency(fee.penaltyCurrency);
            if (currency)
              noDec = currency->noDec();
          }

          construct.openElement(xml2::ChangeFee);
          construct.addAttributeBoolean(xml2::HighestChangeFeeIndicator, fee.highest);
          construct.addAttributeDouble(xml2::ChangeFeeAmount, fee.penaltyAmount, noDec);
          construct.addAttribute(xml2::ChangeFeeCurrency, fee.penaltyCurrency);
          construct.addAttributeBoolean(xml2::ChangeFeeNotApplicable, fee.notApplicable);
          construct.addAttributeBoolean(xml2::ChangeFeeWaived, fee.waived);
          construct.closeElement();
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::electronicTicketInd
// ----------------------------------------------------------------------------

// depricated, should be removed with fallback EXTRACT_REISSUE_EXCHANGE_FORMATTER
void
PricingResponseFormatter::electronicTicketInd(const Indicator& electronicTicketIndicator,
                                              XMLConstruct& construct)
{
  if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_BLANK)
  {
    construct.addAttributeChar(xml2::ElectronicTicketRequired, 'F');
    construct.addAttributeChar(xml2::ElectronicTicketNotAllowed, 'F');
  }
  else if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED)
  {
    construct.addAttributeChar(xml2::ElectronicTicketRequired, 'T');
    construct.addAttributeChar(xml2::ElectronicTicketNotAllowed, 'F');
  }
  else if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED)
  {
    construct.addAttributeChar(xml2::ElectronicTicketRequired, 'F');
    construct.addAttributeChar(xml2::ElectronicTicketNotAllowed, 'T');
  }
}

void
PricingResponseFormatter::addAdditionalPaxInfo(PricingTrx& pricingTrx,
                                               CalcTotals& calcTotals,
                                               uint16_t paxNumber,
                                               XMLConstruct& construct)
{
  addUsDotItinIndicator(construct, calcTotals.farePath->itin(), pricingTrx);

  construct.addAttribute(xml2::RequestedPassengerType, calcTotals.requestedPaxType);
  construct.addAttributeInteger(xml2::WpnOptionNumber, paxNumber);
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::prepareFarePath
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::prepareFarePath(PricingTrx& pricingTrx,
                                          CalcTotals& calcTotals,
                                          const CurrencyNoDec& noDecCalc,
                                          const CurrencyNoDec& noDecEquiv,
                                          uint16_t paxNumber,
                                          bool stopoverFlag,
                                          const FuFcIdMap& fuFcIdCol,
                                          XMLConstruct& construct)
{
  const Itin* itin = calcTotals.farePath->itin();
  std::vector<FarePath*>::const_iterator farePathI;
  farePathI = itin->farePath().begin();
  for (; farePathI != itin->farePath().end(); farePathI++)
  {
    if ((*farePathI)->paxType() != calcTotals.farePath->paxType())
      continue;
    traverseTravelSegs(pricingTrx,
                       calcTotals,
                       noDecCalc,
                       noDecEquiv,
                       **farePathI,
                       paxNumber,
                       stopoverFlag,
                       fuFcIdCol,
                       construct);
  }
}

/// Traverse through travelSegments, align segmentOrder and create CAL
void
PricingResponseFormatter::traverseTravelSegs(PricingTrx& pricingTrx,
                                             CalcTotals& calcTotals,
                                             const CurrencyNoDec& noDecCalc,
                                             const CurrencyNoDec& noDecEquiv,
                                             const FarePath& farePath,
                                             uint16_t paxNumber,
                                             bool stopoverFlag,
                                             const FuFcIdMap& fuFcIdCol,
                                             XMLConstruct& construct)
{
  const Itin* itin = calcTotals.farePath->itin();
  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;
  std::vector<const FareUsage*> fuPlusUpsShown;
  //
  // Loop through the original itin TravelSeg in direction of travel
  //
  uint16_t segmentOrder = 0;
  setBuildPlusUp(pricingTrx);

  std::vector<TravelSeg*>::const_iterator tvlSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndIter = itin->travelSeg().end();

  if (!itin->travelSeg().empty() && RtwUtil::isRtwArunk(pricingTrx, itin->travelSeg().back()))
    --tvlSegEndIter;

  std::vector<MoneyAmount> faeValues;
  std::vector<MoneyAmount> ftsValues;
  uint16_t fcCount = 0;

  for (; tvlSegIter != tvlSegEndIter; tvlSegIter++)
  {
    if (itin->segmentOrder(*tvlSegIter) < segmentOrder)
      continue;

    pricingUnitI = farePath.pricingUnit().begin();
    uint16_t puCount = 0;

    for (; pricingUnitI != farePath.pricingUnit().end(); pricingUnitI++)
    {
      ++puCount;
      fareUsageI = (*pricingUnitI)->fareUsage().begin();

      for (; fareUsageI != (*pricingUnitI)->fareUsage().end(); fareUsageI++)
      {
        std::vector<TravelSeg*>::const_iterator tvlSegFuIter = (*fareUsageI)->travelSeg().begin();
        std::vector<TravelSeg*>::const_iterator tvlSegEndFUIter = (*fareUsageI)->travelSeg().end();

        for (; tvlSegFuIter != tvlSegEndFUIter; tvlSegFuIter++)
        {
          FareUsage* fu = *fareUsageI;

          if (itin->segmentOrder(*tvlSegIter) != itin->segmentOrder(*tvlSegFuIter))
            continue;

          segmentOrder = itin->segmentOrder(*tvlSegIter);

          prepareFareCalcs(pricingTrx,
                           calcTotals,
                           *fu,
                           **pricingUnitI,
                           noDecCalc,
                           farePath,
                           stopoverFlag,
                           puCount,
                           fcCount,
                           segmentOrder,
                           construct,
                           fuPlusUpsShown,
                           faeValues,
                           ftsValues,
                           fuFcIdCol);

          ++fcCount;

          break;
        }
      }
    }
  }

  if (_buildPlusUp)
  {
    // Loop through the pricing units again to get plus ups
    pricingUnitI = farePath.pricingUnit().begin();
    for (; pricingUnitI != farePath.pricingUnit().end(); pricingUnitI++)
    {
      preparePricingUnitPlusUps(**pricingUnitI, noDecCalc, pricingTrx, construct);
    }

    prepareFarePathPlusUps(farePath, noDecCalc, construct);
  }

  // Prepare Plus ups from Net Remit fare Path
  if (calcTotals.netRemitCalcTotals != nullptr)
  {
    construct.openElement(xml2::NetRemitInfo);

    const FarePath& netRemitFp = *(calcTotals.netRemitCalcTotals->farePath);
    std::vector<PricingUnit*>::const_iterator puI = netRemitFp.pricingUnit().begin();
    for (; puI != netRemitFp.pricingUnit().end(); ++puI)
    {
      preparePricingUnitPlusUps(**puI, noDecCalc, pricingTrx, construct);
    }

    prepareFarePathPlusUps(netRemitFp, noDecCalc, construct);

    construct.closeElement();
  }
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::scanTotalsItin
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::scanTotalsItin(CalcTotals& calcTotals,
                                         bool& fpFound,
                                         bool& infantMessage,
                                         char& nonRefundable,
                                         MoneyAmount& moneyAmountAbsorbtion)
{
  const Itin* itin = calcTotals.farePath->itin();
  std::vector<FarePath*>::const_iterator farePathI;
  farePathI = itin->farePath().begin();
  for (; farePathI != itin->farePath().end(); farePathI++)
  {
    const FarePath& farePath = **farePathI;

    if (farePath.paxType() != calcTotals.farePath->paxType())
      continue;

    fpFound = true;

    scanFarePath(farePath, infantMessage, nonRefundable, moneyAmountAbsorbtion);
  }
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::scanFarePath
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::scanFarePath(const FarePath& farePath,
                                       bool& infantMessage,
                                       char& nonRefundable,
                                       MoneyAmount& moneyAmountAbsorbtion)
{
  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::const_iterator fareUsageI;
  pricingUnitI = farePath.pricingUnit().begin();
  for (; pricingUnitI != farePath.pricingUnit().end(); pricingUnitI++)
  {
    const PricingUnit& pricingUnit = **pricingUnitI;

    fareUsageI = pricingUnit.fareUsage().begin();
    for (; fareUsageI != pricingUnit.fareUsage().end(); fareUsageI++)
    {
      const FareUsage& fareUsage = **fareUsageI;

      moneyAmountAbsorbtion += fareUsage.absorptionAdjustment();

      if (fallback::fixed::noNraAttrInShoppingResponse())
      {
        if (fallback::fixed::validateAllCat16Records())
        {
          // please remove also isAppendNR
          if (fareUsage.isAppendNR() &&
              (fareUsage.paxTypeFare()->penaltyRestInd() ==
               YES)) // PL#9760 Per Saranya the NR is in the penaltyRestInd CAT16 penalties.cpp
            nonRefundable = TRUE;
        }

        if (fareUsage.isNonRefundable())
          nonRefundable = TRUE;
      }
    }
  }

  if (!fallback::fixed::noNraAttrInShoppingResponse())
  {
    if (CommonParserUtils::isNonRefundable(farePath))
    {
      nonRefundable = TRUE;
    }
  }

  if (PaxTypeUtil::isInfant(farePath.paxType()->paxType(), farePath.paxType()->vendorCode()))
  {
    infantMessage = true;
  }
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::getValidatingCarrier
// ----------------------------------------------------------------------------
const CarrierCode
PricingResponseFormatter::getValidatingCarrier(const PricingTrx& pricingTrx) const
{
  if (!pricingTrx.getRequest())
    return BAD_CARRIER;

  if (!pricingTrx.getRequest()->validatingCarrier().empty())
  {
    return MCPCarrierUtil::swapToPseudo(&pricingTrx, pricingTrx.getRequest()->validatingCarrier());
  }
  else
  {
    std::vector<Itin*>::const_iterator itinI = pricingTrx.itin().begin();

    for (; itinI != pricingTrx.itin().end(); itinI++)
    {
      if (!(*itinI)->validatingCarrier().empty())
      {
        return MCPCarrierUtil::swapToPseudo(&pricingTrx, (*itinI)->validatingCarrier());
      }
    }
    return BAD_CARRIER;
  }
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::addBillingInfo
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::addBillingInfo(PricingTrx& pricingTrx,
                                         CalcTotals& calcTotals,
                                         XMLConstruct& construct)
{
  if (!pricingTrx.getRequest() || !pricingTrx.getRequest()->ticketingAgent() ||
      pricingTrx.getRequest()->ticketingAgent()->axessUser())
    return;

  if (calcTotals.farePath == nullptr)
    return;

  const Itin* itin = calcTotals.farePath->itin();

  if (itin == nullptr)
    return;

  const AirSeg* airSeg = nullptr;
  bool intlSegFound = false;
  std::vector<TravelSeg*>::const_iterator tvlSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndIter = itin->travelSeg().end();
  if (itin->geoTravelType() == GeoTravelType::International)
  {
    for (; tvlSegIter != tvlSegEndIter; tvlSegIter++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*tvlSegIter);
      if (airSeg == nullptr)
        continue;

      if (airSeg->geoTravelType() == GeoTravelType::International)
      {
        intlSegFound = true;
        break;
      }
    }
  }

  if (!intlSegFound)
    tvlSegIter = itin->travelSeg().begin();

  bool found = false;
  const FarePath& fp = *(calcTotals.farePath);
  std::vector<PricingUnit*>::const_iterator puI = fp.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puE = fp.pricingUnit().end();

  std::vector<FareUsage*>::const_iterator fuI;
  std::vector<FareUsage*>::const_iterator fuE;

  std::vector<TravelSeg*>::const_iterator tvlSegFuIter;
  std::vector<TravelSeg*>::const_iterator tvlSegEndFUIter;

  for (; (puI != puE); puI++)
  {
    const PricingUnit& pu = *(*puI);
    fuI = pu.fareUsage().begin();
    fuE = pu.fareUsage().end();

    for (; (fuI != fuE); fuI++)
    {
      const FareUsage& fareUsage = *(*fuI);
      tvlSegFuIter = fareUsage.travelSeg().begin();
      tvlSegEndFUIter = fareUsage.travelSeg().end();

      for (; (tvlSegFuIter != tvlSegEndFUIter); tvlSegFuIter++)
      {
        if ((*tvlSegIter) == (*tvlSegFuIter))
        {
          found = true;
          break;
        }
      }
      if (found)
        break;
    }
    if (found)
      break;
  }

  if (!found)
    return;

  const FareUsage& fu = *(*fuI);
  construct.openElement(xml2::AbacusBillingData);

  std::string fareCreateDate = fu.paxTypeFare()->createDate().dateToString(YYYYMMDD, "");
  adjustSize(fareCreateDate, 8);
  construct.addAttribute(xml2::ContractIssueDate, fareCreateDate);

  if (pricingTrx.dataHandle().getVendorType(fu.paxTypeFare()->vendor()) == 'T')
  {
    std::string conNum;
    const std::vector<ContractPreference*>& contractPrefVec =
        pricingTrx.dataHandle().getContractPreferences(fu.paxTypeFare()->vendor(),
                                                       fu.paxTypeFare()->carrier(),
                                                       fu.paxTypeFare()->ruleNumber(),
                                                       pricingTrx.ticketingDate());
    if (!contractPrefVec.empty())
      conNum = contractPrefVec.front()->algorithmName();

    adjustSize(conNum, 30);
    construct.addAttribute(xml2::ContractNumber, conNum);
  }

  char tmpBuf[128];
  sprintf(tmpBuf, "%d", fu.paxTypeFare()->fareTariff());
  std::string tariff(tmpBuf);
  adjustSize(tariff, 7);
  construct.addAttribute(xml2::FareTariff, tariff);

  std::string rule = fu.paxTypeFare()->ruleNumber();
  adjustSize(rule, 4);
  construct.addAttribute(xml2::FareRule, rule);

  std::string dbSource = fu.paxTypeFare()->vendor();
  adjustSize(dbSource, 5);
  construct.addAttribute(xml2::DBSource, dbSource);

  std::string cxr = MCPCarrierUtil::swapToPseudo(&pricingTrx, fu.paxTypeFare()->carrier());
  adjustSize(cxr, 2);
  construct.addAttribute(xml2::FareSourceCarrier, cxr);

  if (pricingTrx.dataHandle().getVendorType(fu.paxTypeFare()->vendor()) == 'T')
  {
    std::string farePcc = fu.paxTypeFare()->vendor();
    adjustSize(farePcc, 4);
    construct.addAttribute(xml2::FareSourcePcc, farePcc);
  }

  std::string fareBasis = fu.paxTypeFare()->createFareBasis(pricingTrx, false);
  adjustSize(fareBasis, 15);
  construct.addAttribute(xml2::FareBasisTktDesig, fareBasis);

  std::string orig = fu.paxTypeFare()->fareMarket()->boardMultiCity();
  adjustSize(orig, 3);
  construct.addAttribute(xml2::FareOrig, orig);

  std::string dest = fu.paxTypeFare()->fareMarket()->offMultiCity();
  adjustSize(dest, 3);
  construct.addAttribute(xml2::FareDest, dest);

  construct.closeElement();

  return;
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::adjustSize
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::adjustSize(std::string& attr, const uint16_t len)
{
  if (attr.size() == len)
    return;

  if (attr.size() < len)
  {
    std::string tmpSpaces(len - attr.size(), ' ');
    attr.append(tmpSpaces);
    return;
  }

  attr = attr.substr(0, len);
  return;
}

void
PricingResponseFormatter::prepareHostPortInfo(PricingTrx& pricingTrx, XMLConstruct& construct)
{
  std::vector<std::string> hostInfo;
  std::vector<std::string> buildInfo;
  std::vector<std::string> dbInfo;
  std::vector<std::string> configInfo;

  if (hostDiagString(hostInfo))
  {
    for (const auto& elem : hostInfo)
      prepareResponseText(elem, construct);
  }

  buildDiagString(buildInfo);
  for (const auto& elem : buildInfo)
    prepareResponseText(elem, construct);

  dbDiagString(dbInfo);
  for (const auto& elem : dbInfo)
    prepareResponseText(elem, construct);

  if (pricingTrx.getOptions() && pricingTrx.getOptions()->fareX())
  {
    prepareResponseText("FAREX ENTRY", construct);
  }
  if (configDiagString(configInfo, pricingTrx))
  {
    for (const auto& elem : configInfo)
      prepareResponseText(elem, construct);
  }
}

void
PricingResponseFormatter::setBuildPlusUp(PricingTrx& pricingTrx)
{
  if (pricingTrx.excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    ExchangePricingTrx& excTrx = static_cast<ExchangePricingTrx&>(pricingTrx);
    if (!excTrx.exchangeOverrides().dummyFCSegs().empty())
      _buildPlusUp = false;
  }
}

void
PricingResponseFormatter::isStartOrEndOfSideTrip(const FarePath& fp,
                                                 const TravelSeg* tvlSeg,
                                                 bool& isStart,
                                                 bool& isEnd)
{
  isStart = false;
  isEnd = false;

  for (PricingUnit* pu : fp.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      for (std::vector<TravelSeg*>& sideTrip : fu->paxTypeFare()->fareMarket()->sideTripTravelSeg())
      {
        if (sideTrip.empty())
          continue;

        if (tvlSeg == sideTrip.front())
          isStart = true;
        if (tvlSeg == sideTrip.back())
          isEnd = true;
        if (isStart || isEnd)
          return;
      }
    }
  }
}

void
PricingResponseFormatter::prepareMinMaxTaxInfo(XMLConstruct& construct, const TaxItem& taxItem)
{
  bool minMaxFound = false;
  if (taxItem.minTax() > 0.0)
  {
    construct.addAttributeDouble(
        xml2::MinTaxAmount, taxItem.minTax(), taxItem.minMaxTaxCurrencyNoDec());
    minMaxFound = true;
  }
  if (taxItem.maxTax() > 0.0)
  {
    construct.addAttributeDouble(
        xml2::MaxTaxAmount, taxItem.maxTax(), taxItem.minMaxTaxCurrencyNoDec());
    minMaxFound = true;
  }

  if (minMaxFound)
  {
    construct.addAttribute(xml2::MinMaxTaxCurrency, taxItem.minMaxTaxCurrency());
  }

  construct.addAttributeDouble(xml2::TaxRateUsed, taxItem.taxAmt(), taxItem.taxNodec());
}

CurrencyCode
PricingResponseFormatter::retrieveCurrencyCode(const CalcTotals* calc) const
{
  CurrencyCode code{};
  if (calc != nullptr)
    code = calc->equivCurrencyCode.empty() ? calc->convertedBaseFareCurrencyCode
                                           : calc->equivCurrencyCode;

  return code;
}

bool
PricingResponseFormatter::shouldCalculateMaxOBFee(const uint32_t limit,
                                                  const FareCalcCollector& collector) const
{
  return std::any_of(collector.passengerCalcTotals().cbegin(),
                     collector.passengerCalcTotals().cend(),
                     [limit](const auto* calc)
                     {
    return calc->farePath->processed() && limit < calc->farePath->collectedTktOBFees().size();
  });
}

// ----------------------------------------------------------------------------
// PricingResponseFormatter::OBFees
// ----------------------------------------------------------------------------
void
PricingResponseFormatter::checkLimitOBFees(PricingTrx& pricingTrx,
                                           FareCalcCollector& fareCalcCollector)
{
  if (!pricingTrx.getRequest()->isCollectOBFee())
    return;

  const bool calculateOnlyMaxOBFee = !fallback::virtualFOPMaxOBCalculation(&pricingTrx) &&
                                     pricingTrx.getRequest()->returnMaxOBFeeOnly();
  uint32_t maxOBFeesOptions = calculateOnlyMaxOBFee ? 1 : TrxUtil::getConfigOBFeeOptionMaxLimit();

  // At last one PTC has more than limit matching fees
  // trigger for finding maximum one.
  // However we do this iff we didn't explicitly requested the
  // Maximum OB Fee.
  if (!calculateOnlyMaxOBFee && !shouldCalculateMaxOBFee(maxOBFeesOptions, fareCalcCollector))
    return;

  std::pair<const TicketingFeesInfo*, MoneyAmount> maximum(nullptr, MoneyAmount{});
  CurrencyCode paymentCurrency;
  for (const auto* totals : fareCalcCollector.passengerCalcTotals())
  {
    if (totals->farePath->processed())
    {
      bool isZeroMax = false;
      if (maxOBFeesOptions < totals->farePath->collectedTktOBFees().size())
        isZeroMax = checkForZeroMaximum(pricingTrx, *totals);

      if (isZeroMax && totals->farePath->collectedTktOBFees().empty())
        continue;
      if (!isZeroMax)
      {
        std::pair<const TicketingFeesInfo*, MoneyAmount> maxForPax =
            computeMaximumOBFeesPercent(pricingTrx, *totals);
        if (maxForPax.first)
        {
          if (maximum.second < maxForPax.second)
            maximum = std::move(maxForPax);
        }
      }
      if (paymentCurrency.empty())
        paymentCurrency = retrieveCurrencyCode(totals);
    }
  }
  if (!maximum.first)
  {
    // When calculating the max obfee, zero fees are ignored. However, in the
    // worst case we still need to show one entry (max ob fee) even if it is zero.
    // For this reason, in case we have maximum.first == nullptr (all zeroes)
    // we simply return the last.
    if (!calculateOnlyMaxOBFee || fareCalcCollector.passengerCalcTotals().empty())
      return;

    const auto& lastElement = fareCalcCollector.passengerCalcTotals().back();
    const auto& lastElementFees = lastElement->farePath->collectedTktOBFees();
    if (lastElementFees.empty())
      return;

    maximum.first = lastElementFees.back();
    maximum.second = MoneyAmount{};

    paymentCurrency = retrieveCurrencyCode(lastElement);
  }

  const FarePath* lastOnePTC = clearAllFeesAndGetLastPTC(fareCalcCollector.passengerCalcTotals());

  const_cast<FarePath*>(lastOnePTC)->maximumObFee() =
      mockOBFeeInPaymentCurrency(pricingTrx, *(maximum.first), maximum.second, paymentCurrency);
  _limitMaxOBFees = true;
}

const FarePath*
PricingResponseFormatter::clearAllFeesAndGetLastPTC(const std::vector<CalcTotals*>& calcTotals)
    const
{
  std::vector<CalcTotals*>::const_iterator totalsIter = calcTotals.begin();
  const std::vector<CalcTotals*>::const_iterator totalsIterEnd = calcTotals.end();

  const FarePath* lastOnePTC = nullptr;

  for (; totalsIter != totalsIterEnd; totalsIter++)
  {
    if ((*totalsIter)->farePath->processed())
    {
      const_cast<FarePath*>((*totalsIter)->farePath)->collectedTktOBFees().clear();

      lastOnePTC = (*totalsIter)->farePath;
    }
  }

  return lastOnePTC;
}

bool
PricingResponseFormatter::checkForZeroMaximum(PricingTrx& pricingTrx, const CalcTotals& calcTotals)
    const
{
  const std::vector<TicketingFeesInfo*>::const_iterator feesBegin =
      calcTotals.farePath->collectedTktOBFees().begin();
  const std::vector<TicketingFeesInfo*>::const_iterator feesEnd =
      calcTotals.farePath->collectedTktOBFees().end();

  if (find_if(feesBegin, feesEnd, NonZeroAmount()) != feesEnd)
    return false;

  // Apparently this is not what we want. The current logic is basically broken
  // when we want to calculate the max of all zeroes: we do not return anything.
  // For MAX OB Fees we want to -ALWAYS- have a max ob fee, even if it is zero.
  const bool calculateOnlyMaxOBFee = !fallback::virtualFOPMaxOBCalculation(&pricingTrx) &&
                                     pricingTrx.getRequest()->returnMaxOBFeeOnly();
  if (calculateOnlyMaxOBFee)
    return true;

  // all fees zero, make last matched zero fee only one for this PTC
  // but only when it not contain FOP
  std::vector<TicketingFeesInfo*> lastZeroIfNotFOP;
  if (calcTotals.farePath->collectedTktOBFees().back()->fopBinNumber().empty())
    lastZeroIfNotFOP.push_back(calcTotals.farePath->collectedTktOBFees().back());

  const_cast<FarePath*>(calcTotals.farePath)->collectedTktOBFees() = lastZeroIfNotFOP;
  return true;
}

MoneyAmount
PricingResponseFormatter::calculateObFeeAmountFromAmountMax(PricingTrx& pricingTrx,
                                                            const CalcTotals& calcTotals,
                                                            const TicketingFeesInfo* feeInfo) const
{
  if (feeInfo->feeAmount() < 0.0 ||
      (calcTotals.equivCurrencyCode != "" && feeInfo->cur() == calcTotals.equivCurrencyCode) ||
      (calcTotals.equivCurrencyCode.empty() &&
       feeInfo->cur() == calcTotals.convertedBaseFareCurrencyCode))
  {
    return feeInfo->feeAmount();
  }
  else
  {
    Money targetMoney = convertOBFeeCurrency(pricingTrx, calcTotals, feeInfo);
    return targetMoney.value();
  }
}

MoneyAmount
PricingResponseFormatter::calculateObFeeAmountFromPercentageMax(PricingTrx& pricingTrx,
                                                                const CalcTotals& calcTotals,
                                                                const TicketingFeesInfo* feeInfo)
    const
{
  MoneyAmount totalPaxAmount = calcTotals.getTotalAmountPerPax();
  MoneyAmount calcAmount = calculateResidualObFeeAmount(pricingTrx, totalPaxAmount, feeInfo);
  const CurrencyCode& paymentCurrency = calcTotals.equivCurrencyCode.empty()
                                            ? calcTotals.convertedBaseFareCurrencyCode
                                            : calcTotals.equivCurrencyCode;

  Money targetMoneyC(calcAmount, paymentCurrency);
  roundOBFeeCurrency(pricingTrx, targetMoneyC);
  calcAmount = targetMoneyC.value();

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  Money targetMoney(paymentCurrency);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); //+rounding
  maxAmount = targetMoney.value();

  return getLowestObFeeAmount(feeInfo->maxFeeCur(), calcAmount, maxAmount);
}

std::pair<const TicketingFeesInfo*, MoneyAmount>
PricingResponseFormatter::computeMaximumOBFeesPercent(PricingTrx& pricingTrx,
                                                      const CalcTotals& calcTotals)
{
  // TODO: When removing this fallback we should completely rewrite this method by
  // using a range for loop.
  const auto& fees = calcTotals.farePath->collectedTktOBFees();
  auto feesBegin = fees.begin();
  auto feesEnd = fees.end();

  std::pair<const TicketingFeesInfo*, MoneyAmount> maximumForPax(nullptr, 0.0);
  for (; feesBegin != feesEnd; ++feesBegin)
  {
    const TicketingFeesInfo* feeInfo = *feesBegin;
    MoneyAmount feeAmt = 0.0;
    if (feeInfo->feePercent() > 0.0)
      feeAmt = calculateObFeeAmountFromPercentageMax(pricingTrx, calcTotals, feeInfo);
    else
      feeAmt = calculateObFeeAmountFromAmountMax(pricingTrx, calcTotals, feeInfo);

    if (maximumForPax.second < feeAmt)
    {
      maximumForPax.first = feeInfo;
      maximumForPax.second = feeAmt;
    }
  }

  if (!maximumForPax.first && !fees.empty())
  {
    if (fallback::virtualFOPMaxOBCalculation(&pricingTrx))
      maximumForPax.first = *feesBegin;
    else
      maximumForPax.first = *fees.begin();
  }

  return maximumForPax;
}

const TicketingFeesInfo*
PricingResponseFormatter::mockOBFeeInPaymentCurrency(PricingTrx& pricingTrx,
                                                     const TicketingFeesInfo& sourceFee,
                                                     const MoneyAmount& feeAmount,
                                                     const CurrencyCode& paymentCurrency) const
{
  TicketingFeesInfo* convertedFee;
  pricingTrx.dataHandle().get(convertedFee);
  convertedFee->getValuableData(sourceFee);

  Money targetMoney(feeAmount, paymentCurrency);
  roundOBFeeCurrency(pricingTrx, targetMoney);
  convertedFee->cur() = paymentCurrency;
  convertedFee->feeAmount() = targetMoney.value();
  convertedFee->noDec() = targetMoney.noDec();

  return convertedFee;
}

void
PricingResponseFormatter::prepareOBFee(PricingTrx& pricingTrx,
                                       CalcTotals& calcTotals,
                                       XMLConstruct& construct,
                                       const TicketingFeesInfo* feeInfo,
                                       MoneyAmount feeAmt,
                                       MoneyAmount fareAmtWith2CCFee) const
{
  construct.openElement(xml2::ServiceFeeDetailedInfo);

  construct.addAttribute(xml2::ServiceTypecode,
                         feeInfo->serviceTypeCode() + feeInfo->serviceSubTypeCode());

  MoneyAmount totalFeeFareAmount(0.0);
  CurrencyNoDec noDec = 0;

  if (OBFeesUtils::fallbackObFeesWPA(&pricingTrx))
  {
    if (Money::isZeroAmount(fareAmtWith2CCFee) && !pricingTrx.isProcess2CC())
      calculateObFeeAmount(pricingTrx, calcTotals, construct, feeInfo, totalFeeFareAmount);
  }
  else
  {
    if (Money::isZeroAmount(fareAmtWith2CCFee) && !pricingTrx.isProcess2CC())
    {
      calculateObFeeAmount(pricingTrx, calcTotals, feeInfo, totalFeeFareAmount, noDec);
      construct.addAttributeDouble(xml2::ServiceFeeAmount, totalFeeFareAmount, noDec);
    }
  }

  OBFeeSubType subType = TrxUtil::getOBFeeSubType(feeInfo->serviceSubTypeCode());
  if (subType == OBFeeSubType::OB_F_TYPE && !_limitMaxOBFees)
  {
    totalFeeFareAmount += calcTotals.getTotalAmountPerPax();
    Money moneyEquiv(calcTotals.equivCurrencyCode);
    if (Money::isZeroAmount(fareAmtWith2CCFee) && !pricingTrx.isProcess2CC())
      construct.addAttributeDouble(xml2::ServiceFeeAmountTotal,
                                   totalFeeFareAmount,
                                   moneyEquiv.noDec(pricingTrx.ticketingDate()));
    else
    {
      construct.addAttributeDouble(
          xml2::ServiceFeeAmount, feeAmt, moneyEquiv.noDec(pricingTrx.ticketingDate()));

      construct.addAttributeDouble(xml2::ServiceFeeAmountTotal,
                                   fareAmtWith2CCFee,
                                   moneyEquiv.noDec(pricingTrx.ticketingDate()));
    }
  }

  construct.addAttribute(xml2::FopBINNumber, feeInfo->fopBinNumber());
  std::ostringstream combineITATAind;
  combineITATAind << feeInfo->refundReissue() << feeInfo->commission() << feeInfo->interline();
  construct.addAttribute(xml2::IATAindCombined, combineITATAind.str());
  construct.addAttributeChar(xml2::NoChargeInd, feeInfo->noCharge());
  if (feeInfo->feePercent() > 0.0)
  {
    construct.addAttributeDouble(
        xml2::ServiceFeePercent, feeInfo->feePercent(), feeInfo->feePercentNoDec());
    const CurrencyCode& paymentCurrency = calcTotals.equivCurrencyCode.empty()
                                              ? calcTotals.convertedBaseFareCurrencyCode
                                              : calcTotals.equivCurrencyCode;
    MoneyAmount maxAmount = feeInfo->maxFeeAmount();
    if (feeInfo->maxFeeCur() == paymentCurrency)
    {
      construct.addAttributeDouble(xml2::MaxServiceFeeAmt, maxAmount, feeInfo->maxFeeNoDec());
    }
    else
    {
      const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
      Money targetMoney(paymentCurrency);
      convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); //+ do rounding
      maxAmount = targetMoney.value();
      construct.addAttributeDouble(
          xml2::MaxServiceFeeAmt, maxAmount, targetMoney.noDec(pricingTrx.ticketingDate()));
    }
  }

  static ObFeeDescriptors obFeeDesc;
  std::string serviceDescription = feeInfo->commercialName().empty()
                                       ? obFeeDesc.getDescription(feeInfo->serviceSubTypeCode())
                                       : feeInfo->commercialName();
  construct.addAttribute(xml2::ServiceDescription, serviceDescription);

  if ((!fallback::fallbackValidatingCxrMultiSp(&pricingTrx) || pricingTrx.overrideFallbackValidationCXRMultiSP())
      && calcTotals.farePath &&
      !calcTotals.farePath->defaultValidatingCarrier().empty())
  {
    construct.addAttribute(xml2::ValidatingCxrCode,
                           calcTotals.farePath->defaultValidatingCarrier());
  }
}

void
PricingResponseFormatter::prepareOBFee(PricingTrx& pricingTrx,
                                       CalcTotals& calcTotals,
                                       XMLConstruct& construct,
                                       TicketingFeesInfo* feeInfo,
                                       const FopBinNumber& fopBin,
                                       const MoneyAmount& chargeAmount,
                                       const CurrencyNoDec& numDec,
                                       const MoneyAmount& feeAmt,
                                       const MoneyAmount& fareAmtWith2CCFee) const
{
  if (feeInfo)
    prepareOBFee(pricingTrx, calcTotals, construct, feeInfo, feeAmt, fareAmtWith2CCFee);
  else
  {
    construct.openElement(xml2::ServiceFeeDetailedInfo);
    Money moneyEquiv(calcTotals.equivCurrencyCode);
    construct.addAttributeDouble(xml2::ServiceFeeAmountTotal,
                                 fareAmtWith2CCFee,
                                 moneyEquiv.noDec(pricingTrx.ticketingDate()));
  }

  construct.addAttribute(xml2::RequestedBin, fopBin);
  construct.addAttributeDouble(xml2::CardChargeAmount, chargeAmount, numDec);
  construct.closeElement();
}

void
PricingResponseFormatter::calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                                                         CalcTotals& calcTotals,
                                                         XMLConstruct& construct,
                                                         const TicketingFeesInfo* feeInfo,
                                                         MoneyAmount& totalFeeFareAmount,
                                                         bool fillXML) const
{
  if (feeInfo->feeAmount() < 0.0 ||
      (calcTotals.equivCurrencyCode != "" && feeInfo->cur() == calcTotals.equivCurrencyCode) ||
      (calcTotals.equivCurrencyCode.empty() &&
       feeInfo->cur() == calcTotals.convertedBaseFareCurrencyCode))
  {
    if (fillXML)
      construct.addAttributeDouble(xml2::ServiceFeeAmount, feeInfo->feeAmount(), feeInfo->noDec());
    totalFeeFareAmount = feeInfo->feeAmount();
  }
  else
  {
    const DateTime& ticketingDate = pricingTrx.ticketingDate();
    Money targetMoney = convertOBFeeCurrency(pricingTrx, calcTotals, feeInfo);
    if (fillXML)
      construct.addAttributeDouble(
          xml2::ServiceFeeAmount, targetMoney.value(), targetMoney.noDec(ticketingDate));
    totalFeeFareAmount = targetMoney.value();
  }
}

void
PricingResponseFormatter::calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                                                         CalcTotals& calcTotals,
                                                         const TicketingFeesInfo* feeInfo,
                                                         MoneyAmount& totalFeeFareAmount,
                                                         CurrencyNoDec& noDec) const
{
  if (feeInfo->feeAmount() < 0.0 ||
      (calcTotals.equivCurrencyCode != "" && feeInfo->cur() == calcTotals.equivCurrencyCode) ||
      (calcTotals.equivCurrencyCode.empty() &&
       feeInfo->cur() == calcTotals.convertedBaseFareCurrencyCode))
  {
    totalFeeFareAmount = feeInfo->feeAmount();
    noDec = feeInfo->noDec();
  }
  else
  {
    const DateTime& ticketingDate = pricingTrx.ticketingDate();
    Money targetMoney = convertOBFeeCurrency(pricingTrx, calcTotals, feeInfo);
    totalFeeFareAmount = targetMoney.value();
    noDec = targetMoney.noDec(ticketingDate);
  }
}

void
PricingResponseFormatter::calculateObFeeAmount(PricingTrx& pricingTrx,
                                               CalcTotals& calcTotals,
                                               XMLConstruct& construct,
                                               const TicketingFeesInfo* feeInfo,
                                               MoneyAmount& totalFeeFareAmount,
                                               const MoneyAmount& chargeAmount) const
{
  if (feeInfo->feePercent() > 0)
    calculateObFeeAmountFromPercentage(
        pricingTrx, calcTotals, construct, feeInfo, totalFeeFareAmount, chargeAmount);
  else
    calculateObFeeAmountFromAmount(pricingTrx, calcTotals, construct, feeInfo, totalFeeFareAmount);
}

void
PricingResponseFormatter::calculateObFeeAmount(PricingTrx& pricingTrx,
                                               CalcTotals& calcTotals,
                                               const TicketingFeesInfo* feeInfo,
                                               MoneyAmount& totalFeeFareAmount,
                                               CurrencyNoDec& noDec,
                                               const MoneyAmount& chargeAmount) const
{
  if (feeInfo->feePercent() > 0)
    calculateObFeeAmountFromPercentage(
        pricingTrx, calcTotals, feeInfo, totalFeeFareAmount, noDec, chargeAmount);
  else
    calculateObFeeAmountFromAmount(pricingTrx, calcTotals, feeInfo, totalFeeFareAmount, noDec);
}

MoneyAmount
PricingResponseFormatter::calculateResidualObFeeAmount(PricingTrx& pricingTrx,
                                                       const MoneyAmount& totalPaxAmount,
                                                       const TicketingFeesInfo* feeInfo) const
{
  bool residual = pricingTrx.getRequest()->chargeResidualInd();
  MoneyAmount amountFop = pricingTrx.getRequest()->paymentAmountFop();
  MoneyAmount calcAmt = 0.0;

  if (residual && amountFop != 0)
    calcAmt = (totalPaxAmount - amountFop);
  else if (!residual)
    calcAmt = amountFop;
  else
    calcAmt = totalPaxAmount;

  if (calcAmt <= 0.0)
    calcAmt = 0.0;
  MoneyAmount calcAmount = (calcAmt * feeInfo->feePercent()) / 100.0f;
  return calcAmount;
}

MoneyAmount
PricingResponseFormatter::getLowestObFeeAmount(const CurrencyCode& maxFeeCur,
                                               const MoneyAmount& calcAmount,
                                               const MoneyAmount& maxAmount) const
{
  if (maxFeeCur.empty())
    return calcAmount;
  if (calcAmount > maxAmount)
    return maxAmount;
  return calcAmount;
}

void
PricingResponseFormatter::calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                                             CalcTotals& calcTotals,
                                                             XMLConstruct& construct,
                                                             const TicketingFeesInfo* feeInfo,
                                                             MoneyAmount& totalFeeFareAmount,
                                                             const MoneyAmount& chargeAmount) const
{
  if (_limitMaxOBFees) // calculation+conversion was done by Max
    construct.addAttributeDouble(xml2::ServiceFeeAmount, feeInfo->feeAmount(), feeInfo->noDec());
  else
  {
    Money targetMoney(NUC);
    calculateObFeeAmountFromPercentage(
        pricingTrx, calcTotals, feeInfo, targetMoney, totalFeeFareAmount, chargeAmount);
    construct.addAttributeDouble(
        xml2::ServiceFeeAmount, totalFeeFareAmount, targetMoney.noDec(pricingTrx.ticketingDate()));
  }
}

void
PricingResponseFormatter::calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                                             CalcTotals& calcTotals,
                                                             const TicketingFeesInfo* feeInfo,
                                                             MoneyAmount& totalFeeFareAmount,
                                                             CurrencyNoDec& noDec,
                                                             const MoneyAmount& chargeAmount) const
{
  if (_limitMaxOBFees) // calculation+conversion was done by Max
  {
    totalFeeFareAmount = feeInfo->feeAmount();
    noDec = feeInfo->noDec();
  }
  else
  {
    Money targetMoney(NUC);
    calculateObFeeAmountFromPercentage(
        pricingTrx, calcTotals, feeInfo, targetMoney, totalFeeFareAmount, chargeAmount);

    noDec = targetMoney.noDec(pricingTrx.ticketingDate());
  }
}

void
PricingResponseFormatter::calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                                             CalcTotals& calcTotals,
                                                             const TicketingFeesInfo* feeInfo,
                                                             Money& targetMoney,
                                                             MoneyAmount& totalFeeFareAmount,
                                                             const MoneyAmount& chargeAmount) const
{
  MoneyAmount calcAmount;
  if (!Money::isZeroAmount(chargeAmount))
    calcAmount = (chargeAmount * feeInfo->feePercent()) / 100.0f;
  else
  {
    OBFeeSubType subType = TrxUtil::getOBFeeSubType(feeInfo->serviceSubTypeCode());

    if (subType == OBFeeSubType::OB_T_TYPE || subType == OBFeeSubType::OB_R_TYPE)
    {
      MoneyAmount moneyAmount;
      if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
        moneyAmount = calcTotals.convertedBaseFare;
      else
        moneyAmount = calcTotals.equivFareAmount;

      calcAmount = calculateResidualObFeeAmount(pricingTrx, moneyAmount, feeInfo);
    }
    else
    {
      calcAmount =
          calculateResidualObFeeAmount(pricingTrx, calcTotals.getTotalAmountPerPax(), feeInfo);
    }
  }

  const CurrencyCode& paymentCurrency = calcTotals.equivCurrencyCode.empty()
                                            ? calcTotals.convertedBaseFareCurrencyCode
                                            : calcTotals.equivCurrencyCode;

  Money targetMoneyC(calcAmount, paymentCurrency);
  roundOBFeeCurrency(pricingTrx, targetMoneyC);
  calcAmount = targetMoneyC.value();

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  targetMoney.setCode(paymentCurrency);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); //+rounding
  maxAmount = targetMoney.value();

  MoneyAmount lowestCalcAmount = getLowestObFeeAmount(feeInfo->maxFeeCur(), calcAmount, maxAmount);
  totalFeeFareAmount = lowestCalcAmount;
}

void
PricingResponseFormatter::prepareOBFees(PricingTrx& pricingTrx,
                                        CalcTotals& calcTotals,
                                        XMLConstruct& construct)
{
  if (!OBFeesUtils::fallbackObFeesWPA(&pricingTrx) && pricingTrx.isSingleMatch())
    return;
  size_t maxNumFType = 0, maxNumTType = 0, maxNumRType = 0;
  OBFeeUtil::getNumberOfOBFees(
      pricingTrx, *calcTotals.farePath, maxNumFType, maxNumTType, maxNumRType);

  if (pricingTrx.getRequest()->isCollectOBFee())
  {
    const bool calculateOnlyMaxOBFee = !fallback::virtualFOPMaxOBCalculation(&pricingTrx) &&
                                       pricingTrx.getRequest()->returnMaxOBFeeOnly();
    auto max = calcTotals.farePath->maximumObFee();
    bool shouldAlwaysDisplay = max != nullptr;
    if (!calculateOnlyMaxOBFee)
      shouldAlwaysDisplay = shouldAlwaysDisplay && (EPSILON < max->feeAmount());

    if (shouldAlwaysDisplay)
    {
      const TicketingFeesInfo* feeInfo = max;

      prepareOBFee(pricingTrx, calcTotals, construct, feeInfo);
      construct.addAttributeChar(xml2::ShowNoObFees, 'T');
      std::ostringstream trailerMsg;
      trailerMsg << "MAXIMUM AMOUNT PER PASSENGER - ";
      trailerMsg.precision(Money(feeInfo->cur()).noDec(pricingTrx.ticketingDate()));
      trailerMsg.setf(std::ios::fixed, std::ios::floatfield);
      trailerMsg << feeInfo->feeAmount();
      construct.addAttribute(xml2::MessageText, trailerMsg.str());

      construct.closeElement();
      // this statement is added to display MAX item only and do not display anything else
      // it'll happening only when there are more than 50 OB Fees are qualified
    }
    else
    {
      const std::vector<TicketingFeesInfo*>& collectedOBFees =
          calcTotals.farePath->collectedTktOBFees();

      if (pricingTrx.isProcess2CC() && 2 == collectedOBFees.size())
        prepare2CardsOBFee(pricingTrx, calcTotals, construct, collectedOBFees);
      else
        prepareAllOBFees(pricingTrx, calcTotals, construct, maxNumFType, collectedOBFees);
    }
  }

  if (pricingTrx.getRequest()->isCollectTTypeOBFee())
    prepareAllOBFees(pricingTrx,
                     calcTotals,
                     construct,
                     maxNumTType,
                     calcTotals.farePath->collectedTTypeOBFee());

  if (pricingTrx.getRequest()->isCollectRTypeOBFee())
    prepareAllOBFees(pricingTrx,
                     calcTotals,
                     construct,
                     maxNumRType,
                     calcTotals.farePath->collectedRTypeOBFee());

}

void
PricingResponseFormatter::prepare2CardsOBFee(PricingTrx& pricingTrx,
                                             CalcTotals& calcTotals,
                                             XMLConstruct& construct,
                                             const std::vector<TicketingFeesInfo*>& collectedOBFees)
{
  MoneyAmount secondCardChargeAmount =
      std::min(pricingTrx.getRequest()->paymentAmountFop(), calcTotals.getTotalAmountPerPax());
  MoneyAmount firstCardChargeAmount = calcTotals.getTotalAmountPerPax() - secondCardChargeAmount;
  Money moneyEquiv(calcTotals.equivCurrencyCode);
  CurrencyNoDec numDec = moneyEquiv.noDec(pricingTrx.ticketingDate());

  MoneyAmount totalFeeFareAmount, feeAmt1(0.0), feeAmt2(0.0);
  if (OBFeesUtils::fallbackObFeesWPA(&pricingTrx))
  {
    if (collectedOBFees.front() && !Money::isZeroAmount(firstCardChargeAmount))
      calculateObFeeAmount(pricingTrx,
                           calcTotals,
                           construct,
                           collectedOBFees.front(),
                           feeAmt1,
                           firstCardChargeAmount);
    if (collectedOBFees.back() && !Money::isZeroAmount(secondCardChargeAmount))
      calculateObFeeAmount(pricingTrx,
                           calcTotals,
                           construct,
                           collectedOBFees.back(),
                           feeAmt2,
                           secondCardChargeAmount);
  }
  else
  {
    CurrencyNoDec noDec = 0;
    if (collectedOBFees.front() && !Money::isZeroAmount(firstCardChargeAmount))
      calculateObFeeAmount(
          pricingTrx, calcTotals, collectedOBFees.front(), feeAmt1, noDec, firstCardChargeAmount);
    if (collectedOBFees.back() && !Money::isZeroAmount(secondCardChargeAmount))
      calculateObFeeAmount(
          pricingTrx, calcTotals, collectedOBFees.back(), feeAmt2, noDec, secondCardChargeAmount);
    construct.addAttributeDouble(xml2::ServiceFeeAmount, totalFeeFareAmount, noDec);
  }

  totalFeeFareAmount = calcTotals.getTotalAmountPerPax() + feeAmt1 + feeAmt2;

  prepareOBFee(pricingTrx,
               calcTotals,
               construct,
               collectedOBFees.front(),
               pricingTrx.getRequest()->formOfPayment(),
               firstCardChargeAmount,
               numDec,
               feeAmt1,
               totalFeeFareAmount);
  prepareOBFee(pricingTrx,
               calcTotals,
               construct,
               collectedOBFees.back(),
               pricingTrx.getRequest()->secondFormOfPayment(),
               secondCardChargeAmount,
               numDec,
               feeAmt2,
               totalFeeFareAmount);
}

void
PricingResponseFormatter::prepareAllOBFees(PricingTrx& pricingTrx,
                                           CalcTotals& calcTotals,
                                           XMLConstruct& construct,
                                           size_t maxOBFeesOptions,
                                           const std::vector<TicketingFeesInfo*>& collectedOBFees)
{
  if (collectedOBFees.empty())
    return;

  uint32_t numProcessed = 0;

  for (TicketingFeesInfo* feeInfo : collectedOBFees)
  {
    if (numProcessed >= maxOBFeesOptions)
      break;
    prepareOBFee(pricingTrx, calcTotals, construct, feeInfo);
    construct.closeElement();
    ++numProcessed;
  }
}

Money
PricingResponseFormatter::convertOBFeeCurrency(PricingTrx& pricingTrx,
                                               const CalcTotals& calcTotals,
                                               const TicketingFeesInfo* feeInfo) const
{
  const Money sourceMoney(feeInfo->feeAmount(), feeInfo->cur());
  Money targetMoney(calcTotals.equivCurrencyCode);

  if (calcTotals.equivCurrencyCode.empty() &&
      feeInfo->cur() != calcTotals.convertedBaseFareCurrencyCode)
  {
    targetMoney.setCode(calcTotals.convertedBaseFareCurrencyCode);
  }

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney);

  return targetMoney;
}

void
PricingResponseFormatter::convertOBFeeCurrency(PricingTrx& pricingTrx,
                                               const Money& sourceMoney,
                                               Money& targetMoney) const
{
  CurrencyConversionFacade converter;
  converter.convert(targetMoney, sourceMoney, pricingTrx, false, CurrencyConversionRequest::TAXES);
  roundOBFeeCurrency(pricingTrx, targetMoney);
}

void
PricingResponseFormatter::roundOBFeeCurrency(PricingTrx& pricingTrx, Money& targetMoney) const
{
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;

  if (getFeeRounding(pricingTrx, targetMoney.code(), roundingFactor, roundingNoDec, roundingRule))
  {
    CurrencyConverter curConverter;
    curConverter.round(targetMoney, roundingFactor, roundingRule);
  }
}

bool
PricingResponseFormatter::getFeeRounding(PricingTrx& pricingTrx,
                                         const CurrencyCode& currencyCode,
                                         RoundingFactor& roundingFactor,
                                         CurrencyNoDec& roundingNoDec,
                                         RoundingRule& roundingRule) const
{
  const DateTime& tickDate = pricingTrx.ticketingDate();
  const Currency* currency = nullptr;
  currency = pricingTrx.dataHandle().getCurrency(currencyCode);

  if (!currency)
  {
    LOG4CXX_ERROR(logger, "DBAccess getCurrency returned null currency pointer");
    return false;
  }

  if (currency->taxOverrideRoundingUnit() > 0)
  {
    roundingFactor = currency->taxOverrideRoundingUnit();
    roundingNoDec = currency->taxOverrideRoundingUnitNoDec();
    roundingRule = currency->taxOverrideRoundingRule();

    return true;
  }

  const std::string controllingEntityDesc = currency->controllingEntityDesc();
  LOG4CXX_INFO(logger, "Currency country description: " << currency->controllingEntityDesc());

  bool foundNationalCurrency = false;
  bool foundNation = false;
  NationCode nationWithMatchingNationalCurrency;
  NationCode nationCode;

  CurrencyUtil::getControllingNationCode(pricingTrx,
                                         controllingEntityDesc,
                                         nationCode,
                                         foundNation,
                                         foundNationalCurrency,
                                         nationWithMatchingNationalCurrency,
                                         tickDate,
                                         currencyCode);

  if (foundNation)
  {
    const TaxNation* taxNation = pricingTrx.dataHandle().getTaxNation(nationCode, tickDate);

    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  else if (foundNationalCurrency)
  {
    const TaxNation* taxNation =
        pricingTrx.dataHandle().getTaxNation(nationWithMatchingNationalCurrency, tickDate);

    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  return false;
}

Indicator
PricingResponseFormatter::cabinChar(Indicator& cabin) const
{
  if (cabin == PREMIUM_FIRST_CLASS_ANSWER)
    return PREMIUM_FIRST_CLASS;
  if (cabin == PREMIUM_ECONOMY_CLASS_ANSWER)
    return PREMIUM_ECONOMY_CLASS;
  return cabin;
}

void
PricingResponseFormatter::addDateAttr(const DateTime& date,
                                      std::string attr,
                                      XMLConstruct& construct)
{
  std::string dateString = date.dateToSqlString();
  if (dateString != NO_DATE_STRING)
    construct.addAttribute(attr, dateString);
}

void
PricingResponseFormatter::formatOCFeesResponse(XMLConstruct& construct,
                                               PricingTrx& pricingTrx,
                                               ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger)
{
  Itin* itin = getItin(pricingTrx);

  bool prefixInd = addPrefixWarningForOCTrailer(pricingTrx, true);

  // If WP and check if R7 Tuning logic
  if (pricingTrx.getOptions() && !pricingTrx.getOptions()->isProcessAllGroups())
  {
    if (isR7TuningAndWP(pricingTrx, construct, prefixInd))
    {
      return;
    }
  }

  if (itin->ocFeesGroup().empty()) // no Service Group
  {
    isTimeOutBeforeStartOCFees(pricingTrx, construct, prefixInd);
    return;
  }

  if (anyTimeOutMaxCharCountIssue(pricingTrx, construct, prefixInd))
    return;

  if (isGenericTrailer(pricingTrx, construct, prefixInd))
    return;

  if (!buildOCFeesFullResponse(pricingTrx, construct, prefixInd, ocFeesMerger))
  {
    if (pricingTrx.getOptions() && !pricingTrx.getOptions()->serviceGroupsVec().empty()) // WPAE-XX
      timeOutMaxCharCountRequestedOCFeesReturned(pricingTrx, construct, prefixInd);
    else
      timeOutMaxCharCountNoOCFeesReturned(pricingTrx, construct, prefixInd);
  }
}

bool
PricingResponseFormatter::buildOCFeesFullResponse(PricingTrx& pricingTrx,
                                                  XMLConstruct& constructA,
                                                  const bool prefixInd,
                                                  ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger)
{
  XMLConstruct construct;
  construct.openElement(xml2::OCFeesDisplayInfo);

  if (prefixInd)
    construct.addAttributeBoolean(xml2::AttnInd, prefixInd);

  if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    formatOCHeaderMsg(construct);
  }
  formatOCFees(pricingTrx, construct, /*timeOutMax=*/false, ocFeesMerger);

  construct.closeElement();

  if (construct.getXMLData().size() > (_maxTotalBuffSize - 20 - constructA.getXMLData().size()))
    return false;

  constructA.addElementData(construct);
  return true;
}

bool
PricingResponseFormatter::isR7TuningAndWP(PricingTrx& pricingTrx,
                                          XMLConstruct& construct,
                                          const bool prefixInd)
{
  Itin* itin = getItin(pricingTrx);

  if (itin->moreFeesAvailable())
  {
    construct.openElement(xml2::OCFeesDisplayInfo); // Start OCM
    if (prefixInd)
      construct.addAttributeBoolean(xml2::AttnInd, prefixInd); // ATTN indicator

    construct.addAttribute(xml2::OCGroupStatusCode, "AE"); // ST0 - AE
    construct.closeElement(); // Close for OCM
    return true;
  } // for else cond. there will be no OCM
  return false;
}

void
PricingResponseFormatter::formatOCHeaderMsg(XMLConstruct& construct)
{
  construct.openElement(xml2::OCGroupHeader);
  prepareResponseText(
      "AIR EXTRAS", construct, false, &PricingResponseFormatter::prepareSimpleMessage);
  construct.closeElement();
}

bool
PricingResponseFormatter::isGenericTrailer(PricingTrx& pricingTrx,
                                           XMLConstruct& construct,
                                           const bool prefixInd)
{
  Itin* itin = getItin(pricingTrx);

  if (pricingTrx.getOptions() &&
      (pricingTrx.getOptions()->isProcessAllGroups() ||
       !pricingTrx.getOptions()->serviceGroupsVec().empty())) // WPAE or WPAE-XX
  {
    if (itin->allSegsUnconfirmed()) // All segs Unconfirm
    {
      return builTrailerOCF(
          pricingTrx, construct, "AIR EXTRAS APPLICABLE TO CONFIRMED SEGMENTS ONLY", prefixInd);
    }
    else if (!checkIfAnyGroupValid(pricingTrx)) // Non Ticketed Itin and no groups
    {
      return builTrailerOCF(pricingTrx, construct, "AIR EXTRAS NOT FOUND", prefixInd);
    }
  }
  else if (pricingTrx.getOptions() && !pricingTrx.getOptions()->isProcessAllGroups() &&
           pricingTrx.getOptions()->serviceGroupsVec().empty() && !checkIfAnyGroupValid(pricingTrx))
  {
    construct.openElement(xml2::OCFeesDisplayInfo);

    if (prefixInd)
      construct.addAttributeBoolean(xml2::AttnInd, prefixInd);

    createOCGSection(pricingTrx, construct); // create OCG section
    construct.closeElement();
    return true;
  }

  return false;
}

bool
PricingResponseFormatter::builTrailerOCF(PricingTrx& pricingTrx,
                                         XMLConstruct& construct,
                                         std::string msg,
                                         const bool prefixInd)
{
  construct.openElement(xml2::OCFeesDisplayInfo);

  if (prefixInd)
    construct.addAttributeBoolean(xml2::AttnInd, prefixInd);
  if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    formatOCGenericMsg(construct, msg);
  }
  createOCGSection(pricingTrx, construct); // create OCG section
  construct.closeElement();
  return true;
}

void
PricingResponseFormatter::createOCGSection(PricingTrx& pricingTrx, XMLConstruct& construct)
{
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter, sfgIterEnd;
  Itin* itin = getItin(pricingTrx);
  sfgIter = itin->ocFeesGroup().begin();
  sfgIterEnd = itin->ocFeesGroup().end();

  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    const ServiceGroup sfgGroupCode = sfg->groupCode();
    construct.openElement(xml2::OCGroupInfo);
    construct.addAttribute(xml2::OCFeeGroupCode, sfgGroupCode);
    construct.closeElement();
  }
}

bool
PricingResponseFormatter::checkIfAnyGroupValid(PricingTrx& pricingTrx)
{
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter, sfgIterEnd;
  Itin* itin = getItin(pricingTrx);
  sfgIter = itin->ocFeesGroup().begin();
  sfgIterEnd = itin->ocFeesGroup().end();

  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    if (sfg->state() == ServiceFeesGroup::VALID)
      return true;
  }
  return false;
}

void
PricingResponseFormatter::replaceSpecialCharInDisplay(std::string& srvcGrpDesc)
{
  std::replace(srvcGrpDesc.begin(), srvcGrpDesc.end(), '(', '-');
  std::replace(srvcGrpDesc.begin(), srvcGrpDesc.end(), ')', '-');
}

bool
PricingResponseFormatter::addPrefixWarningForOCTrailer(PricingTrx& pricingTrx, bool warning)
{
  // Check if we should set indicator for ATTN*
  if (warning)
  {
    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(pricingTrx);
    if (fcConfig && fcConfig->warningMessages() == FareCalcConsts::FC_YES)
      return true;
  }
  return false;
}

void
PricingResponseFormatter::formatOCFees(PricingTrx& pricingTrx,
                                       XMLConstruct& construct,
                                       bool timeOutMax,
                                       ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger)
{
  Itin* itin = getItin(pricingTrx);
  std::vector<ServiceGroup> groupCodes; // requested Group codes
  pricingTrx.getOptions()->getGroupCodes(pricingTrx.getOptions()->serviceGroupsVec(), groupCodes);

  uint16_t maxNumberOfOCFees = maxNumberOfFees.getValue();
  uint16_t feesCount = 0;
  bool maxNumOfFeesReached = false;
  uint16_t dispOnlyFeesCount = 0;
  std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages>>> groupFeesVector;

  // iterate groups
  // we want to check if all fees are display only - in this case we don't display the trailer
  // indicators
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter = itin->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = itin->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    const ServiceGroup sfgGroupCode = sfg->groupCode();
    std::vector<PaxOCFeesUsages> paxOcFees;
    switch (sfg->state())
    {
    case ServiceFeesGroup::VALID:
    {
      if (timeOutMax &&
          (find(groupCodes.begin(), groupCodes.end(), sfgGroupCode) == groupCodes.end()))
      {
        // group code is NOT in the request
        ServiceFeesGroup* sfg1 = const_cast<ServiceFeesGroup*>(sfg);
        sfg1->state() = ServiceFeesGroup::EMPTY;
        break;
      }
      if (maxNumOfFeesReached)
      {
        break;
      }
      ServiceFeeUtil::createOCFeesUsages(*sfg, pricingTrx);
      if (ocFeesMerger)
      {
        paxOcFees = ocFeesMerger->mergeFeesUsagesForSfg(*sfg);
        auto predOcfIsDisplayOnly = [](PaxOCFeesUsages& ocf)
        {
          return ocf.fees()->isDisplayOnly() ||
                 ocf.fees()->optFee()->notAvailNoChargeInd() == 'X' ||
                 ocf.fees()->optFee()->notAvailNoChargeInd() == 'G';
        };
        feesCount += paxOcFees.size();
        dispOnlyFeesCount +=
            std::count_if(paxOcFees.begin(), paxOcFees.end(), predOcfIsDisplayOnly);
        maxNumOfFeesReached = ocFeesMerger->isMaxFeesCountReached();
      }
      else
      {
        paxOcFees = ServiceFeeUtil::getSortedFeesUsages(*sfg, pricingTrx.paxType());
        // iterate fees
        std::vector<PaxOCFeesUsages>::iterator paxOcFeesIter = paxOcFees.begin();
        std::vector<PaxOCFeesUsages>::iterator paxOcFeesIterEnd = paxOcFees.end();
        for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter, ++feesCount)
        {
          if (feesCount >= maxNumberOfOCFees)
          {
            maxNumOfFeesReached = true;
            // remove fees exceeding MAX_NUMBER_OF_FEES
            paxOcFees.erase(paxOcFeesIter, paxOcFeesIterEnd);
            break;
          }
          if ((*paxOcFeesIter).fees()->isDisplayOnly() ||
              (*paxOcFeesIter).fees()->optFee()->notAvailNoChargeInd() == 'X' ||
              (*paxOcFeesIter).fees()->optFee()->notAvailNoChargeInd() == 'G')
          {
            dispOnlyFeesCount++;
          }
        }
      }
    }
    break;

    case ServiceFeesGroup::EMPTY:
    case ServiceFeesGroup::NOT_AVAILABLE:
      break;

    default:
      break;
    }
    groupFeesVector.push_back(std::make_pair(sfg, paxOcFees));
  }

  bool allFeesDisplayOnly = false;
  if (feesCount && (isOcFeesTrxDisplayOnly(pricingTrx) || (dispOnlyFeesCount == feesCount)))
  {
    allFeesDisplayOnly = true;
  }

  if (maxNumOfFeesReached || timeOutMax)
  {
    if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
    {
      formatOCGenericMsg(construct, "MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS");
    }
  }

  formatOCFeesGroups(pricingTrx, construct, groupFeesVector, allFeesDisplayOnly, timeOutMax);

  if (allFeesDisplayOnly)
  {
    formatOCTrailer(construct, "AL"); // ST0 - AL
  }
}

const char
PricingResponseFormatter::getFootnoteByHirarchyOrder(const OCFeesUsage& fee) const
{
  if (fee.isDisplayOnly() || fee.optFee()->notAvailNoChargeInd() == 'X' ||
      fee.optFee()->notAvailNoChargeInd() == 'G')
    return '*';
  if (fee.subCodeInfo()->serviceGroup().equalToConst("SA"))
    return '@';
  if (!fee.optFee())
    return ' ';
  if (fee.optFee()->refundReissueInd() == 'N' || fee.optFee()->refundReissueInd() == 'R')
    return 'N';
  if (fee.optFee()->advPurchTktIssue() == 'X')
    return 'P';
  switch (fee.optFee()->frequentFlyerMileageAppl())
  {
  case '3':
    return '/';
  case '4':
    return 'X';
  case '5':
    return '-';
  default:
    return ' ';
  }
}

const char
PricingResponseFormatter::getFootnoteByHirarchyOrder(const OCFees& fee) const
{
  if (fee.isDisplayOnly() || fee.optFee()->notAvailNoChargeInd() == 'X' ||
      fee.optFee()->notAvailNoChargeInd() == 'G')
    return '*';
  if (fee.subCodeInfo()->serviceGroup().equalToConst("SA"))
    return '@';
  if (!fee.optFee())
    return ' ';
  if (fee.optFee()->refundReissueInd() == 'N' || fee.optFee()->refundReissueInd() == 'R')
    return 'N';
  if (fee.optFee()->advPurchTktIssue() == 'X')
    return 'P';
  switch (fee.optFee()->frequentFlyerMileageAppl())
  {
  case '3':
    return '/';
  case '4':
    return 'X';
  case '5':
    return '-';
  default:
    return ' ';
  }
}

void
PricingResponseFormatter::formatOCFeesGroups(
    PricingTrx& pricingTrx,
    XMLConstruct& construct,
    const std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages>>>&
        groupFeesVector,
    const bool allFeesDisplayOnly,
    bool timeOutMax)
{
  uint16_t currentIndex = 1;
  if (pricingTrx.getRequest()->multiTicketActive() &&
      (MultiTicketUtil::getTicketSolution(pricingTrx) ==
           MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT ||
       MultiTicketUtil::getTicketSolution(pricingTrx) == MultiTicketUtil::SINGLETKT_NOT_APPLICABLE))
    currentIndex = _currentIndexForMT;
  bool dispValid = false;
  Itin* itin = getItin(pricingTrx);
  const bool mixedSegmentsInItin = !itin->allSegsConfirmed() && !itin->allSegsUnconfirmed();

  typedef std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages>>>
  GroupFeesVector;
  GroupFeesVector::const_iterator groupFeesVectorIter = groupFeesVector.begin();
  GroupFeesVector::const_iterator groupFeesVectorIterEnd = groupFeesVector.end();
  for (; groupFeesVectorIter != groupFeesVectorIterEnd; ++groupFeesVectorIter)
  {
    const ServiceFeesGroup* sfg = (*groupFeesVectorIter).first;
    const ServiceGroup sfgGroupCode = sfg->groupCode();

    construct.openElement(xml2::OCGroupInfo);
    construct.addAttribute(xml2::OCFeeGroupCode, sfgGroupCode);
    if (!timeOutMax)
    {
      if (sfg->state() == ServiceFeesGroup::EMPTY)
      {
        construct.addAttribute(xml2::OCGroupStatusCode, "NF"); // ST0 - NF
      }
      else if (sfg->state() == ServiceFeesGroup::NOT_AVAILABLE)
      {
        construct.addAttribute(xml2::OCGroupStatusCode, "NA"); // ST0 - NA
      }
    }

    std::vector<PaxOCFeesUsages>::const_iterator feesIter = (*groupFeesVectorIter).second.begin();
    std::vector<PaxOCFeesUsages>::const_iterator feesIterEnd = (*groupFeesVectorIter).second.end();
    if (feesIter != feesIterEnd)
    {
      if (sfg->state() == ServiceFeesGroup::VALID)
      {
        dispValid = true;
        // use a different construct so we can add status indicators to the group tag
        // once all fees are processed
        XMLConstruct feesConstruct;

        // format group header
        std::ostringstream outputLine;
        outputLine.setf(std::ios::left, std::ios::adjustfield);
        // replace character '(' and ')' characters with * character
        std::string grpDesc(sfg->groupDescription());
        replaceSpecialCharInDisplay(grpDesc);
        outputLine << sfgGroupCode << "-" << std::setw(34) << grpDesc << "CXR SEG/CPA          FEE";

        if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
        {
          prepareResponseText(outputLine.str(),
                              feesConstruct,
                              false,
                              &PricingResponseFormatter::prepareSimpleMessage);
        }
        std::string st1Ind = "";
        // format individual OC fee lines
        feesIter = (*groupFeesVectorIter).second.begin();
        feesIterEnd = (*groupFeesVectorIter).second.end();
        for (; feesIter != feesIterEnd; ++feesIter)
        {
          if (allFeesDisplayOnly)
          {
            // request is display only or all fees marked as display only
            formatOCFeesLine(pricingTrx, feesConstruct, *feesIter, 0, ' ');
          }
          else
          {
            const char footnoteChar = getFootnoteByHirarchyOrder(*(*feesIter).fees());
            if (footnoteChar != ' ')
              buildST1data(st1Ind, &footnoteChar);
            if (footnoteChar == '@' || footnoteChar == '*')
              formatOCFeesLine(pricingTrx, feesConstruct, *feesIter, 0, footnoteChar);
            else
            {
              formatOCFeesLine(pricingTrx, feesConstruct, *feesIter, currentIndex, footnoteChar);
              ++currentIndex;
            }
          }
        }
        if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
        {
          prepareResponseText(
              " \n", feesConstruct, false, &PricingResponseFormatter::prepareSimpleMessage);
        }
        if (!st1Ind.empty())
        {
          construct.addAttribute(xml2::OCGroupStatusInd, st1Ind);
        }

        construct.addElementData(feesConstruct.getXMLData().c_str());
      }
    }
    construct.closeElement();
  }
  if (mixedSegmentsInItin && dispValid)
    formatOCTrailer(construct, "UF"); // ST0 - UF
  if (pricingTrx.getRequest()->multiTicketActive() &&
      (MultiTicketUtil::getTicketSolution(pricingTrx) ==
           MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT ||
       MultiTicketUtil::getTicketSolution(pricingTrx) == MultiTicketUtil::SINGLETKT_NOT_APPLICABLE))
    _currentIndexForMT = currentIndex;
}

void
PricingResponseFormatter::buildST1data(std::string& st1Ind, const char* indicator)
{
  if (st1Ind.empty() || st1Ind.find(*indicator) == std::string::npos)
    st1Ind.append(1, *indicator);
}

void
PricingResponseFormatter::formatOCFeesLine(PricingTrx& pricingTrx,
                                           XMLConstruct& construct,
                                           const PaxOCFeesUsages& paxOcFees,
                                           const uint16_t index,
                                           const char& indicator)
{
  Itin* itin = getItin(pricingTrx);
  ServiceFeeUtil util(pricingTrx);
  std::ostringstream paxOCFeeLine;

  // In SearchForBrandsPricing response the OCFees are incapsulated into another element with an
  // identifier, which
  // is referenced from SUM elements. This is done to reduce the response size since multiple
  // solutions may have
  // the same OCFee attached to them.
  if (pricingTrx.activationFlags().isSearchForBrandsPricing() &&
      !fallback::fallbackOCFeesInSearchForBrandsPricing(&pricingTrx))
  {
    construct.openElement(xml2::OCFeesReference);
    construct.addAttributeInteger(xml2::OCFeesReferenceNumber, paxOcFees.getId());
  }

  paxOCFeeLine.setf(std::ios::left, std::ios::adjustfield);
  // we assume that indices are 1-based and 0 is a special value
  if (index)
  {
    paxOCFeeLine << std::setw(3) << index;
  }
  else
  {
    paxOCFeeLine << "-- ";
  }

  paxOCFeeLine << paxOcFees.paxType() << "-" << std::setw(30)
               << getCommercialName(paxOcFees.fees());
  paxOCFeeLine.unsetf(std::ios::left);
  paxOCFeeLine.setf(std::ios::right, std::ios::adjustfield);
  paxOCFeeLine << std::setw(3) << paxOcFees.fees()->carrierCode() << " ";
  paxOCFeeLine << std::setw(2) << itin->segmentPnrOrder(paxOcFees.fees()->travelStart()) << "-"
               << paxOcFees.fees()->travelStart()->origin()->loc()
               << paxOcFees.fees()->travelEnd()->destination()->loc();
  paxOCFeeLine.setf(std::ios::fixed, std::ios::floatfield);

  const DateTime& ticketingDate = pricingTrx.ticketingDate();
  Money targetMoney = util.convertOCFeeCurrency(*(paxOcFees.fees())); // TODO PL add taxes

  if (paxOcFees.fees()->optFee()->notAvailNoChargeInd() != 'X')
  {
    paxOCFeeLine.precision(targetMoney.noDec(ticketingDate));

    DataHandle dataHandle;
    OCFeesPrice* ocFeesPrice = OCFeesPrice::create(*paxOcFees.fees(), pricingTrx, dataHandle);

    paxOCFeeLine << std::setw(11)
                 << ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(*paxOcFees.fees(),
                                                                        targetMoney);
  }
  else
  {
    paxOCFeeLine << std::setw(11) << "  NOT AVAIL";
  }

  paxOCFeeLine << " " << indicator << "\n";

  if (pricingTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    prepareResponseText(
        paxOCFeeLine.str(), construct, false, &PricingResponseFormatter::prepareSimpleMessage);
  }
  buildPNRData(pricingTrx, construct, paxOcFees, index, targetMoney);

  if (pricingTrx.activationFlags().isSearchForBrandsPricing() &&
      !fallback::fallbackOCFeesInSearchForBrandsPricing(&pricingTrx))
  {
    construct.closeElement(); // OCI
  }
}

bool
PricingResponseFormatter::isTaxExempted(const std::string& taxCode,
                                        const bool isExemptAllTaxes,
                                        const bool isExemptSpecificTaxes,
                                        const std::vector<std::string>& taxIdExempted)
{
  if (isExemptAllTaxes)
    return true;

  if (isExemptSpecificTaxes)
  {
    if (taxIdExempted.empty())
      return true;

    std::vector<std::string>::const_iterator taxIdExemptedI = taxIdExempted.begin();
    std::vector<std::string>::const_iterator taxIdExemptedEndI = taxIdExempted.end();

    for (; taxIdExemptedI != taxIdExemptedEndI; taxIdExemptedI++)
    {
      if (taxCode.compare(0, taxIdExemptedI->size(), *taxIdExemptedI) == 0)
        return true;
    }
  }

  return false;
}

std::vector<OCFees::TaxItem>::const_iterator
PricingResponseFormatter::setEndTaxOnOcIterator(const std::vector<OCFees::TaxItem>& taxItems)
{
  std::vector<OCFees::TaxItem>::const_iterator iEndTaxItem;

  if (taxItems.size() > TAX_ON_OC_BUFF_SIZE)
  {
    iEndTaxItem = taxItems.begin();
    advance(iEndTaxItem, TAX_ON_OC_BUFF_SIZE);
  }
  else
    iEndTaxItem = taxItems.end();

  return iEndTaxItem;
}

void
PricingResponseFormatter::buildOcTaxData(XMLConstruct& construct,
                                         const OCFeesUsage* ocFees,
                                         const bool isExemptAllTaxes,
                                         const bool isExemptSpecificTaxes,
                                         const std::vector<std::string>& taxIdExempted)
{
  if (ocFees->getTaxes().empty())
    return;

  std::vector<OCFees::TaxItem>::const_iterator iTaxItem = ocFees->getTaxes().begin();
  std::vector<OCFees::TaxItem>::const_iterator iEndTaxItem =
      setEndTaxOnOcIterator(ocFees->getTaxes());

  for (; iTaxItem != iEndTaxItem; iTaxItem++)
  {
    if (isTaxExempted(
            iTaxItem->getTaxCode(), isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted))
      construct.openElement(xml2::TaxExempt);
    else
      construct.openElement(xml2::TaxInformation);

    construct.addAttribute(xml2::ATaxCode, iTaxItem->getTaxCode());
    construct.addAttributeDouble(
        xml2::TaxAmount, iTaxItem->getTaxAmount(), iTaxItem->getNumberOfDec());
    construct.closeElement();
  }
}

void
PricingResponseFormatter::buildPNRData(const PricingTrx& pricingTrx,
                                       XMLConstruct& construct,
                                       const PaxOCFeesUsages& paxOcFees,
                                       const uint16_t index,
                                       const Money& equivPrice)
{
  // Build all the PNR information
  construct.openElement(xml2::OCFeesPNRInfo);

  // Build SHI - Service Fee Line Number
  if (index)
  {
    std::ostringstream lnNum;
    lnNum << index;
    construct.addAttribute(xml2::ServiceLineNumber, lnNum.str());
  }
  else
    construct.addAttribute(xml2::ServiceLineNumber, "--");

  const DateTime& ticketingDate = pricingTrx.ticketingDate();

  DataHandle dataHandle;
  OCFeesPrice* ocFeesPrice = OCFeesPrice::create(*paxOcFees.fees(), pricingTrx, dataHandle);

  // SFA, SFB - Base Price and Base Currency
  if (paxOcFees.fees()->feeCurrency() != "")
  {
    construct.addAttributeDouble(
        xml2::ServiceBaseCurPrice, paxOcFees.fees()->feeAmount(), paxOcFees.fees()->feeNoDec());
    construct.addAttribute(xml2::ServiceBaseCurCode, paxOcFees.fees()->feeCurrency());
  }
  else // If base price currency is blank use equivalent currecny code for for base price decimal
  // calculation
  {
    construct.addAttributeDouble(
        xml2::ServiceBaseCurPrice,
        ocFeesPrice->getBasePrice(*paxOcFees.fees(), paxOcFees.fees()->feeAmount()),
        equivPrice.noDec(ticketingDate));
    // PNR needs this currency code when base price is 0
    construct.addAttribute(xml2::ServiceEquiCurCode, equivPrice.code());
  }

  if (paxOcFees.fees()->feeCurrency() != "" && paxOcFees.fees()->feeCurrency() != equivPrice.code())
  {
    // SFC, SFH - Equivalent Price and Equivalent Currency
    construct.addAttributeDouble(
        xml2::ServiceEquiCurPrice,
        ocFeesPrice->getEquivalentBasePrice(*paxOcFees.fees(), equivPrice.value()),
        equivPrice.noDec(ticketingDate));
    construct.addAttribute(xml2::ServiceEquiCurCode, equivPrice.code());
  }

  // Tax Indicator, Tax Amount , Tax Code -- Not for R4 : SFI, SHD, SHE, SFE
  if (paxOcFees.fees()->optFee()->taxInclInd() != ' ')
    construct.addAttributeChar(xml2::ServiceTaxIndicator, paxOcFees.fees()->optFee()->taxInclInd());

  construct.addAttributeDouble(
      xml2::ServiceTotalPrice,
      ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(*paxOcFees.fees(), equivPrice),
      equivPrice.noDec(ticketingDate));

  // SFF - Commercial name
  construct.addAttribute(xml2::ServiceCommercialName, getCommercialName(paxOcFees.fees()));

  // SSG - Segment number
  processPNRSegmentNumber(pricingTrx, construct, paxOcFees);

  if (paxOcFees.fees()->optFee()->notAvailNoChargeInd() != ' ')
  { // SNN - No Charge or Not Available Indicator
    construct.addAttributeChar(xml2::ServiceNChrgNAvlb,
                               paxOcFees.fees()->optFee()->notAvailNoChargeInd());
  }

  // SHF - Passenger Type Code
  construct.addAttribute(xml2::ServicePaxTypeCode, paxOcFees.paxType());

  // SFD - Display Only Indicator
  if (index)
    construct.addAttributeChar(xml2::ServiceDisplayOnly, 'N');
  else // For display only send an indicator
    construct.addAttributeChar(xml2::ServiceDisplayOnly, 'Y');

  // SFK - Carrier Code
  construct.addAttribute(xml2::ServiceOwningCxr, paxOcFees.fees()->subCodeInfo()->carrier());

  // SFS - Name Number Association
  // construct.addAttribute(xml2::ServiceFeeNameNum, ""); // Logic For R7

  // SFU - Group Code
  construct.addAttribute(xml2::ServiceFeeGroupCd, paxOcFees.fees()->subCodeInfo()->serviceGroup());

  // SHK - RFIC Subcode
  construct.addAttribute(xml2::ServiceRFICSubCode,
                         paxOcFees.fees()->subCodeInfo()->serviceSubTypeCode());

  // SFV - Vendor Code
  construct.addAttribute(xml2::ServiceVendorCode, paxOcFees.fees()->subCodeInfo()->vendor());

  if (index) // For Non-display only fees add these
  {
    buildPNRDataForNonDisplayFees(pricingTrx, construct, paxOcFees);
  }

  const bool isExemptAllTaxes = pricingTrx.getRequest()->isExemptAllTaxes();
  const bool isExemptSpecificTaxes = pricingTrx.getRequest()->isExemptSpecificTaxes();
  const std::vector<std::string>& taxIdExempted = pricingTrx.getRequest()->taxIdExempted();

  buildOcTaxData(
      construct, paxOcFees.fees(), isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted);

  construct.closeElement();
}

void
PricingResponseFormatter::buildPNRDataForNonDisplayFees(const PricingTrx& pricingTrx,
                                                        XMLConstruct& construct,
                                                        const PaxOCFeesUsages& paxOcFees)
{
  if (paxOcFees.fees()->subCodeInfo()->rfiCode() != ' ')
  { // SFG - RFIC
    construct.addAttributeChar(xml2::ServiceRFICCode, paxOcFees.fees()->subCodeInfo()->rfiCode());
  }

  if (paxOcFees.fees()->subCodeInfo()->ssrCode() != "")
  { // SHL - SSR code
    construct.addAttribute(xml2::ServiceSSRCode, paxOcFees.fees()->subCodeInfo()->ssrCode());
  }

  // SFJ - EMD Type
  construct.addAttributeChar(xml2::ServiceEMDType, paxOcFees.fees()->subCodeInfo()->emdType());

  // SFL - IATA Application Indicators
  std::ostringstream iataInd;
  iataInd << paxOcFees.fees()->optFee()->refundReissueInd()
          << paxOcFees.fees()->optFee()->commissionInd()
          << paxOcFees.fees()->optFee()->interlineInd();

  if (iataInd.str() != "   ")
    construct.addAttribute(xml2::ServiceIATAApplInd, iataInd.str());

  // SFM - Sector-Portion Ind
  if (paxOcFees.fees()->optFee()->fltTktMerchInd() == PREPAID_BAGGAGE)
    construct.addAttributeChar(xml2::ServiceSecPorInd, PREPAID_BAGGAGE);
  else
    construct.addAttributeChar(xml2::ServiceSecPorInd,
                               paxOcFees.fees()->optFee()->sectorPortionInd());

  if (paxOcFees.fees()->subCodeInfo()->bookingInd() != "")
  { // SFN - Booking Ind
    construct.addAttribute(xml2::ServiceBooking, paxOcFees.fees()->subCodeInfo()->bookingInd());
  }

  if (paxOcFees.fees()->optFee()->frequentFlyerMileageAppl() != ' ')
  { // SHG - Fee Application Ind
    construct.addAttributeChar(xml2::ServiceFeeApplInd,
                               paxOcFees.fees()->optFee()->frequentFlyerMileageAppl());
  }

  if (paxOcFees.fees()->optFee()->formOfFeeRefundInd() != ' ')
  { // SFP - Fee Refund
    construct.addAttributeChar(xml2::ServiceFeeRefund,
                               paxOcFees.fees()->optFee()->formOfFeeRefundInd());
  }

  // SFR - Fee Guarantee
  if (_ancNonGuarantee)
  {
    construct.addAttributeBoolean(xml2::ServiceFeeGuarInd, false);
  }
  else
    construct.addAttributeBoolean(xml2::ServiceFeeGuarInd, paxOcFees.fees()->isFeeGuaranteed());

  // SFT - Ticket Number
  // construct.addAttribute(xml2::ServiceFeeTktNum, "");

  if (paxOcFees.fees()->optFee()->advPurchTktIssue() != ' ')
  { // SFX - Simultaneous Ticket Indicator
    construct.addAttributeChar(xml2::ServiceSimResTkt,
                               paxOcFees.fees()->optFee()->advPurchTktIssue());
  }

  if (paxOcFees.fees()->subCodeInfo()->ssimCode() != ' ')
  { // SFW - SSIM Code
    construct.addAttributeChar(xml2::ServiceSSIMCode, paxOcFees.fees()->subCodeInfo()->ssimCode());
  }

  // SFZ - Travel Dates Effective
  processTicketEffDate(pricingTrx, construct, paxOcFees);

  // SHA - Travel Dates Discontinue
  processTicketDiscDate(pricingTrx, construct, paxOcFees);

  // SHM - Purchase By Date
  DateTime time = calculatePurchaseByDate(pricingTrx);
  construct.addAttribute(xml2::ServicePurchaseByDate, time.dateToString(YYYYMMDD, "-").c_str());

  if (paxOcFees.fees()->optFee()->frequentFlyerStatus() != 0)
  { // SHB - FQTV carrier filed tier level
    construct.addAttributeShort(xml2::ServiceTierStatus,
                                paxOcFees.fees()->optFee()->frequentFlyerStatus());
  }

  if (paxOcFees.fees()->optFee()->tourCode() != "")
  { // SHC - Tour Code
    construct.addAttribute(xml2::ServiceTourCode, paxOcFees.fees()->optFee()->tourCode());
  }

  // SHN - Send Origin, Destination when it is Portion
  if (paxOcFees.fees()->optFee()->sectorPortionInd() == 'P')
  {
    std::ostringstream origDest;
    origDest << paxOcFees.fees()->travelStart()->origin()->loc()
             << paxOcFees.fees()->travelEnd()->destination()->loc();
    construct.addAttribute(xml2::ServiceOrigDest, origDest.str());
  }

  if (paxOcFees.fees()->optFee()->availabilityInd() == 'Y' &&
      paxOcFees.fees()->optFee()->notAvailNoChargeInd() != 'X')
  {
    // AX1 - R7 Avail Service Tag
    construct.addAttributeBoolean(xml2::AvailService, true);
  }
}

DateTime
PricingResponseFormatter::calculatePurchaseByDate(const PricingTrx& pricingTrx)
{
  DateTime time = DateTime::localTime(); // pricingTrx.ticketingDate();
  short utcOffset = 0;
  const Loc* hdqLoc = pricingTrx.dataHandle().getLoc("HDQ", time);
  const Loc* pccLoc = pricingTrx.getRequest()->ticketingAgent()->agentLocation();

  if (LocUtil::getUtcOffsetDifference(
          *pccLoc, *hdqLoc, utcOffset, pricingTrx.dataHandle(), time, time))
  {
    time = time.addSeconds(utcOffset * 60);
    time = time.nextDay();
  }
  return time;
}

void
PricingResponseFormatter::processPNRSegmentNumber(const PricingTrx& pricingTrx,
                                                  XMLConstruct& construct,
                                                  const PaxOCFeesUsages& paxOcFees)
{
  // SSG - Segment number
  Itin* itin = getItin(pricingTrx);
  if (itin->segmentPnrOrder(paxOcFees.fees()->travelStart()) ==
      itin->segmentPnrOrder(paxOcFees.fees()->travelEnd()))
  {
    construct.addAttributeShort(xml2::ServiceSegment,
                                itin->segmentPnrOrder(paxOcFees.fees()->travelStart()));
  }
  else // For Portion
  {
    checkPNRSegmentNumberLogic(pricingTrx,
                               construct,
                               itin->segmentPnrOrder(paxOcFees.fees()->travelStart()),
                               itin->segmentPnrOrder(paxOcFees.fees()->travelEnd()));
  }
}

void
PricingResponseFormatter::checkPNRSegmentNumberLogic(const PricingTrx& pricingTrx,
                                                     XMLConstruct& construct,
                                                     int trvlStart,
                                                     int trvlEnd)
{
  Itin* itin = getItin(pricingTrx);
  std::vector<TravelSeg*>::const_iterator tvlSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndIter = itin->travelSeg().end();
  bool match = false;

  for (; tvlSegIter != tvlSegEndIter; tvlSegIter++)
  {
    if (itin->segmentPnrOrder(*tvlSegIter) == trvlStart)
    {
      match = true;
      break;
    }
  }
  if (match)
  {
    std::ostringstream temp;
    for (; itin->segmentPnrOrder(*tvlSegIter) != trvlEnd; tvlSegIter++)
    {
      const TravelSeg* trvlSeg = (*tvlSegIter);
      if (trvlSeg->segmentType() != tse::Arunk && trvlSeg->segmentType() != tse::Surface)
      {
        temp << itin->segmentPnrOrder(trvlSeg) << "/";
      }
    }

    const TravelSeg* trvlSeg = (*tvlSegIter);
    if (trvlSeg->segmentType() != tse::Arunk && trvlSeg->segmentType() != tse::Surface)
    {
      temp << itin->segmentPnrOrder(*tvlSegIter);
      construct.addAttribute(xml2::ServiceSegment, temp.str());
    }
    else // For Arunk
    {
      std::string ssgResponse = temp.str();
      ssgResponse.erase(ssgResponse.end() - 1);
      construct.addAttribute(xml2::ServiceSegment, ssgResponse);
    }
  }
}

void
PricingResponseFormatter::processTicketEffDate(const PricingTrx& trx,
                                               XMLConstruct& construct,
                                               const PaxOCFeesUsages& paxOcFees)
{
  // SFZ - Travel Dates Effective
  std::ostringstream temp;
  std::ostringstream effYear;
  std::ostringstream effMonth;
  std::ostringstream effDay;
  if (ServiceFeeUtil::isStartDateSpecified(*(paxOcFees.fees()->optFee())))
  {
    temp << std::setw(4) << std::setfill('0') << paxOcFees.fees()->optFee()->tvlStartYear() << "-"
         << std::setw(2) << std::setfill('0') << paxOcFees.fees()->optFee()->tvlStartMonth() << "-"
         << std::setw(2) << std::setfill('0') << paxOcFees.fees()->optFee()->tvlStartDay();
  }
  else if (paxOcFees.fees()->optFee()->ticketEffDate().isValid())
  {
    temp << paxOcFees.fees()->optFee()->ticketEffDate().dateToSqlString();
  }
  else
  {
    effYear << paxOcFees.fees()->optFee()->ticketEffDate().year();
    effMonth << std::setw(2) << std::setfill('0')
             << ((unsigned short)paxOcFees.fees()->optFee()->ticketEffDate().date().month());
    effDay << std::setw(2) << std::setfill('0')
           << paxOcFees.fees()->optFee()->ticketEffDate().date().day();

    temp << effYear.str() << "-" << effMonth.str() << "-" << effDay.str();
  }
  construct.addAttribute(xml2::ServiceTravelDateEff, temp.str());
}

void
PricingResponseFormatter::processTicketDiscDate(const PricingTrx& trx,
                                                XMLConstruct& construct,
                                                const PaxOCFeesUsages& paxOcFees)
{
  // SHA - Travel Dates Discontinue
  std::ostringstream temp;

  if (paxOcFees.fees()->optFee()->ticketDiscDate().isValid())
  {
    temp << paxOcFees.fees()->optFee()->ticketDiscDate().dateToSqlString();
  }
  else if (paxOcFees.fees()->optFee()->ticketDiscDate().isInfinity())
  {
    temp << "9999-12-31";
  }
  else
    temp << std::setw(4) << std::setfill('0') << paxOcFees.fees()->optFee()->tvlStopYear() << "-"
         << std::setw(2) << std::setfill('0') << paxOcFees.fees()->optFee()->tvlStopMonth() << "-"
         << std::setw(2) << std::setfill('0') << paxOcFees.fees()->optFee()->tvlStopDay();
  construct.addAttribute(xml2::ServiceTravelDateDisc, temp.str());
}

void
PricingResponseFormatter::formatOCTrailer(XMLConstruct& construct, const std::string& statusCode)
{
  construct.openElement(xml2::OCGroupTrailer); // OCT - Open
  construct.addAttribute(xml2::OCGroupStatusCode, statusCode);
  construct.closeElement(); // OCT - Close
}

void
PricingResponseFormatter::formatOCGenericMsg(XMLConstruct& construct, const std::string& msg)
{
  construct.openElement(xml2::OCGenericMsg); // OCF - Open
  prepareResponseText(msg, construct, false, &PricingResponseFormatter::prepareSimpleMessage);
  construct.closeElement(); // OCF - Close
}

bool
PricingResponseFormatter::isOcFeesTrxDisplayOnly(const PricingTrx& pricingTrx)
{
  // check if WPB or WPNC or WPNCS was used in pricing request
  return (pricingTrx.getOptions()->isOCHistorical() ||
          pricingTrx.getRequest()->isLowFareNoRebook());
}

void
PricingResponseFormatter::isTimeOutBeforeStartOCFees(PricingTrx& trx,
                                                     XMLConstruct& construct,
                                                     const bool prefixInd)
{
  Itin* itin = getItin(trx);
  if (!itin->timeOutForExceeded())
    return;
  // time out for WPAE or WPAE-BG entry right at OC process starts
  construct.openElement(xml2::OCFeesDisplayInfo);

  if (prefixInd)
    construct.addAttributeBoolean(xml2::AttnInd, prefixInd);
  if (trx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    formatOCGenericMsg(construct, "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER OR SEG SELECT");
  }
  construct.closeElement();
}

bool
PricingResponseFormatter::anyTimeOutMaxCharCountIssue(PricingTrx& trx,
                                                      XMLConstruct& construct,
                                                      const bool prefixInd)
{
  Itin* itin = getItin(trx);
  if (itin->timeOutForExceeded() || itin->timeOutOCFForWP())
  {
    timeOutMaxCharCountNoOCFeesReturned(trx, construct, prefixInd);
    return true;
  }
  if (itin->timeOutForExceededSFGpresent())
  {
    timeOutMaxCharCountRequestedOCFeesReturned(trx, construct, prefixInd);
    return true;
  }
  return false;
}

void
PricingResponseFormatter::timeOutMaxCharCountNoOCFeesReturned(PricingTrx& trx,
                                                              XMLConstruct& construct,
                                                              const bool prefixInd)
{
  Itin* itin = getItin(trx);
  // time out for WP, WPAE or WPAE-BG entry before completing OC Fee process
  // build OCM element for all Group code elements with status "AL"
  construct.openElement(xml2::OCFeesDisplayInfo);

  if (prefixInd)
    construct.addAttributeBoolean(xml2::AttnInd, prefixInd);

  if (trx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    if (itin->timeOutOCFForWP() || (trx.getOptions() && !trx.getOptions()->isProcessAllGroups() &&
                                    trx.getOptions()->serviceGroupsVec().empty()))
      formatOCGenericMsg(construct, "AIR EXTRAS MAY APPLY - USE WPAE WITH SERVICE QUALIFIER");
    else
      formatOCGenericMsg(construct, "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER OR SEG SELECT");
  }
  // iterate groups
  std::vector<ServiceFeesGroup*>::iterator sfgIter = itin->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::iterator sfgIterEnd = itin->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    ServiceFeesGroup* sfg = (*sfgIter);
    ServiceGroup groupCode = sfg->groupCode();

    construct.openElement(xml2::OCGroupInfo);
    construct.addAttribute(xml2::OCFeeGroupCode, groupCode);
    construct.closeElement();
  }
  construct.closeElement();
}

typedef std::map<size_t, std::deque<Trx::Latency>> LatencyDataMap;

void
PricingResponseFormatter::prepareLatencyDataInResponse(Trx& trx, XMLConstruct& construct)
{
  bool areMetricsRequested =
      trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::TOPLINE_METRICS) ||
      trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::FULL_METRICS);

  if (!TrxUtil::needMetricsInResponse() && !areMetricsRequested)
    return;

  std::ostringstream oss;
  MetricsUtil::trxLatency(oss, trx);

  construct.openElement(xml2::ExtraLatencyInfo);

  for (const Trx::Latency& latencyData : trx.latencyData())
  {
    if (TrxUtil::needMetricsInResponse(latencyData.description) || areMetricsRequested)
    {
      construct.openElement(xml2::LatencyDetail);
      construct.addAttribute(xml2::LatencyDescription, latencyData.description);
      construct.addAttributeULong(xml2::LatencyNCalls, latencyData.nItems);
      construct.addAttributeDouble(xml2::LatencyWallTime, latencyData.wallTime);
      construct.addAttributeDouble(xml2::LatencyCpuTime,
                                   latencyData.userTime + latencyData.systemTime);

      construct.closeElement();
    }
  }

  construct.closeElement();
}

void
PricingResponseFormatter::timeOutMaxCharCountRequestedOCFeesReturned(PricingTrx& trx,
                                                                     XMLConstruct& constructA,
                                                                     const bool prefixInd)
{
  XMLConstruct construct;
  // time out for WP, WPAE or WPAE-BG entry before completing OC Fee process
  construct.openElement(xml2::OCFeesDisplayInfo);

  if (prefixInd)
    construct.addAttributeBoolean(xml2::AttnInd, prefixInd);
  if (trx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    formatOCHeaderMsg(construct);
  }
  formatOCFees(trx, construct, true);

  construct.closeElement();

  if (construct.getXMLData().size() < (_maxTotalBuffSize - 20 - constructA.getXMLData().size()))
  {
    constructA.addElementData(construct.getXMLData().c_str(), construct.getXMLData().size());
    return;
  }

  timeOutMaxCharCountNoOCFeesReturned(trx, constructA, prefixInd);
}

bool
PricingResponseFormatter::isAncillaryNonGuarantee(PricingTrx& pricingTrx, const FarePath* farePath)
{
  const PricingOptions* options = pricingTrx.getOptions();
  PricingRequest* request = pricingTrx.getRequest();

  bool isDAorPAentry = false;
  bool isDPorPPentry = false;
  if (TrxUtil::newDiscountLogic(pricingTrx))
  {
    isDAorPAentry = request && request->isDAorPAentry();
    isDPorPPentry = request && request->isDPorPPentry();
  }
  else
  {
    isDAorPAentry = request && request->isDAEntry();
    isDPorPPentry = request && request->isDPEntry();
  }

  if ((request &&
       (request->isIndustryFareOverrideEntry() || request->isGoverningCarrierOverrideEntry() ||
        isDAorPAentry || isDPorPPentry || request->isPtsOverride() ||
        request->rateAmountOverride() || request->secondRateAmountOverride())) ||
      (options && (options->bookingCodeOverride() || options->fbcSelected())) ||
      farePath->fuelSurchargeIgnored())
    return true;

  return false;
}

std::string
PricingResponseFormatter::getCommercialName(const OCFeesUsage* ocFeesUsage)
{
  std::string result = ocFeesUsage->oCFees()->subCodeInfo()->commercialName();

  if (ocFeesUsage && ocFeesUsage->upgradeT198CommercialName() != EMPTY_STRING())
    result = ocFeesUsage->upgradeT198CommercialName();

  return result;
}

void
PricingResponseFormatter::populateSubCodeDefinitions(const PricingTrx& trx)
{
  std::vector<std::string> restrictionDescriptions;
  fetchSubCodeDefinitions(trx, restrictionDescriptions);

  typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
  boost::char_separator<char> sep("/");

  for (std::string restrictionDescription : restrictionDescriptions)
  {
    ServiceGroupDescription baggageServiceGroupDescription = EMPTY_STRING();
    float baggageSizeWeightLimit = NO_BAGGAGE_SIZE_WEIGHT;
    char baggageSizeWeightUnitType = NO_BAGGAGE_SIZE_WEIGHT_UNIT;
    char baggageSizeWeightLimitType = NO_BAGGAGE_SIZE_WEIGHT_TYPE;

    t_tokenizer tok(restrictionDescription, sep);
    t_tokenizer::iterator it = tok.begin();
    t_tokenizer::iterator end = tok.end();

    if (it != end)
    {
      baggageServiceGroupDescription = *it;
      if (++it != end)
      {
        try
        {
          baggageSizeWeightLimit = boost::lexical_cast<float>(*it);
        }
        catch (boost::bad_lexical_cast&)
        {
        }
        if (++it != end)
        {
          baggageSizeWeightUnitType = (*it)[0];
          if (++it != end)
          {
            baggageSizeWeightLimitType = (*it)[0];
          }
        }
      }
    }
    if (baggageServiceGroupDescription != EMPTY_STRING() &&
        baggageSizeWeightLimit != NO_BAGGAGE_SIZE_WEIGHT &&
        baggageSizeWeightUnitType != NO_BAGGAGE_SIZE_WEIGHT_UNIT &&
        baggageSizeWeightLimitType != NO_BAGGAGE_SIZE_WEIGHT_TYPE)
    {
      _baggageSizeWeightRestrictions.push_back(
          BaggageSizeWeightDescription(baggageServiceGroupDescription,
                                       baggageSizeWeightLimit,
                                       baggageSizeWeightUnitType,
                                       baggageSizeWeightLimitType));
    }
  }
}

void
PricingResponseFormatter::fetchSubCodeDefinitions(const PricingTrx& trx,
                                                  std::vector<std::string>& definitions)
{
  std::string restrictionDescriptions = TrxUtil::subCodeDefinitionsData(trx);
  if (restrictionDescriptions == EMPTY_STRING())
    return;

  typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
  boost::char_separator<char> sep("|");
  t_tokenizer tok(restrictionDescriptions, sep);
  std::copy(tok.begin(), tok.end(), std::back_inserter(definitions));
}

void
PricingResponseFormatter::formatBaggageResponse(PricingTrx& pricingTrx, XMLConstruct& construct)
{
  if (!_subCodesForOSC.empty())
    populateSubCodeDefinitions(pricingTrx);

  for (const SubCodeInfo* subCode : _subCodesForOSC)
    buildOSC(pricingTrx, subCode, construct);
}

void
PricingResponseFormatter::buildOSC(PricingTrx& pricingTrx,
                                   const SubCodeInfo* subCodeInfo,
                                   XMLConstruct& construct) const
{
  construct.openElement(xml2::OCS5Info);

  // SHK - Extended Sub Code Key
  construct.addAttribute(
      xml2::ExtendedSubCodeKey,
      buildExtendedSubCodeKey(subCodeInfo, BaggageProvisionType(subCodeInfo->fltTktMerchInd())));

  // SFF - Commercial Name
  construct.addAttribute(xml2::ServiceCommercialName, subCodeInfo->commercialName());

  // SF0 - Service Group
  construct.addAttribute(xml2::ServiceGroup, subCodeInfo->serviceGroup());

  // ASG - Service Sub Group
  if (!subCodeInfo->serviceSubGroup().empty())
    construct.addAttribute(xml2::AncillaryServiceSubGroup, subCodeInfo->serviceSubGroup());

  // ASD - Service Sub Group Description
  const ServicesSubGroup* servicesSubGroup = pricingTrx.dataHandle().getServicesSubGroup(
      subCodeInfo->serviceGroup(), subCodeInfo->serviceSubGroup());
  if (servicesSubGroup && !servicesSubGroup->definition().empty())
  {
    construct.addAttribute(xml2::AncillaryServiceSubGroupDescription,
                           servicesSubGroup->definition());
  }

  // Description Code and Text
  buildSubCodeDescription(pricingTrx, subCodeInfo, construct);

  // Weight and Size Restrictions
  buildSubCodeRestrictions(subCodeInfo, construct);

  // N01 - Reason For Issuance Code
  if (subCodeInfo->rfiCode() != BLANK)
    construct.addAttributeChar(xml2::S5RFICCode, subCodeInfo->rfiCode());

  // SHL - SSR Code
  if (!subCodeInfo->ssrCode().empty())
    construct.addAttribute(xml2::ServiceSSRCode, subCodeInfo->ssrCode());

  // N02 - EMD Type
  if (subCodeInfo->emdType() != BLANK)
    construct.addAttributeChar(xml2::S5EMDType, subCodeInfo->emdType());

  // SFN - Booking Indicator
  if (!subCodeInfo->bookingInd().empty())
    construct.addAttribute(xml2::ServiceBooking, subCodeInfo->bookingInd());

  construct.closeElement();
}

std::string
PricingResponseFormatter::buildExtendedSubCodeKey(const SubCodeInfo* subCodeInfo,
                                                  const BaggageProvisionType& baggageProvision)
    const
{
  return subCodeInfo->serviceSubTypeCode() +
         (subCodeInfo->vendor() == ATPCO_VENDOR_CODE ? 'A' : 'M') + baggageProvision +
         subCodeInfo->carrier();
}

void
PricingResponseFormatter::buildSubCodeDescription(PricingTrx& pricingTrx,
                                                  const SubCodeInfo* subCodeInfo,
                                                  XMLConstruct& construct) const
{
  if (!subCodeInfo->description1().empty())
  {
    const ServicesDescription* description1 =
        pricingTrx.dataHandle().getServicesDescription(subCodeInfo->description1());
    if (description1)
    {
      // DC1 - Description 1 Code
      construct.addAttribute(xml2::Desc1Code, subCodeInfo->description1());
      // D01 - Description 1 Text
      construct.addAttribute(xml2::Desc1Text, description1->description());
      if (!subCodeInfo->description2().empty())
      {
        const ServicesDescription* description2 =
            pricingTrx.dataHandle().getServicesDescription(subCodeInfo->description2());
        if (description2)
        {
          // DC2 - Description 2 Code
          construct.addAttribute(xml2::Desc2Code, subCodeInfo->description2());
          // D02 - Description 2 Text
          construct.addAttribute(xml2::Desc2Text, description2->description());
        }
      }
    }
  }
}

void
PricingResponseFormatter::buildSubCodeRestrictions(const SubCodeInfo* subCodeInfo,
                                                   XMLConstruct& construct) const
{
  std::vector<const BaggageSizeWeightDescription*> sizeWeightRestrictions;
  getSubCodeRestrictions(subCodeInfo->description1(), sizeWeightRestrictions);
  getSubCodeRestrictions(subCodeInfo->description2(), sizeWeightRestrictions);

  const char POUNDS = 'L';
  const char KILOS = 'K';
  const char INCHES = 'I';
  const char CENTIMETERS = 'C';
  const char OVER = 'O';
  const char UP_TO = 'U';

  SubCodeRestrictionsComparator minWeight(POUNDS, KILOS, OVER);
  buildSubCodeRestrictionWithAlternativeUnits(xml2::MinimumWeightW01,
                                              xml2::MinimumWeightWU1,
                                              xml2::MinimumWeightW02,
                                              xml2::MinimumWeightWU2,
                                              minWeight,
                                              sizeWeightRestrictions,
                                              construct);

  SubCodeRestrictionsComparator maxWeight(POUNDS, KILOS, UP_TO);
  buildSubCodeRestrictionWithAlternativeUnits(xml2::MaximumWeightW03,
                                              xml2::MaximumWeightWU3,
                                              xml2::MaximumWeightW04,
                                              xml2::MaximumWeightWU4,
                                              maxWeight,
                                              sizeWeightRestrictions,
                                              construct);

  SubCodeRestrictionsComparator minSize(INCHES, CENTIMETERS, OVER);
  buildSubCodeRestrictionWithAlternativeUnits(xml2::MinimumSizeS01,
                                              xml2::MinimumSizeSU1,
                                              xml2::MinimumSizeS02,
                                              xml2::MinimumSizeSU2,
                                              minSize,
                                              sizeWeightRestrictions,
                                              construct);

  SubCodeRestrictionsComparator maxSize(INCHES, CENTIMETERS, UP_TO);
  buildSubCodeRestrictionWithAlternativeUnits(xml2::MaximumSizeS03,
                                              xml2::MaximumSizeSU3,
                                              xml2::MaximumSizeS04,
                                              xml2::MaximumSizeSU4,
                                              maxSize,
                                              sizeWeightRestrictions,
                                              construct);
}

void
PricingResponseFormatter::buildSubCodeRestrictionWithAlternativeUnits(
    const std::string& valueAtt,
    const std::string& unitAtt,
    const std::string& altValueAtt,
    const std::string& altUnitAtt,
    const SubCodeRestrictionsComparator& restrictionsComparator,
    const std::vector<const BaggageSizeWeightDescription*>& sizeWeightRestrictions,
    XMLConstruct& construct) const
{
  std::vector<const BaggageSizeWeightDescription*>::const_iterator it = std::find_if(
      sizeWeightRestrictions.begin(), sizeWeightRestrictions.end(), restrictionsComparator);
  if (it != sizeWeightRestrictions.end())
  {
    buildSubCodeRestriction(valueAtt,
                            (*it)->_baggageSizeWeightLimit,
                            unitAtt,
                            (*it)->_baggageSizeWeightUnitType,
                            construct);

    it = std::find_if(++it, sizeWeightRestrictions.end(), restrictionsComparator);
    if (it != sizeWeightRestrictions.end())
    {
      buildSubCodeRestriction(altValueAtt,
                              (*it)->_baggageSizeWeightLimit,
                              altUnitAtt,
                              (*it)->_baggageSizeWeightUnitType,
                              construct);
    }
  }
}

void
PricingResponseFormatter::buildSubCodeRestriction(const std::string& valueAtt,
                                                  const float value,
                                                  const std::string& unitAtt,
                                                  const char unit,
                                                  XMLConstruct& construct) const
{
  std::ostringstream stream;
  stream << value;
  construct.addAttribute(valueAtt, stream.str());
  construct.addAttributeChar(unitAtt, unit);
}

void
PricingResponseFormatter::getSubCodeRestrictions(
    const ServiceGroupDescription& description,
    std::vector<const BaggageSizeWeightDescription*>& restrictions) const
{
  if (!description.empty())
    for (const BaggageSizeWeightDescription& baggageSizeWeight : _baggageSizeWeightRestrictions)
      if (baggageSizeWeight._serviceDescription == description)
        restrictions.push_back(&baggageSizeWeight);
}

PricingResponseFormatter::SubCodeRestrictionsComparator::SubCodeRestrictionsComparator(
    const char unit, const char alternativeUnit, const char limitType)
  : _unit(unit), _alternativeUnit(alternativeUnit), _limitType(limitType)
{
}

bool
PricingResponseFormatter::SubCodeRestrictionsComparator::
operator()(const BaggageSizeWeightDescription* baggageSizeWeight) const
{
  return (baggageSizeWeight->_baggageSizeWeightUnitType == _unit ||
          baggageSizeWeight->_baggageSizeWeightUnitType == _alternativeUnit) &&
         (baggageSizeWeight->_baggageSizeWeightLimitType == _limitType);
}

void
PricingResponseFormatter::buildSpanishDiscountIndicator(PricingTrx& pricingTrx,
                                                        CalcTotals& calcTotals,
                                                        XMLConstruct& construct) const
{
  const FarePath* const farePath = calcTotals.farePath;
  if (farePath == nullptr)
    return;

  const Itin* itin = farePath->itin();
  if (itin == nullptr)
    return;

  const std::string indicator = SLFUtil::getIndicator(pricingTrx, *itin, *farePath);
  if (!indicator.empty())
    construct.addAttribute(xml2::SpanishDiscountIndicator, indicator);
}

void
PricingResponseFormatter::setFopBinNumber(const FopBinNumber& formOfPayment,
                                          XMLConstruct& construct,
                                          std::string attributeName) const
{
  if (formOfPayment.size() == FOP_BIN_SIZE &&
      std::find_if(formOfPayment.begin(), formOfPayment.end(), !boost::bind<bool>(&isDigit, _1)) ==
          formOfPayment.end())
  {
    construct.addAttribute(attributeName, formOfPayment);
  }
}

bool
PricingResponseFormatter::insertBaggageData(XMLConstruct& construct,
                                            BaggageResponseBuilder& baggageBuilder,
                                            const std::string& baggageTag,
                                            int& msgIndex,
                                            PricingTrx& pricingTrx) const
{
  std::vector<std::string> baggageLines;
  baggageBuilder.setBaggageTag(baggageTag);
  if (pricingTrx.getRequest()->multiTicketActive())
    baggageBuilder.setIsUsDotForMultiTkt(pricingTrx, getItin(pricingTrx));
  baggageBuilder.getGsResponse(baggageLines);
  const bool isNonUsDot =
      !pricingTrx.itin().empty() && !getItin(pricingTrx)->getBaggageTripType().isUsDot();

  for (const std::string& line : baggageLines)
  {
    msgIndex++;

    if (isXmlSizeLimit(construct, msgIndex))
      return true;

    if (TrxUtil::isBaggage302GlobalDisclosureActivated(pricingTrx) &&
        validSchemaVersion(pricingTrx) && isNonUsDot)
      prepareMessage(construct, Message::TYPE_BAGGAGE_NONUSDOT, msgIndex, line);
    else
      prepareMessage(construct, Message::TYPE_GENERAL, msgIndex, line);
  }
  return false;
}

bool
PricingResponseFormatter::insertPricingData(XMLConstruct& construct,
                                            int& msgIndex,
                                            const std::string& line) const
{
  msgIndex++;
  bool xmlSizeLimit = isXmlSizeLimit(construct, msgIndex);
  if (!xmlSizeLimit)
    prepareMessage(construct, Message::TYPE_GENERAL, msgIndex, line);

  return xmlSizeLimit;
}

void
PricingResponseFormatter::formatGreenScreenMsg(XMLConstruct& construct,
                                               const std::string& responseString,
                                               PricingTrx& pricingTrx,
                                               const FareCalcCollector* fareCalcCollector) const
{
  std::vector<std::string> lines;
  XformMsgFormatter::splitGreenScreeText(responseString, AVAILABLE_SIZE_FOR_MSG, lines);
  BaggageResponseBuilder baggageBuilder(
      pricingTrx, fareCalcCollector->baggageTagMap(), responseString);

  int index = 2;
  bool xmlSizeLimit = false;
  for (std::string& line : lines)
  {
    if (std::string::npos != (line.find(FreeBaggageUtil::BaggageTagHead)))
      xmlSizeLimit = insertBaggageData(construct, baggageBuilder, line, index, pricingTrx);
    else
      xmlSizeLimit = insertPricingData(construct, index, line);

    if (xmlSizeLimit)
      break;
  }
}

bool
PricingResponseFormatter::validSchemaVersion(const PricingTrx& pricingTrx) const
{
  if (TrxUtil::isBaggage302ExchangeActivated(pricingTrx))
  {
    return pricingTrx.isNotExchangeTrx() ? pricingTrx.getRequest()->checkSchemaVersion(1, 1, 1)
                                         : true;
  }
  else
  {
    return (pricingTrx.getRequest()->checkSchemaVersion(1, 1, 1));
  }
}

bool
PricingResponseFormatter::isOldPricingTrx(const PricingTrx& pricingTrx) const
{
  const MultiExchangeTrx* multiExchangeTrx =
      dynamic_cast<const MultiExchangeTrx*>(pricingTrx.getParentTrx());

  return multiExchangeTrx ? (multiExchangeTrx->newPricingTrx() != &pricingTrx) : false;
}

void
PricingResponseFormatter::addUsDotItinIndicator(XMLConstruct& construct,
                                                const Itin* itin,
                                                const PricingTrx& pricingTrx) const
{
  if (validSchemaVersion(pricingTrx) && !isOldPricingTrx(pricingTrx) &&
      TrxUtil::isBaggage302GlobalDisclosureActivated(pricingTrx))
  {
    if (TrxUtil::isBaggageCTAMandateActivated(pricingTrx))
    {
      BaggageTripType btt = itin ? itin->getBaggageTripType() : BaggageTripType::OTHER;
      construct.addAttributeChar(xml2::UsDotItinIndicator, btt.getIndicator());
    }
    else
    {
      const bool isUSDot = itin && itin->getBaggageTripType().isUsDot();
      construct.addAttributeBoolean(xml2::UsDotItinIndicator, isUSDot);
    }
  }
}

bool
PricingResponseFormatter::checkForStructuredData(const PricingTrx& pricingTrx,
                                                 bool isBaggageXmlResponse,
                                                 bool isBaggageExchange,
                                                 bool isBaggageGlobalDisclosure) const
{
  if (!isBaggageXmlResponse || pricingTrx.itin().empty())
    return false;

  if (pricingTrx.altTrxType() == PricingTrx::WPA ||
      pricingTrx.altTrxType() == PricingTrx::WPA_NOMATCH)
    return false;

  if (!pricingTrx.isNotExchangeTrx())
    return isBaggageExchange;

  return getItin(pricingTrx)->getBaggageTripType().isUsDot()
             ? pricingTrx.getRequest()->checkSchemaVersion(1, 1, 0)
             : isBaggageGlobalDisclosure;
}
bool
PricingResponseFormatter::isXmlSizeLimit(XMLConstruct& construct, int msgIndex) const
{
  if (construct.getXMLData().size() > _maxTotalBuffSize)
  {
    const char* msg = "RESPONSE TOO LONG FOR CRT";
    prepareMessage(construct, Message::TYPE_GENERAL, msgIndex, msg);
    return true;
  }
  return false;
}

void
PricingResponseFormatter::formatPricingResponseMTActive(
    XMLConstruct& construct,
    const std::string& responseString,
    bool displayOnly,
    PricingTrx& pricingTrx,
    FareCalcCollector* fareCalcCollector,
    ErrorResponseException::ErrorResponseCode tktErrCode)
{
  std::vector<std::string> respMsgVec = MultiTicketUtil::getAllRespondMsgs();
  bool multiTicketOffered = false;
  MultiTicketUtil::TicketSolution ts = MultiTicketUtil::getTicketSolution(pricingTrx);
  std::string tsString = responseString;

  switch (ts)
  {
  case MultiTicketUtil::MULTITKT_NOT_APPLICABLE:
  case MultiTicketUtil::MULTITKT_NOT_FOUND:
  case MultiTicketUtil::SINGLETKT_LOWER_THAN_MULTITKT:
  case MultiTicketUtil::SINGLETKT_SAME_AS_MULTITKT:
    tsString = "SINGLE TICKET REQUIRED\n \n";
    break;
  case MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT:
  case MultiTicketUtil::SINGLETKT_NOT_APPLICABLE:
    multiTicketOffered = true;
    break;
  default:
    break;
  }

  construct.addAttributeBoolean(xml2::MultiTicketResp, multiTicketOffered);

  if (!multiTicketOffered)
  {
    Itin* itin = getItin(pricingTrx);
    if (ts == MultiTicketUtil::NO_SOLUTION_FOUND && itin && itin->errResponseCode() > 0)
    {
      tsString = itin->errResponseMsg();
      tktErrCode = itin->errResponseCode();
    }
    else
    {
      tsString += respMsgVec[0];
    }
    formatPricingResponseMTInactive(
        tsString, displayOnly, tktErrCode, construct, pricingTrx, fareCalcCollector);
  }
  else
    formatPricingResponseMTOffered(
        construct, respMsgVec, displayOnly, pricingTrx, fareCalcCollector, tktErrCode, ts);

  MultiTicketUtil::cleanUpMultiTickets(pricingTrx);
}

void
PricingResponseFormatter::formatPricingResponseMTOffered(
    XMLConstruct& construct,
    const std::vector<std::string>& respMsgVec,
    bool displayOnly,
    PricingTrx& pricingTrx,
    FareCalcCollector* fareCalcCollector,
    ErrorResponseException::ErrorResponseCode tktErrCode,
    MultiTicketUtil::TicketSolution ts)
{
  uint16_t subItin = 1;
  uint16_t totalSubItins = 2;
  uint16_t fareCalcCollectorSize = 2;

  if (ts == MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT)
  { // Here we have either MULTITKT_LOWER_THAN_SINGLETKT or
    // SINGLETKT_NOT_APPLICABLE. For SINGLETKT_NOT_APPLICABLE,
    // since single itin is not available, so there are only
    // two fareCalcCollectors. For MULTITKT_LOWER_THAN_SINGLETKT,
    // there will be 3 fareCalcCollectors, one for single itin,
    // one for subitin 1 and one for subitin2.
    fareCalcCollectorSize = 3;
  }

  if (pricingTrx.fareCalcCollector().size() < fareCalcCollectorSize)
    return;

  if (!displayOnly && fareCalcCollector)
  {
    prepareAgent(pricingTrx, construct);
    prepareBilling(pricingTrx, *fareCalcCollector, construct);
  }
  bool prepareAgentAndBilling = false;
  if (pricingTrx.diagnostic().diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(pricingTrx, construct);
  }

  Money multiTicketTotal = MultiTicketUtil::getMultiTicketTotal();
  std::string multiTktString = "\n2 TICKETS REQUIRED - TOTAL FARE " +
                               multiTicketTotal.toString(pricingTrx.ticketingDate()) + "\n";
  std::string singleTktString;
  if (ts == MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT)
  {
    Money singleTicketTotal = MultiTicketUtil::getSingleTicketTotal();
    singleTktString = "SINGLE TICKET OPTION - TOTAL FARE " +
                      singleTicketTotal.toString(pricingTrx.ticketingDate()) + "\n";
  }
  else
  {
    singleTktString = "SINGLE TICKET OPTION - NOT AVAILABLE\n";
  }

  _currentIndexForMT = 1;
  // subItin 1 & subItin 2
  for (; subItin <= totalSubItins; subItin++)
  {
    _subCodesForOSC.clear();
    uint16_t currItin = subItin;
    if (ts == MultiTicketUtil::SINGLETKT_NOT_APPLICABLE)
      currItin = static_cast<uint16_t>(subItin - 1);
    _itin = MultiTicketUtil::getMultiTicketItin(pricingTrx, subItin);
    FareCalcCollector* fcCollector = pricingTrx.fareCalcCollector()[currItin];

    if (fcCollector && _itin)
    {
      std::string newRespString = "\n \nTKT" + boost::lexical_cast<std::string>(subItin) + " S";
      if (subItin == 1) // subItin1 so display the banner msgs
      {
        newRespString.insert(0, singleTktString);
        newRespString.insert(0, multiTktString);
      }

      construct.openElement(xml2::MultiTicketExt);
      formatPricingCriteria(newRespString, pricingTrx, subItin, construct);
      newRespString += respMsgVec[subItin];

      formatPricingResponse(construct,
                            newRespString,
                            displayOnly,
                            pricingTrx,
                            fcCollector,
                            tktErrCode,
                            prepareAgentAndBilling);

      if (pricingTrx.getRequest()->isCollectOCFees() && fareCalcCollector)
        formatOCFeesResponse(construct, pricingTrx);

      if (checkForStructuredData(pricingTrx,
                                 TrxUtil::isBaggageActivationXmlResponse(pricingTrx),
                                 TrxUtil::isBaggage302ExchangeActivated(pricingTrx),
                                 TrxUtil::isBaggage302GlobalDisclosureActivated(pricingTrx)))
        formatBaggageResponse(pricingTrx, construct);

      construct.closeElement();
    }
  }
}

Itin*
PricingResponseFormatter::getItin(const PricingTrx& trx) const
{
  return (_itin) ? _itin : trx.itin().front();
}

void
PricingResponseFormatter::formatPricingCriteria(std::string& respString,
                                                PricingTrx& pricingTrx,
                                                uint16_t subItin,
                                                XMLConstruct& construct)
{
  construct.openElement(xml2::PricingCriteria);
  construct.addAttributeShort(xml2::MultiTicketNum, subItin);

  uint16_t totalSeg = static_cast<uint16_t>(_itin->travelSeg().size());
  int segmentNum = 0;
  std::string segString;
  for (uint16_t j = 0; j < totalSeg; j++)
  {
    segmentNum = _itin->travelSeg()[j]->pnrSegment();
    segString.clear();
    segString = boost::lexical_cast<std::string>(segmentNum);
    buildQ00(pricingTrx, construct, segmentNum);
    if (j == (totalSeg - 1)) // last segment
      respString += segString + "\n";
    else
      respString += segString + "/";
  }

  construct.closeElement();
}

bool
PricingResponseFormatter::isAltPricingTrxInVer(const PricingTrx& pricingTrx) const
{
  return isTrxInProperVersion(pricingTrx, PricingTrx::WPA_NOMATCH, 1, 1, 5) ||
         isTrxInProperVersion(pricingTrx, PricingTrx::WP_NOMATCH, 1, 1, 5) ||
         isTrxInProperVersion(pricingTrx, PricingTrx::WPA, 1, 0, 4);
}

bool
PricingResponseFormatter::isPricingTrxInVer(const PricingTrx& pricingTrx,
                                            const short major,
                                            const short minor,
                                            const short revision) const
{
  return pricingTrx.getRequest()->checkSchemaVersion(major, minor, revision) &&
      (pricingTrx.altTrxType() == PricingTrx::WP ||
       pricingTrx.altTrxType() == PricingTrx::WP_NOMATCH ||
       pricingTrx.altTrxType() == PricingTrx::WPA_NOMATCH);
}

bool
PricingResponseFormatter::isTrxInProperVersion(const PricingTrx& pricingTrx,
                                               const PricingTrx::AltTrxType trxType,
                                               const short major,
                                               const short minor,
                                               const short revision) const
{
  return pricingTrx.altTrxType() == trxType &&
         pricingTrx.getRequest()->checkSchemaVersion(major, minor, revision);
}

void
PricingResponseFormatter::buildTotalTTypeOBFeeAmount(PricingTrx& pricingTrx,
                                                     CalcTotals& calcTotals,
                                                     XMLConstruct& construct) const
{
  if (!pricingTrx.getRequest()->isCollectTTypeOBFee())
    return;

  MoneyAmount tTypeFeeTotal = 0;

  for (TicketingFeesInfo* feeInfo : calcTotals.farePath->collectedTTypeOBFee())
  {
    MoneyAmount totalFeeFareAmount;
    if (feeInfo->feePercent() > 0)
    {
      Money targetMoney(NUC);
      calculateObFeeAmountFromPercentage(
          pricingTrx, calcTotals, feeInfo, targetMoney, totalFeeFareAmount);
    }
    else
    {
      XMLConstruct dummyConstruct;
      calculateObFeeAmountFromAmount(
          pricingTrx, calcTotals, dummyConstruct, feeInfo, totalFeeFareAmount, false);
    }

    tTypeFeeTotal += totalFeeFareAmount;
  }

  Money moneyEquiv(calcTotals.equivCurrencyCode);

  if (fallback::sswvt22412ObFeeTCurConv(&pricingTrx))
    construct.addAttributeDouble(
        xml2::TotalFareAmountWithTTypeFees,
        (calcTotals.equivFareAmount + calcTotals.taxAmount() + tTypeFeeTotal),
        moneyEquiv.noDec(pricingTrx.ticketingDate()));
  else
  {
    const MoneyAmount& fareAmt =
        (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
            ? calcTotals.convertedBaseFare
            : calcTotals.equivFareAmount;

    construct.addAttributeDouble(xml2::TotalFareAmountWithTTypeFees,
                                 (fareAmt + calcTotals.taxAmount() + tTypeFeeTotal),
                                 moneyEquiv.noDec(pricingTrx.ticketingDate()));
  }

  construct.addAttributeDouble(
      xml2::TotalTTypeOBFeeAmount, tTypeFeeTotal, moneyEquiv.noDec(pricingTrx.ticketingDate()));

  return;
}

void
PricingResponseFormatter::prepareMarkupAndCommissionAmount(PricingTrx& pricingTrx,
                                                           XMLConstruct& construct,
                                                           const FarePath& fp,
                                                           const CurrencyNoDec noDec) const
{
  const CollectedNegFareData* cNegFareData = fp.collectedNegFareData();
  if (cNegFareData)
  {
    construct.addAttributeDouble(xml2::Cat35MarkupAmount, cNegFareData->markUpAmount(), noDec);

    MoneyAmount markUpAmount = cNegFareData->markUpAmount();
    if (!fallback::fallbackFixPQNRedistributedWholesale(&pricingTrx) && (markUpAmount < 0))
      markUpAmount = 0; // negative markup causes invalid amounts

    if (fp.commissionAmount() - markUpAmount >= 0)
    {
      construct.addAttributeDouble(xml2::TotalCommissionAmount,
                                   (fp.commissionAmount() - markUpAmount),
                                   noDec);
    }
  }
}

void
PricingResponseFormatter::prepareMarkupAndCommissionAmountOld(XMLConstruct& construct,
                                                              const FarePath& fp,
                                                              const CurrencyNoDec noDec) const
{
  const CollectedNegFareData* cNegFareData = fp.collectedNegFareData();
  if (cNegFareData != nullptr &&
      (fp.commissionAmount() - cNegFareData->markUpAmount() > 0 ||
       fabs(fp.commissionAmount() - cNegFareData->markUpAmount()) <= EPSILON))
  {
    // fp.commissionAmount() must be same or more than cNegFareData->markUpAmount()
    construct.addAttributeDouble(xml2::Cat35MarkupAmount, cNegFareData->markUpAmount(), noDec);
    construct.addAttributeDouble(
        xml2::TotalCommissionAmount, (fp.commissionAmount() - cNegFareData->markUpAmount()), noDec);
  }
}

void
PricingResponseFormatter::constructElementVCC(PricingTrx& trx,
                                              XMLConstruct& xml,
                                              const FarePath& fp,
                                              const CarrierCode& valCxr,
                                              const MoneyAmount& commAmount,
                                              const CurrencyNoDec noDec,
                                              FuFcIdMap& fuFcIdCol,
                                              uint16_t& fcId) const

{
  xml.openElement(xml2::ValidatingCarrierCommissionInfo);
  xml.addAttribute(xml2::ValidatingCarrier, valCxr);
  xml.addAttributeDouble(xml2::TotalCommissionAmount, commAmount, noDec);

  if (!fallback::fallbackAMCPhase2(&trx))
  {
    for (auto pu : fp.pricingUnit())
    {
      if (!pu)
        continue;
      for (auto fu : pu->fareUsage())
      {
        if (!fu)
          continue;

        // Do we have comm on FC?
        auto fcCommIt = fu->fcCommInfoCol().find(valCxr);
        if (fcCommIt != fu->fcCommInfoCol().end() && fcCommIt->second)
        {
          uint16_t q6d = 0;
          auto it = fuFcIdCol.find(fu);
          if (it != fuFcIdCol.end())
            q6d = it->second;
          else
          {
            fuFcIdCol[fu] = ++fcId;
            q6d = fcId;
          }
          constructElementFCB(*fcCommIt->second, q6d, noDec, xml);
        }
      }
    }
  }

  xml.closeElement();
}

void
PricingResponseFormatter::constructElementFCB(const amc::FcCommissionData& fcCommInfo,
                                              uint16_t q6d,
                                              const CurrencyNoDec noDec,
                                              XMLConstruct& xml) const
{
  xml.openElement(xml2::FareComponentBreakDown);
  xml.addAttributeShort(xml2::FareComponentNumber, q6d); // "Q6D",
  xml.addAttributeDouble(xml2::TotalCommissionAmount, fcCommInfo.fcCommAmt(), noDec); // "C58",

  if (fcCommInfo.commRuleData().commRuleInfo())
    xml.addAttributeULong(xml2::CommissionRuleId,
                          fcCommInfo.commRuleData().commRuleInfo()->commissionId()); // "C3R",

  if (fcCommInfo.commRuleData().commProgInfo())
    xml.addAttributeULong(xml2::CommissionProgramId,
                          fcCommInfo.commRuleData().commProgInfo()->programId()); // "C3P",

  if (fcCommInfo.commRuleData().commContInfo())
    xml.addAttributeULong(xml2::CommissionContractId,
                          fcCommInfo.commRuleData().commContInfo()->contractId()); // "C3C",
  xml.closeElement();
}

void
PricingResponseFormatter::prepareCommissionForValidatingCarriers(PricingTrx& trx,
                                                                 XMLConstruct& construct,
                                                                 const FarePath& fp,
                                                                 FuFcIdMap& fuFcIdCol,
                                                                 const CurrencyNoDec noDec) const
{
  if (fallback::fallbackAMCPhase2(&trx))
  {
    if (fp.valCxrCommissionAmount().empty())
      return;
  }

  if (trx.getRequest()->ticketingAgent() &&
      !trx.getRequest()->ticketingAgent()->agentCommissionType().empty())
  {
    const int UNABLE_TO_OVERRIDE_NEGDATA_FOR_COMMISSION = 174;
    construct.addAttributeUShort(xml2::Cat35Warning, UNABLE_TO_OVERRIDE_NEGDATA_FOR_COMMISSION);
  }

  CarrierCode defValCxr = trx.isValidatingCxrGsaApplicable()
                              ? fp.defaultValidatingCarrier()
                              : (fp.itin() ? fp.itin()->validatingCarrier() : "");

  // Need to delete method fp.doesValCarriersHaveDiffComm from the code with fallbackAMCPhase2
  if (fallback::fallbackAMCPhase2(&trx))
  {
    if (fp.doesValCarriersHaveDiffComm(defValCxr))
      construct.addAttributeBoolean(xml2::DifferentCommissionAmount, true);
  }

  uint16_t fcId = 0;
  auto it = fp.valCxrCommissionAmount().find(defValCxr);
  auto itEnd = fp.valCxrCommissionAmount().end();
  if (it != itEnd)
    constructElementVCC(trx, construct, fp, defValCxr, it->second, noDec, fuFcIdCol, fcId);

  // Are there commission for alt val-cxrs?
  if (fp.valCxrCommissionAmount().size() > 1 || it == itEnd)
  {
    // Collect and group common commission alt val-cxr together
    for (const auto& p : fp.valCxrCommissionAmount())
    {
      if (p.first != defValCxr)
        constructElementVCC(trx, construct, fp, p.first, p.second, noDec, fuFcIdCol, fcId);
    }
  }
}

void
PricingResponseFormatter::constructElementVCCForCat35(PricingTrx& trx,
                                                      XMLConstruct& construct,
                                                      const FarePath& fp,
                                                      const CurrencyNoDec noDec) const
{
  construct.openElement(xml2::ValidatingCarrierCommissionInfo);
  const CollectedNegFareData* cNegFareData = fp.collectedNegFareData();
  if (cNegFareData != nullptr &&
      (fp.commissionAmount() - cNegFareData->markUpAmount() > 0 ||
       fabs(fp.commissionAmount() - cNegFareData->markUpAmount()) <= EPSILON))
  {
    // Create C58 - Total Commission only
    construct.addAttributeDouble(
        xml2::TotalCommissionAmount, (fp.commissionAmount() - cNegFareData->markUpAmount()), noDec);
  }
  // Create C60 - Total Commission amount including Mark-Up
  construct.addAttributeDouble(xml2::Cat35CommissionAmount, fp.commissionAmount(), noDec);
  // Create C61 - Only Cat 35 Commission Percent
  if (fp.commissionPercent() <= HUNDRED)
  {
    const uint16_t Cat35CommissionPercentageNoDec = 2;
    construct.addAttributeDouble(
        xml2::Cat35CommissionPercentage, fp.commissionPercent(), Cat35CommissionPercentageNoDec);
  }
  else
  {
    construct.addAttributeDouble(xml2::Cat35CommissionPercentage, fp.commissionPercent());
  }
  construct.closeElement();
}

void
PricingResponseFormatter::appendCDataToResponse(Trx& trx, XMLConstruct& construct)
{
  if (fallback::fallbackDebugOverrideBrandingServiceResponses(&trx))
    return;

  Diagnostic& diag = trx.diagnostic();
  if (!diag.getAdditionalData().empty())
  {
    construct.addElementData(diag.getAdditionalData().c_str(), diag.getAdditionalData().size());
  }
}

size_t
PricingResponseFormatter::numOfValidCalcTotals(PricingTrx& pricingTrx,
                                               FareCalcCollector& fareCalcCollector)
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

    if (pricingTrx.getOptions() && !pricingTrx.getOptions()->isPDOForFRRule() &&
        calcTotals.adjustedCalcTotal != nullptr &&
        !calcTotals.adjustedCalcTotal->wpaInfo.wpnDetailResponse.empty())
      count++;
  }
  return count;
}

Indicator
PricingResponseFormatter::getCommissionSourceIndicator(PricingTrx& pricingTrx,
                                                       CalcTotals& calcTotals)
{
  Indicator commSrcIndicator = ' ';
  if (calcTotals.farePath && !calcTotals.farePath->valCxrCommissionAmount().empty())
  {
    commSrcIndicator = 'A'; // Agency Managed Commission (AMC)
  }
  else if (calcTotals.farePath && calcTotals.farePath->collectedNegFareData() &&
           calcTotals.farePath->collectedNegFareData()->indicatorCat35() &&
           (calcTotals.farePath->collectedNegFareData()->comPercent() !=
                RuleConst::PERCENT_NO_APPL ||
            calcTotals.farePath->collectedNegFareData()->comAmount() != 0))
  {
    commSrcIndicator = 'C'; // Cat 35 Commission
  }
  else if (pricingTrx.getRequest()->ticketingAgent() &&
           !pricingTrx.getRequest()->ticketingAgent()->agentCommissionType().empty())
  {
    commSrcIndicator = 'M'; // Manual Commission
  }
  return commSrcIndicator;
}
} // namespace tse
