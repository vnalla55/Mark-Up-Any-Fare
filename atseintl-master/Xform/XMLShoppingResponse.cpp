////-------------------------------------------------------------------
//
//
//  File:  XMLShoppingResponse.cpp
//  Created:  January 20, 2005
//  Authors:  David White, Siriwan Chootongchai
//
//  Description: Implements XML responses to shopping requests
//
//  Copyright Sabre 2005
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------
#include "Xform/XMLShoppingResponse.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/Assert.h"
#include "Common/BrandingUtil.h"
#include "Common/BSRCollectionResults.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigurableValuesAdapterPool.h"
#else
#include "Common/Config/ConfigurableValuesPool.h"
#endif
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/ObFeeDescriptors.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TaxRound.h"
#include "Common/TNBrands/BrandingOptionSpacesDeduplicator.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseSrvStats.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/WnSnapUtil.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PrivateIndicator.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TNBrandsTypes.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/MaxStayRestriction.h"
#include "DBAccess/Nation.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "Diagnostic/Diag900Collector.h"
#include "Diagnostic/Diag931Collector.h"
#include "Diagnostic/Diag988Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagTools.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcHelper.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/TicketingEndorsement.h"
#include "Server/TseServer.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/Common/TaxSplitter.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Xform/CommonParserUtils.h"
#include "Xform/BrandingResponseBuilder.h"
#include "Xform/BrandsOptionsFilterForDisplay.h"
#include "Xform/CommonFormattingUtils.h"
#include "Xform/GenerateSIDForShortCutPricing.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/ResponseFormatter.h"
#include "Xform/XformUtil.h"
#include "Xform/XMLCommonTags.h"
#include "Xform/XMLShoppingSoloCarnivalResponse.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <numeric>
#include <vector>
#include <string>
#include <sstream>
#include <utility>

namespace tse
{
FALLBACK_DECL(fallbackFixFRRHpuForNet)
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(cutFFSolutions)
namespace
{
ConfigurableValue<std::string>
port("SERVER_SOCKET_ADP", "PORT");
ConfigurableValue<uint32_t>
numberOfDatapair("SHOPPING_OPT", "NUMBER_OF_DATEPAIR", 40);
ConfigurableValue<uint32_t>
numberOfSchedulesPerDate("SHOPPING_OPT", "NUMBER_OF_SCHEDULES_PER_DATE", 40);
}

FALLBACK_DECL(fallbackSkipVTAForGSA);
FALLBACK_DECL(fallbackSettingVTAToTrueForGSA);
FALLBACK_DECL(fallbackValidatingCxrGTC);
FALLBACK_DECL(fallbackDivideParty);
FALLBACK_DECL(fallbackMipHPUFix);
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackXMLShoppingResponseSlimming);
FALLBACK_DECL(generateCPLInShoppingResponse);
FALLBACK_DECL(endorsementExpansion);
FIXEDFALLBACK_DECL(noNraAttrInShoppingResponse);
FALLBACK_DECL(fallbackDebugOverrideBrandingServiceResponses);
FALLBACK_DECL(fallbackCommissionManagement);
FALLBACK_DECL(fallbackRemoveAttrMNVInVCL);
FALLBACK_DECL(fallbackValidatingCarrierInItinOrder);
FALLBACK_DECL(fallbackVCLForISShortcut);
FALLBACK_DECL(fallbackAMCPhase2);
FALLBACK_DECL(fallbackAMC2ShoppingChange);
FALLBACK_DECL(fallbackASLDisplayC57);
FALLBACK_DECL(fallbackAMC2Cat35CommInfo);
FALLBACK_DECL(fallbackJira1908NoMarkup);
FALLBACK_DECL(fallbackShoppingSPInHierarchyOrder);
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);
FALLBACK_DECL(fallbackValidatingCarrierInItinOrderMultiSp);
FALLBACK_DECL(fallbackEndorsementsRefactoring)
FALLBACK_DECL(fallbackFRROrgChange);
FALLBACK_DECL(dffOaFareCreation);
FALLBACK_DECL(fallbackIbfNumberOfSoldouts);
FALLBACK_DECL(fallbackSegFromGriRemoval);

namespace
{
Logger
_logger("atseintl.Xform.XMLShoppingResponse");

typedef std::vector<skipper::BrandingOptionSpace>::size_type BrandingOptionSpaceSizeType;
typedef std::vector<QualifiedBrand>::size_type QualifiedBrandSizeType;
const int NO_FAMILY = 99999;
} // namespace

XMLShoppingResponse::Monetary::Monetary()
  : currency(INVALID_CURRENCYCODE), noDec(INVALID_CURRENCY_NO_DEC)
{
}

XMLShoppingResponse::Monetary::Monetary(const MoneyAmount& amount_,
                                        const CurrencyCode& currency_,
                                        const CurrencyNoDec& noDec_)
  : amount(amount_), currency(currency_), noDec(noDec_)
{
}

void
XMLShoppingResponse::Monetary::add(const Monetary& rhs, XMLShoppingResponse& resp)
{
  if (currency == INVALID_CURRENCYCODE)
  {
    currency = rhs.currency;
    noDec = rhs.noDec;
  }

  if (currency != rhs.currency)
  {
    amount += resp.getMoneyAmountInCurrency(rhs.amount, rhs.currency, currency);
  }
  else
  {
    amount += rhs.amount;
  }
}

bool
XMLShoppingResponse::ItineraryTotals::
operator<(const ItineraryTotals& rhs) const
{
  return equivalentAndTax < rhs.equivalentAndTax;
}

void
XMLShoppingResponse::ItineraryTotals::add(const XMLShoppingResponse::ItineraryTotals& rhs,
                                          XMLShoppingResponse& resp)
{
  construction.add(rhs.construction, resp);
  base.add(rhs.base, resp);
  equivalent.add(rhs.equivalent, resp);
  tax.add(rhs.tax, resp);
  const bool convertCurrency = (equivalent.currency != rhs.equivalent.currency);

  if (convertCurrency)
  {
    equivalentAndTax += resp.getMoneyAmountInCurrency(
        rhs.equivalentAndTax, rhs.equivalent.currency, equivalent.currency);
  }
  else
  {
    equivalentAndTax += rhs.equivalentAndTax;
  }

  if (rhs.psgMileage)
  {
    if (psgMileage)
      *psgMileage += *rhs.psgMileage;
    else
      psgMileage = rhs.psgMileage;
  }

  if (rhs.feesAmount)
  {
    MoneyAmount rhsFeesAmount = *rhs.feesAmount;

    if (convertCurrency)
    {
      rhsFeesAmount = resp.getMoneyAmountInCurrency(
          rhsFeesAmount, rhs.equivalent.currency, equivalent.currency);
    }

    if (feesAmount)
      *feesAmount += rhsFeesAmount;
    else
      feesAmount = rhsFeesAmount;
  }
}

bool
XMLShoppingResponse::generateXML(PricingTrx& trx, std::string& res)
{
  if (trx.getOptions()->isCarnivalSumOfLocal())
  {
    XMLShoppingSoloCarnivalResponse(trx).getXML(res);
  }
  else
  {
    XMLShoppingResponse(trx).getXML(res);
  }

  return true;
}

bool
XMLShoppingResponse::generateXML(PricingTrx& trx, ErrorResponseException& ere, std::string& res)
{
  XMLShoppingResponse(trx, &ere).getXML(res);
  return true;
}

void
XMLShoppingResponse::generateHostname(Node& nodeShoppingResponse)
{
  {
    const uint16_t HOSTNAME_SIZE = 256;
    char hostname[HOSTNAME_SIZE];
    if (gethostname(hostname, HOSTNAME_SIZE) == 0)
    {
      nodeShoppingResponse.convertAttr(xml2::ServerHostname, hostname);
    }
  }
}

bool
XMLShoppingResponse::generateXML(ErrorResponseException& ere, std::string& res)
{
  XMLWriter writer;
  {
    Node nodeShoppingResponse(writer, "ShoppingResponse");

    nodeShoppingResponse.convertAttr("Q0S", 0);
    nodeShoppingResponse.convertAttr("Q0F", 0);
    XMLShoppingResponse::generateHostname(nodeShoppingResponse);
    {
      Node nodeDIA(writer, "DIA");
      nodeDIA.convertAttr("Q0K", ere.code());
      {
        Node nodeMTP(writer, "MTP");
        nodeMTP.addData(ere.message());
      }
    }
  }
  writer.result(res);
  return true;
}

XMLShoppingResponse::XMLShoppingResponse(PricingTrx& trx, ErrorResponseException* ere)
  : _trx(trx),
    _shoppingTrx(dynamic_cast<const ShoppingTrx*>(&trx)),
    _ffTrx(dynamic_cast<const FlightFinderTrx*>(&trx)),
    _rexTrx(dynamic_cast<RexPricingTrx*>(&trx)),
    _error(ere),
    _q4qGrouping(_shoppingTrx),
    _baggageResponse(_writer)
{
  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(_trx) &&
      TrxUtil::isRbdByCabinValueInShopResponse(_trx))
    _rbdByCabinVSinHouse = true;
}

void
XMLShoppingResponse::getXML(std::string& res)
{
  generateResponse();
  _writer.result(res);
}

namespace
{
typedef std::pair<SopIdVec, GroupFarePath*> ISSolution;
bool
compareISSolutions(const ISSolution& a, const ISSolution& b)
{
  if (a.second == nullptr && b.second == nullptr)
  {
    const int sum1 = std::accumulate(a.first.begin(), a.first.end(), 0);
    const int sum2 = std::accumulate(b.first.begin(), b.first.end(), 0);
    return sum1 < sum2;
  }

  if (b.second == nullptr)
  {
    return false;
  }

  if (a.second == nullptr)
  {
    return true;
  }

  if (a.second->getTotalNUCAmount() == b.second->getTotalNUCAmount())
  {
    const int sum1 = std::accumulate(a.first.begin(), a.first.end(), 0);
    const int sum2 = std::accumulate(b.first.begin(), b.first.end(), 0);
    return sum1 < sum2;
  }

  return a.second->getTotalNUCAmount() < b.second->getTotalNUCAmount();
}
}

void
XMLShoppingResponse::generateError()
{
  std::string errorMessage = _error->message();
  if (errorMessage.empty())
  {
    errorMessage = "UNKNOWN ISMIP ERROR";
  }

  std::string diagnosticString = "";
  if (_trx.diagnostic().diagnosticType() != DiagnosticNone)
    diagnosticString = _trx.diagnostic().toString();

  const std::string& fullMessage =
      utils::truncateDiagIfNeeded(errorMessage + "\n" + diagnosticString, _trx);
  if (isFF())
  {
    {
      Node nodeDIA(_writer, "DIA");
      nodeDIA.convertAttr("Q0K", _error->code());

      {
        Node nodeMTP(_writer, "MTP");
        nodeMTP.addData(fullMessage);
      }
    }
    {
      if ((_trx.recordMetrics() == true) ||
          (_trx.diagnostic().diagnosticType() != DiagnosticNone) ||
          (isFF() && _ffTrx->ignoreDiagReq()))
      {
        std::ostringstream metrics;
        Node nodeDIA(_writer, "DIA");
        MetricsUtil::trxLatency(metrics, _trx);
        Node nodeMTP(_writer, "MTP");
        nodeMTP.addData(metrics.str());
      }
    }
  }
  else
  {
    Node nodeDIA(_writer, "DIA");
    nodeDIA.convertAttr("Q0K", _error->code());

    {
      Node nodeMTP(_writer, "MTP");
      nodeMTP.addData(fullMessage);
    }

    if ((_trx.recordMetrics() == true) || (_trx.diagnostic().diagnosticType() != DiagnosticNone) ||
        (isFF() && _ffTrx->ignoreDiagReq()))
    {
      std::ostringstream metrics;
      MetricsUtil::trxLatency(metrics, _trx);
      Node nodeMTP(_writer, "MTP");
      nodeMTP.addData(metrics.str());
    }
  }
}

namespace
{
bool
comparePaxTypeInputOrder(const PaxType* pax1, const PaxType* pax2)
{
  return pax1->inputOrder() < pax2->inputOrder();
}

// helper functions used to order fare paths into the original input order
bool
compareFarePathInputOrder(const FarePath* path1, const FarePath* path2)
{
  return path1->paxType()->inputOrder() < path2->paxType()->inputOrder();
}

bool
compareFPPQItemInputOrder(const FPPQItem* path1, const FPPQItem* path2)
{
  return compareFarePathInputOrder(path1->farePath(), path2->farePath());
}
}

size_t
XMLShoppingResponse::countMIPSolutions(const std::vector<Itin*>& itins) const
{
  return count_if(itins.cbegin(),
                  itins.cend(),
                  [this](const Itin* const itin)
                  { return this->itinIsValidSolution(itin); });
}

size_t
countSolCarnivalSolutions(const PricingTrx& trx)
{
  size_t res = 0;
  typedef tse::PricingTrx::SOLItinGroupsMap::value_type GroupPairType;
  typedef boost::optional<SOLItinGroups::GroupType> OptGroupType;

  std::map<OptGroupType, size_t> counters;

  for (const GroupPairType& group : trx.solItinGroupsMap())
  {
    OptGroupType cheapestGroup = group.second->getCheapestItinGroup();
    ++counters[cheapestGroup];
  }
  res = counters[SOLItinGroups::ORIGINAL] + counters[SOLItinGroups::INTERLINE_BY_CXR_AND_LEG] +
        counters[SOLItinGroups::INTERLINE_BY_CXR_GROUPING] +
        counters[SOLItinGroups::ONLINE_BY_LEG] +
        counters[SOLItinGroups::ONLINE_BY_DOM_INT_GROUPING];
  return res;
}

std::vector<std::string>
getConfigInfo(const PricingTrx& trx)
{
  std::string configMsg;
  std::vector<tse::ConfigMan::NameValue> configs;
  std::vector<std::string> configMsgVec;
  std::string cfgValue = trx.diagnostic().diagParamMapItem("DD");

  if (cfgValue == "ALL")
  {
    tse::ConfigMan* dynamicCfg = trx.dynamicCfg().get();

    if (dynamicCfg)
      dynamicCfg->getValues(configs);
    else
      Global::config().getValues(configs);
  }
  else if (cfgValue == "ALLSTATIC")
  {
    Global::config().getValues(configs);
  }
  else if (cfgValue == "ALLDYNAMIC")
  {

#ifdef CONFIG_HIERARCHY_REFACTOR
    ConfigurableValuesAdapterPool::collectConfigMsg(trx, configMsgVec);
#else
    DynamicConfigurableValuesPool::collectConfigMsg(trx, configMsgVec);
#endif

  }
  else
  {
    Global::config().getValues(configs, cfgValue);
  }

  std::string temp = "";

  for (const auto& configItem : configs)
  {
    std::string name = configItem.name;
    std::replace(name.begin(), name.end(), '_', ' ');
    std::string value = configItem.value;
    std::replace(value.begin(), value.end(), '_', ' ');
    std::string group = configItem.group;
    std::replace(group.begin(), group.end(), '_', ' ');

    if (temp != group)
    {
      configMsg = "\n***** " + group + " *****\n" + name + ":" + value;
      temp = group;
    }
    else
    {
      configMsg = name + ":" + value;
    }

    std::transform(configMsg.begin(), configMsg.end(), configMsg.begin(), (int (*)(int))toupper);
    configMsgVec.push_back(configMsg);
  }

  if (!configMsgVec.empty())
  {
    configMsgVec.insert(configMsgVec.begin(), "***** CONFIGURATION DIAGNOSTIC START *****");
    configMsgVec.push_back("***** CONFIGURATION DIAGNOSTIC END *****");
  }

  return configMsgVec;
}

void
XMLShoppingResponse::generateResponse()
{
  Node nodeShoppingResponse(_writer, "ShoppingResponse");

  if (!generateCommonContent(nodeShoppingResponse))
  {
    return;
  }

  if (_trx.getRequest()->isSettlementTypesRequest() && _trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    generateNodeForSettlementTypes();
    return;
  }

  if (isIS())
  {
    generateISResponse();
  }
  else
  {
    generateMIPResponse();
  }

  prepareLatencyInfo();
}

bool
XMLShoppingResponse::generateCommonContent(Node& nodeShoppingResponse)
{
  if (_trx.getRequest()->processVITAData())
  {
    _trx.dataHandle().get(_interlineTicketCarrierData);
  }

  if (!_trx.token().empty())
  {
    nodeShoppingResponse.convertAttr("STK", _trx.token());
  }

  generateHurryLogicFlag(nodeShoppingResponse);

  if (_trx.getRequest()->isDisplayBaseline())
    nodeShoppingResponse.convertAttr("BSL", BUILD_LABEL_STRING);

  if (_trx.segmentFeeApplied())
  {
    nodeShoppingResponse.convertAttr("S07", "TOTAL INCLUDES CARRIER IMPOSED SURCHARGES");
  }

  if (_error != nullptr)
  {
    nodeShoppingResponse.convertAttr("Q0S", 0);
    nodeShoppingResponse.convertAttr("Q0F", 0);
    if (!fallback::fallbackIbfNumberOfSoldouts(&_trx) &&
        _trx.getTrxType() == PricingTrx::MIP_TRX &&
        _trx.getRequest()->isBrandedFaresRequest())
    {
      nodeShoppingResponse.convertAttr("Q1S", 0);
    }
    XMLShoppingResponse::generateHostname(nodeShoppingResponse);
    generateError();
    generateNodeADSforCalendarOrAwardAltDates();

    if (_trx.isExchangeTrx() && _trx.getRequest()->isBrandedFaresRequest() &&
        _trx.getTrxType() == PricingTrx::MIP_TRX)
    {
      generateNodeForExchangeWithBrands();
    }

    if (_trx.isExchangeTrx() && _trx.getTrxType() == PricingTrx::MIP_TRX &&
        _trx.billing()->actionCode() == "WFR.C")
    {
      prepareUnflownItinPriceInfo(); // generate CSO UFL part
    }

    return false;
  }

  if (_trx.snapRequest())
  {
    preparePrimeItins();
  }

  std::size_t solutions = 0;

  if (_trx.getTrxType() == PricingTrx::FF_TRX)
  {
    if (fallback::cutFFSolutions(&_trx))
      truncateSolutionNumber();

    solutions = countFFSolutionNumber();
  }
  else
  {
    if (_trx.snapRequest())
    {
      solutions = _trx.itin().size();
    }
    else if (_trx.getOptions()->isCarnivalSumOfLocal())
    {
      solutions = countSolCarnivalSolutions(_trx);
    }
    else
    {
      solutions = _shoppingTrx != nullptr
                      ? _shoppingTrx->flightMatrix().size() + _shoppingTrx->estimateMatrix().size()
                      : countMIPSolutions(_trx.itin());
    }
  }

  nodeShoppingResponse.convertAttr("Q0S", solutions);
  nodeShoppingResponse.convertAttr("Q0F", updateTotalBrandSolutions());
  if (!fallback::fallbackIbfNumberOfSoldouts(&_trx) &&
      _trx.getTrxType() == PricingTrx::MIP_TRX &&
      _trx.getRequest()->isBrandedFaresRequest())
  {
    nodeShoppingResponse.convertAttr("Q1S", std::count_if(_trx.itin().begin(), _trx.itin().end(),
          [](const Itin* itn) { return itn->farePath().empty(); }));
  }

  if (solutions && isIS() && (_trx.diagnostic().diagnosticType() == Diagnostic931))
    formatDiag931();

  XMLShoppingResponse::generateHostname(nodeShoppingResponse);

  if (isMIP() && _trx.getRequest()->processVITAData())
  {
    if (_trx.diagnostic().diagnosticType() == Diagnostic988)
    {
      formatDiag988();
    }
  }

  if (_trx.altDateCutOffNucThreshold() > 0)
  {
    nodeShoppingResponse.convertAttr("C6W", _trx.altDateCutOffNucThreshold());
  }

  if (isIS() && _shoppingTrx->isSumOfLocalsProcessingEnabled() && _shoppingTrx->getSolNoLocalInd())
    nodeShoppingResponse.convertAttr("NLF", "T");

  generateResponseDiagnostic();
  return true;
}

std::size_t
XMLShoppingResponse::updateTotalBrandSolutions()
{
  size_t res = 0;
  for (const auto itin : _trx.itin())
  {
    if (itin->farePath().empty() || (itin->errResponseCode() != ErrorResponseException::NO_ERROR) ||
        (_trx.isBRAll() && itin->farePath().empty()))
    {
      continue;
    }

    if (LIKELY(!itin->farePath().empty()))
    {
      res = res + itin->farePath().size();
    }
  }
  if (0 != res)
  {
    res = res / _trx.paxType().size();
  }
  return (res);
}

int
XMLShoppingResponse::computeFamilyNumber(const std::vector<ISSolution>& paths,
                                         const std::vector<int>& sops,
                                         bool jjBrazilDomestic,
                                         std::map<std::pair<size_t, bool>, size_t>& q5qMapGTC,
                                         std::map<std::pair<int, bool>, int>& q5qJJ,
                                         const std::string& mapKey)
{
  int familyNumber = NO_FAMILY;
  for (std::vector<ISSolution>::const_iterator j = paths.begin(); j != paths.end(); ++j)
  {
    if (j->first == sops)
    {
      familyNumber = j - paths.begin();
      break;
    }
  }

  if (familyNumber == NO_FAMILY)
    return NO_FAMILY;

  if (!fallback::fallbackValidatingCxrGTC(&_trx) && _trx.isValidatingCxrGsaApplicable())
  {
    familyNumber = generateQ5QForGTC(sops, q5qMapGTC, familyNumber);
  }

  if (jjBrazilDomestic) // check if this trx has Brazil domestic JJ FarePaths
  {
    calculateQ5Q(q5qJJ, &familyNumber, mapKey);
  }

  return familyNumber;
}

void
XMLShoppingResponse::generateISResponse()
{
  generateOND();
  generateDFL(); // generate FF part of respond XML if(_trx.getTrxType() == PricingTrx::FF_TRX)
  _atLeastOneDomItin = false;
  _atLeastOneInterItin = false;
  std::vector<ISSolution> paths;
  std::multimap<MoneyAmount, ISSolution> shortCutResponse;

  for (const ShoppingTrx::FlightMatrix::value_type& fMatrixIter : _shoppingTrx->flightMatrix())
  {
    if (UNLIKELY((fMatrixIter.second) && fMatrixIter.second->isShortCutPricing()))
    {
      shortCutResponse.insert(std::make_pair(fMatrixIter.second->getTotalNUCAmount(), fMatrixIter));
    }
    else
    {
      paths.push_back(fMatrixIter);
    }
  }

  if (_trx.awardRequest())
    moveMileageToAmt(paths);

  std::sort(paths.begin(), paths.end(), compareISSolutions);
  std::map<SopIdVec, int> q4qMap;
  std::map<SopIdVec, int> q5qMap;
  std::map<SopIdVec, std::string> q6qMap;
  std::map<std::pair<size_t, bool>, size_t> q5qMapGTC;
  std::map<std::pair<int, bool>, int> q5qJJ; // Map contains ((old)familyNumber(Q5Q), bool(T(JJ),
  // F(non JJ) Carrier) as Key, (new)familyNumber(Q5Q) as
  // value
  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);

  for (std::vector<ISSolution>::const_iterator i = paths.begin(); i != paths.end(); ++i)
  {
    std::string mapKey;
    const int carrierGroupIndex = _q4qGrouping.generateQ4Q(i->first, mapKey);
    q6qMap[i->first] = mapKey;
    int familyNumber;

    // WARNING
    // The logic here is completely messed up. The computed family number is just
    // the position of i inside the paths vector...
    // For now, make sure that is true using a TSE_ASSERT. This code needs a serious rewrite,
    // though that will (hopefully) happen later.

    familyNumber = computeFamilyNumber(paths,
                                        i->first,
                                        shoppingTrx.isThroughFarePrecedenceFoundInvalidFarePath(),
                                        q5qMapGTC,
                                        q5qJJ,
                                        mapKey);
    TSE_ASSERT(familyNumber == (i - paths.begin()));

    if (familyNumber != NO_FAMILY)
      q5qMap[i->first] = familyNumber;

    if (i->second == nullptr || i->second->groupFPPQItem().empty())
    {
      MoneyAmount totalNUCAmount = 0.0;
      MoneyAmount totalNUCBaseFareAmount = 0.0;

      if (i->second != nullptr)
      {
        totalNUCAmount = i->second->getTotalNUCAmount();
        totalNUCBaseFareAmount = i->second->getTotalNUCBaseFareAmount();
      }

      generateITN(i->first,
                  totalNUCAmount,
                  totalNUCBaseFareAmount,
                  nullptr,
                  &carrierGroupIndex,
                  &familyNumber,
                  &(q6qMap[i->first]));
    }
    else
    {
      std::sort(i->second->groupFPPQItem().begin(),
                i->second->groupFPPQItem().end(),
                compareFPPQItemInputOrder);

      TSE_ASSERT(familyNumber != NO_FAMILY);

      q4qMap[i->first] = carrierGroupIndex;

      generateITN(i->first, i->second, paths, &q4qMap[i->first], &familyNumber, &q6qMap[i->first]);
    }
  }

  std::vector<ISSolution> estimates;

  for (const ShoppingTrx::EstimateMatrix::value_type& eMatrixIter : _shoppingTrx->estimateMatrix())
  {
    if (UNLIKELY((eMatrixIter.second.second) && (eMatrixIter.second.second->isShortCutPricing())))
    {
      shortCutResponse.insert(
          std::make_pair(eMatrixIter.second.second->getTotalNUCAmount(),
                         ISSolution(eMatrixIter.first, eMatrixIter.second.second)));
    }
    else
    {
      estimates.push_back(ISSolution(eMatrixIter.first, eMatrixIter.second.second));
    }
  }

  std::sort(estimates.begin(), estimates.end(), compareISSolutions);

  for (std::vector<ISSolution>::const_iterator i = estimates.begin(); i != estimates.end(); ++i)
  {
    ShoppingTrx::EstimateMatrix::const_iterator itor =
        _shoppingTrx->estimateMatrix().find(i->first);
    TSE_ASSERT(itor != _shoppingTrx->estimateMatrix().end());

    const std::vector<int>& baseSolution = itor->second.first;

    int familyNumber = q5qMap[baseSolution];
    if (!fallback::fallbackValidatingCxrGTC(&_trx) && _trx.isValidatingCxrGsaApplicable())
    {
      familyNumber = generateQ5QForGTC(i->first, q5qMapGTC, familyNumber);
    }

    if (i->second == nullptr)
    {
      generateITN(
          i->first, 0.0, 0.0, i->second, &q4qMap[baseSolution], &familyNumber, &q6qMap[i->first]);
    }
    else
    {
      std::sort(i->second->groupFPPQItem().begin(),
                i->second->groupFPPQItem().end(),
                compareFPPQItemInputOrder);

      const GroupFarePath* gfp = i->second;
      generateITN(i->first,
                  gfp->getTotalNUCAmount(),
                  gfp->getTotalNUCBaseFareAmount(),
                  gfp,
                  &q4qMap[baseSolution],
                  &familyNumber,
                  &q6qMap[baseSolution]);
    }
  }

  for (auto& shortCutResp : shortCutResponse)
  {
    Itin* itin = shortCutResp.second.second->groupFPPQItem().back()->farePath()->itin();
    ItineraryTotals t;
    GenerateSIDForShortCutPricing genSID(*this, shortCutResp.second.first, _writer, _shoppingTrx);
    generateITNbody(
        itin, "ITN", boost::bind(&tse::GenerateSIDForShortCutPricing::process, &genSID, itin), t);
  }

  // If itins return from IS are mixed between international and domestic itin, INF/PBR = "T"

  if (_atLeastOneDomItin && _atLeastOneInterItin)
  {
    Node(_writer, "INF").convertAttr("PBR", "T");
  }
}

void
XMLShoppingResponse::generateMIPResponse()
{
  // output MIP response sorted by cheapest total money amount
  generateNodeADSforCalendarOrAwardAltDates();

  if ((_trx.diagnostic().diagnosticType() == DiagnosticNone) ||
      (_trx.fareCalcCollector().empty() == false) ||
      (_trx.getRequest()->isParityBrandsPath())) // for IBF always print output
  {
    generateNodeADPifCutOffReached();
    prepareAndGenerateItineraries();
  }
}

void
XMLShoppingResponse::prepareLatencyInfo()
{
  if (!TrxUtil::needMetricsInResponse())
    return;

  Node ltcNode(_writer, xml2::ExtraLatencyInfo);
  std::ostringstream stringStream;
  stringStream.setf(std::ios::fixed, std::ios::floatfield);
  stringStream.precision(3);

  for (const Trx::Latency& latencyData : _trx.latencyData())
  {
    if (TrxUtil::needMetricsInResponse(latencyData.description))
    {
      Node ldtNode(_writer, xml2::LatencyDetail);
      ldtNode.attr(xml2::LatencyDescription, latencyData.description);
      ldtNode.convertAttr(xml2::LatencyNCalls, latencyData.nItems);
      stringStream.str("");
      stringStream << latencyData.wallTime;
      ldtNode.convertAttr(xml2::LatencyWallTime, stringStream.str());
      stringStream.str("");
      stringStream << (latencyData.userTime + latencyData.systemTime);
      ldtNode.convertAttr(xml2::LatencyCpuTime, stringStream.str());
    }
  }
}

void
XMLShoppingResponse::prepareAndGenerateItineraries()
{
  if (_trx.diagnostic().diagnosticType() != DiagnosticNone &&
      _trx.getRequest()->isBrandedFaresRequest() && !_trx.getRequest()->isCatchAllBucketRequest() &&
      _trx.validBrands().empty())
  {
    return;
  }

  for (const auto itin : _trx.itin())
  {
    std::sort(itin->farePath().begin(), itin->farePath().end(), compareFarePathInputOrder);
  }

  generateItineraries();
}

void
XMLShoppingResponse::generateItineraries()
{
  if (_trx.snapRequest() && !_taxSplitter)
  {
    _taxSplitter = &_trx.dataHandle().safe_create<TaxSplitter>(static_cast<ShoppingTrx&>(_trx));
  }

  for (const auto itin : _trx.itin())
  {
    if (_trx.snapRequest())
    {
      generateSnapITN(itin);
    }
    else
    {
      ItineraryTotals t;
      generateITNbody(itin, "ITN", boost::bind(&XMLShoppingResponse::generateSID, this, _1), t);
    }
  }
}

void
XMLShoppingResponse::generateNodeADPifCutOffReached()
{
  // if the trx has sthe setCutOffReached()
  // build the ADP altdatePair that reach the cutoff threshold
  if (_trx.cutOffReached())
  {
    for (const auto& altDatePair : _trx.altDatePairs())
    {
      if (altDatePair.second->numOfSolutionNeeded > 0)
      {
        Node nodeADP(_writer, "ADP");
        nodeADP.convertAttr("D01", altDatePair.first.first.dateToString(YYYYMMDD, "-"));

        if (altDatePair.first.second != DateTime::emptyDate())
        {
          nodeADP.convertAttr("D02", altDatePair.first.second.dateToString(YYYYMMDD, "-"));
        }
      }
    }
  }
}

void
XMLShoppingResponse::generateResponseDiagnostic()
{
  if ((_trx.recordMetrics() == false) && ((_trx.diagnostic().diagnosticType() == DiagnosticNone) ||
                                          (isFF() && _ffTrx->ignoreDiagReq())))
  {
    return;
  }

  std::string metrics = utils::getMetricsInfo(_trx);

  Node nodeDIA(_writer, "DIA");
  nodeDIA.convertAttr("Q0A", static_cast<int>(_trx.diagnostic().diagnosticType()));
  // Displaying hostname/port  and database
  std::string hostname = getenv("HOSTNAME");

  for (auto& hostNameLetter : hostname)
    hostNameLetter = std::toupper(hostNameLetter);
  std::string hostNPort = "HOSTNAME : " + hostname + " PORT : " + port.getValue() + "\n";
  std::string dbInfo = utils::getDbInfo();
  std::string buildMsg;
  buildMsg = "BASELINE: " BUILD_LABEL_STRING;
  std::transform(buildMsg.begin(), buildMsg.end(), buildMsg.begin(), (int (*)(int))toupper);

  if (TseServer::getInstance())
  {
    buildMsg += "\nPROCESS IMAGE NAME: ";
    buildMsg += TseServer::getInstance()->getProcessImageName();
  }

  buildMsg += "\n\n";

  if ((_trx.diagnostic().diagnosticType() == Diagnostic854) ||
      (_trx.diagnostic().diagnosticType() == Diagnostic930))
  {
    std::vector<std::string> configMsgVec;
    configMsgVec = getConfigInfo(_trx);

    for (const auto& configMsg : configMsgVec)
    {
      buildMsg = buildMsg + configMsg + "\n";
    }
  }

  nodeDIA.addData(utils::truncateDiagIfNeeded(
      hostNPort + dbInfo + buildMsg + _trx.diagnostic().toString() + metrics, _trx));

  if (!fallback::fallbackDebugOverrideBrandingServiceResponses(&_trx))
  {
    if (UNLIKELY(!_trx.diagnostic().getAdditionalData().empty()))
      nodeDIA.addRawText(_trx.diagnostic().getAdditionalData());
  }
}

std::string
XMLShoppingResponse::formatAmount(const MoneyAmount amount, const CurrencyNoDec decimalPlaces) const
{
  std::ostringstream str;
  str.setf(std::ios::fixed, std::ios::floatfield);
  str.precision(decimalPlaces);
  str << amount;
  return str.str();
}

std::string
XMLShoppingResponse::formatAmount(const Money& money) const
{
  return formatAmount(money.value(), money.noDec(_trx.ticketingDate()));
}

void
XMLShoppingResponse::generateITN(const SopIdVec& sops,
                                 const GroupFarePath* path,
                                 const std::vector<ISSolution>& paths,
                                 int* q4q,
                                 int* q5q,
                                 std::string* q6qMapValue)
{
  if (path == nullptr)
  {
    return;
  }

  Node nodeITN(_writer, "ITN");
  {
    generateP95(sops, nodeITN);

    if (LIKELY(q4q != nullptr))
    {
      nodeITN.convertAttr("Q4Q", *q4q);
    }

    if (LIKELY(q6qMapValue))
      nodeITN.convertAttr("Q6Q", *q6qMapValue);

    if (LIKELY(path->groupFPPQItem().empty() == false))
    {
      FarePath* farePath = (*(path->groupFPPQItem().begin()))->farePath();

      if (LIKELY(farePath != nullptr))
      {
        const MoneyAmount estTax = farePath->itin()->estimatedTax();
        nodeITN.convertAttr("C65", estTax);
      }
    }

    if (LIKELY(q5q != nullptr) && (*q5q) != NO_FAMILY)
    {
      nodeITN.convertAttr("Q5Q", *q5q);
    }

    generatePVTandFBR(path, nodeITN);
    generatePTF(path, nodeITN);
    if (_shoppingTrx->isSumOfLocalsProcessingEnabled())
      generateTFO(static_cast<const shpq::SoloGroupFarePath*>(path), nodeITN);

    if (path->groupFPPQItem().empty() == false && true == isJcbItin(*path))
    {
      nodeITN.convertAttr("PXP", "T");
    }
    else
    {
      nodeITN.convertAttr("PXP", "F");
    }

    generateBKKAttr(*path, sops);

    for (const auto fppqItem : path->groupFPPQItem())
    {
      const FarePath& farePath = *fppqItem->farePath();
      Node nodePSG(_writer, "PSG");
      generatePSGAttr(nodePSG, farePath);
    }

    for (const auto excPaxType : _shoppingTrx->excludedPaxType())
    {
      // For excluded PaxType like CNN/INF etc putting 0 amount
      Node nodePSG(_writer, "PSG");
      generatePSGAttr(nodePSG, excPaxType->number(), excPaxType->paxType(), 0);
    }

    generateTOT(path->getTotalNUCAmount(),
                path->getTotalNUCBaseFareAmount(),
                _trx.itin().empty() ? NUC : _trx.itin().front()->calculationCurrency());
  }
}

void
XMLShoppingResponse::generateITN(const SopIdVec& sops,
                                 const MoneyAmount totalNucAmount,
                                 const MoneyAmount totalNucBaseFareAmount,
                                 const GroupFarePath* path,
                                 const int* q4q,
                                 const int* q5q,
                                 const std::string* q6q)
{
  Node nodeITN(_writer, "ITN");
  {
    if (LIKELY(q4q != nullptr))
    {
      nodeITN.convertAttr("Q4Q", *q4q);
      if (LIKELY(q6q))
        nodeITN.convertAttr("Q6Q", *q6q);
    }

    if ((path != nullptr) && (path->groupFPPQItem().empty() == false))
    {
      const FarePath* farePath = (*(path->groupFPPQItem().begin()))->farePath();

      if (LIKELY(farePath != nullptr))
      {
        const MoneyAmount estTax = farePath->itin()->estimatedTax();
        nodeITN.convertAttr("C65", estTax);
      }
    }

    if (LIKELY(q5q != nullptr) && (*q5q) != NO_FAMILY)
    {
      nodeITN.convertAttr("Q5Q", *q5q);
    }

    generatePVTandFBR(path, nodeITN);
    generatePTF(path, nodeITN);
    if (_shoppingTrx->isSumOfLocalsProcessingEnabled())
      generateTFO(static_cast<const shpq::SoloGroupFarePath*>(path), nodeITN);

    generateP95(sops, nodeITN);
    generateFMT(sops, nodeITN);
    ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);
    size_t sopsSize = sops.size();

    if (path != nullptr && (path->groupFPPQItem().empty() == false))
    {
      if (isJcbItin(*path))
      {
        nodeITN.convertAttr("PXP", "T");
      }
      else
      {
        nodeITN.convertAttr("PXP", "F");
      }

      generateBKKAttr(*path, sops);
    }
    else
    {
      nodeITN.convertAttr("PXP", "F");

      for (uint16_t n = 0; n != sopsSize; ++n)
      {
        Node nodeSID(_writer, "SID");
        nodeSID.convertAttr("Q14", n).convertAttr("Q15", ShoppingUtil::findSopId(_trx, n, sops[n]));
        // format BKK
        Itin* itin = shoppingTrx.legs()[n].sop()[sops[n]].itin();

        for (const auto travelSegment : itin->travelSeg())
        {
          if (travelSegment->segmentType() == Arunk)
          {
            continue;
          }

          CabinType prefferedCabin = travelSegment->bookedCabin();

          if (shoppingTrx.diversity().getModel() == DiversityModelType::EFA)
          {
            // because we generate only flight combinations
            // we must push cabin received from input XML
            prefferedCabin = shoppingTrx.legs()[n].preferredCabinClass();
          }

          BookingCode bkc = travelSegment->getBookingCode();

          if (bkc == DUMMY_BOOKING)
          {
            bkc = travelSegment->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse);
          }

          Node(_writer, "BKK")
              .attr("B30", bkc)
              .convertAttr("Q13", travelSegment->bookedCabin().getCabinIndicator())
              .convertAttr("B31", prefferedCabin.getClassAlphaNum(_rbdByCabinVSinHouse));
        } // SID
      }
    }

    // generatePSGAttr ()
    MoneyAmount totalNucAmountPerPax;
    size_t numberOfPax = _trx.paxType().size() - _shoppingTrx->excludedPaxType().size();

    if ((totalNucAmount == 0) || (numberOfPax < 1))
    {
      totalNucAmountPerPax = totalNucAmount;
    }
    else
    {
      totalNucAmountPerPax = totalNucAmount / numberOfPax;
    }

    for (const auto paxType : _trx.paxType())
    {
      Node nodePSG(_writer, "PSG");
      generatePSGAttr(nodePSG, paxType->number(), paxType->paxType(), totalNucAmountPerPax);
    }

    for (const auto excludedPaxType : _shoppingTrx->excludedPaxType())
    {
      Node nodePSG(_writer, "PSG");
      // For excluded PaxType like CNN/INF etc putting 0 amount
      generatePSGAttr(nodePSG, excludedPaxType->number(), excludedPaxType->paxType(), 0);
    }

    // generat TOTAttr()
    Node(_writer, "TOT").attr("C40", "NUC").convertAttr("C5A", totalNucAmount).convertAttr(
        "C67", totalNucBaseFareAmount);
  }
}

void
XMLShoppingResponse::generatePTF(const GroupFarePath* path, Node& nodeITN)
{
  if (path && !path->groupFPPQItem().empty())
  {
    const FarePath* farePath = path->groupFPPQItem().front()->farePath();

    for (PricingUnit* pu : farePath->pricingUnit())
    {
      for (FareUsage* fu : pu->fareUsage())
      {
        FareMarket* fm = fu->paxTypeFare()->fareMarket();
        if (FareMarket::SOL_FM_LOCAL == fm->getFmTypeSol())
        {
          nodeITN.convertAttr("PTF", 'F');
          return;
        }
      }
    }
    // All Faremarkets in the FarePath are Thru
    nodeITN.convertAttr("PTF", 'T');
  }
}

void
XMLShoppingResponse::generateTFO(const shpq::SoloGroupFarePath* path, Node& nodeITN)
{
  if (path && path->getProcessThruOnlyHint())
    nodeITN.convertAttr("TFO", 'T');
}

void
XMLShoppingResponse::generateBKKAttr(const GroupFarePath& path, const SopIdVec& sops)
{
  TSE_ASSERT(!path.groupFPPQItem().empty());
  const size_t sopsSize = sops.size();
  const FarePath* farePath = path.groupFPPQItem().front()->farePath();
  FareUsagesPerLeg fareUsagesPerLeg;
  collectFareUsagesPerLeg(*farePath, sops, fareUsagesPerLeg);

  shpq::CxrKeyPerLeg cxrKeyPerLeg;
  uint64_t duration(0);

  if (_shoppingTrx->isSumOfLocalsProcessingEnabled() ||
      _shoppingTrx->isIataFareSelectionApplicable())
  {
    ShoppingUtil::collectSopsCxrKeys(*_shoppingTrx, sops, cxrKeyPerLeg);

    if (_shoppingTrx->isAltDates())
      duration = ShoppingAltDateUtil::getDuration(*_shoppingTrx, sops);
  }

  for (std::size_t legIndex = 0; legIndex < sopsSize; ++legIndex)
  {
    TSE_ASSERT(_shoppingTrx->legs()[legIndex].sop()[sops[legIndex]].itin()->fareMarket().empty() ==
               false);

    if (UNLIKELY(fareUsagesPerLeg[legIndex].empty()))
      continue;

    const Itin* itin = _shoppingTrx->legs()[legIndex].sop()[sops[legIndex]].itin();

    FareUsage* fu = fareUsagesPerLeg[legIndex].front().second;
    const FareMarket* fuFareMarket = fu->paxTypeFare()->fareMarket();
    const uint16_t fuLegIndex = fuFareMarket->legIndex();
    const bool acrossStopOver = _shoppingTrx->legs()[fuLegIndex].stopOverLegFlag();

    if (!acrossStopOver)
    {
      Node nodeSID(_writer, "SID");
      nodeSID.convertAttr("Q14", legIndex);
      nodeSID.convertAttr("Q15", ShoppingUtil::findSopId(_trx, legIndex, sops[legIndex]));

      for (const OrderedFareUsage& ofu : fareUsagesPerLeg[legIndex])
      {
        FareUsage* fu = ofu.second;

        if (_shoppingTrx->isSumOfLocalsProcessingEnabled() ||
            _shoppingTrx->isIataFareSelectionApplicable())
        {
          fu->paxTypeFare()->setComponentValidationForCarrier(
              cxrKeyPerLeg[legIndex], _shoppingTrx->isAltDates(), duration);
        }
        formatBKK(*itin, legIndex, sops[legIndex], *fu);
      }
    }
    else
    {
      TSE_ASSERT(fareUsagesPerLeg[legIndex].size() == 1);

      if (_shoppingTrx->isSumOfLocalsProcessingEnabled())
        fu->paxTypeFare()->setComponentValidationForCarrier(
            cxrKeyPerLeg[fuLegIndex], _shoppingTrx->isAltDates(), duration);

      formatBKKAso(*itin, sops, *fuFareMarket, *fu, legIndex);
    }
  }
}

struct CompareOrderedFareUsages
{
  bool operator()(const XMLShoppingResponse::OrderedFareUsage& l,
                  const XMLShoppingResponse::OrderedFareUsage& r) const
  {
    return l.first < r.first;
  }
};

void
XMLShoppingResponse::collectFareUsagesPerLeg(const FarePath& farePath,
                                             const SopIdVec& sops,
                                             FareUsagesPerLeg& fareUsagesPerLeg) const
{
  fareUsagesPerLeg.resize(sops.size());

  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      TSE_ASSERT(!fu->travelSeg().empty());

      const uint16_t fuLegIndex = fu->paxTypeFare()->fareMarket()->legIndex();
      const bool acrossStopOver = _shoppingTrx->legs()[fuLegIndex].stopOverLegFlag();

      if (acrossStopOver)
      {
        for (std::size_t legIndex = 0, sopsSize = sops.size(); legIndex < sopsSize; ++legIndex)
        {
          const Itin* itin = _shoppingTrx->legs()[legIndex].sop()[sops[legIndex]].itin();
          TSE_ASSERT(!itin->travelSeg().empty());

          if (itin->travelSeg().front() == fu->travelSeg().front())
          {
            OrderedFareUsage ofu(0, fu);
            fareUsagesPerLeg[legIndex].push_back(ofu);
            break;
          }
        }
      }
      else
      {
        TSE_ASSERT(fuLegIndex < sops.size());
        const Itin* itin = _shoppingTrx->legs()[fuLegIndex].sop()[sops[fuLegIndex]].itin();
        const int fuSegmentOrder = itin->segmentOrder(fu->travelSeg().front());
        TSE_ASSERT(fuSegmentOrder >= 0);

        OrderedFareUsage ofu(fuSegmentOrder, fu);
        fareUsagesPerLeg[fuLegIndex].push_back(ofu);
      }
    }
  }

  for (std::vector<OrderedFareUsage>& fareUsagesForLeg : fareUsagesPerLeg)
  {
    std::sort(fareUsagesForLeg.begin(), fareUsagesForLeg.end(), CompareOrderedFareUsages());
  }
}

bool
XMLShoppingResponse::formatBKK(const Itin& itin,
                               uint16_t legId,
                               uint16_t sopId,
                               const FareUsage& fu)
{
  // match the right sop
  TSE_ASSERT(itin.travelSeg().empty() == false);
  TSE_ASSERT(fu.travelSeg().empty() == false);

  const ShoppingUtil::ExternalSopId extId = ShoppingUtil::createExternalSopId(legId, sopId);
  const uint32_t bitmapNumber = ShoppingUtil::getFlightBitIndex(*_shoppingTrx, extId);

  const PaxTypeFare::FlightBit& bitMap = fu.paxTypeFare()->flightBitmap()[bitmapNumber];

  if (bitMap._segmentStatus.empty())
  {
    for (TravelSeg* tvlSeg : fu.travelSeg())
    {
      if (tvlSeg->segmentType() == Arunk)
      {
        continue;
      }

      Node(_writer, "BKK")
          .attr("B30", tvlSeg->getBookingCode())
          .convertAttr("Q13", tvlSeg->bookedCabin().getCabinIndicator())
          .convertAttr("B31", tvlSeg->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
    }
  }
  else
  {
    // new element
    {
      const std::vector<TravelSeg*>& travelSeg = fu.travelSeg(); // lint !e530
      int16_t tvlItem = 0;
      int16_t tvlSegSize = travelSeg.size();

      for (const PaxTypeFare::SegmentStatus& segItem : bitMap._segmentStatus)
      {
        TSE_ASSERT(tvlItem < tvlSegSize);

        if (UNLIKELY(travelSeg[tvlItem]->segmentType() == Arunk))
        {
          continue;
        }

        Node nodeBKK(_writer, "BKK");

        if (segItem._bkgCodeReBook.empty())
        {
          nodeBKK.attr("B30", travelSeg[tvlItem]->getBookingCode())
              .convertAttr("Q13", travelSeg[tvlItem]->bookedCabin().getCabinIndicator())
              .convertAttr(
                  "B31", travelSeg[tvlItem]->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
        }
        else
        {
          if (travelSeg[tvlItem]->bookedCabin() > segItem._reBookCabin)
          {
            bool bookingCodeFound = false;

            for (ClassOfService* cos : travelSeg[tvlItem]->classOfService())
            {
              if (travelSeg[tvlItem]->bookedCabin() == cos->cabin())
              {
                nodeBKK.attr("B30", cos->bookingCode());
                bookingCodeFound = true;
                break;
              }
            }

            if (bookingCodeFound == false)
            {
              nodeBKK.attr("B30", "Y");
            }

            nodeBKK.convertAttr("Q13", travelSeg[tvlItem]->bookedCabin().getCabinIndicator())
                .convertAttr(
                    "B31",
                    travelSeg[tvlItem]->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
          }
          else
          {
            nodeBKK.attr("B30", segItem._bkgCodeReBook);
            nodeBKK.convertAttr("Q13", segItem._reBookCabin.getCabinIndicator())
                .convertAttr("B31", segItem._reBookCabin.getClassAlphaNum(_rbdByCabinVSinHouse));
          }
        }

        ++tvlItem;
      }
    }
  }

  return true;
}

// JIRA SHM-942: New function introduced to get the proper index for accessing FU->travelSeg
// for across stopover
bool
XMLShoppingResponse::formatBKKAso(const Itin& itin,
                                  const SopIdVec& sops,
                                  const FareMarket& fuFareMarket,
                                  const FareUsage& fu,
                                  std::size_t& outLastProcessedLeg)
{
  TSE_ASSERT(itin.travelSeg().empty() == false);
  TSE_ASSERT(fu.travelSeg().empty() == false);
  const TravelSeg* fmTravelSeg = itin.travelSeg().front();
  const TravelSeg* fuFmTravelSeg = fu.travelSeg().front();
  const uint16_t fuLegIndex = fuFareMarket.legIndex();

  if (fmTravelSeg == fuFmTravelSeg)
  {
    // build the vector of SOPs for this across stop over leg
    const IndexVector& jumped = _shoppingTrx->legs()[fuLegIndex].jumpedLegIndices();
    SopIdVec legSops;

    for (IndexVector::const_iterator i = jumped.begin(); i != jumped.end(); ++i)
    {
      const std::size_t index = std::size_t(*i);

      if (index == ASOLEG_SURFACE_SECTOR_ID)
      {
        continue;
      }

      TSE_ASSERT(index < sops.size());
      legSops.push_back(sops[index]);
    }

    // find the bitmap number for this set of SOPs
    uint32_t bitmapNumber =
        ShoppingUtil::getBitmapForASOLeg(const_cast<ShoppingTrx&>(*_shoppingTrx),
                                         fuLegIndex,
                                         fuFareMarket.governingCarrier(),
                                         legSops);
    std::size_t fuIndex = 0; // Index of FU items
    std::size_t legNumber = 0;

    for (IndexVector::const_iterator i = jumped.begin(); i != jumped.end(); ++i)
    {
      legNumber = *i;
      if (legNumber == ASOLEG_SURFACE_SECTOR_ID)
      {
        continue;
      }

      const int sop = sops[legNumber];
      Node nodeSID(_writer, "SID");
      nodeSID.convertAttr("Q14", legNumber)
          .convertAttr("Q15", ShoppingUtil::findSopId(_trx, legNumber, sop));
      const PaxTypeFare::FlightBit& bitMap = fu.paxTypeFare()->flightBitmap()[bitmapNumber];

      const std::vector<TravelSeg*>& travelSeg = fu.travelSeg(); // lint !e530
      TSE_ASSERT(!travelSeg.empty());

      if (bitMap._segmentStatus.empty())
      {
        for (; (fuIndex < travelSeg.size()) &&
                   (legNumber == static_cast<std::size_t>(travelSeg[fuIndex]->legId()));
             ++fuIndex)
        {
          if (travelSeg[fuIndex]->segmentType() == Arunk)
          {
            continue;
          }

          Node(_writer, "BKK")
              .attr("B30", travelSeg[fuIndex]->getBookingCode())
              .convertAttr("Q13", travelSeg[fuIndex]->bookedCabin().getCabinIndicator())
              .convertAttr(
                  "B31", travelSeg[fuIndex]->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
        }
      }
      else
      {
        for (; (fuIndex < travelSeg.size()) &&
                   (legNumber == static_cast<std::size_t>(travelSeg[fuIndex]->legId()));
             ++fuIndex)
        {
          Node nodeBKK(_writer, "BKK");

          // if the rebookCabin is in higher cabin than the requested cabin, return the requested
          // cabin
          if (travelSeg[fuIndex]->bookedCabin() > bitMap._segmentStatus[fuIndex]._reBookCabin)
          {
            bool bookingCodeFound = false;

            for (const auto classOfService : travelSeg[fuIndex]->classOfService())
            {
              if (travelSeg[fuIndex]->bookedCabin() == classOfService->cabin())
              {
                nodeBKK.attr("B30", classOfService->bookingCode());
                bookingCodeFound = true;
                break;
              }
            }

            if (bookingCodeFound == false)
            {
              nodeBKK.attr("B30", "Y");
            }

            nodeBKK.convertAttr("Q13", travelSeg[fuIndex]->bookedCabin().getCabinIndicator())
                .convertAttr(
                    "B31",
                    travelSeg[fuIndex]->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
          }
          else
          {
            if (bitMap._segmentStatus[fuIndex]._bkgCodeReBook.empty())
            {
              nodeBKK.attr("B30", travelSeg[fuIndex]->getBookingCode())
                  .convertAttr("Q13", travelSeg[fuIndex]->bookedCabin().getCabinIndicator())
                  .convertAttr(
                      "B31",
                      travelSeg[fuIndex]->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
            }
            else
            {
              nodeBKK.attr("B30", bitMap._segmentStatus[fuIndex]._bkgCodeReBook)
                  .convertAttr("Q13",
                               bitMap._segmentStatus[fuIndex]._reBookCabin.getCabinIndicator())
                  .convertAttr("B31",
                               bitMap._segmentStatus[fuIndex]._reBookCabin.getClassAlphaNum(
                                   _rbdByCabinVSinHouse));
            }
          }
        }
      }
    }

    outLastProcessedLeg = legNumber;
    return true;
  }

  return false;
}

void
XMLShoppingResponse::generatePSGAttr(Node& node, const FarePath& path)
{
  generatePSGAttr(
      node, path.paxType()->number(), path.paxType()->paxType(), path.getTotalNUCAmount());
}

void
XMLShoppingResponse::generatePSGAttr(Node& node,
                                     const uint16_t number,
                                     const PaxTypeCode& paxTypeCode,
                                     const MoneyAmount totalNuc)
{
  node.convertAttr("Q0U", number).convertAttr("B70", paxTypeCode).convertAttr("C5A", totalNuc);
}

void
XMLShoppingResponse::generateTOT(const MoneyAmount totalNUCAmount,
                                 const MoneyAmount totalNUCBaseFareAmount,
                                 const CurrencyCode& currency)
{
  MoneyAmount convertedAmount = 0.0;
  MoneyAmount convertedBaseFareAmount = 0.0;
  Money calculationCurrency(currency);

  if (LIKELY(_currencyConverter.convert(calculationCurrency,
                                        Money(totalNUCAmount, NUC),
                                        _trx,
                                        currency,
                                        convertedAmount,
                                        true) &&
             _currencyConverter.convert(calculationCurrency,
                                        Money(totalNUCBaseFareAmount, NUC),
                                        _trx,
                                        currency,
                                        convertedBaseFareAmount,
                                        true)))
  {
    Node(_writer, "TOT").attr("C40", currency).convertAttr("C5A", convertedAmount).convertAttr(
        "C67", convertedBaseFareAmount);
  }
}

// this value defines General Private Fare filed without Account Code
const std::string PILLOW = "@";
// this value defines General Private Fare filed via Cat 35
const std::string SLASH = "/";
// this value defines General Private Fare filed via Cat 35 ticketing ineligible
const std::string TICKETING_INELIGIBLE = "X";
// this value defines General Private Fare filed with Account Code
const std::string START = "*";

void
XMLShoppingResponse::generateITNIdAttribute(Node& nodeITN, const Itin* itin)
{
  if (LIKELY(itin->itinNum() != -1))
    nodeITN.convertAttr("NUM", itin->itinNum());
}

void
XMLShoppingResponse::generateITNFamilyAttribute(Node& nodeITN, const Itin& itin)
{
  if (LIKELY(itin.getItinFamily() != INVALID_INT_INDEX))
  {
    if (itin.isHeadOfFamily())
    {
      nodeITN.attr("FML", "M");
    }
    else
    {
      nodeITN.convertAttr("FML", itin.getItinFamily());
    }
  }
}

std::vector<std::string>
XMLShoppingResponse::generateITNAttributes(Node& nodeITN,
                                           const Itin* itin,
                                           /*const*/ FareCalcCollector* calc)
{
  // return validating carrier
  nodeITN.attr("B05", itin->validatingCarrier());

  generateSpanishDiscountIndicator(nodeITN, itin);

  // For BFA and IBF every GRI has its own values for these.
  // Aggregate value for the whole ITN makes no sense.
  if (LIKELY(!_trx.isBRAll() && !_trx.getRequest()->isBrandedFaresRequest()))
  {
    if (!calc->lastTicketDay().empty())
    {
      nodeITN.attr("D00", calc->lastTicketDay())
          .convertAttr("PBC", (calc->simultaneousResTkt() ? "T" : "F"))
          .attr("D14", calc->lastTicketDay())
          .attr("D16", calc->lastTicketDay());
    }
  }

  if (isMIPResponse() && _trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const FarePath* farePath = calc->passengerCalcTotals()[0]->farePath;

    if (farePath->exchangeReissue() != BLANK)
    {
      nodeITN.convertAttr("N27", farePath->exchangeReissue());
      BaseExchangeTrx& excTrx = static_cast<BaseExchangeTrx&>(_trx);

      if (farePath->exchangeReissue() == EXCHANGE)
      {
        nodeITN.attr("D95", excTrx.currentTicketingDT().dateToSqlString());
        if (excTrx.currentTicketingDT().historicalIncludesTime())
          nodeITN.attr("T95", excTrx.currentTicketingDT().timeToString(HHMM, ":"));
      }
      else if (farePath->exchangeReissue() == REISSUE)
      {
        if (excTrx.previousExchangeDT().isEmptyDate())
        {
          nodeITN.attr("D95", excTrx.originalTktIssueDT().dateToSqlString());
          if (excTrx.originalTktIssueDT().historicalIncludesTime())
            nodeITN.attr("T95", excTrx.originalTktIssueDT().timeToString(HHMM, ":"));
        }
        else
        {
          nodeITN.attr("D95", excTrx.previousExchangeDT().dateToSqlString());
          if (excTrx.previousExchangeDT().historicalIncludesTime())
            nodeITN.attr("T95", excTrx.previousExchangeDT().timeToString(HHMM, ":"));
        }
      }
    }
  }

  // WPNI Current Itinerary will set P0N = "T" if there is a class of service change
  if (_trx.billing()->actionCode() == "WPNI.C" || _trx.billing()->actionCode() == "WFR.C")
  {
    generateP0N(itin, nodeITN);
  }

  generateP0J(_trx, itin, nodeITN);

  if (isMIPResponse() && _trx.getRequest()->processVITAData())
  {
    if (!fallback::fallbackSkipVTAForGSA(&_trx))
    {
      if (!_trx.isValidatingCxrGsaApplicable())
      {
        generateVTA(_trx, itin, nodeITN);
      }
    }
    else
    {
      generateVTA(_trx, itin, nodeITN);
    }
  }

  if (LIKELY(isMIPResponse()))
  {
    generatePVTandFBR(itin, nodeITN);
    generateSTI(itin, nodeITN);
    generateCPL(itin, nodeITN);
  }

  if (calc->passengerCalcTotals().front()->dispSegmentFeeMsg())
  {
    nodeITN.attr("SFY", "T");
  }

  // set divide party

  if (fallback::fallbackDivideParty(&_trx))
  {
    if (calc->passengerCalcTotals().size() > 1)
      generateP35(*calc, nodeITN);
  }
  else
  {
    if (calc->passengerCalcTotals().size() > 1 && _trx.paxType().size() > 1)
      generateP35(*calc, nodeITN);
  }

  // populate private fare for PSG and ITN
  std::vector<std::string> privateIndPerPax;
  std::string privateInd = " ";
  generatePrivateInd(*itin, *calc, privateIndPerPax);

  for (const auto& pInd : privateIndPerPax)
  {
    if (privateInd == " ")
    {
      privateInd = pInd;
    }
    else if (pInd != " ")
    {
      if (privateInd != pInd)
      {
        privateInd = PILLOW;
      }
    }
  }

  if (privateInd != " ")
  {
    nodeITN.convertAttr("N1U", privateInd);
  }

  return privateIndPerPax;
}

void
XMLShoppingResponse::generateITNbody(
    Itin* itin,
    const std::string& tag,
    const ItinIdentificationFunction& generateItinIdentificationElements,
    XMLShoppingResponse::ItineraryTotals& itineraryTotals,
    const uint16_t ittIndex,
    const bool isCarnivalSolFlexfare)
{
  // if the Itin doesn't have any fare paths then it's not
  // valid -- don't return it. We may have to consider changing
  // this to send an error instead.
  if (itinIsValidSolution(itin) == false)
  {
    // There are no error messages per itin outside IBF
    if (!_trx.getRequest()->isBrandedFaresRequest())
      return;

    // Soldout messages are printed for options that are priced in context of a specific brand
    // Context shopping is an exception because ANY_BRAND is used if all legs are fixed
    // In that case we still want to get soldout status for each fixed brand.
    if (!ShoppingUtil::isAnyBrandReal(itin->brandCodes()) && !_trx.isContextShopping())
      return;

    Node nodeITN(_writer, tag);
    if (tag == "ITN")
    {
      generateITNIdAttribute(nodeITN, itin);
      for (uint16_t brandIndex = 0; brandIndex < _trx.validBrands().size(); ++brandIndex)
      {
        generateBrandError(itin, brandIndex);
      }
    }
  }
  FareCalcCollector* calc = FareCalcUtil::getFareCalcCollectorForItin(_trx, itin);

  if (nullptr == calc)
    return;

  Node nodeITN(_writer, tag);

  if (tag == "ITN")
  {
    generateITNIdAttribute(nodeITN, itin);

    if (LIKELY(isMIPResponse()))
      generateITNFamilyAttribute(nodeITN, *itin);
  }

  const std::vector<std::string>& privateIndPerPax = generateITNAttributes(nodeITN, itin, calc);
  generateItinIdentificationElements(itin);
  // get the fare  line from calc totals
  const std::vector<CalcTotals*>& calcTotals = calc->passengerCalcTotals();

  if (calcTotals.empty())
    return;

  // when changing the code below (series of ifs, except flexfare case, please update
  // Diag894Collector::calculateMipGriToPrint() accordingly
  bool isIbf = !_trx.validBrands().empty() && !_trx.isBrandsForTnShopping();
  uint16_t brandSize = 0;
  if (_trx.getRequest()->brandedFareEntry())
    brandSize = _trx.getRequest()->getBrandedFareSize();
  else if (isIbf)
    brandSize = _trx.validBrands().size();

  if (isMIPResponse() &&
      (_trx.billing()->actionCode() == "WFR.C" || _trx.billing()->actionCode() == "WFR") &&
      _trx.excTrxType() == PricingTrx::AR_EXC_TRX && !isIbf)
  {
    generateITNContent(*calc, itin, itineraryTotals);

    if (_trx.billing()->actionCode() == "WFR.C")
      prepareUnflownItinPriceInfo(); // generate CSO UFL part
  }
  else if ((brandSize == 1) && !isIbf)
    generateITNlowerBody(itin, calcTotals, calc, privateIndPerPax, 0, itineraryTotals);
  else if ((brandSize > 1) || isIbf)
  {
    for (uint16_t brandIndex = 0; brandIndex < brandSize; ++brandIndex)
    {
      if (itinIsValidBrandSolution(itin, brandIndex) == false)
      {
        if (!_trx.getRequest()->brandedFareEntry())
        {
          generateBrandError(itin, brandIndex);
        }
        continue;
      }
      Node nodeGRI(_writer, "GRI");
      if (_trx.getRequest()->brandedFareEntry())
      {
        nodeGRI.convertAttr(xml2::BrandCode, _trx.getRequest()->brandId(brandIndex));
        nodeGRI.convertAttr(xml2::ProgramCode, _trx.getRequest()->programId(brandIndex));
      }
      else
      {
        std::string brandCode = ShoppingUtil::getBrandCodeString(_trx, brandIndex);
        if (!brandCode.empty())
          nodeGRI.convertAttr(xml2::BrandCode, brandCode);
        else
          nodeGRI.convertAttr(xml2::IbfInternalPath,
                              ShoppingUtil::getFakeBrandString(_trx, brandIndex));

        generateGRIlastTicketingDay(nodeGRI, itin, calc, brandIndex);
      }
      ItineraryTotals brandItineraryTotals;
      generateITNlowerBody(
          itin, calcTotals, calc, privateIndPerPax, brandIndex, brandItineraryTotals);

      if (itineraryTotals < brandItineraryTotals)
        itineraryTotals = brandItineraryTotals;
    }
  }
  else if (_trx.isFlexFare())
  {
    if (isCarnivalSolFlexfare)
    {
      generateITNlowerBody(itin, calcTotals, calc, privateIndPerPax, ittIndex, itineraryTotals);
    }
    else
    {
      flexFares::GroupsData::const_iterator iterFlexFareGroup =
          _trx.getRequest()->getFlexFaresGroupsData().begin();
      flexFares::GroupsData::const_iterator iteratorEnd =
          _trx.getRequest()->getFlexFaresGroupsData().end();

      for (; iterFlexFareGroup != iteratorEnd; iterFlexFareGroup++)
      {
        if (!isFlexFareGroupValidForItin(itin, iterFlexFareGroup->first))
        {
          generateFlexFareError(iterFlexFareGroup->first);
          continue;
        }
        else
        {
          Node nodeGRI(_writer, "GRI");
          nodeGRI.convertAttr("Q17", iterFlexFareGroup->first);
          generateITNlowerBody(
              itin, calcTotals, calc, privateIndPerPax, iterFlexFareGroup->first, itineraryTotals);
        }
      }
    }
  }
  else if (_trx.isBRAll())
  {
    BrandsOptionsFilterForDisplay brandOptionFilter(_trx);
    BrandsOptionsFilterForDisplay::BrandingSpacesWithSoldoutVector filteredOptionsVector =
        brandOptionFilter.collectOptionsForDisplay(*itin);

    _legToDirectionMap.clear();
    for (const BrandsOptionsFilterForDisplay::BrandingOptionWithSoldout& option :
         filteredOptionsVector)
    {
      BrandsOptionsFilterForDisplay::BrandingOptionSpaceIndex spaceIndex = option.first;
      const BrandsOptionsFilterForDisplay::SoldoutStatus& soldout = option.second;

      Node nodeGRI(_writer, "GRI");
      if (soldout)
      {
        nodeGRI.convertAttr("SBL", ShoppingUtil::getIbfErrorCode(soldout.get()));
        generateBrandCombinationError(nodeGRI, itin, spaceIndex);
      }
      else
      {
        generateGRIlastTicketingDay(nodeGRI, itin, calc, spaceIndex);
        generateITNlowerBody(itin, calcTotals, calc, privateIndPerPax, spaceIndex, itineraryTotals);
      }
    }
  }
  else
    generateITNlowerBody(
        itin, calcTotals, calc, privateIndPerPax, INVALID_BRAND_INDEX, itineraryTotals);
}

void
XMLShoppingResponse::generateGRIlastTicketingDay(Node& nodeGRI,
                                                 const Itin* itin,
                                                 FareCalcCollector* calc,
                                                 const uint16_t brandIndex)
{
  // Let's use the first requested pax type, same as single-brand logic
  auto paxType =
      std::min_element(_trx.paxType().cbegin(), _trx.paxType().cend(), comparePaxTypeInputOrder);
  TSE_ASSERT(paxType != _trx.paxType().cend()); // Should never happen because we have pax :)

  auto totals = BrandingResponseUtils::findFarePathAndCalcTotalsForPaxTypeAndBrand(
                    *itin, *calc, *paxType, brandIndex, _trx).second;

  if (totals != nullptr && !totals->lastTicketDay.empty())
  {
    nodeGRI.attr("D00", totals->lastTicketDay);
    nodeGRI.convertAttr("PBC", (totals->simultaneousResTkt ? "T" : "F"));
  }
}

std::string
XMLShoppingResponse::getDirectionality(const FareUsage& fareUsage)
{
  std::string directionality;

  if (fareUsage.isInbound() && !fareUsage.dirChangeFromOutbound())
  {
    directionality = "TO";
  }
  else
  {
    directionality = "FR";
  }

  return directionality;
}

size_t
XMLShoppingResponse::getFareUsageId(const FareUsage* fu)
{
  return _fareUsageIds.insert(std::make_pair(fu, _fareUsageIds.size())).first->second;
}

std::string
XMLShoppingResponse::addFareBrandDetails(Node& nodeFDC,
                                         const PaxTypeFare& ptf,
                                         const BrandCode& brandCodeFromOuterSpace,
                                         Direction fareUsageDirection) const
{
  // If fare that is used is not valid for brandCombination it is used in, something is seriously
  // wrong:)
  TSE_ASSERT(ptf.isValidForBrand(_trx, &brandCodeFromOuterSpace));

  // In NO_BRAND (the cheapest scenario) we still want to display brands if they are defined for a
  // given fare
  const BrandCode fareBrandCode = ShoppingUtil::isThisBrandReal(brandCodeFromOuterSpace)
                                      ? brandCodeFromOuterSpace
                                      : ptf.getFirstValidBrand(_trx, fareUsageDirection);

  if (fareBrandCode.empty())
    return "";

  const unsigned int index = ptf.getValidBrandIndex(_trx, &fareBrandCode, fareUsageDirection);

  if (index == INVALID_BRAND_INDEX)
    return "";

  const QualifiedBrand& qb = _trx.brandProgramVec()[index];
  const std::string brandCodeUpperCase = boost::to_upper_copy(fareBrandCode);
  if (xform::formatBrandProgramData(nodeFDC, qb, brandCodeUpperCase))
  {
    // TODO(karol.jurek): remove this attribute (SC0)
    nodeFDC.attr(xml2::ProgramCode, boost::to_upper_copy(std::string(qb.first->programCode())));
  }
  else
  {
    LOG4CXX_ERROR(_logger, "Could not format brand program data");
    return "";
  }
  return brandCodeUpperCase;
}

void
XMLShoppingResponse::generateFDC(const FareUsage& fareUsage,
                                 CalcTotals& calcTotal,
                                 const Itin* itin,
                                 uint16_t* segmentOrder,
                                 uint16_t* index,
                                 PricingUnit& pu,
                                 bool firstSideTrip,
                                 bool* preAvl,
                                 uint16_t paxProcOrder,
                                 bool& minMaxCatFound,
                                 const boost::optional<FareCalc::SplitTaxInfo>& taxes,
                                 uint16_t& fcId)
{
  std::string B70 = fareUsage.paxTypeFare()->fcasPaxType();

  if (B70.empty())
  {
    B70 = "ADT";
  }

  std::string globalDirStr;
  globalDirectionToStr(globalDirStr, fareUsage.paxTypeFare()->fareMarket()->getGlobalDirection());
  FareBreakPointInfo& breakPoint = calcTotal.getFareBreakPointInfo(&fareUsage);
  const tse::CurrencyNoDec nucNoDec = 2;
  const std::vector<TravelSeg*>& travelSeg = fareUsage.travelSeg(); // lint !e530
  Node nodeFDC(_writer, "FDC");

  if (!fallback::fallbackAMCPhase2(&_trx) && !fallback::fallbackAMC2ShoppingChange(&_trx) &&
      fcId > 0)
    nodeFDC.convertAttr("Q6D", fcId);

  nodeFDC.convertAttr("Q20", getFareUsageId(&fareUsage));
  nodeFDC.attr("A01", travelSeg.front()->origAirport())
      .attr("A02", travelSeg.back()->destAirport())
      .attr("A11", travelSeg.front()->boardMultiCity())
      .attr("A12", travelSeg.back()->offMultiCity())
      .attr("B00", fareUsage.paxTypeFare()->fareMarket()->governingCarrier())
      .attr("B50", breakPoint.fareBasisCode)
      .attr("B70", B70)
      .convertAttr("C40", fareUsage.paxTypeFare()->currency())
      .attr("C50",
            formatAmount(breakPoint.fareAmount,
                         calcTotal.useNUC ? nucNoDec : fareUsage.paxTypeFare()->numDecimal()))
      .attr("C51",
            formatAmount(fareUsage.paxTypeFare()->originalFareAmount(),
                         fareUsage.paxTypeFare()->numDecimal()))
      .convertAttr("A60", globalDirStr)
      .convertAttr("S70", getDirectionality(fareUsage))
      .convertAttr("P0F", fareUsage.paxTypeFare()->isWebFare() ? 'T' : 'F');

  if (!fallback::dffOaFareCreation(&_trx))
  {
    addVendorCodeToFDCNode(nodeFDC, fareUsage.paxTypeFare()->vendor());
  }

  if (!fallback::fallbackFRRProcessingRetailerCode(&_trx))
  {
    std::string fareRetailerCode = AdjustedSellingUtil::getFareRetailerCodeForNet(fareUsage);
    if (!fareRetailerCode.empty())
      nodeFDC.attr(xml2::FareRetailerCodeNet, fareRetailerCode);

    fareRetailerCode = AdjustedSellingUtil::getFareRetailerCodeForAdjusted(fareUsage);
    if (!fareRetailerCode.empty())
      nodeFDC.attr(xml2::FareRetailerCodeAdjusted, fareRetailerCode);
  }

  std::string ticketDesignator;
  std::string::iterator slash =
      std::find(breakPoint.fareBasisCode.begin(), breakPoint.fareBasisCode.end(), '/');

  if (slash != breakPoint.fareBasisCode.end())
  {
    nodeFDC.attr("BE0", std::string(slash + 1, breakPoint.fareBasisCode.end()));
  }

  if (UNLIKELY(_trx.awardRequest()))
  {
    std::stringstream s;
    s << fareUsage.paxTypeFare()->mileage();
    nodeFDC.attr("C5L", s.str());
  }

  if (UNLIKELY(_trx.getRequest()->isBrandedFaresRequest()))
  {
    // GRI level brand code
    std::string brandCode = ShoppingUtil::getFarePathBrandCode(calcTotal.farePath);

    if (_trx.isContextShopping())
    {
      // Display brand from fare component (as it may be different than on
      // GRI level if on fixed leg)
      // Additionally if all legs are fixed then fare path's brand may be NO_BRAND
      if (skipper::TNBrandsFunctions::isAnySegmentOnFixedLeg(
              fareUsage.paxTypeFare()->fareMarket()->travelSeg(), _trx.getFixedLegs()))
        brandCode = fareUsage.paxTypeFare()->getFirstValidBrand(_trx, fareUsage.getFareUsageDirection());
    }

    if (brandCode == ANY_BRAND_LEG_PARITY)
    {
      // In Cheapest with Leg Parity path there's no GRI level brand at all
      brandCode = fareUsage.getBrandCode();
    }
    // CBS is excluded in ANY_BRAND_LEG_PARITY and in Catch All Bucket
    else if (_trx.getRequest()->isChangeSoldoutBrand() && !brandCode.empty())
    {
      if (!_trx.isContextShopping() ||
          !skipper::TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(
              fareUsage.paxTypeFare()->fareMarket()->travelSeg(), _trx.getFixedLegs()))
      {
        // getStatusForBrand(brandCode, Direction::BOTHWAYS) should be used for all branded
        // requests except BRALL

        IbfErrorMessage internalSoldoutStatus =
           fareUsage.paxTypeFare()->fareMarket()->getStatusForBrand(brandCode, Direction::BOTHWAYS);

        // in new CBS we also may get status NO_FARES_FILED that is also subject to CBS
        IbfErrorMessage ibfErrorMessage = _trx.getRequest()->isUseCbsForNoFares() ?
           IbfAvailabilityTools::translateForOutput_newCbs(internalSoldoutStatus):
           IbfAvailabilityTools::translateForOutput(internalSoldoutStatus);

        if (ibfErrorMessage != IbfErrorMessage::IBF_EM_NO_FARE_FOUND)
        {
          // In "change brand for soldout", for legs that are sold out we print
          // brandCode that was used instead of the main brand.
          brandCode = fareUsage.paxTypeFare()->getFirstValidBrand(_trx, fareUsage.getFareUsageDirection());
          // Sold out reason is also printed. Even if new status NO_FARES_FILED is used
          // we still return "F" as a reason.
          ibfErrorMessage = IbfAvailabilityTools::translateForOutput(ibfErrorMessage);

          nodeFDC.convertAttr("SBL", ShoppingUtil::getIbfErrorCode(ibfErrorMessage));
        }
      }
    }

    if (!brandCode.empty())
      if (_trx.isNotExchangeTrx() || !fareUsage.paxTypeFare()->isFromFlownOrNotShoppedFM())
      {
        const unsigned int index =
          fareUsage.paxTypeFare()->getValidBrandIndex(_trx, &brandCode, fareUsage.getFareUsageDirection());

        if (index != INVALID_BRAND_INDEX)
        {
          const QualifiedBrand& qb = _trx.brandProgramVec().at(index);
          if (xform::formatBrandProgramData(nodeFDC, qb, brandCode))
          {
            // TODO(karol.jurek): remove this attribute (SC0)
            nodeFDC.attr(
                xml2::ProgramCode,
                ShoppingUtil::getFareBrandProgram(_trx, brandCode, fareUsage.paxTypeFare(),
                                                  fareUsage.getFareUsageDirection()));
          }
          else
          {
            LOG4CXX_ERROR(_logger, "Could not format brand program data");
          }
        }
      }
  }

  if (pu.puType() == PricingUnit::Type::ONEWAY)
    nodeFDC.convertAttr("P04", 'T');
  else
    nodeFDC.convertAttr("P04", 'F');

  if (pu.puType() == PricingUnit::Type::ROUNDTRIP || pu.puType() == PricingUnit::Type::CIRCLETRIP ||
      pu.puType() == PricingUnit::Type::OPENJAW)
    nodeFDC.convertAttr("P05", 'T');
  else
    nodeFDC.convertAttr("P05", 'F');

  //--- Add type of fare public or negotiate
  //    nodeFDC.convertAttr ("N0K", (fareUsage.paxTypeFare()->isNegotiated() ? 'N' : 'P'))
  nodeFDC.attr("N0K", (fareUsage.paxTypeFare()->isNegotiated() ? "N" : "P"))
      .convertAttr("D00", (travelSeg.front()->departureDT().dateToSqlString()));
  // populate private fare type indicator
  std::string privateFare = generateN1U(*fareUsage.paxTypeFare());

  if (privateFare != " ")
  {
    nodeFDC.convertAttr("N1U", privateFare);
  }

  if ((fareUsage.paxTypeFare() != nullptr && fareUsage.paxTypeFare()->matchedCorpID()) ||
      (fareUsage.paxTypeFare()->isFareByRule() &&
       (fareUsage.paxTypeFare()->getFbrRuleData() != nullptr &&
        fareUsage.paxTypeFare()->getFbrRuleData()->fbrApp() != nullptr &&
        fareUsage.paxTypeFare()->getFbrRuleData()->fbrApp()->accountCode().empty() == false)))
  {
    nodeFDC.convertAttr("PBU", 'T');
  }

  const PaxTypeFare* ptf = fareUsage.paxTypeFare();
  std::string accountCode;

  if (ptf->isFareByRule())
  {
    accountCode = ptf->fbrApp().accountCode().c_str();
  }

  if (accountCode.empty() && ptf->matchedCorpID())
  {
    accountCode = ptf->matchedAccCode();
  }

  if (!accountCode.empty())
  {
    nodeFDC.attr("S11", accountCode);
  }

  const int16_t firstFuSegOrder = itin->segmentOrder(travelSeg.front());
  std::string notValidDateStr = calcTotal.tvlSegNVB[firstFuSegOrder].dateToSqlString();

  if (notValidDateStr != "N/A")
  {
    nodeFDC.attr("D06", notValidDateStr); // D06      Not valid  date
  }

  notValidDateStr = calcTotal.tvlSegNVA[firstFuSegOrder].dateToSqlString();

  if (LIKELY(notValidDateStr != "N/A"))
  {
    nodeFDC.attr("D05", notValidDateStr); // D05      Not valid after date
  }

  //--- Add mileage information
  bool mileageInd =
      !(fareUsage.paxTypeFare()->isRouting() || fareUsage.paxTypeFare()->isPSRApplied());

  if (mileageInd && fareUsage.travelSeg().size() > 1 &&
      itin->geoTravelType() != GeoTravelType::Domestic)
  {
    nodeFDC.attr("PAY", "T");
    nodeFDC.convertAttr("Q48", fareUsage.paxTypeFare()->mileageSurchargePctg());
  }

  MinFarePlusUp::const_iterator pupIterEnd = fareUsage.minFarePlusUp().end();
  MinFarePlusUp::const_iterator pupIter = fareUsage.minFarePlusUp().find(HIP);

  if (pupIter != pupIterEnd)
  {
    const MinFarePlusUpItem& plusUp = *(pupIter->second);
    nodeFDC.attr("A13", plusUp.boardPoint) // HIPOrigCity
        .attr("A14", plusUp.offPoint); // HIPDestCity

    if (!plusUp.constructPoint.empty())
    {
      nodeFDC.attr("A18", plusUp.constructPoint); // ConstructedHIPCity
    }
  }

  generateQ18(nodeFDC, paxProcOrder, fareUsage);

  // Add brand info on FareComponent if BRall is set
  std::string brandCode = "";
  if (_trx.isBrandsForTnShopping())
  {
    BrandCode brandCodeFromOuterSpace = NO_BRAND;
    // in multiple brand mode we need to know brand context we're in
    // in case a fare is valid for more than one brand we have to display one actually used.
    if (_trx.isBRAll())
    {
      const uint16_t brandIndex = calcTotal.farePath->brandIndex();
      const uint16_t segmentIndex = firstFuSegOrder - 1;
      const CarrierCode& carrierCode = ptf->fareMarket()->governingCarrier();

      const skipper::ItinBranding& itinBranding = itin->getItinBranding();

      Direction direction = Direction::BOTHWAYS;
      if (BrandingUtil::isDirectionalityToBeUsed(_trx))
        direction = ptf->getDirection();
      brandCodeFromOuterSpace =
        itinBranding.getBrandCode(
          brandIndex, segmentIndex, carrierCode, direction, fareUsage.getFareUsageDirection());
    }

    brandCode = addFareBrandDetails(nodeFDC, *ptf, brandCodeFromOuterSpace, fareUsage.getFareUsageDirection());
  }

  size_t tvlItem = 0;
  const size_t tvlSegSize = travelSeg.size();
  std::vector<PaxTypeFare::SegmentStatus>::const_iterator segStatusIter =
      fareUsage.segmentStatus().begin();
  std::vector<PaxTypeFare::SegmentStatus>::const_iterator segStatusEnd =
      fareUsage.segmentStatus().end();

  std::vector<TravelSeg*>::const_iterator tvlSegI = fareUsage.travelSeg().begin();

  for (; tvlSegI != fareUsage.travelSeg().end(); ++tvlSegI, ++tvlItem, ++segStatusIter)
  {
    if (itin->segmentOrder(*tvlSegI) == *segmentOrder)
    {
      break;
    }
  }

  int16_t segmentOrderTemp = *segmentOrder;

  for (; (tvlItem < tvlSegSize) && (segStatusIter != segStatusEnd);
       ++segStatusIter, ++tvlItem, ++segmentOrderTemp, tvlSegI++)
  {
    BookingCode bookingCode = BLANK_CODE; // for seat remain

    // do not format
    if (itin->segmentOrder(*tvlSegI) != segmentOrderTemp)
    {
      break;
    }

    TravelSeg* segment = travelSeg[tvlItem];
    const int16_t segOrder = firstFuSegOrder + tvlItem;

    Node nodeBKC(_writer, "BKC");
    bool diffFound = false;

    if (segment->segmentType() == Arunk)
    {
      continue;
    }

    diffFound = isDifferentialFound(fareUsage, nodeBKC, *segment, bookingCode, breakPoint);

    if (diffFound)
    {
      // find the seat remainder
      if (bookingCode != BLANK_CODE)
      {
        std::size_t seatRemain =
            numberOfSeatRemain(*calcTotal.farePath, bookingCode, *segment, segOrder);

        if (seatRemain == 0)
        {
          seatRemain = 1;
        }
        nodeBKC.convertAttr("Q3S", seatRemain);
      }

      continue;
    }

    bookingCode = calculateBookingCode(*segStatusIter, segment);
    const FareMarket* fm = fareUsage.paxTypeFare()->fareMarket();

    std::vector<ClassOfService*>* cosVec = nullptr;
    if (tvlItem < fm->classOfServiceVec().size())
    {
      cosVec = fm->classOfServiceVec()[tvlItem];
    }

    const CabinType& cabin = calculateCabin(*segStatusIter, segment, cosVec);
    nodeBKC.attr("B30", bookingCode).convertAttr("Q13", cabin.getCabinIndicator()).convertAttr(
        "B31", cabin.getClassAlphaNum(_rbdByCabinVSinHouse));
    nodeBKC.attr("B52", breakPoint.fareBasisCode);

    // find the seat remainder
    if (LIKELY(bookingCode != BLANK_CODE))
    {
      std::size_t seatRemain =
          numberOfSeatRemain(*calcTotal.farePath, bookingCode, *segment, segOrder);

      if (seatRemain == 0)
      {
        seatRemain = 1;
      }
      nodeBKC.convertAttr("Q3S", seatRemain);
    }

    if (_trx.isBrandsForTnShopping() && !brandCode.empty())
      nodeBKC.attr(xml2::BrandCode, brandCode);
  }

  if (LIKELY(false == _trx.snapRequest()))
  {
    // Add arunk at the end
    for (const auto travelSeg : itin->travelSeg())
    {
      if (itin->segmentOrder(travelSeg) != segmentOrderTemp)
      {
        continue;
      }

      if (travelSeg->segmentType() == Arunk)
      {
        Node nodeBKC(_writer, "BKC");
        break;
      }
    }
  }

  // SURCHARGES
  tvlSegI = fareUsage.travelSeg().begin();

  for (; tvlSegI != fareUsage.travelSeg().end(); ++tvlSegI)
  {
    if (itin->segmentOrder(*tvlSegI) == *segmentOrder)
      break;
  }

  int16_t segmentOrderTmp = *segmentOrder;

  for (; tvlSegI != fareUsage.travelSeg().end(); ++segmentOrderTmp, ++tvlSegI)
  {
    if (itin->segmentOrder(*tvlSegI) != segmentOrderTmp)
      break;

    prepareSurcharges(**tvlSegI, calcTotal);
  }

  // format availability type nodeFAT
  tvlItem = 0;
  bool previousAvail = true;

  tvlSegI = fareUsage.travelSeg().begin();
  for (std::vector<PaxTypeFare::SegmentStatus>::const_iterator
           i = fareUsage.segmentStatus().begin();
       i != fareUsage.segmentStatus().end(), tvlSegI != fareUsage.travelSeg().end();
       ++i, ++tvlSegI)
  {
    Node nodeFAT(_writer, "FAT");
    nodeFAT.convertAttr("PAW",
                        i->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK) ? 'T' : 'F');
    TSE_ASSERT(tvlItem < tvlSegSize);

    if (travelSeg[tvlItem]->carrierPref() != nullptr)
    {
      const AirSeg& airSegJourney = *(dynamic_cast<AirSeg*>(travelSeg[tvlItem]));

      if (airSegJourney.flowJourneyCarrier())
        nodeFAT.convertAttr("N1L", 'F');
      else if (airSegJourney.localJourneyCarrier())
        nodeFAT.convertAttr("N1L", 'L');
    }

    // set Q0D to 1 if segment is a thru avail and 2 if segment is a solo
    if ((previousAvail) && (i->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK)))
    {
      nodeFAT.convertAttr("Q0D", '2');
    }
    else
    {
      nodeFAT.convertAttr("Q0D", '1');
    }

    previousAvail = ((*tvlSegI)->segmentType() != Arunk) &&
                    i->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK);

    tvlItem++;

    if (tvlItem == tvlSegSize)
    {
      nodeFAT.convertAttr("P2F", 'T'); // fare break point = true
    }
  }

  generateFDS(fareUsage, calcTotal, *itin, segmentOrder, index, pu, firstSideTrip, preAvl);
  Money constructionCurrency(itin->calculationCurrency());
  constructionCurrency.setCode(calcTotal.farePath->calculationCurrency());
  prepareDifferential(fareUsage, constructionCurrency.noDec(_trx.ticketingDate()));

  if (!fallback::fallbackFRRProcessingRetailerCode(&_trx))
  {
    prepareHPUForNet(fareUsage);

    prepareHPUForAdjusted(fareUsage);
  }

  prepareFareUsagePlusUps(fareUsage, constructionCurrency.noDec(_trx.ticketingDate()));

  prepareRuleCategoryIndicator(fareUsage, calcTotal.equivNoDec, minMaxCatFound);

  if (UNLIKELY(taxes))
  {
    generateTADForSplittedTaxes(calcTotal, *taxes);
  }
}

void
XMLShoppingResponse::addVendorCodeToFDCNode(Node& nodeFDC, const VendorCode& vendorCode)
{
  nodeFDC.attr("S37", vendorCode);
}

const CabinType&
XMLShoppingResponse::calculateCabin(const PaxTypeFare::SegmentStatus& segStatus,
                                    const TravelSeg* travelSeg,
                                    const std::vector<ClassOfService*>* cosVec)
{
  if (segStatus._bkgCodeReBook.empty() || !segStatus._reBookCabin.isValidCabin())
  {
    if ((_trx.billing()->actionCode() == "WPNI.C" || _trx.billing()->actionCode() == "WFR.C") &&
        cosVec)
    {
      for (const auto classOfService : *cosVec)
      {
        if (travelSeg->getBookingCode() == classOfService->bookingCode())
        {
          return classOfService->cabin();
        }
      }
    }

    return travelSeg->bookedCabin();
  }

  return segStatus._reBookCabin;
}

const CabinType&
XMLShoppingResponse::calculateCabin(const PaxTypeFare::SegmentStatus& segStatus,
                                    const TravelSeg* travelSeg)
{
  if (segStatus._bkgCodeReBook.empty() || !segStatus._reBookCabin.isValidCabin())
  {
    return travelSeg->bookedCabin();
  }

  return segStatus._reBookCabin;
}

BookingCode
XMLShoppingResponse::calculateBookingCode(const PaxTypeFare::SegmentStatus& segStatus,
                                          const TravelSeg* travelSeg)
{
  if (segStatus._bkgCodeReBook.empty() || !segStatus._reBookCabin.isValidCabin())
  {
    return travelSeg->getBookingCode();
  }
  return segStatus._bkgCodeReBook;
}

XMLShoppingResponse::Node&
XMLShoppingResponse::generateStopoverSurcharge(
    const FareUsage::StopoverSurchargeMultiMap& stopoverSurcharges,
    const TravelSeg* const travelSeg,
    Node& node)
{
  TSE_ASSERT(travelSeg);

  const auto stopoverSurchargeItem = stopoverSurcharges.find(travelSeg);
  if (stopoverSurchargeItem == stopoverSurcharges.end())
    return node;

  const auto stopOverSurcharge = stopoverSurchargeItem->second;
  TSE_ASSERT(stopOverSurcharge);

  if (stopOverSurcharge->amount() < EPSILON)
    return node;

  node.attr("SOS", formatAmount(stopOverSurcharge->amount(), stopOverSurcharge->noDecimals()))
      .attr("SOC", stopOverSurcharge->currencyCode());

  return node;
}

void
XMLShoppingResponse::generateFDS(const FareUsage& fareUsage,
                                 CalcTotals& calcTotal,
                                 const Itin& itin,
                                 uint16_t* segmentOrder,
                                 uint16_t* index,
                                 PricingUnit& pu,
                                 bool firstSideTrip,
                                 bool* preAvl)
{
  if (UNLIKELY(fareUsage.travelSeg().empty()))
  {
    return;
  }

  std::vector<TravelSeg*>::const_iterator travelSegI = fareUsage.travelSeg().begin();

  for (; travelSegI != fareUsage.travelSeg().end(); ++travelSegI)
  {
    if (itin.segmentOrder(*travelSegI) == *segmentOrder)
    {
      break;
    }
  }

  bool fuSideTrip = false;

  // check if fu has a sideTrip and it is the first part then set the P2N for that segment
  if ((fareUsage.hasSideTrip()) && (travelSegI == fareUsage.travelSeg().begin()))
  {
    fuSideTrip = true;
  }

  for (; travelSegI != fareUsage.travelSeg().end(); ++travelSegI, ++(*segmentOrder), ++(*index))
  {
    TSE_ASSERT(*index < itin.travelSeg().size());

    if (itin.segmentOrder(*travelSegI) != *segmentOrder)
    {
      break;
    }

    Node nodeFDS(_writer, "FDS");
    nodeFDS.attr("A01", (*travelSegI)->origAirport())
        .attr("A02", (*travelSegI)->destAirport())
        .attr("A11", (*travelSegI)->boardMultiCity())
        .attr("A12", (*travelSegI)->offMultiCity());
    const FareUsage* currentFareUsage = calcTotal.getFareUsage(*travelSegI);

    if (LIKELY(currentFareUsage != nullptr))
    {
      if ((calcTotal.extraMileageTravelSegs.count(*travelSegI) != 0) ||
          (calcTotal.extraMileageFareUsages.count(currentFareUsage) != 0))
      {
        nodeFDS.convertAttr("P2I", 'T');
      }
    }
    else if ((*travelSegI)->segmentType() == Arunk)
    {
      nodeFDS.convertAttr("S09", 'T');
    }

    bool isStopOver = false;

    std::vector<bool> stopOvers =
        TravelSegUtil::calculateStopOvers(itin.travelSeg(), itin.geoTravelType());
    int16_t segIndex = itin.segmentOrder(*travelSegI);

    if (UNLIKELY(segIndex == -1))
    {
      isStopOver = (*travelSegI)->stopOver();
    }
    else
    {
      isStopOver = stopOvers[segIndex - 1];
    }

    if (isStopOver)
    {
      nodeFDS.convertAttr("P2M", 'T');
      generateStopoverSurcharge(fareUsage.stopoverSurcharges(), *travelSegI, nodeFDS);
    }
    else
    {
      nodeFDS.convertAttr("P2H", 'T');
    }

    if ((*travelSegI)->segmentType() == Arunk)
    {
      nodeFDS.convertAttr("S10", 'T');
    }
    else
    {
      // format return booking code
      nodeFDS.convertAttr("PAW",
                          calcTotal.bkgCodeSegStatus[*index].isSet(PaxTypeFare::BKSS_AVAIL_BREAK)
                              ? 'T'
                              : 'F')
          .convertAttr("P0F", fareUsage.paxTypeFare()->isWebFare() ? 'T' : 'F');

      if ((*preAvl) && (calcTotal.bkgCodeSegStatus[*index].isSet(PaxTypeFare::BKSS_AVAIL_BREAK)))
      {
        nodeFDS.convertAttr("Q0D", '2');
      }
      else
      {
        nodeFDS.convertAttr("Q0D", '1');
      }

      *preAvl = calcTotal.bkgCodeSegStatus[*index].isSet(PaxTypeFare::BKSS_AVAIL_BREAK);
      std::vector<TravelSeg*>::const_iterator fuTravelSeg = std::find(
          currentFareUsage->travelSeg().begin(), currentFareUsage->travelSeg().end(), *travelSegI);

      if (UNLIKELY(fuTravelSeg == currentFareUsage->travelSeg().end()))
      {
        nodeFDS.attr("B30", (*travelSegI)->getBookingCode())
            .convertAttr("Q13", (*travelSegI)->bookedCabin().getCabinIndicator())
            .convertAttr("B31",
                         (*travelSegI)->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
      }
      else
      {
        PaxTypeFare::SegmentStatus* diffSegStatus =
            getDiffSegStatus(*currentFareUsage, **fuTravelSeg);

        if (diffSegStatus)
        {
          nodeFDS.attr("B30", diffSegStatus->_bkgCodeReBook)
              .convertAttr("Q13", diffSegStatus->_reBookCabin.getCabinIndicator())
              .convertAttr("B31",
                           diffSegStatus->_reBookCabin.getClassAlphaNum(_rbdByCabinVSinHouse));
        }
        else
        {
          size_t tvlSegIndex = static_cast<size_t>(
              std::distance(currentFareUsage->travelSeg().begin(), fuTravelSeg));
          const PaxTypeFare::SegmentStatus& segStatus =
              currentFareUsage->segmentStatus()[tvlSegIndex];
          BookingCode bookingCode = calculateBookingCode(segStatus, *travelSegI);
          const FareMarket* fm = currentFareUsage->paxTypeFare()->fareMarket();

          std::vector<ClassOfService*>* cosVec = nullptr;
          if (tvlSegIndex < fm->classOfServiceVec().size())
          {
            cosVec = fm->classOfServiceVec()[tvlSegIndex];
          }

          const CabinType& cabin = calculateCabin(segStatus, *travelSegI, cosVec);
          nodeFDS.attr("B30", bookingCode)
              .convertAttr("Q13", cabin.getCabinIndicator())
              .convertAttr("B31", cabin.getClassAlphaNum(_rbdByCabinVSinHouse));
        }
      }

      // check sideTrip
      if ((fuSideTrip) && ((*travelSegI) != fareUsage.travelSeg().back()) &&
          (itin.segmentOrder(*(travelSegI + 1)) != ((*segmentOrder) + 1)))
      {
        nodeFDS.convertAttr("P2N", 'T');
      }

      if (pu.isSideTripPU())
      {
        nodeFDS.convertAttr("P2N", 'T'); // P2N     Side trip indicator

        if ((firstSideTrip) && (fareUsage.travelSeg().front() == *travelSegI))
        {
          nodeFDS.convertAttr("S07", 'T'); // S07     segSideTripBegin
        }

        if ((!firstSideTrip) && (fareUsage.travelSeg().back() == *travelSegI))
        {
          nodeFDS.convertAttr("S08", 'T'); // S08     segSideTripEnd
        }
      }

      if (*travelSegI == fareUsage.travelSeg().back())
      {
        nodeFDS.convertAttr("P2F", 'T'); // fare break point = true
      }
    }
  }

  if (LIKELY(false == _trx.snapRequest()))
  {
    // add ARUNK to the end
    for (const auto travelSeg : itin.travelSeg())
    {
      if (itin.segmentOrder(travelSeg) != *segmentOrder)
      {
        continue;
      }

      if (travelSeg->segmentType() == Arunk)
      {
        Node nodeFDS(_writer, "FDS");
        nodeFDS.attr("A01", travelSeg->origAirport())
            .attr("A02", travelSeg->destAirport())
            .attr("A11", travelSeg->boardMultiCity())
            .attr("A12", travelSeg->offMultiCity())
            .convertAttr("P2M", 'T'); // stopover segment

        generateStopoverSurcharge(fareUsage.stopoverSurcharges(), travelSeg, nodeFDS)
            .convertAttr("S09", 'T')
            .convertAttr("S10", 'T'); // pure surface
        ++(*segmentOrder);
        ++(*index);
        return;
      }
    }
  }

  return;
}

template <typename TaxRecords>
void
XMLShoppingResponse::generateTaxBreakDownItem(const TaxItem& taxItem,
                                              std::string& taxCodes,
                                              const TaxRecords& taxRecords,
                                              CurrencyNoDec taxNoDec)
{
  // for exempt all taxes and fees (WPNI'TN or JR....../BET-ITL/TN
  // used taxrecord instead of tax item

  if (taxItem.failCode() || taxItem.taxAmount() < 0.01)
  {
    return;
  }

  std::string taxCode = taxItem.taxCode();
  std::string::size_type pos =
      _trx.snapRequest() ? taxCodes.find(getSnapKey(taxItem)) : taxCodes.find(taxCode);

  if (UNLIKELY(pos != std::string::npos))
  {
    return;
  }

  MoneyAmount moneyAmount = taxItem.taxAmount();

  if (UNLIKELY(taxItem.multioccconvrndInd() == YES))
  {
    taxCodes += _trx.snapRequest() ? getSnapKey(taxItem) : taxCode;

    for (const TaxRecord* taxRecord : taxRecords)
    {
      if (_trx.snapRequest())
      {
        if (!((taxRecord->carrierCode() == taxItem.carrierCode()) &&
              (taxRecord->legId() == taxItem.legId())))
        {
          continue;
        }
      }

      if (taxRecord->taxCode() == taxCode)
      {
        moneyAmount = taxRecord->getTaxAmount();
        break;
      }
    }
  }

  MoneyAmount amountPublished = taxItem.taxAmt();
  Money target(taxItem.taxCur());
  CurrencyNoDec amountPublishedNoDec = target.noDec(_trx.ticketingDate());

  if (taxItem.taxType() == Tax::PERCENTAGE)
  {
    Money source(taxItem.taxAmount(), taxItem.paymentCurrency());
    TaxUtil::convertTaxCurrency(_trx, source, target);
    amountPublished = target.value();
  }

  Node nodeTax(_writer, "TAX");
  nodeTax.attr("BC0", taxItem.taxCode())
      .attr("C6B", formatAmount(moneyAmount, taxItem.paymentCurrencyNoDec()))
      .attr("S04", taxItem.taxDescription())
      .attr("S05", taxItem.taxLocalBoard())
      .attr("C41", taxItem.taxCur())
      .attr("C6A", formatAmount(amountPublished, amountPublishedNoDec));

  if (UNLIKELY(_trx.snapRequest()))
  {
    nodeTax.attr("B00", taxItem.carrierCode());
  }

  if (!taxItem.nation().empty())
  {
    nodeTax.attr("A40", taxItem.nation());
  }

  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX))
  {
    nodeTax.convertAttr("PXF", taxItem.reissueTaxInfo().reissueRestrictionApply ? 'T' : 'F')
        .convertAttr("PXG", taxItem.reissueTaxInfo().taxApplyToReissue ? 'T' : 'F')
        .convertAttr("PXH", taxItem.reissueTaxInfo().reissueTaxRefundable ? 'T' : 'F');

    if (taxItem.reissueTaxInfo().reissueTaxAmount != 0)
    {
      nodeTax.attr("C79", taxItem.reissueTaxInfo().reissueTaxCurrency)
          .attr("C80", formatAmount(taxItem.reissueTaxInfo().reissueTaxAmount, taxNoDec));
    }

    if (taxItem.partialTax() || taxItem.mixedTax())
    {
      nodeTax.convertAttr("A05", 'M');
    }
    else
    {
      nodeTax.convertAttr("A05", taxItem.taxType());
    }

    // Min max tax info
    if (taxItem.minTax() > 0.0)
    {
      nodeTax.attr("C6D", formatAmount(taxItem.minTax(), taxItem.taxCurNodec()));
    }

    if (taxItem.maxTax() > 0.0)
    {
      nodeTax.attr("C6E", formatAmount(taxItem.maxTax(), taxItem.taxCurNodec()));
    }

    if (taxItem.minTax() > 0.0 || taxItem.maxTax() > 0.0)
    {
      nodeTax.attr("C47", taxItem.taxCur());
    }

    nodeTax.attr("C6F", formatAmount(taxItem.taxAmt(), taxItem.taxNodec()));
  }
}

template <typename TaxItems, typename TaxRecords>
void
XMLShoppingResponse::generateTaxBreakDown(const TaxItems& taxItems,
                                          const TaxRecords& taxRecords,
                                          CurrencyNoDec taxNoDec)
{
  std::string taxCodes;
  for (const TaxItem* taxItemPtr : taxItems)
  {
    generateTaxBreakDownItem(*taxItemPtr, taxCodes, taxRecords, taxNoDec);
  }
}

template void
XMLShoppingResponse::generateTaxBreakDown(const std::vector<TaxItem*>& taxItems,
                                          const std::vector<TaxRecord*>& taxRecords,
                                          CurrencyNoDec taxNoDec);

void
XMLShoppingResponse::generateTaxOverrides(const CurrencyCode& currency)
{
  // Tax records for tax override
  const tse::CurrencyNoDec noDec = Money(currency).noDec(_trx.ticketingDate());

  for (const TaxOverride* taxOv : _trx.getRequest()->taxOverride())
  {
    if (taxOv->taxAmt() <= EPSILON)
      continue;

    Node(_writer, "TAX")
        .attr("BC0", taxOv->taxCode())
        .attr("C6B", formatAmount(taxOv->taxAmt(), noDec))
        .attr("C41", "OV");
  }
}

void
XMLShoppingResponse::generateTaxPfcItem(const PfcItem& pfcItem, const CurrencyCode& taxCurrencyCode)
{
  TaxRound taxRound;
  MoneyAmount amount = pfcItem.pfcAmount();
  CurrencyNoDec amountNoDec = pfcItem.pfcDecimals();

  if (pfcItem.pfcCurrencyCode() != taxCurrencyCode)
  {
    Money source(pfcItem.pfcAmount(), pfcItem.pfcCurrencyCode());
    Money target(taxCurrencyCode);
    _currencyConverter.convert(target, source, _trx, false, CurrencyConversionRequest::TAXES);
    amount = target.value();
    amountNoDec = target.noDec(_trx.ticketingDate());
    CurrencyConverter currencyConverter;
    RoundingFactor roundingUnit = 0.1;
    RoundingRule roundingRule = NONE;
    taxRound.retrieveNationRoundingSpecifications(_trx, roundingUnit, amountNoDec, roundingRule);
    Money targetMoney2(amount, taxCurrencyCode);
    amountNoDec = targetMoney2.noDec(_trx.ticketingDate());

    if (currencyConverter.round(targetMoney2, roundingUnit, roundingRule))
      amount = targetMoney2.value();
  }

  if (amount <= EPSILON)
  {
    return;
  }

  Node nodeTAX(_writer, "TAX");
  nodeTAX.attr("BC0", "XF")
      .attr("C6B", formatAmount(amount, amountNoDec))
      .attr("S04", "PASSENGER FACILITY CHARGES")
      .attr("S05", pfcItem.pfcAirportCode())
      .attr("C41", pfcItem.pfcCurrencyCode())
      .attr("C6A", formatAmount(pfcItem.pfcAmount(), pfcItem.pfcDecimals()))
      .attr("A40", "US");

  if (_trx.snapRequest())
  {
    nodeTAX.attr("B00", pfcItem.carrierCode());
  }
}

void
XMLShoppingResponse::generateTaxExchange(CalcTotals& calcTotal)
{
  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    std::vector<TaxItem*>::const_iterator taxItemIter = calcTotal.taxItems().begin();

    for (; taxItemIter != calcTotal.taxItems().end(); taxItemIter++)
    {
      const TaxItem& taxItem = **taxItemIter;

      if ((taxItem.failCode() != TaxItem::EXEMPT_ALL_TAXES) &&
          (taxItem.failCode() != TaxItem::EXEMPT_SPECIFIED_TAXES))
      {
        continue;
      }

      Node nodeTBE(_writer, "TBE");
      nodeTBE.attr("BC0", taxItem.taxCode())
          .attr("C6B", formatAmount(taxItem.taxExemptAmount(), taxItem.paymentCurrencyNoDec()))
          .attr("A04", taxItem.carrierCode())
          .attr("S05", taxItem.taxLocalBoard())
          .attr("C40", taxItem.paymentCurrency())
          .convertAttr("A06", taxItem.taxType());

      // Min max tax info
      if (taxItem.minTax() > 0.0)
      {
        nodeTBE.attr("C6D", formatAmount(taxItem.minTax(), taxItem.taxCurNodec()));
      }

      if (taxItem.maxTax() > 0.0)
      {
        nodeTBE.attr("C6E", formatAmount(taxItem.maxTax(), taxItem.taxCurNodec()));
      }

      if (taxItem.minTax() > 0.0 || taxItem.maxTax() > 0.0)
      {
        nodeTBE.attr("C47", taxItem.taxCur());
      }

      nodeTBE.attr("C6F", formatAmount(taxItem.taxAmt(), taxItem.taxNodec()));
    }
  }
}

template <typename PfcItems>
void
XMLShoppingResponse::generateTaxInformationRecord(const TaxRecord& taxRecord,
                                                  const PfcItems& pfcItems,
                                                  CurrencyNoDec taxNoDec)
{
  if (UNLIKELY(taxRecord.isTaxFeeExempt()))
  {
    Node nodeTSM(_writer, "TSM");
    nodeTSM.attr("BC0", taxRecord.taxCode()).attr("C6B", "0").attr("S05", "TE").attr("C41", "TE");

    if (_trx.snapRequest())
    {
      nodeTSM.attr("B00", taxRecord.carrierCode());
    }
  }
  else
  {
    if (UNLIKELY(taxRecord.getTaxAmount() < EPSILON)) // 0 tax amount for non-exempted tax
    {
      return;
    }

    Node nodeTSM(_writer, "TSM");
    nodeTSM.attr("BC0", taxRecord.taxCode())
        .attr("C6B", formatAmount(taxRecord.getTaxAmount(), taxNoDec));

    if (UNLIKELY(_trx.snapRequest()))
    {
      nodeTSM.attr("B00", taxRecord.carrierCode());
    }

    if (taxRecord.taxCode().equalToConst("XF"))
    {
      for (const PfcItem* pfcItemPtr : pfcItems)
      {
        const PfcItem& pfcItem = *pfcItemPtr;

        if (_trx.snapRequest() && ((pfcItem.carrierCode() != taxRecord.carrierCode() ||
                                    pfcItem.legId() != taxRecord.legId())))
        {
          continue;
        }

        if (pfcItem.pfcAmount() == taxRecord.getTaxAmount())
        {
          nodeTSM.attr("S05", pfcItem.pfcAirportCode());
        }
      }
    }
    else
    {
      nodeTSM.attr("S05", taxRecord.localBoard());
    }

    Money moneyPub(taxRecord.publishedCurrencyCode());
    nodeTSM.attr("S04", taxRecord.taxDescription())
        .attr("C6A",
              formatAmount(taxRecord.publishedAmount(), moneyPub.noDec(_trx.ticketingDate())))
        .attr("C41", taxRecord.publishedCurrencyCode());

    if (!taxRecord.taxNation().empty())
    {
      nodeTSM.attr("A40", taxRecord.taxNation());
    }
  }
}

template <typename TaxExempts>
void
XMLShoppingResponse::generateTaxExempt(const TaxExempts& taxExempts)
{
  for (const TaxCode& code : taxExempts)
  {
    Node(_writer, "TSM").attr("BC0", code).attr("C6B", "0").attr("S05", "TE").attr("C41", "TE");
  }
}

void
XMLShoppingResponse::generateTAD(const TaxResponse* tax, CalcTotals& calcTotal)
{
  Node nodeTAD(_writer, "TAD");
  nodeTAD.attr("C40", calcTotal.taxCurrencyCode());

  generateTaxBreakDown(calcTotal.taxItems(), calcTotal.taxRecords(), calcTotal.taxNoDec());

  if (tax && !tax->changeFeeTaxItemVector().empty())
  {
    generateTaxBreakDown(
        calcTotal.changeFeeTaxItems(), calcTotal.taxRecords(), calcTotal.taxNoDec());
  }

  generateTaxOverrides(calcTotal.taxCurrencyCode());

  generateTaxPfc(calcTotal.pfcItems(), calcTotal.taxCurrencyCode());

  generateTaxExchange(calcTotal);

  generateTaxInformation(calcTotal.taxRecords(), calcTotal.pfcItems(), calcTotal.taxNoDec());

  generateTaxExempt(calcTotal.getTaxExemptCodes());
}

void
XMLShoppingResponse::generateTADForSplittedTaxes(CalcTotals& calcTotal,
                                                 const FareCalc::SplitTaxInfo& group)
{
  {
    Node nodeTAD(_writer, "TAD");
    nodeTAD.attr("C40", calcTotal.taxCurrencyCode());

    generateTaxBreakDown(group.taxItems, group.taxRecords, calcTotal.taxNoDec());

    generateTaxPfc(group.pfcItems, calcTotal.taxCurrencyCode());

    generateTaxInformation(group.taxRecords, group.pfcItems, calcTotal.taxNoDec());

    generateTaxExempt(group.taxExempts);
  }

  {
    Node nodeTOT(_writer, "TOT");
    nodeTOT.attr("C43", group.construction.code());
    nodeTOT.attr("C5E", formatAmount(group.construction));

    nodeTOT.attr("C40", group.baseFare.code());
    nodeTOT.attr("C5A", formatAmount(group.baseFare));

    nodeTOT.attr("C45", group.equivalent.code());
    nodeTOT.attr("C5F", formatAmount(group.equivalent));

    nodeTOT.attr("C46", group.totalTax.code()); // tax and total currency
    nodeTOT.attr("C65", formatAmount(group.totalTax));

    nodeTOT.attr("C56", formatAmount(group.total));

    if (_trx.hasPriceDynamicallyDeviated())
    {
      nodeTOT.attr("EPD", formatAmount(group.effectiveDeviation));
    }

    if (_trx.awardRequest())
    {
      nodeTOT.convertAttr("C5L", group.mileage);
    }
  }
}

void
XMLShoppingResponse::generateTaxInfoPerLeg(CalcTotals& calcTotal)
{
  FareCalc::FcTaxInfo::TaxesPerLeg taxesPerLeg;
  calcTotal.getMutableFcTaxInfo().getTaxesSplitByLeg(taxesPerLeg);

  for (const FareCalc::FcTaxInfo::TaxesPerLeg::value_type& leg : taxesPerLeg)
  {
    Node nodeLEG(_writer, "LEG");
    nodeLEG.convertAttr("Q14", leg.first);

    generateTADForSplittedTaxes(calcTotal, leg.second);
  }
}

void
XMLShoppingResponse::generateOBG(CalcTotals& calcTotal)
{
  bool outputFType = false, outputTType = false, outputRType = false;

  if (_trx.getRequest()->isCollectOBFee() && !calcTotal.farePath->collectedTktOBFees().empty())
    outputFType = true;

  if (_trx.getRequest()->isCollectRTypeOBFee() &&
      !calcTotal.farePath->collectedRTypeOBFee().empty())
    outputRType = true;

  if (_trx.getRequest()->isCollectTTypeOBFee() &&
      !calcTotal.farePath->collectedTTypeOBFee().empty())
    outputTType = true;

  if (outputFType || outputTType || outputRType)
  {
    Node nodeOBG(_writer, "OBG");
    CurrencyCode targetCurrency = (calcTotal.equivCurrencyCode != "")
                                      ? calcTotal.equivCurrencyCode
                                      : calcTotal.convertedBaseFareCurrencyCode;
    size_t maxNumFType = 0, maxNumTType = 0, maxNumRType = 0;
    OBFeeUtil::getNumberOfOBFees(_trx, *calcTotal.farePath, maxNumFType, maxNumTType, maxNumRType);

    if (outputFType)
    {
      MoneyAmount highestAmount = 0;
      for (TicketingFeesInfo* feeInfo : calcTotal.farePath->collectedTktOBFees())
      {
        MoneyAmount currentObFeeAmount = 0;
        if (feeInfo->feePercent() > 0)
        {
          currentObFeeAmount =
              calculateObFeeAmountFromPercentage(_trx, calcTotal, feeInfo, targetCurrency);
        }
        else
        {
          currentObFeeAmount = calculateObFeeAmountFromAmount(_trx, feeInfo, targetCurrency);
        }

        getAdjustedObFeeAmount(currentObFeeAmount);
        if (highestAmount < currentObFeeAmount)
          highestAmount = currentObFeeAmount;
      }

      Node nodeOBF(_writer, "OBF");
      Money targetMoney(highestAmount, targetCurrency);
      nodeOBF.attr("SF1", formatAmount(highestAmount, targetMoney.noDec(_trx.ticketingDate())));
    }
    if (outputTType)
    {
      writeOutOBFee(
          calcTotal.farePath->collectedTTypeOBFee(), calcTotal, maxNumTType, targetCurrency);
    }
    if (outputRType)
    {
      writeOutOBFee(
          calcTotal.farePath->collectedRTypeOBFee(), calcTotal, maxNumRType, targetCurrency);
    }
  }
}

void
XMLShoppingResponse::writeOutOBFee(const std::vector<TicketingFeesInfo*>& collectedOBFeeVect,
                                   CalcTotals& calcTotal,
                                   size_t maxOBFeesOptions,
                                   CurrencyCode& targetCurrency)
{
  size_t totalFeesOutput = 0;
  for (TicketingFeesInfo* feeInfo : collectedOBFeeVect)
  {
    if (totalFeesOutput >= maxOBFeesOptions)
      break;

    MoneyAmount currentObFeeAmount = 0;
    if (feeInfo->feePercent() > 0)
    {
      currentObFeeAmount =
          calculateObFeeAmountFromPercentage(_trx, calcTotal, feeInfo, targetCurrency);
    }
    else
    {
      currentObFeeAmount = calculateObFeeAmountFromAmount(_trx, feeInfo, targetCurrency);
    }

    getAdjustedObFeeAmount(currentObFeeAmount);

    Node nodeOBF(_writer, "OBF");
    nodeOBF.attr("SF0", feeInfo->serviceTypeCode() + feeInfo->serviceSubTypeCode());
    Money targetMoney(currentObFeeAmount, targetCurrency);
    nodeOBF.attr("SF1", formatAmount(currentObFeeAmount, targetMoney.noDec(_trx.ticketingDate())));
    static ObFeeDescriptors obFeeDesc;
    std::string serviceDescription = feeInfo->commercialName().empty()
                                         ? obFeeDesc.getDescription(feeInfo->serviceSubTypeCode())
                                         : feeInfo->commercialName();
    nodeOBF.attr("SDD", serviceDescription);
    ++totalFeesOutput;
  }
}

MoneyAmount
XMLShoppingResponse::calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                                                    const TicketingFeesInfo* feeInfo,
                                                    const CurrencyCode& paymentCur)
{
  if (feeInfo->feeAmount() < 0.0 || paymentCur == feeInfo->cur())
    return (feeInfo->feeAmount());
  else
  {
    Money targetMoney = convertOBFeeCurrency(pricingTrx, feeInfo, paymentCur);
    return (targetMoney.value());
  }
}

void
XMLShoppingResponse::getAdjustedObFeeAmount(MoneyAmount& currentObFeeAmount)
{
  DateTime emptyDate;
  // Check if PYA and P2D are in the request. This indicates an inbound request.
  if (_trx.getRequest()->originBasedRTPricing() &&
      (_trx.outboundDepartureDate() != emptyDate || _trx.inboundDepartureDate() != emptyDate))
  {
    currentObFeeAmount /= 2;
  }
}

Money
XMLShoppingResponse::convertOBFeeCurrency(PricingTrx& pricingTrx,
                                          const TicketingFeesInfo* feeInfo,
                                          const CurrencyCode& paymentCur)
{
  const Money sourceMoney(feeInfo->feeAmount(), feeInfo->cur());
  Money targetMoney(paymentCur);

  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney);
  return targetMoney;
}

void
XMLShoppingResponse::convertOBFeeCurrency(PricingTrx& pricingTrx,
                                          const Money& sourceMoney,
                                          Money& targetMoney)
{
  _currencyConverter.convert(
      targetMoney, sourceMoney, pricingTrx, false, CurrencyConversionRequest::TAXES);
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;

  if (LIKELY(getFeeRounding(
          pricingTrx, targetMoney.code(), roundingFactor, roundingNoDec, roundingRule)))
  {
    CurrencyConverter curConverter;
    curConverter.round(targetMoney, roundingFactor, roundingRule);
  }
}

MoneyAmount
XMLShoppingResponse::calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                                        CalcTotals& calcTotal,
                                                        const TicketingFeesInfo* feeInfo,
                                                        const CurrencyCode& paymentCurrency)
{
  MoneyAmount totalPaxAmount = calcTotal.getTotalAmountPerPax();
  MoneyAmount calcAmount = (totalPaxAmount * feeInfo->feePercent()) / 100.0f;
  MoneyAmount lowestCalcAmount = calcAmount;

  Money targetMoneyCalc(calcAmount, paymentCurrency);
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;
  if (getFeeRounding(
          pricingTrx, targetMoneyCalc.code(), roundingFactor, roundingNoDec, roundingRule))
  {
    CurrencyConverter curConverter;
    curConverter.round(targetMoneyCalc, roundingFactor, roundingRule);
    calcAmount = targetMoneyCalc.value();
  }
  if (feeInfo->maxFeeCur().empty())
    return calcAmount;

  MoneyAmount maxAmount = feeInfo->maxFeeAmount();
  const Money sourceMoney(maxAmount, feeInfo->maxFeeCur());
  Money targetMoney(paymentCurrency);
  convertOBFeeCurrency(pricingTrx, sourceMoney, targetMoney); //+rounding
  maxAmount = targetMoney.value();
  if (calcAmount > maxAmount)
    lowestCalcAmount = maxAmount;

  return lowestCalcAmount;
}

MoneyAmount
XMLShoppingResponse::convertCurrency(const Money& money, const CurrencyCode& to)
{
  Money target(to.empty() ? NUC : to);

  if (target.code() == money.code())
  {
    return money.value();
  }

  const bool res = _currencyConverter.convert(target, money, _trx);

  if (res == false)
  {
    throw ErrorResponseException(ErrorResponseException::CANNOT_CALCULATE_CURRENCY,
                                 "Could not convert currency in response");
  }

  return target.value();
}

void
XMLShoppingResponse::generateP0N(const Itin* itin, Node& nodeITN)
{
  TSE_ASSERT(!itin->farePath().empty());
  const FarePath* farePath = itin->farePath().front();

  for (const auto pricingUnit : farePath->pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      for (const auto& segStatusItem : fareUsage->segmentStatus())
      {
        if (!segStatusItem._bkgCodeReBook.empty())
        {
          nodeITN.attr("P0N", "T");
          return;
        }
      }
    }
  }
}

void
XMLShoppingResponse::generateP0J(PricingTrx& trx, const Itin* itin, Node& nodeITN)
{
  std::vector<FarePath*>::const_iterator farePathI = itin->farePath().begin();

  for (const auto pricingUnit : (*farePathI)->pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      if (UNLIKELY(!fareUsage->paxTypeFare()->isElectronicTktable()))
      {
        nodeITN.attr("P0J", "F");
        return;
      }
    }
  }

  nodeITN.attr("P0J", "T");
  return;
}

void
XMLShoppingResponse::generateVTA(PricingTrx& trx, const Itin* itin, Node& nodeITN)
{
  bool result = true;

  if (_trx.isValidatingCxrGsaApplicable())
  {
    if (!fallback::fallbackSettingVTAToTrueForGSA(&_trx))
      result = true;
  }
  else
  {
    if (!performVTAValidation(*itin))
    {
      result = true;
    }
    else
    {
      if (!_interlineTicketCarrierData)
      {
        result = false;
      }
      else
      {
        result = _interlineTicketCarrierData->validateInterlineTicketCarrierAgreement(
            trx, itin->validatingCarrier(), itin->travelSeg());
      }
    }
  }

  if (result)
  {
    nodeITN.attr("VTA", "F");
  }
  else
  {
    nodeITN.attr("VTA", "T");
  }

  return;
}

void
XMLShoppingResponse::generateSTI(const Itin* itin, Node& nodeITN) const
{
  const bool isSidetrip = std::any_of(itin->fareMarket().cbegin(),
                                      itin->fareMarket().cend(),
                                      [](const FareMarket* const fm)
                                      { return !fm->sideTripTravelSeg().empty(); });

  (isSidetrip) ? nodeITN.attr("STI", "T") : nodeITN.attr("STI", "F");
}

void
XMLShoppingResponse::generateCPL(const Itin* itin, Node& nodeITN) const
{
  nodeITN.attr("CPL", generateCPLValueString(ShoppingUtil::getGoverningCarriersPerLeg(*itin)));
}

std::string
XMLShoppingResponse::generateCPLValueString(const std::vector<CarrierCode>& gcPerLeg) const
{
  if (gcPerLeg.empty())
    return "";

  std::string cplValue;
  cplValue.reserve(gcPerLeg.size() * 3 - 1);
  for (const CarrierCode& cxr : gcPerLeg)
  {
    cplValue.append(cxr);
    if (&cxr != &gcPerLeg.back())
      cplValue.append("|");
  }
  return cplValue;
}

void
XMLShoppingResponse::generatePVTandFBR(const Itin* itin, Node& nodeITN)
{
  bool isPrivate = false;
  bool isFBR = false;

  std::vector<FarePath*>::const_iterator farePathI = itin->farePath().begin();
  generatePVTandFBRImpl(*(*farePathI), isPrivate, isFBR);

  (isPrivate) ? nodeITN.attr("PVT", "T") : nodeITN.attr("PVT", "F");
  (isFBR) ? nodeITN.attr("FBR", "T") : nodeITN.attr("FBR", "F");

  return;
}

void
XMLShoppingResponse::generatePVTandFBR(const GroupFarePath* path, Node& nodeITN)
{
  bool isPrivate = false;
  bool isFBR = false;

  if (path != nullptr)
  {
    for (std::vector<FPPQItem*>::const_iterator i = path->groupFPPQItem().begin();
         (i != path->groupFPPQItem().end()) && ((false == isPrivate) || (false == isFBR));
         ++i)
    {
      generatePVTandFBRImpl(*(*i)->farePath(), isPrivate, isFBR);
    }
  }

  nodeITN.convertAttr("PVT", (isPrivate) ? 'T' : 'F');
  nodeITN.convertAttr("FBR", (isFBR) ? 'T' : 'F');
}

void
XMLShoppingResponse::generatePVTandFBRImpl(const FarePath farePath, bool& isPrivate, bool& isFBR)
{
  for (std::vector<PricingUnit*>::const_iterator puItem = farePath.pricingUnit().begin();
       (puItem != farePath.pricingUnit().end()) && ((false == isPrivate) || (false == isFBR));
       ++puItem)
  {
    for (std::vector<FareUsage*>::const_iterator fuItem = (*puItem)->fareUsage().begin();
         fuItem != (*puItem)->fareUsage().end() && ((false == isPrivate) || (false == isFBR));
         ++fuItem)
    {
      PaxTypeFare* paxFare = (*fuItem)->paxTypeFare();
      if (RuleConst::PRIVATE_TARIFF == paxFare->tcrTariffCat())
      {
        isPrivate = true;
      }
      if (paxFare->isFareByRule())
      {
        isFBR = true;
      }
    }
  }
}

bool
XMLShoppingResponse::generateP27(const FarePath& path)
{
  for (std::vector<PricingUnit*>::const_iterator pu = path.pricingUnit().begin();
       pu != path.pricingUnit().end();
       ++pu)
  {
    for (std::vector<FareUsage*>::const_iterator fu = (*pu)->fareUsage().begin();
         fu != (*pu)->fareUsage().end();
         ++fu)
    {
      if (((*fu)->isAppendNR()) && ((*fu)->paxTypeFare()->penaltyRestInd() == YES))
      {
        return true;
      }

      if ((*fu)->isNonRefundable())
      {
        return true;
      }
    }
  }

  return false;
}

// generateCCD : currency conversion
void
XMLShoppingResponse::generateCCD(PricingTrx& trx, const CalcTotals& calcTotal)
{
  const char OVERRIDE_CONVERSION = 'O';

  if (trx.getOptions()->returnAllData() == GDS)
  {
    std::map<uint16_t, TravelSeg*, std::less<uint16_t>>::const_iterator j =
        calcTotal.travelSegs.begin();
    TravelSeg* travelSeg = (*j).second;

    if ((LocUtil::isUS(*trx.getRequest()->ticketingAgent()->agentLocation())) &&
        (LocUtil::isUS(*travelSeg->origin())))
    {
      return;
    }
  }

  if (TrxUtil::isIcerActivated(trx))
  {
    Node nodeCCD(_writer, "CCD");
    nodeCCD.attr("C41", calcTotal.convertedBaseFareCurrencyCode)
        .attr("C42", calcTotal.equivCurrencyCode);
    nodeCCD.attr("C54", formatAmount(calcTotal.bsrRate1, calcTotal.bsrRate1NoDec))
        .convertAttr("Q05", calcTotal.bsrRate1NoDec);
  }
  // Always display FROM currency based on Agent currency
  else if ((calcTotal.equivCurrencyCode == trx.getOptions()->currencyOverride()) ||
           (calcTotal.equivCurrencyCode == trx.getRequest()->ticketingAgent()->currencyCodeAgent()))
  {
    Node nodeCCD(_writer, "CCD");
    nodeCCD.attr("C41", calcTotal.convertedBaseFareCurrencyCode)
        .attr("C42", calcTotal.equivCurrencyCode);
    nodeCCD.attr("C54",
                 formatAmount(calcTotal.bsrRate1 == 0 ? 999 : 1 / calcTotal.bsrRate1,
                              calcTotal.bsrRate1NoDec)).convertAttr("Q05", calcTotal.bsrRate1NoDec);

    if (calcTotal.bsrRate2 > 0.0)
    {
      nodeCCD.attr("C55",
                   formatAmount(calcTotal.bsrRate2 == 0 ? 999 : 1 / calcTotal.bsrRate2,
                                calcTotal.bsrRate2NoDec))
          .convertAttr("Q06", calcTotal.bsrRate2NoDec);
    }

    nodeCCD.convertAttr("N02", OVERRIDE_CONVERSION);
  }
  else
  {
    Node nodeCCD(_writer, "CCD");
    nodeCCD.attr("C41", calcTotal.convertedBaseFareCurrencyCode)
        .attr("C42", calcTotal.equivCurrencyCode);
    nodeCCD.attr("C54", formatAmount(calcTotal.bsrRate1, calcTotal.bsrRate1NoDec))
        .convertAttr("Q05", calcTotal.bsrRate1NoDec);

    if (calcTotal.bsrRate2 > 0.0)
    {
      nodeCCD.attr("C55", formatAmount(calcTotal.bsrRate2, calcTotal.bsrRate2NoDec))
          .convertAttr("Q06", calcTotal.bsrRate2NoDec);
    }
  }

  return;
}

void
XMLShoppingResponse::prepareFareUsagePlusUps(const FareUsage& fareUsage,
                                             const tse::CurrencyNoDec& noDec)
{
  if (fareUsage.minFarePlusUp().empty())
    return;

  MinFarePlusUp::const_iterator pupIterEnd = fareUsage.minFarePlusUp().end();
  MinFarePlusUp::const_iterator pupIter = fareUsage.minFarePlusUp().find(DMC);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("DMC", *(pupIter->second), noDec, "");
  }

  pupIter = fareUsage.minFarePlusUp().find(COM);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("COM", *(pupIter->second), noDec, "");
  }

  pupIter = fareUsage.minFarePlusUp().find(BHC);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("BHC", *(pupIter->second), noDec, "");
  }
}

void
XMLShoppingResponse::preparePupElement(const std::string& payload,
                                       const MinFarePlusUpItem& plusUp,
                                       const tse::CurrencyNoDec& noDec,
                                       const NationCode& countryOfPmt)
{
  Node nodePUP(_writer, "PUP");
  nodePUP.attr("C6L", formatAmount(plusUp.plusUpAmount, noDec)) // plusUpAmount
      .attr("A11", plusUp.boardPoint) // plusUpOrigCity
      .attr("A12", plusUp.offPoint); // plusUpDestCity

  if (payload == "BHC")
  {
    const BhcPlusUpItem& bhcPlusUp = dynamic_cast<const BhcPlusUpItem&>(plusUp);
    nodePUP.attr("A13", bhcPlusUp.fareBoardPoint) //  PlusUpFareOrigCity = "A13"
        .attr("A14", bhcPlusUp.fareOffPoint); //  PlusUpFareDestCity = "A14"
  }
  else if (payload == "LCM") // COP
  {
    nodePUP.attr("A40", countryOfPmt); // PlusUpCountryOfPmt = "A40"
  }

  if (!plusUp.constructPoint.empty())
  {
    nodePUP.attr("A18", plusUp.constructPoint); // PlusUpViaCity = "A18"
  }

  nodePUP.attr("S68", payload); // PlusUpMessage = "S68"
}

void
XMLShoppingResponse::prepareRuleCategoryIndicator(const FareUsage& fareUsage,
                                                  CurrencyNoDec noDec,
                                                  bool& minMaxCatFound)
{
  const uint16_t CATEGORY_BEGIN = 1;
  const uint16_t CATEGORY_END = 24;
  const uint16_t CAT_10 = 10;
  const uint16_t CAT_25 = 25;
  const uint16_t CAT_35 = 35;
  const uint16_t CAT_MINSTAY = 6;
  const uint16_t CAT_MAXSTAY = 7;
  minMaxCatFound = false;
  const PaxTypeFare& ptFare = *fareUsage.paxTypeFare();
  std::ostringstream dataStream;
  bool categoryFound = false;
  PaxTypeFareRuleData* ptfrd = nullptr;

  for (uint16_t category = CATEGORY_BEGIN; category < CATEGORY_END; ++category)
  {
    if (category == CAT_10 && fareUsage.rec2Cat10() != nullptr)
    {
      if (categoryFound)
      {
        dataStream << " ";
      }

      dataStream << CAT_10;
      categoryFound = true;
      continue;
    }

    ptfrd = ptFare.paxTypeFareRuleData(category);

    if (ptfrd && (ptfrd->categoryRuleInfo() || ptfrd->ruleItemInfo() ||
                  PTFRuleData::toFBRPaxTypeFare(ptfrd)))
    {
      if (categoryFound)
      {
        dataStream << " ";
      }

      dataStream << category;
      categoryFound = true;

      if (category == CAT_MAXSTAY)
      {
        const MaxStayRestriction* maxStayRule =
            dynamic_cast<const MaxStayRestriction*>(ptfrd->ruleItemInfo());

        if (maxStayRule != nullptr)
        {
          PeriodOfStay maxStayPeriod(maxStayRule->maxStay(), maxStayRule->maxStayUnit());

          if (!maxStayPeriod.isOneYear())
          {
            minMaxCatFound = true;
          }
        }
      }

      if (category == CAT_MINSTAY)
      {
        minMaxCatFound = true;
      }
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

  if (LIKELY(categoryFound))
  {
    Node nodeCat(_writer, "CAT");
    nodeCat.addSimpleText(dataStream.str().c_str());
  }
}

void
XMLShoppingResponse::prepareDifferential(const FareUsage& fareUsage,
                                         const tse::CurrencyNoDec& noDec)
{
  if (fareUsage.differentialAmt() == 0)
    return;

  std::vector<DifferentialData*>::const_iterator differentialIter =
      fareUsage.differentialPlusUp().begin();
  std::vector<DifferentialData*>::const_iterator differentialIterEnd =
      fareUsage.differentialPlusUp().end();

  for (; differentialIter != differentialIterEnd; differentialIter++)
  {
    DifferentialData::STATUS_TYPE aStatus = (*differentialIter)->status();

    if (aStatus != DifferentialData::SC_PASSED && aStatus != DifferentialData::SC_CONSOLIDATED_PASS)
      continue;

    if (!(*differentialIter))
      continue;

    Node nodeHIP(_writer, "HIP");
    MoneyAmount moneyAmount = 0.0;
    DifferentialData& di = **differentialIter;

    if (di.hipAmount() <= EPSILON)
    {
      moneyAmount = di.amount();
    }
    else
    {
      moneyAmount = di.hipAmount();
    }

    if (di.throughFare()->fareMarket()->direction() == FMDirection::INBOUND)
    {
      nodeHIP.attr("A13", (*di.fareMarket().begin())->offMultiCity()) //  OrigCityHIP    = "A13"
          .attr("A14", (*di.fareMarket().begin())->boardMultiCity()); //  DestCityHIP    = "A14"
    }
    else
    {
      nodeHIP.attr("A13", (*di.fareMarket().begin())->boardMultiCity()) //  OrigCityHIP    = "A13"
          .attr("A14", (*di.fareMarket().begin())->offMultiCity()); //  DestCityHIP    = "A14"
    }

    if (!di.fareClassLow().empty())
    {
      nodeHIP.attr("B30", di.fareClassLow().c_str()); // FareClassLow   = "B30"
    }

    if (!di.fareClassHigh().empty())
    {
      nodeHIP.attr("BJ0", di.fareClassHigh().c_str()); // FareClassHigh  = "BJ0"
    }

    // Process mileage
    PaxTypeFare* ptf = di.throughFare();

    if (!ptf)
    {
      continue;
    }

    bool cabinHighSet = false;

    if (!di.hipLowOrigin().empty() || !di.hipLowDestination().empty() ||
        !di.hipHighOrigin().empty() || !di.hipHighDestination().empty()) // HIP
    {
      //
      // Legacy requires both Low and Hight city pairs but might not require Cabin???
      //
      //          if (di.hipLowOrigin () == di.hipHighOrigin() && di.hipLowDestination () ==
      // di.hipHighDestination())  // 3.
      //          {

      //              construct.addAttribute(xml2::LowOrigHIP, di.hipLowOrigin ());
      //              construct.addAttribute(xml2::LowDestHIP, di.hipLowDestination ());
      //          }
      //          else
      //          {
      // LowOrigHIP     = "A01";
      // LowDestHIP     = "A02";
      // HighOrigHIP    = "A03";

      // HighDestHIP    = "A04";
      if (di.hipLowOrigin().empty() == false)
      {
        nodeHIP.attr("A01", di.hipLowOrigin()).attr("A02", di.hipLowDestination());
      }

      if (di.hipHighOrigin().empty() == false)
      {
        nodeHIP.attr("A03", di.hipHighOrigin()).attr("A04", di.hipHighDestination());
      }

      if ((di.hipLowOrigin().empty() && di.hipLowDestination().empty()) || // 4.
          (di.hipHighOrigin().empty() && di.hipHighDestination().empty()))
      {
        if (di.hipHighOrigin().empty())
        {
          //  CabinLowHIP    = "N00"
          nodeHIP.convertAttr("N00", di.hipCabinLow());
        }
        else
        {
          // CabinHighHIP   = "N04"
          nodeHIP.convertAttr("N04", di.hipCabinHigh());
          cabinHighSet = true;
        }
      }
      else // 5.
      {
        nodeHIP.convertAttr("N00", di.hipCabinLow()).convertAttr("N04", di.hipCabinHigh());
        cabinHighSet = true;
      }
    }

    if (!cabinHighSet)
    {
      nodeHIP.convertAttr("N04", di.fareClassHigh()[0]);
    }

    nodeHIP.attr("C50", formatAmount(moneyAmount, noDec)); // AmountHIP      = "C50"

    if (di.fareHigh()->mileageSurchargePctg())
    {
      nodeHIP.convertAttr("Q48", di.fareHigh()->mileageSurchargePctg());
    }
  } // end for (; diffIter != diffIterEnd; diffIter++)
}

void
XMLShoppingResponse::prepareHPUForNet(const FareUsage& fareUsage)
{
  const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();

  if (!paxTypeFare->hasCat35Filed())
    return;

  const NegPaxTypeFareRuleData* negRuleData = paxTypeFare->getNegRuleData();

  if (!negRuleData)
    return;

  if (!fallback::fallbackFixFRRHpuForNet(&_trx) &&
       !negRuleData->fareRetailerRuleId())
    return;

  const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(negRuleData->ruleItemInfo());

  if (!negFareRest)
    return;

  if (negRuleData->sourcePseudoCity().empty())
    return;

  Node nodeHPU(_writer, "HPU");

  nodeHPU.attr(xml2::MarkupFeeAppId, "NT");
  //nodeHPU.attr(xml2::MarkupTypeCode, "X"); // TO DO

  MoneyAmount netAmt = negRuleData->netAmount();
  MoneyAmount selAmt = paxTypeFare->fareAmount();

  // if this fare also matched an FRR rule in ASL level, we need to display
  // pre-ASL amount to match pricing
  if (!fallback::fallbackMipHPUFix(&_trx))
  {
    const AdjustedSellingCalcData* adjSellingCalcData  = paxTypeFare->getAdjustedSellingCalcData();
    if (adjSellingCalcData)
      selAmt = adjSellingCalcData->getCalculatedAmt() - adjSellingCalcData->getMarkupAdjAmt();
  }

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

  nodeHPU.attr(xml2::FareAmountAfterMarkup, formatAmount(faAfterMarkUp, paxTypeFare->numDecimal()));
  nodeHPU.attr(xml2::MarkupAmount, formatAmount(markupAmt, paxTypeFare->numDecimal()));
  nodeHPU.attr(xml2::AmountCurrency, paxTypeFare->currency());
  nodeHPU.attr(xml2::MarkupRuleSourcePCC, negRuleData->sourcePseudoCity());

  try
  {
    std::string strFareRetailerRuleId(boost::lexical_cast<std::string>(negRuleData->fareRetailerRuleId()));
    nodeHPU.attr(xml2::MarkupRuleItemNumber, strFareRetailerRuleId);
  }
  catch (const boost::bad_lexical_cast&) {}
}

void
XMLShoppingResponse::prepareHPUForAdjusted(const FareUsage& fareUsage)
{
  const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();

  if (!paxTypeFare->getAdjustedSellingCalcData())
    return;

  const AdjustedSellingCalcData* adjSellingCalcData  = paxTypeFare->getAdjustedSellingCalcData();

  if (!adjSellingCalcData)
    return;

  Node nodeHPU(_writer, "HPU");

  nodeHPU.attr(xml2::MarkupFeeAppId, "AJ");
  //nodeHPU.attr(xml2::MarkupTypeCode, "X"); // TO DO
  nodeHPU.attr(xml2::FareAmountAfterMarkup,
               formatAmount(adjSellingCalcData->getCalculatedAmt(), paxTypeFare->numDecimal()));
  nodeHPU.attr(xml2::MarkupAmount,
               formatAmount(adjSellingCalcData->getMarkupAdjAmt(), paxTypeFare->numDecimal()));
  nodeHPU.attr(xml2::AmountCurrency, paxTypeFare->currency());
  nodeHPU.attr(xml2::MarkupRuleSourcePCC, adjSellingCalcData->getSourcePcc());
  try
  {
    std::string strFareRetailerRuleId(boost::lexical_cast<std::string>(adjSellingCalcData->getFareRetailerRuleId()));
    nodeHPU.attr(xml2::MarkupRuleItemNumber, strFareRetailerRuleId);
  }
  catch (const boost::bad_lexical_cast&) {}
}

void
XMLShoppingResponse::preparePricingUnitPlusUps(const PricingUnit& pricingUnit,
                                               const tse::CurrencyNoDec& noDec,
                                               PricingTrx& pricingTrx)
{
  if (pricingUnit.minFarePlusUp().empty())
    return;

  MinFarePlusUp::const_iterator pupIterEnd = pricingUnit.minFarePlusUp().end();
  MinFarePlusUp::const_iterator pupIter = pricingUnit.minFarePlusUp().find(CTM);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("CTM", *(pupIter->second), noDec, "");
  }

  pupIter = pricingUnit.minFarePlusUp().find(COP);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    const Loc* saleLoc = TrxUtil::saleLoc(pricingTrx);

    if (saleLoc != nullptr)
    {
      preparePupElement("LCM", *(pupIter->second), noDec, saleLoc->nation());
    }
  }

  pupIter = pricingUnit.minFarePlusUp().find(OJM);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("OJM", *(pupIter->second), noDec, "");
  }

  pupIter = pricingUnit.minFarePlusUp().find(CPM);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("CPM", *(pupIter->second), noDec, "");
  }

  pupIter = pricingUnit.minFarePlusUp().find(HRT);

  if (pupIter != pupIterEnd && pupIter->second != nullptr)
  {
    preparePupElement("HRTC", *(pupIter->second), noDec, "");
  }
}

void
XMLShoppingResponse::prepareFarePathPlusUps(const FarePath& farePath,
                                            const tse::CurrencyNoDec& noDec)
{
  // For OSC
  const std::vector<FarePath::OscPlusUp*>& oscPlusUps = farePath.oscPlusUp();
  std::vector<FarePath::OscPlusUp*>::const_iterator oscIter = oscPlusUps.begin();

  for (; oscIter != oscPlusUps.end(); oscIter++)
  {
    MinFarePlusUpItem* pup = (MinFarePlusUpItem*)*oscIter;
    preparePupElement("OSC", *pup, noDec, "");
  }

  // For RSC
  const std::vector<FarePath::RscPlusUp*>& rscPlusUps = farePath.rscPlusUp();
  std::vector<FarePath::RscPlusUp*>::const_iterator rscIter = rscPlusUps.begin();

  for (; rscIter != rscPlusUps.end(); rscIter++)
  {
    MinFarePlusUpItem* pup = (MinFarePlusUpItem*)*rscIter;
    preparePupElement("RSC", *pup, noDec, "");
  }
}

void
XMLShoppingResponse::generateP35(const FareCalcCollector& calc, Node& nodeITN)
{
  const std::vector<CalcTotals*>& calcTotals = calc.passengerCalcTotals();
  std::vector<CalcTotals*>::const_iterator calcItem = calcTotals.begin();
  std::vector<std::string> bookingCodeRebook = (*calcItem)->bookingCodeRebook;
  calcItem++;

  for (; calcItem != calcTotals.end(); ++calcItem)
  {
    if (bookingCodeRebook != (*calcItem)->bookingCodeRebook)
    {
      nodeITN.attr("P35", "T");
      return;
    }
  }

  return;
}

void
XMLShoppingResponse::generateP95(const SopIdVec& sops, Node& nodeITN)
{
  bool interItin = isInternationalItin(sops);

  if (interItin)
  {
    nodeITN.attr("P95", "T");
    _atLeastOneInterItin = true;
  }
  else
  {
    _atLeastOneDomItin = true;
  }

  return;
}

void
XMLShoppingResponse::generateFMT(const SopIdVec& sops, Node& nodeITN)
{
  const ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);
  if (_trx.getRequest()->isReturnIllogicalFlights())
  {
    if (!ShoppingUtil::checkMinConnectionTime(_trx.getOptions(), sops, shoppingTrx.legs()))
    {
      nodeITN.attr("FMT", "T");
    }
  }
}

bool
XMLShoppingResponse::isInternationalItin(const SopIdVec& sops)
{
  bool interItin = false;
  size_t sopsSize = sops.size();

  for (uint16_t n = 0; n != sopsSize; ++n)
  {
    ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);
    Itin* itin = shoppingTrx.legs()[n].sop()[sops[n]].itin();

    for (std::vector<TravelSeg*>::const_iterator i = itin->travelSeg().begin();
         i != itin->travelSeg().end();
         ++i)
    {
      // check domestic or international itin
      if (LocUtil::isInternational(*((*i)->origin()), *((*i)->destination())) && !interItin)
      {
        interItin = true;
        break;
      }
    }
  }

  return interItin;
}

void
XMLShoppingResponse::formatDiag931()
{
  Diag931Collector* const collector =
      dynamic_cast<Diag931Collector*>(DCFactory::instance()->create(_trx));
  TSE_ASSERT(collector != nullptr);

  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);

  Diag931Collector& stream = *collector;
  stream.enable(Diagnostic931);
  stream.outputHeader(shoppingTrx);
  std::vector<ISSolution> paths(_shoppingTrx->flightMatrix().begin(),
                                _shoppingTrx->flightMatrix().end());
  std::sort(paths.begin(), paths.end(), compareISSolutions);
  int16_t itemNo = 0;

  for (std::vector<ISSolution>::const_iterator i = paths.begin(); i != paths.end(); ++i, ++itemNo)
  {
    std::string mapKey;
    int16_t carrierGroupIndex = _q4qGrouping.generateQ4Q(i->first, mapKey);
    bool interItin = isInternationalItin(i->first);
    stream.outputItin(i->first, carrierGroupIndex, shoppingTrx, itemNo, interItin, mapKey);
  }

  stream.flushMsg();
}

void
XMLShoppingResponse::formatDiag988()
{
  DCFactory* factory = DCFactory::instance();
  Diag988Collector* collector = dynamic_cast<Diag988Collector*>(factory->create(_trx));
  TSE_ASSERT(collector != nullptr);
  collector->enable(Diagnostic988);
  (*collector) << "\n\nMIP RESPONSE\n";

  if (!InterlineTicketCarrier::isPriceInterlineActivated(_trx))
  {
    *collector << "IET PRICING IS NOT ACTIVE\n";
    collector->flushMsg();
    return;
  }

  if (!_interlineTicketCarrierData)
  {
    (*collector) << "NO VITA DATA AVAILABLE\n";
  }

  for (std::vector<Itin*>::iterator i = _trx.itin().begin(); i != _trx.itin().end(); ++i)
  {
    if (itinIsValidSolution(*i) == false)
    {
      continue;
    }

    std::string validationMessage;
    bool result = false;
    if (!_interlineTicketCarrierData)
    {
      result = false;
    }
    else if (performVTAValidation(**i))
    {
      result = _interlineTicketCarrierData->validateInterlineTicketCarrierAgreement(
          _trx, (*i)->validatingCarrier(), (*i)->travelSeg(), &validationMessage);
    }
    else
    {
      result = true;
    }

    if (_interlineTicketCarrierData)
    {
      (*collector) << _interlineTicketCarrierData->getInterlineCarriersForValidatingCarrier(
          _trx, (*i)->validatingCarrier());
    }

    collector->outputVITAData(_trx, **i, result, validationMessage);
    collector->flushMsg();
  }

  return;
}

void
XMLShoppingResponse::checkDuplicateTvlSeg(const FarePath& path)
{
  std::set<TravelSeg*> tvlSegSet;

  for (const auto pricingUnit : path.pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      for (const auto tvlSeg : fareUsage->travelSeg())
      {
        const bool res = tvlSegSet.insert(tvlSeg).second;

        if (UNLIKELY(res == false))
        {
          throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                       "TravelSeg Duplicate in PU");
        }
      }
    }
  }
}

const CarrierCode
XMLShoppingResponse::getValidatingCarrier(PricingTrx& pricingTrx)
{
  if (!pricingTrx.getRequest()->validatingCarrier().empty())
  {
    return pricingTrx.getRequest()->validatingCarrier();
  }
  else
  {
    for (const auto itin : pricingTrx.itin())
    {
      if (!itin->validatingCarrier().empty())
      {
        return itin->validatingCarrier();
      }
    }

    return "XX";
  }
}

void
XMLShoppingResponse::generateOND()
{
  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);
  std::vector<PricingTrx::OriginDestination> odThruFM;

  for (const auto fareMarket : shoppingTrx.fareMarket())
  {
    if (shoppingTrx.isSumOfLocalsProcessingEnabled() &&
        (fareMarket->getFmTypeSol() == FareMarket::SOL_FM_LOCAL))
      continue;

    checkDuplicateThruFM(odThruFM, *fareMarket);
  }

  // format the OND
  for (const auto& ond : odThruFM)
  {
    Node nodeOND(_writer, "OND");
    nodeOND.attr("A01", ond.boardMultiCity).attr("A02", ond.offMultiCity).convertAttr(
        "D01", ond.travelDate.dateToSqlString());
  }

  return;
}

void
XMLShoppingResponse::checkDuplicateThruFM(std::vector<PricingTrx::OriginDestination>& odThruFM,
                                          FareMarket& fareMarket)
{
  for (const auto& ond : odThruFM)
  {
    if ((fareMarket.boardMultiCity() == ond.boardMultiCity) &&
        (fareMarket.offMultiCity() == ond.offMultiCity) &&
        (fareMarket.travelSeg().front()->departureDT().date() == ond.travelDate.date()))
    {
      return;
    }
  }

  PricingTrx::OriginDestination thruFM;
  thruFM.boardMultiCity = fareMarket.boardMultiCity();
  thruFM.offMultiCity = fareMarket.offMultiCity();
  thruFM.travelDate = fareMarket.travelSeg().front()->departureDT();
  odThruFM.push_back(thruFM);
  return;
}

std::string
XMLShoppingResponse::generateN1U(const PaxTypeFare& ptFare)
{
  std::string privateInd = " ";

  // check the PCC and carrier Preference table for privateFareInd
  if (_privateIndRetrieved == false)
  {
    _privateIndRetrieved = true;
    DataHandle& dh = _trx.dataHandle();

    if (!_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    {
      const std::vector<Customer*> customerTable =
          dh.getCustomer(_trx.getRequest()->ticketingAgent()->tvlAgencyPCC());

      if (customerTable.empty())
      {
        return privateInd;
      }

      Customer* customer = customerTable.front();

      if (customer->privateFareInd() == 'N')
      {
        return privateInd;
      }
    }
    else
    {
      const CarrierPreference* cxrPref =
          dh.getCarrierPreference(_trx.billing()->partitionID(), ptFare.fareMarket()->travelDate());

      if (cxrPref == nullptr)
      {
        return privateInd;
      }

      if (cxrPref->privateFareInd() == 'N')
      {
        return privateInd;
      }
    }

    _privateIndAllowed = true;
  }

  if (_privateIndAllowed == false)
  {
    return privateInd;
  }

  if (ptFare.fcaDisplayCatType() == 'C')
    PrivateIndicator::privateIndicator(ptFare, privateInd, false);
  else
    PrivateIndicator::privateIndicatorOld(ptFare, privateInd, false);

  return privateInd;
}

void
XMLShoppingResponse::generatePrivateInd(const Itin& itin,
                                        const FareCalcCollector& calc,
                                        std::vector<std::string>& privateIndPerPax)
{
  std::vector<PaxType*> paxTypes = _trx.paxType();
  std::sort(paxTypes.begin(), paxTypes.end(), comparePaxTypeInputOrder);
  std::string privateInd = " ";

  // find the privateFare Indicator for each pax
  for (std::vector<PaxType*>::const_iterator pax = paxTypes.begin(); pax != paxTypes.end(); ++pax)
  {
    const FarePath* path = nullptr;
    CalcTotals* totals = nullptr;
    privateInd = " ";

    // if farePath is exist. find the privateFareIndicator
    for (std::vector<FarePath*>::const_iterator fP = itin.farePath().begin();
         fP != itin.farePath().end();
         ++fP)
    {
      if (*pax == (*fP)->paxType())
      {
        path = *fP;
        break;
      }
    }

    if (path != nullptr)
    {
      totals = calc.findCalcTotals(path);
    }
    else
    {
      totals = calc.findCalcTotals(*pax);
    }

    if ((path == nullptr) || (totals == nullptr))
    {
      privateIndPerPax.push_back(privateInd);
      continue;
    }

    std::string prePrivateInd = privateInd;

    for (const auto pricingUnit : path->pricingUnit())
    {
      for (const auto fareUsage : pricingUnit->fareUsage())
      {
        privateInd = generateN1U(*fareUsage->paxTypeFare());

        if (prePrivateInd == " ")
        {
          prePrivateInd = privateInd;
        }
        else if (privateInd != " ")
        {
          if (prePrivateInd != privateInd)
          {
            prePrivateInd = PILLOW;
          }
        }
      }
    }

    privateIndPerPax.push_back(prePrivateInd);
  }
}

void
XMLShoppingResponse::generateDFL()
{
  if (_trx.getTrxType() == PricingTrx::FF_TRX)
  {
    FlightFinderTrx& flightFinderTrx = dynamic_cast<FlightFinderTrx&>(_trx);

    if (!(flightFinderTrx.outboundDateflightMap().empty()) &&
        (flightFinderTrx.diagnostic().diagnosticType() == DiagnosticNone ||
         flightFinderTrx.ignoreDiagReq() || flightFinderTrx.diagnostic().processAllServices()))
    {
      // generate DFL
      Node nodeDFL(_writer, "DFL");
      FlightFinderTrx::OutBoundDateFlightMapIC itODFM =
          flightFinderTrx.outboundDateflightMap().begin();
      // if the fare is bookingCode override
      PaxTypeFare* outBoundFare = getFrontPaxTypeFare((itODFM->second)->flightInfo);

      if (outBoundFare && false == outBoundFare->getChangeFareBasisBkgCode().empty())
      {
        nodeDFL.convertAttr("PBJ", 'T');
      }
      else
      {
        FlightFinderTrx::InboundDateFlightMap::const_iterator itIDFM =
            (itODFM->second)->iBDateFlightMap.begin();

        if (itIDFM != (itODFM->second)->iBDateFlightMap.end())
        {
          PaxTypeFare* inBoundFare = getFrontPaxTypeFare(*(itIDFM->second));

          if (inBoundFare && false == inBoundFare->getChangeFareBasisBkgCode().empty())
          {
            nodeDFL.convertAttr("PBJ", 'T');
          }
        }
      }

      for (; itODFM != flightFinderTrx.outboundDateflightMap().end(); ++itODFM)
      {
        Node nodeOBL(_writer, "OBL");
        nodeOBL.convertAttr("D01", itODFM->first.dateToString(YYYYMMDD, "-")); // DataTime outBound

        if (showFareBasisCode(flightFinderTrx, true))
        {
          PaxTypeFare* outBoundFare = getFrontPaxTypeFare(itODFM->second->flightInfo);

          if (outBoundFare)
          {
            nodeOBL.attr("B50",
                         outBoundFare->createFareBasis(flightFinderTrx,
                                                       false)); // fareInfo->fareBasiscode outBound

            if (flightFinderTrx.avlInS1S3Request())
            {
              nodeOBL.convertAttr("P1V",
                                  itODFM->second->flightInfo.onlyApplicabilityFound ? "F" : "T");
            }
          }
        }

        if (showSOPs(flightFinderTrx, true))
        {
          // outbound flights
          std::vector<FlightFinderTrx::SopInfo*>::const_iterator itOBFL =
              (itODFM->second)->flightInfo.flightList.begin();

          for (; itOBFL != (itODFM->second)->flightInfo.flightList.end(); ++itOBFL)
          {
            // outbound flights
            Node nodeFID(_writer, "FID");
            nodeFID.convertAttr("Q14", 0); // LEG = 0 for outbound
            nodeFID.convertAttr("Q15",
                                getOriginalSopIndex(flightFinderTrx, 0, (*itOBFL)->sopIndex));
            {
              // bkg codes data
              std::vector<FlightFinderTrx::BookingCodeData>::const_iterator bkgCodeDataIter =
                  (*itOBFL)->bkgCodeDataVect.front().begin();

              for (; bkgCodeDataIter != (*itOBFL)->bkgCodeDataVect.front().end(); ++bkgCodeDataIter)
              {
                Node nodeBKK(_writer, "BKK");
                nodeBKK.attr("B30", bkgCodeDataIter->bkgCode);
                nodeBKK.convertAttr("Q3S", bkgCodeDataIter->numSeats);
              }
            }
          }
        }

        // iBDateFlightMap
        FlightFinderTrx::InboundDateFlightMap::const_iterator itIDFM =
            (itODFM->second)->iBDateFlightMap.begin();

        for (; itIDFM != (itODFM->second)->iBDateFlightMap.end(); ++itIDFM)
        {
          Node nodeIBL(_writer, "IBL");
          nodeIBL.convertAttr("D01", itIDFM->first.dateToString(YYYYMMDD, "-"));

          if (showFareBasisCode(flightFinderTrx, false))
          {
            PaxTypeFare* inBoundFare = getFrontPaxTypeFare(*(itIDFM->second));

            if (inBoundFare)
            {
              nodeIBL.attr("B50", inBoundFare->createFareBasis(flightFinderTrx, false));

              if (flightFinderTrx.avlInS1S3Request())
              {
                nodeIBL.convertAttr("P1V", itIDFM->second->onlyApplicabilityFound ? "F" : "T");
              }
            }
          }

          if (showSOPs(flightFinderTrx, false))
          {
            // inbound flights
            std::vector<FlightFinderTrx::SopInfo*>::const_iterator itIBFL =
                itIDFM->second->flightList.begin();

            for (; itIBFL != itIDFM->second->flightList.end(); ++itIBFL)
            {
              // inbound flights
              Node nodeFID(_writer, "FID");
              nodeFID.convertAttr("Q14", 1); // LEG = 1 for inbound
              nodeFID.convertAttr("Q15",
                                  getOriginalSopIndex(flightFinderTrx, 1, (*itIBFL)->sopIndex));
              {
                // bkg codes data
                std::vector<FlightFinderTrx::BookingCodeData>::const_iterator bkgCodeDataIter =
                    (*itIBFL)->bkgCodeDataVect.front().begin();

                for (; bkgCodeDataIter != (*itIBFL)->bkgCodeDataVect.front().end();
                     ++bkgCodeDataIter)
                {
                  Node nodeBKK(_writer, "BKK");
                  nodeBKK.attr("B30", bkgCodeDataIter->bkgCode);
                  nodeBKK.convertAttr("Q3S", bkgCodeDataIter->numSeats);
                }
              }
            }
          }
        }
      }
    }
  }
}

uint32_t
XMLShoppingResponse::getOriginalSopIndex(FlightFinderTrx& flightFinderTrx,
                                         const uint32_t legId,
                                         const uint32_t sopIndex)
{
  std::vector<ShoppingTrx::SchedulingOption>& sop = flightFinderTrx.legs()[legId].sop();
  return sop[sopIndex].originalSopId();
}

bool
XMLShoppingResponse::showFareBasisCode(FlightFinderTrx& flightFinderTrx, bool outbound)
{
  if (!flightFinderTrx.isBffReq())
    return true;

  if (outbound)
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_1 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_2 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_5)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else // inbound
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_3 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_4 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_6)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool
XMLShoppingResponse::showSOPs(FlightFinderTrx& flightFinderTrx, bool outbound)
{
  if (!flightFinderTrx.isBffReq())
    return true;

  if (outbound)
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_5)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else // inbound
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_6)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

PaxTypeFare*
XMLShoppingResponse::getFrontPaxTypeFare(const FlightFinderTrx::FlightDataInfo& flightInfo)
{
  if (!flightInfo.altDatesPaxTypeFareVect.empty())
  {
    return flightInfo.altDatesPaxTypeFareVect.front();
  }
  else
  {
    if (flightInfo.flightList.empty())
    {
      return nullptr;
    }
    else
    {
      return flightInfo.flightList.front()->paxTypeFareVect.front();
    }
  }
}
void
XMLShoppingResponse::truncateSolutionNumber()
{
  FlightFinderTrx& flightFinderTrx = dynamic_cast<FlightFinderTrx&>(_trx);
  uint32_t numberOfDatePairs = 0;
  uint32_t cfgNumberOfDatePairs = numberOfDatapair.getValue();
  uint32_t cfgNumberOfSchedulesPerDate = 0;

  FlightFinderTrx::OutBoundDateFlightMapI itODFM = flightFinderTrx.outboundDateflightMap().begin();

  for (; itODFM != flightFinderTrx.outboundDateflightMap().end(); ++itODFM)
  {
    numberOfDatePairs += itODFM->second->iBDateFlightMap.size();
  }

  // activate logic only when number of date pairs to high
  if (numberOfDatePairs > cfgNumberOfDatePairs)
  {
    // Limit number of schedules
    cfgNumberOfSchedulesPerDate = numberOfSchedulesPerDate.getValue();

    itODFM = flightFinderTrx.outboundDateflightMap().begin();

    for (; itODFM != flightFinderTrx.outboundDateflightMap().end(); ++itODFM)
    {
      std::vector<FlightFinderTrx::SopInfo*>& flightList = itODFM->second->flightInfo.flightList;

      if (flightList.size() > cfgNumberOfSchedulesPerDate)
      {
        flightList.erase(flightList.begin() + cfgNumberOfSchedulesPerDate, flightList.end());
      }

      FlightFinderTrx::InboundDateFlightMap::iterator itIDFM =
          itODFM->second->iBDateFlightMap.begin();

      for (; itIDFM != (itODFM->second)->iBDateFlightMap.end(); ++itIDFM)
      {
        std::vector<FlightFinderTrx::SopInfo*>& flightList = itIDFM->second->flightList;

        if (flightList.size() > cfgNumberOfSchedulesPerDate)
        {
          flightList.erase(flightList.begin() + cfgNumberOfSchedulesPerDate, flightList.end());
        }
      }
    }
  }
}
uint32_t
XMLShoppingResponse::countFFSolutionNumber()
{
  FlightFinderTrx& flightFinderTrx = dynamic_cast<FlightFinderTrx&>(_trx);
  uint32_t numberOfSolutions = 0;
  uint32_t numberOfOutbounds = 0;
  uint32_t numberOfOutboundFlights = 0;
  uint32_t numberOfInbounds;
  uint32_t numberOfInboundFlights;

  if (!flightFinderTrx.outboundDateflightMap().empty())
  {
    FlightFinderTrx::OutBoundDateFlightMapI itODFM =
        flightFinderTrx.outboundDateflightMap().begin();

    for (; itODFM != flightFinderTrx.outboundDateflightMap().end(); ++itODFM)
    {
      // outbounds
      ++numberOfOutbounds;
      // outbound flights
      numberOfOutboundFlights = itODFM->second->flightInfo.flightList.size();
      numberOfInbounds = 0;
      numberOfInboundFlights = 0;
      FlightFinderTrx::InboundDateFlightMap::iterator itIDFM =
          itODFM->second->iBDateFlightMap.begin();

      for (; itIDFM != (itODFM->second)->iBDateFlightMap.end(); ++itIDFM)
      {
        // outbounds
        ++numberOfInbounds;
        // inbound flights
        numberOfInboundFlights += itIDFM->second->flightList.size();
      }

      // step 1 & 3 - count flights
      // round trip
      if (0 != numberOfOutboundFlights && 0 != numberOfInboundFlights)
      {
        numberOfSolutions += numberOfOutboundFlights * numberOfInboundFlights;
      }
      // one way
      else if (0 != numberOfOutboundFlights)
      {
        numberOfSolutions += numberOfOutboundFlights;
      }

      // step 2 - count inbounds
      if (0 == numberOfOutboundFlights && 0 == numberOfInboundFlights)
      {
        // round trip
        if (0 != numberOfInbounds)
        {
          numberOfSolutions += numberOfInbounds;
        }
      }
    }

    // step 2 - count outbounds
    // one way
    if (0 == numberOfSolutions)
    {
      numberOfSolutions = numberOfOutbounds;
    }
  }

  return numberOfSolutions;
}

size_t
XMLShoppingResponse::numberOfSeatRemain(const FarePath& farePath,
                                        const BookingCode& bkgCode,
                                        const TravelSeg& travelSeg,
                                        int16_t segOrder)
{
  if (ClassOfService* cos = finalSegCos(farePath, segOrder))
    return cos->numSeats();

  typedef std::vector<ClassOfService*>::const_iterator cosIt;
  cosIt cosBegin(travelSeg.classOfService().begin()), cosEnd(travelSeg.classOfService().end());

  cosIt cos(cosBegin);
  for (; cos != cosEnd; ++cos)
  {
    if (bkgCode == (*cos)->bookingCode())
    {
      return (*cos)->numSeats();
    }
  }

  return 0;
}

ClassOfService*
XMLShoppingResponse::finalSegCos(const FarePath& farePath, int16_t segOrder)
{
  if (segOrder <= 0 || size_t(segOrder) > farePath.finalBooking().size())
    return nullptr;

  return farePath.finalBooking()[segOrder - 1];
}

void
XMLShoppingResponse::updateItinCosForSeatRemain(const Itin& itin, const FarePath& path)
{
  std::vector<TravelSeg*> travelSegVec;

  for (const auto tvlSeg : itin.travelSeg())
  {
    travelSegVec.push_back(tvlSeg);
    std::vector<PricingUnit*>::const_iterator pu = path.pricingUnit().begin();
    std::vector<PricingUnit*>::const_iterator puEnd = path.pricingUnit().end();
    bool matchTvl = false;

    for (; ((pu != puEnd) && (matchTvl == false)); ++pu)
    {
      for (const auto fareUsage : (*pu)->fareUsage())
      {
        const size_t index =
            std::find(fareUsage->travelSeg().begin(), fareUsage->travelSeg().end(), tvlSeg) -
            fareUsage->travelSeg().begin();

        if (index >= fareUsage->travelSeg().size())
        {
          continue;
        }

        matchTvl = true;

        if (tvlSeg->segmentType() == Arunk)
          continue;

        if (fareUsage->segmentStatus()[index]._bkgCodeSegStatus.isSet(
                PaxTypeFare::BKSS_AVAIL_BREAK))
        {
          saveClassOfServiceFromAvailMap(travelSegVec, itin);

          travelSegVec.clear();
        }
      }
    }
  }
}

void
XMLShoppingResponse::saveClassOfServiceFromAvailMap(const std::vector<TravelSeg*>& travelSegVec,
                                                    const Itin& itin)
{
  uint64_t avlKey = ShoppingUtil::buildAvlKey(travelSegVec);
  AvailabilityMap::iterator foundAvl = _trx.availabilityMap().find(avlKey);

  if (foundAvl != _trx.availabilityMap().end())
  {
    std::vector<std::vector<ClassOfService*>>* srcAvail = foundAvl->second;
    size_t idx = 0;

    for (const auto itinTravelSeg : itin.travelSeg())
    {
      if (idx >= travelSegVec.size())
        break;

      if (travelSegVec[idx] == (itinTravelSeg))
      {
        itinTravelSeg->classOfService().clear();
        itinTravelSeg->classOfService() = (*srcAvail)[idx++];
      }
    }
  }
}

void
XMLShoppingResponse::generateQ18(Node& nodeFDC,
                                 const uint16_t paxProcOrder,
                                 const FareUsage& fareUsage)
{
  if (UNLIKELY(_trx.getRequest()->fareGroupRequested() && !_trx.posPaxType()[paxProcOrder].empty()))
  {
    nodeFDC.convertAttr("Q18", (fareUsage.paxTypeFare()->actualPaxTypeItem())[paxProcOrder]);
  }
  else if (UNLIKELY(!_trx.posPaxType().empty() && !_trx.posPaxType()[paxProcOrder].empty()))
  {
    std::vector<PosPaxType*>& posPaxTypeVec = _trx.posPaxType()[paxProcOrder];
    std::vector<PosPaxType*>::iterator posPaxType = posPaxTypeVec.begin();
    uint16_t fgNumber = (*posPaxType)->fgNumber(); // default to the first one in the group

    for (; posPaxType != posPaxTypeVec.end(); ++posPaxType)
    {
      if ((*posPaxType)->paxType() == fareUsage.paxTypeFare()->paxType()->paxType())
      {
        if ((*posPaxType)->positive() == false || (*posPaxType)->pcc().empty() == true)
        {
          fgNumber = (*posPaxType)->fgNumber();
        }

        if (fareUsage.paxTypeFare()->isWebFare())
        {
          if ((*posPaxType)->positive() && (*posPaxType)->pcc().empty() == false)
          {
            fgNumber = (*posPaxType)->fgNumber();
            break;
          }
        }
        else
        {
          if ((*posPaxType)->positive() == false || (*posPaxType)->pcc().empty() == true)
          {
            fgNumber = (*posPaxType)->fgNumber();
            break;
          }
        }
      }
    }

    nodeFDC.convertAttr("Q18", fgNumber);
  }
}

PaxTypeFare::SegmentStatus*
XMLShoppingResponse::getDiffSegStatus(const FareUsage& fareUsage, const TravelSeg& travelSeg)
{
  std::vector<DifferentialData*>::const_iterator differentialIter =
      fareUsage.differentialPlusUp().begin();
  std::vector<DifferentialData*>::const_iterator differentialIterEnd =
      fareUsage.differentialPlusUp().end();
  PaxTypeFare::SegmentStatus* segStatus = nullptr;

  for (; differentialIter != differentialIterEnd; differentialIter++)
  {
    DifferentialData::STATUS_TYPE aStatus = (*differentialIter)->status();

    if (aStatus != DifferentialData::SC_PASSED && aStatus != DifferentialData::SC_CONSOLIDATED_PASS)
      continue;

    if (!(*differentialIter))
      continue;

    std::vector<PaxTypeFare::SegmentStatus>::iterator diffSegStatusIter =
        (*differentialIter)->fareHigh()->segmentStatus().begin();
    std::vector<PaxTypeFare::SegmentStatus>::iterator diffSegStatusIterEnd =
        (*differentialIter)->fareHigh()->segmentStatus().end();
    std::vector<TravelSeg*>::iterator diffTravelSegIter = (*differentialIter)->travelSeg().begin();
    std::vector<TravelSeg*>::iterator diffTravelSegIterEnd = (*differentialIter)->travelSeg().end();

    for (;
         (diffSegStatusIter != diffSegStatusIterEnd) && (diffTravelSegIter != diffTravelSegIterEnd);
         ++diffSegStatusIter, ++diffTravelSegIter)
    {
      if ((*diffTravelSegIter == &travelSeg) && !(*diffSegStatusIter)._bkgCodeReBook.empty())
      {
        segStatus = &(*diffSegStatusIter);
        break;
      }
    }
  }

  return segStatus;
}

bool
XMLShoppingResponse::isDifferentialFound(const FareUsage& fareUsage,
                                         Node& nodeBKC,
                                         const TravelSeg& travelSeg,
                                         BookingCode& bookingCode,
                                         const FareBreakPointInfo& breakPoint,
                                         bool displayFareBasisCode)
{
  PaxTypeFare::SegmentStatus* segStatus = getDiffSegStatus(fareUsage, travelSeg);

  if (segStatus)
  {
    nodeBKC.attr("B30", segStatus->_bkgCodeReBook)
        .convertAttr("Q13", segStatus->_reBookCabin.getCabinIndicator())
        .convertAttr("B31", segStatus->_reBookCabin.getClassAlphaNum(_rbdByCabinVSinHouse));

    if (displayFareBasisCode)
    {
      nodeBKC.attr("B52", breakPoint.fareBasisCode);
    }

    bookingCode = segStatus->_bkgCodeReBook;
  }

  return segStatus;
}

bool
XMLShoppingResponse::isJcbItin(const GroupFarePath& path)
{
  FPPQItem* it = path.groupFPPQItem().front();
  FarePath* farePath = it->farePath();

  for (const auto pricingUnit : farePath->pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      const FareMarket* fareMarket = fareUsage->paxTypeFare()->fareMarket();

      if ((fareMarket == nullptr) || (fareMarket->isJcb() == false))
      {
        return false;
      }
    }
  }

  return true;
}

bool
XMLShoppingResponse::getFeeRounding(PricingTrx& pricingTrx,
                                    const CurrencyCode& currencyCode,
                                    RoundingFactor& roundingFactor,
                                    CurrencyNoDec& roundingNoDec,
                                    RoundingRule& roundingRule)
{
  const DateTime& tickDate = pricingTrx.ticketingDate();
  const Currency* currency = nullptr;
  currency = pricingTrx.dataHandle().getCurrency(currencyCode);

  if (!currency)
  {
    LOG4CXX_ERROR(_logger, "DBAccess getCurrency returned null currency pointer");
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
  LOG4CXX_INFO(_logger, "Currency country description: " << currency->controllingEntityDesc());
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

namespace
{
struct ValidationFlags
{
  mutable bool highest;
  mutable bool notApplicable;
  mutable bool waived;

  ValidationFlags() : highest(false), notApplicable(false), waived(false) {}
};
struct ComparablePenaltyFee : public PenaltyFee, public ValidationFlags
{
  RexPricingTrx& _trx;

  virtual ~ComparablePenaltyFee() {}

  ComparablePenaltyFee(RexPricingTrx& trx) : _trx(trx) {}

  ComparablePenaltyFee(const ComparablePenaltyFee& rhs, RexPricingTrx& trx)
    : PenaltyFee(rhs), ValidationFlags(rhs), _trx(trx)
  {
  }

  ComparablePenaltyFee(const ComparablePenaltyFee& rhs)
    : PenaltyFee(rhs), ValidationFlags(rhs), _trx(rhs._trx)
  {
  }

  ComparablePenaltyFee(const PenaltyFee& rhs, RexPricingTrx& trx) : PenaltyFee(rhs), _trx(trx) {}

  ComparablePenaltyFee& operator=(const ComparablePenaltyFee& rhs)
  {
    PenaltyFee::operator=(rhs);
    ValidationFlags::operator=(rhs);
    return *this;
  }

  ComparablePenaltyFee& operator=(const PenaltyFee& rhs)
  {
    PenaltyFee::operator=(rhs);
    return *this;
  }

  bool operator>(const PenaltyFee& rhs) const
  {
    if (this == &rhs)
      return false;

    ComparablePenaltyFee& lhs((ComparablePenaltyFee&)*this);
    return ReissuePenaltyCalculator::isPenalty1HigherThanPenalty2(
        (PenaltyFee&)lhs, (PenaltyFee&)rhs, ((PenaltyFee&)lhs).penaltyCurrency, _trx);
  }
  bool operator>(const ComparablePenaltyFee& rhs) const
  {
    if (this == &rhs)
      return false;

    ComparablePenaltyFee& lhs((ComparablePenaltyFee&)*this);
    return ReissuePenaltyCalculator::isPenalty1HigherThanPenalty2(
        (PenaltyFee&)lhs, (PenaltyFee&)rhs, ((PenaltyFee&)lhs).penaltyCurrency, _trx);
  }
  bool operator<(const ComparablePenaltyFee& rhs) const
  {
    if (this == &rhs)
      return false;

    ComparablePenaltyFee& lhs((ComparablePenaltyFee&)*this);
    return rhs.operator>(lhs);
  }
};
}

void
XMLShoppingResponse::generateCHG(RexPricingTrx& trx, CalcTotals& calcTotals)
{
  const FarePath* farePathPtr = calcTotals.farePath;

  if (!farePathPtr)
    return;

  const FarePath& farePath = *farePathPtr;

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
    const ReissueCharges* reissueCharges(farePath.reissueCharges());

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
        const std::map<const PaxTypeFare*, PenaltyFee*>& penaltyFees(reissueCharges->penaltyFees);
        std::map<const PaxTypeFare*, PenaltyFee*>::const_iterator pfi(penaltyFees.begin());
        std::map<const PaxTypeFare*, PenaltyFee*>::const_iterator pfe(penaltyFees.end());

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

    std::multiset<ComparablePenaltyFee>::iterator i = rated.begin();
    std::multiset<ComparablePenaltyFee>::iterator e = rated.end();

    for (; i != e; ++i)
    {
      const ComparablePenaltyFee& fee(*i);
      CurrencyNoDec noDec = tse::Money::NUC_DECIMALS;

      if (fee.penaltyCurrency != NUC)
      {
        const tse::Currency* currency = nullptr;
        currency = trx.dataHandle().getCurrency(fee.penaltyCurrency);

        if (currency)
          noDec = currency->noDec();
      }

      Node nodeCHG(_writer, "CHG");
      nodeCHG.convertAttr("PXJ", fee.highest ? 'T' : 'F')
          .attr("C77", formatAmount(fee.penaltyAmount, noDec))
          .attr("C78", fee.penaltyCurrency)
          .convertAttr("PXL", fee.notApplicable ? 'T' : 'F')
          .convertAttr("PXK", fee.waived ? 'T' : 'F');
    }
  }
}

void
XMLShoppingResponse::electronicTicketInd(const Indicator& electronicTicketIndicator, Node& nodeREX)
{
  if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_BLANK)
  {
    nodeREX.convertAttr("PXY", 'F');
    nodeREX.convertAttr("PXZ", 'F');
  }
  else if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED)
  {
    nodeREX.convertAttr("PXY", 'T');
    nodeREX.convertAttr("PXZ", 'F');
  }
  else if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED)
  {
    nodeREX.convertAttr("PXY", 'F');
    nodeREX.convertAttr("PXZ", 'T');
  }
}

void
XMLShoppingResponse::formOfRefundInd(const ProcessTagPermutation& winnerPerm, Node& nodeREX)
{
  std::vector<ProcessTagInfo*>::const_iterator ptIter = winnerPerm.processTags().begin(),
                                               ptIterEnd = winnerPerm.processTags().end();
  char formOfRefundInd = BLANK;
  char mostRestrictiveFormOfRefund = BLANK;

  for (; ptIter != ptIterEnd; ++ptIter)
  {
    formOfRefundInd = (*ptIter)->record3()->formOfRefund();

    if (formOfRefundInd == 'S')
    {
      nodeREX.convertAttr("N20", formOfRefundInd);
      return;
    }
    else if (formOfRefundInd == 'V')
      mostRestrictiveFormOfRefund = 'V';
    else if (formOfRefundInd == 'M')
    {
      if (mostRestrictiveFormOfRefund != 'V')
        mostRestrictiveFormOfRefund = 'M';
    }
  }

  nodeREX.convertAttr("N20", mostRestrictiveFormOfRefund);
}

void
XMLShoppingResponse::generateREX(CalcTotals& calc)
{
  if (_trx.isExchangeTrx())
  {
    if (!calc.farePath->lowestFee31Perm())
      return; // temporary waiting on PO bug fix

    Node nodeREX(_writer, "REX");
    Indicator resPenIndicator = calc.farePath->residualPenaltyIndicator(*_rexTrx);

    if (resPenIndicator != ' ')
      nodeREX.convertAttr("N1X", resPenIndicator);

    const ProcessTagPermutation& winnerPerm = *calc.farePath->lowestFee31Perm();
    formOfRefundInd(winnerPerm, nodeREX);
    const char isTag7Permutation = winnerPerm.hasTag7only() ? 'T' : 'F';
    nodeREX.convertAttr("PXM", isTag7Permutation);
    electronicTicketInd(winnerPerm.electronicTicket(), nodeREX);
    generateCHG(*_rexTrx, calc);
  }
}

void
XMLShoppingResponse::generateSEG(const std::vector<CalcTotals*>& calcTotals, const Itin* itin)
{
  // format P2H, P2M connection or stopover segment
  std::vector<CalcTotals*>::const_iterator calcForPax = calcTotals.begin();

  if ((_trx.getOptions()->isXoFares()) || (_trx.getOptions()->forceCorpFares()))
  {
    std::vector<FarePath*>::const_iterator paxPath = itin->farePath().begin();

    for (; calcForPax != calcTotals.end(); ++calcForPax)
    {
      if ((*calcForPax)->farePath->paxType() == (*paxPath)->paxType())
      {
        break;
      }
    }
  }

  int index = 0;

  for (std::vector<TravelSeg*>::const_iterator i = itin->travelSeg().begin();
       i != itin->travelSeg().end();
       ++i, ++index)
  {
    const FareUsage* currentFareUsage = (*calcForPax)->getFareUsage(*i);

    if (UNLIKELY(true == _trx.snapRequest()))
    {
      if ((currentFareUsage == nullptr) && ((*i)->segmentType() == Arunk))
      {
        continue;
      }
    }

    Node nodeSEG(_writer, "SEG");
    FareBreakPointInfo breakPoint;

    if (currentFareUsage != nullptr)
    {
      breakPoint = (*calcForPax)->getFareBreakPointInfo(currentFareUsage);

      if (((*calcForPax)->extraMileageTravelSegs.find(*i) !=
           (*calcForPax)->extraMileageTravelSegs.end()) ||
          ((*calcForPax)->extraMileageFareUsages.find(currentFareUsage) !=
           (*calcForPax)->extraMileageFareUsages.end()))
      {
        nodeSEG.convertAttr("P2I", 'T');
      }
    }
    else if ((*i)->segmentType() == Arunk)
    {
      nodeSEG.convertAttr("S09", 'T');
    }

    bool isStopOver = false;

    std::vector<bool> stopOvers =
        TravelSegUtil::calculateStopOvers(itin->travelSeg(), itin->geoTravelType());
    int16_t segIndex = itin->segmentOrder(*i);

    if (UNLIKELY(segIndex == -1))
    {
      isStopOver = (*i)->stopOver();
    }
    else
    {
      isStopOver = stopOvers[segIndex - 1];
    }

    if (isStopOver)
    {
      nodeSEG.convertAttr("P2M", 'T');
    }
    else
    {
      nodeSEG.convertAttr("P2H", 'T');
    }

    if ((*i)->segmentType() == Arunk)
    {
      nodeSEG.convertAttr("S10", 'T');
    }
    else if (LIKELY(currentFareUsage != nullptr))
    {
      // format return booking code
      Node nodeBKC(_writer, "BKC");
      bool diffFound = false;

      {
        BookingCode bookingCode;
        bool displayFareBasisCode = false;
        diffFound = isDifferentialFound(
            *currentFareUsage, nodeBKC, **i, bookingCode, breakPoint, displayFareBasisCode);
      }

      if (false == diffFound)
      {
        if (!(*calcForPax)->bookingCodeRebook[index].empty())
        {
          nodeBKC.attr("B30", (*calcForPax)->bookingCodeRebook[index]);
          // get cabin from fu->segmentstatus
          std::vector<TravelSeg*>::const_iterator fuTvlSeg = currentFareUsage->travelSeg().begin();
          std::vector<TravelSeg*>::const_iterator fuTvlSegEnd = currentFareUsage->travelSeg().end();
          std::vector<PaxTypeFare::SegmentStatus>::const_iterator segmentStatus =
              currentFareUsage->segmentStatus().begin();

          for (; fuTvlSeg != fuTvlSegEnd; ++fuTvlSeg, ++segmentStatus)
          {
            if (*i == *fuTvlSeg)
            {
              if (LIKELY(segmentStatus->_bkgCodeReBook.empty() == false))
              {
                nodeBKC.convertAttr("Q13", segmentStatus->_reBookCabin.getCabinIndicator())
                    .convertAttr(
                        "B31", segmentStatus->_reBookCabin.getClassAlphaNum(_rbdByCabinVSinHouse));
              }
              else
              {
                nodeBKC.convertAttr("Q13", (*i)->bookedCabin().getCabinIndicator())
                    .convertAttr("B31", (*i)->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
              }

              break;
            }
          }

          if (UNLIKELY(fuTvlSeg == fuTvlSegEnd))
          {
            nodeBKC.convertAttr("Q13", (*i)->bookedCabin().getCabinIndicator())
                .convertAttr("B31", (*i)->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
          }
        }
        else // booking code rebook is empty
        {
          nodeBKC.attr("B30", (*i)->getBookingCode())
              .convertAttr("Q13", (*i)->bookedCabin().getCabinIndicator())
              .convertAttr("B31", (*i)->bookedCabin().getClassAlphaNum(_rbdByCabinVSinHouse));
        }
      }
    } // else
  } // for
} // SEG

void
XMLShoppingResponse::prepareUnflownItinPriceInfo()
{
  RexPricingTrx* rexTrx = dynamic_cast<RexPricingTrx*>(&_trx);

  if (rexTrx == nullptr)
    return;

  if (!(*rexTrx).isCSOSolutionValid() || (*rexTrx).pricingTrxForCSO()->fareCalcCollector().empty())
    return;

  CalcTotals* calcTotal = (*rexTrx).pricingTrxForCSO()->fareCalcCollector().front()->findCalcTotals(
      rexTrx->lowestCSOFarePath());

  if (!calcTotal)
    return;

  Money moneyEquiv(calcTotal->equivCurrencyCode);
  Node nodeUFL(_writer, "UFL");
  nodeUFL.attr("C46", calcTotal->equivCurrencyCode);
  nodeUFL.attr("C66",
               formatAmount(calcTotal->equivFareAmount + calcTotal->taxAmount(),
                            moneyEquiv.noDec(rexTrx->ticketingDate())));
}

void
XMLShoppingResponse::generatePSG(FareCalcCollector& calcCollector,
                                 const Itin* itin,
                                 const FarePath* path,
                                 CalcTotals* totals,
                                 bool& minMaxCatFound,
                                 MoneyAmount& totalConstructAmount)
{
  Node nodePSG(_writer, "PSG");
  if (TrxUtil::isCat35TFSFEnabled(_trx) && totals->netCalcTotals && path->netFarePath())
    generateMainPSG(nodePSG,
                    calcCollector,
                    itin,
                    path->netFarePath(),
                    totals->netCalcTotals,
                    minMaxCatFound,
                    totalConstructAmount);
  else
    generateMainPSG(
        nodePSG, calcCollector, itin, path, totals, minMaxCatFound, totalConstructAmount);

  // OB Fees
  generateOBG(*totals);
  // currency conversion
  generateCCD(_trx, *totals);
  generateMSG(itin, totals);
  generateREX(*totals);

  if (const MaxPenaltyResponse* maxPenaltyResponse = path->maxPenaltyResponse())
  {
    generatePEN(*maxPenaltyResponse, _trx.ticketingDate(), *path);
  }

  if (isMIPResponse() && _trx.getRequest()->isProcessBaggageForMIP())
    _baggageResponse.generateBDI(*path);
}

Indicator
XMLShoppingResponse::getCommissionSourceIndicator(const CalcTotals& calcTotals)
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
  else if (_trx.getRequest()->ticketingAgent() &&
           !_trx.getRequest()->ticketingAgent()->agentCommissionType().empty())
  {
    commSrcIndicator = 'M'; // Manual Commission
  }
  return commSrcIndicator;
}

void
XMLShoppingResponse::constructElementVCCForCat35(const FarePath& fp, const CurrencyNoDec noDec)
{
  Node nodeVCC(_writer, "VCC");

  const CollectedNegFareData* cNegFareData = fp.collectedNegFareData();
  if (cNegFareData && (fp.commissionAmount() - cNegFareData->markUpAmount() > 0 ||
                       fabs(fp.commissionAmount() - cNegFareData->markUpAmount()) <= EPSILON))
  {
    // Create C58 - Total Commission only
    nodeVCC.attr(xml2::TotalCommissionAmount,
                 formatAmount(fp.commissionAmount() - cNegFareData->markUpAmount(), noDec));
  }
  // Create C60 - Total Commission amount including Mark-Up
  nodeVCC.attr(xml2::Cat35CommissionAmount, formatAmount(fp.commissionAmount(), noDec));
  // Create C61 - Only Cat 35 Commission Percent
  if (fp.commissionPercent() <= HUNDRED)
  {
    const uint16_t Cat35CommissionPercentageNoDec = 2;
    nodeVCC.attr(xml2::Cat35CommissionPercentage,
                 formatAmount(fp.commissionPercent(), Cat35CommissionPercentageNoDec));
  }
  else
  {
    nodeVCC.convertAttr(xml2::Cat35CommissionPercentage, fp.commissionPercent());
  }
}

void
XMLShoppingResponse::generateCommissionInfo(Node& nodePSG,
                                            const FarePath* farePath,
                                            const CalcTotals& calcTotals,
                                            FuFcIdMap& fuFcIdCol)
{
  const DateTime& ticketingDate = _trx.ticketingDate();
  Money moneyEquiv(calcTotals.equivCurrencyCode);
  const int noDec = moneyEquiv.noDec(ticketingDate);

  if (!fallback::fallbackCommissionManagement(&_trx) && !fallback::fallbackASLDisplayC57(&_trx))
    prepareMarkupAndCommissionAmount(nodePSG, *farePath, noDec);

  if (!fallback::fallbackAMCPhase2(&_trx) && !fallback::fallbackAMC2ShoppingChange(&_trx))
  {
    Indicator commSrcIndicator = getCommissionSourceIndicator(calcTotals);
    if (commSrcIndicator != ' ')
    {
      nodePSG.convertAttr(xml2::CommissionSourceIndicator, commSrcIndicator);

      if (commSrcIndicator == 'A')
      {
        CarrierCode defValCxr =
            _trx.isValidatingCxrGsaApplicable()
                ? farePath->defaultValidatingCarrier()
                : (farePath->itin() ? farePath->itin()->validatingCarrier() : "");
        prepareCommissionForValidatingCarriers(farePath, defValCxr, noDec, fuFcIdCol);
      }
      else if (commSrcIndicator == 'C')
      {
        const int Cat35CommissionPercentageNoDec = 2;
        nodePSG.attr(xml2::Cat35CommissionPercentage,
                     formatAmount(farePath->commissionPercent(), Cat35CommissionPercentageNoDec));
        nodePSG.attr(xml2::Cat35CommissionAmount,
                     formatAmount(farePath->commissionAmount(), noDec));

        if (fallback::fallbackASLDisplayC57(&_trx))
          prepareMarkupAndCommissionAmount(nodePSG, *farePath, noDec);

        if (!fallback::fallbackAMC2Cat35CommInfo(&_trx))
          constructElementVCCForCat35(*calcTotals.farePath, noDec);
      }
    }
  }
  else
  {
    if (farePath->collectedNegFareData() == nullptr)
      return;

    const int Cat35CommissionPercentageNoDec = 2;

    const CollectedNegFareData& negFareData = *(farePath->collectedNegFareData());

    if (negFareData.indicatorCat35())
    {
      nodePSG.attr(xml2::Cat35CommissionPercentage,
                   formatAmount(farePath->commissionPercent(), Cat35CommissionPercentageNoDec));

      nodePSG.attr(xml2::Cat35CommissionAmount, formatAmount(farePath->commissionAmount(), noDec));
      prepareMarkupAndCommissionAmount(nodePSG, *farePath, noDec);
    }

    if (!farePath->valCxrCommissionAmount().empty() &&
        (!calcTotals.farePath->collectedNegFareData() ||
         !calcTotals.farePath->collectedNegFareData()->indicatorCat35()) /*non cat35 fares*/)
    {
      CarrierCode defValCxr = _trx.isValidatingCxrGsaApplicable()
                                  ? farePath->defaultValidatingCarrier()
                                  : (farePath->itin() ? farePath->itin()->validatingCarrier() : "");

      if (fallback::fallbackAMCPhase2(&_trx))
      {
        if (farePath->doesValCarriersHaveDiffComm(defValCxr))
          nodePSG.attr(xml2::DifferentCommissionAmount, "T");
      }

      prepareCommissionForValidatingCarriers(farePath, defValCxr, noDec, fuFcIdCol);
    }
  }
}

void
XMLShoppingResponse::prepareMarkupAndCommissionAmount(Node& nodePSG,
                                                      const FarePath& fp,
                                                      const CurrencyNoDec noDec)
{
  const CollectedNegFareData* cNegFareData = fp.collectedNegFareData();
  if (!fallback::fallbackJira1908NoMarkup(&_trx))
  {
    if (cNegFareData)
    {
      nodePSG.attr(xml2::Cat35MarkupAmount, formatAmount(cNegFareData->markUpAmount(), noDec));
      if (fp.commissionAmount() - cNegFareData->markUpAmount() >= 0)
      {
        nodePSG.attr(xml2::TotalCommissionAmount,
                     formatAmount((fp.commissionAmount() - cNegFareData->markUpAmount()), noDec));
      }
    }
  }
  else
  {
    if (cNegFareData != nullptr &&
        (fp.commissionAmount() - cNegFareData->markUpAmount() > 0 ||
         fabs(fp.commissionAmount() - cNegFareData->markUpAmount()) <= EPSILON))
    {
      // fp.commissionAmount() must be same or more than cNegFareData->markUpAmount()
      nodePSG.attr(xml2::Cat35MarkupAmount, formatAmount(cNegFareData->markUpAmount(), noDec));
      nodePSG.attr(xml2::TotalCommissionAmount,
                   formatAmount((fp.commissionAmount() - cNegFareData->markUpAmount()), noDec));
    }
  }
}

void
XMLShoppingResponse::prepareCommissionForValidatingCarriers(const FarePath* farePath,
                                                            const CarrierCode& defValCxr,
                                                            const CurrencyNoDec noDec,
                                                            FuFcIdMap& fuFcIdCol)
{
  uint16_t fcId = 0;
  auto it = farePath->valCxrCommissionAmount().find(defValCxr);
  auto itEnd = farePath->valCxrCommissionAmount().end();
  if (it != itEnd)
    constructElementVCC(*farePath, defValCxr, it->second, noDec, fuFcIdCol, fcId);

  // Are there commission for alt val-cxrs?
  if (farePath->valCxrCommissionAmount().size() > 1 || it == itEnd)
  {
    // Collect and group common commission alt val-cxr together
    for (const auto& p : farePath->valCxrCommissionAmount())
    {
      if (p.first != defValCxr)
        constructElementVCC(*farePath, p.first, p.second, noDec, fuFcIdCol, fcId);
    }
  }
}

void
XMLShoppingResponse::constructElementVCC(const FarePath& fp,
                                         const CarrierCode& cxrs,
                                         const MoneyAmount& commAmount,
                                         const CurrencyNoDec noDec,
                                         FuFcIdMap& fuFcIdCol,
                                         uint16_t& fcId)
{
  Node nodeVCC(_writer, "VCC");

  nodeVCC.attr(xml2::ValidatingCarrier, cxrs);
  nodeVCC.attr(xml2::TotalCommissionAmount, formatAmount(commAmount, noDec));

  if (!fallback::fallbackAMCPhase2(&_trx) && !fallback::fallbackAMC2ShoppingChange(&_trx))
  {
    // add VCC
    for (auto pu : fp.pricingUnit())
    {
      if (!pu)
        continue;

      for (auto fu : pu->fareUsage())
      {
        if (!fu)
          continue;

        // Do we have comm on FC?

        auto fcCommIt = fu->fcCommInfoCol().find(cxrs);
        auto fcCommItEnd = fu->fcCommInfoCol().end();
        if (fcCommIt != fcCommItEnd && fcCommIt->second)
        {
          uint16_t q6d = 0;
          auto it = fuFcIdCol.find(fu);
          if (it != fuFcIdCol.end())
          {
            q6d = it->second;
          }
          else
          {
            fuFcIdCol[fu] = ++fcId;
            q6d = fcId;
          }
          constructElementFCB(*fcCommIt->second, q6d, noDec);
        }
      }
    }
  }
}

void
XMLShoppingResponse::constructElementFCB(amc::FcCommissionData& fcCommInfo,
                                         uint16_t q6d,
                                         const CurrencyNoDec noDec)
{
  Node nodeFCB(_writer, "FCB");
  nodeFCB.convertAttr(xml2::FareComponentNumber, q6d); // "Q6D",

  nodeFCB.attr(xml2::TotalCommissionAmount, formatAmount(fcCommInfo.fcCommAmt(), noDec)); // "C58",

  if (fcCommInfo.commRuleData().commRuleInfo())
    nodeFCB.convertAttr(xml2::CommissionRuleId,
                        fcCommInfo.commRuleData().commRuleInfo()->commissionId()); // "C3R",

  if (fcCommInfo.commRuleData().commProgInfo())
    nodeFCB.convertAttr(xml2::CommissionProgramId,
                        fcCommInfo.commRuleData().commProgInfo()->programId()); // "C3P",

  if (fcCommInfo.commRuleData().commContInfo())
    nodeFCB.convertAttr(xml2::CommissionContractId,
                        fcCommInfo.commRuleData().commContInfo()->contractId()); // "C3C",
}

void
XMLShoppingResponse::generateMainPSG(Node& node,
                                     FareCalcCollector& calcCollector,
                                     const Itin* itin,
                                     const FarePath* origPath,
                                     CalcTotals* total,
                                     bool& minMaxCatFound,
                                     MoneyAmount& totalConstructAmount)
{
  CalcTotals* totals = total;
  const FarePath* path = origPath;

  if (totals->adjustedCalcTotal && _trx.getOptions() &&
      !_trx.getOptions()->isPDOForFRRule()) // not ORG
  {
    totals = total->adjustedCalcTotal;
    path = origPath->adjustedSellingFarePath();
  }

  Money constructionCurrency(totals->farePath->itin()->calculationCurrency());
  constructionCurrency.setCode(totals->farePath->calculationCurrency());
  int16_t constructionDecimals = constructionCurrency.noDec(_trx.ticketingDate());

  generatePSGAttr(calcCollector, itin, path, totals, node, totalConstructAmount);
  checkDuplicateTvlSeg(*path);
  updateItinCosForSeatRemain(*itin, *path);
  callGenerateFDC(itin, path, totals, minMaxCatFound);

  for (PricingUnit* pu : path->pricingUnit())
  {
    preparePricingUnitPlusUps(*pu, constructionDecimals, _trx);
  }

  prepareFarePathPlusUps(*path, constructionDecimals);
  const TaxResponse* taxResponse = TaxResponse::findFor(itin, path);
  generateTAD(taxResponse, *totals);
  generateSellingFareData(total);
}

void
XMLShoppingResponse::getTTypeFeePerPax(CalcTotals& calcTotal,
                                       const FarePath* path,
                                       MoneyAmount& tTypeFeeTotal)
{
  CurrencyCode paymentCurrency = (calcTotal.equivCurrencyCode != "")
                                     ? calcTotal.equivCurrencyCode
                                     : calcTotal.convertedBaseFareCurrencyCode;

  tTypeFeeTotal = 0;
  for (TicketingFeesInfo* feeInfo : path->collectedTTypeOBFee())
  {
    MoneyAmount currentObFeeAmount = 0;
    if (feeInfo->feePercent() > 0)
    {
      currentObFeeAmount =
          calculateObFeeAmountFromPercentage(_trx, calcTotal, feeInfo, paymentCurrency);
    }
    else
    {
      currentObFeeAmount = calculateObFeeAmountFromAmount(_trx, feeInfo, paymentCurrency);
    }

    getAdjustedObFeeAmount(currentObFeeAmount);
    tTypeFeeTotal += currentObFeeAmount;
  }
}

void
XMLShoppingResponse::generatePSGAttr(FareCalcCollector& calcCollector,
                                     const Itin* itin,
                                     const FarePath* path,
                                     CalcTotals* totals,
                                     Node& nodePSG,
                                     MoneyAmount& totalConstructAmount)
{
  CurrencyCode equivCurrency(totals->equivCurrencyCode);
  Money equivCurr(equivCurrency);
  int16_t equivDecimals(equivCurr.noDec(_trx.ticketingDate()));
  CurrencyCode origCurrency(totals->convertedBaseFareCurrencyCode);
  int origDecimals(totals->convertedBaseFareNoDec);
  Money constructionCurrency(totals->farePath->itin()->calculationCurrency());
  constructionCurrency.setCode(totals->farePath->calculationCurrency());
  int16_t constructionDecimals = constructionCurrency.noDec(_trx.ticketingDate());
  MoneyAmount totalEquivAmountPerPax = 0.0, totalOrigAmountPerPax = 0.0, totalTaxesPerPax = 0.0;
  MoneyAmount commissionPercentPerPax = 0.0, commissionAmountPerPax = 0.0,
              totalConstructAmountPerPax = 0.0;
  std::string usePaxType;
  const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(_trx);

  if (fcConfig == nullptr || fcConfig->truePsgrTypeInd() == YES)
  {
    usePaxType = totals->truePaxType;
  }
  else
  {
    usePaxType = totals->requestedPaxType;
  }

  int16_t numPax = totals->farePath->paxType()->number();
  totalEquivAmountPerPax += totals->equivFareAmount;
  totalOrigAmountPerPax += totals->convertedBaseFare;
  totalTaxesPerPax += totals->taxAmount();
  commissionPercentPerPax = totals->farePathInfo.commissionPercent;
  commissionAmountPerPax = totals->farePathInfo.commissionAmount;
  totalConstructAmountPerPax += totals->farePath->getTotalNUCAmount();
  totalConstructAmount += totalConstructAmountPerPax * numPax;

  if (equivCurrency == origCurrency)
  {
    equivDecimals = origDecimals;
    totalEquivAmountPerPax = totalOrigAmountPerPax;
  }

  nodePSG.convertAttr("Q0U", path->paxType()->number())
      .convertAttr("Q79", path->paxType()->totalPaxNumber())
      .convertAttr("B70", usePaxType)
      .attr("C43", totals->farePath->calculationCurrency())
      .attr("C5E", formatAmount(totalConstructAmountPerPax, constructionDecimals))
      .attr("C40", origCurrency)
      .attr("C5A", formatAmount(totalOrigAmountPerPax, origDecimals))
      .attr("C46", equivCurrency)
      .attr("C66", formatAmount(totalEquivAmountPerPax + totalTaxesPerPax, equivDecimals))
      .attr("C65", formatAmount(totalTaxesPerPax, equivDecimals))
      .attr("S07", totals->fareCalculationLine);
  // format private fare
  std::vector<std::string> privateIndPerPax;
  generatePrivateInd(*itin, calcCollector, privateIndPerPax);
  size_t paxNo = 0;

  if ((_trx.paxType().size() > paxNo) && (privateIndPerPax[paxNo] != " "))
    nodePSG.convertAttr("N1U", privateIndPerPax[paxNo]);

  // check if there is COP - country of payment minimum fare (LCM) plus up exists
  for (const auto pricingUnit : path->pricingUnit())
  {
    if (pricingUnit->minFarePlusUp().count(COP) != 0)
    {
      nodePSG.attr("C44", origCurrency)
          .attr("C5C", formatAmount(totalOrigAmountPerPax, origDecimals));
      break;
    }
  }

  uint16_t stopoverCount = 0;
  MoneyAmount stopoverCharges = 0;
  CurrencyCode pubCurr = "NUC";
  bool stopoverFlag = totals->getStopoverSummary(stopoverCount, stopoverCharges, pubCurr);

  if (stopoverFlag)
  {
    nodePSG.attr("C63", formatAmount(stopoverCharges, totals->equivNoDec))
        .convertAttr("Q0X", stopoverCount);
  }

  // Calculation ROE
  if ((fcConfig->domesticNUC() == YES) || (totals->farePath->itin()->calculationCurrency() !=
                                           totals->farePath->itin()->originationCurrency()))
  {
    if (totals->roeRate > 0.0)
    {
      nodePSG.attr("C54", formatAmount(totals->roeRate, totals->roeRateNoDec))
          .convertAttr("Q05", totals->roeRateNoDec);
    }
  }

  // set up Nonrefundable indicator
  if (fallback::fixed::noNraAttrInShoppingResponse())
  {
    // when removing fallback remove also generateP27() function
    if (generateP27(*path))
    {
      nodePSG.convertAttr("P27", "T");
    }
    else
    {
      nodePSG.convertAttr("P27", "F");
    }
  }
  else
  {
    nodePSG.convertAttr("P27", CommonParserUtils::isNonRefundable(*path) ? "T" : "F");
  }

  // setup S30
  if (true == _trx.posPaxType().empty() || true == _trx.posPaxType()[0].empty())
  {
    if (JCB == usePaxType)
    {
      nodePSG.convertAttr("S30", "BLK");
    }
  }

  // setup commission percentage and commission amount
  if (commissionPercentPerPax != RuleConst::PERCENT_NO_APPL)
  {
    nodePSG.convertAttr("C5D", formatAmount(commissionPercentPerPax, equivDecimals));
  }

  nodePSG.convertAttr("C5B", formatAmount(commissionAmountPerPax, equivDecimals));

  if (isMIPResponse() && _trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const FarePath& fp = *totals->farePath;

    Money nonRefAmt(fp.getHigherNonRefundableAmount());
    nonRefAmt = ExchangeUtil::convertCurrency(
        _trx, nonRefAmt, origCurrency, fp.itin()->useInternationalRounding());
    nodePSG.convertAttr("NRA", formatAmount(nonRefAmt.value(), origDecimals));
  }
}

void
XMLShoppingResponse::generateSellingFareData(CalcTotals* total)
{
  if (!total->adjustedCalcTotal)
    return;

  Node nodeSFD(_writer, "SFD");

  nodeSFD.convertAttr("TYP", "ADS");
  nodeSFD.enterNode();

  bool generateHPSData = fallback::fallbackFRROrgChange(&_trx) ?
                           (_trx.getOptions() && _trx.getOptions()->isPDOForFRRule()) :
                           true;

  if (generateHPSData)
    generateHPSAdjustedSellingData(total->adjustedCalcTotal);
}

void
XMLShoppingResponse::generateHPSAdjustedSellingData(CalcTotals* adjTotal)
{
  if (!adjTotal || adjTotal->adjustedSellingDiffInfo.empty())
    return;

  for (const auto& item : adjTotal->adjustedSellingDiffInfo)
  {
    Node nodeHPS(_writer, "HPS");

    nodeHPS.convertAttr("T52", item.typeCode);
    nodeHPS.convertAttr("N52", item.description);
    nodeHPS.convertAttr("C52", item.amount);
  }
}

void
XMLShoppingResponse::callGenerateFDC(const Itin* itin,
                                     const FarePath* path,
                                     CalcTotals* totals,
                                     bool& minMaxCatFound)
{
  uint16_t segmentOrder = 0;
  uint16_t index = 0;
  bool firstSideTrip = false;
  bool preAvl = true;
  bool minMaxCatFoundInFDC;
  uint16_t fcId = 0;

  for (const auto tvlSeg : itin->travelSeg())
  {
    if (itin->segmentOrder(tvlSeg) < segmentOrder)
    {
      continue;
    }

    uint16_t paxProcOrder;

    for (paxProcOrder = 0; paxProcOrder < _trx.paxType().size(); ++paxProcOrder)
    {
      if (_trx.paxType()[paxProcOrder] == path->paxType())
        break;
    }

    for (const auto pricingUnit : path->pricingUnit())
    {
      if (pricingUnit->isSideTripPU() && (!firstSideTrip))
      {
        firstSideTrip = true;
      }
      else
      {
        firstSideTrip = false;
      }

      for (const auto fareUsage : pricingUnit->fareUsage())
      {
        TSE_ASSERT(fareUsage->travelSeg().empty() == false); // lint !e530

        for (const auto fuTvlSeg : fareUsage->travelSeg())
        {
          if (fuTvlSeg == tvlSeg)
          {
            segmentOrder = itin->segmentOrder(tvlSeg);
            generateFDC(*fareUsage,
                        *totals,
                        itin,
                        &segmentOrder,
                        &index,
                        *pricingUnit,
                        firstSideTrip,
                        &preAvl,
                        paxProcOrder,
                        minMaxCatFoundInFDC,
                        boost::none,
                        fcId);

            if (minMaxCatFoundInFDC)
            {
              minMaxCatFound = true;
            }

            break;
          }
        }
      }
    }
  } // end FDC format
}

void
XMLShoppingResponse::addAdjMsg(CalcTotals* totals, const std::string& msg)
{
  if (msg.empty())
    return;

  Node nodeMSG(_writer, "MSG");
  const char msgType = Message::TYPE_GENERAL;
  nodeMSG.convertAttr("N06", msgType).convertAttr("Q0K", 0).addSimpleText(msg);
}

void
XMLShoppingResponse::printAdjMsg(CalcTotals* totals)
{
  addAdjMsg(totals, AdjustedSellingUtil::getADJSellingLevelMessage(_trx, *totals));
  addAdjMsg(totals, AdjustedSellingUtil::getADJSellingLevelOrgMessage(_trx, *totals));
}

void
XMLShoppingResponse::generateMSG(const Itin* itin, CalcTotals* totals)
{
  // MSG
  const FarePath* farePath = totals->farePath;

  printAdjMsg(totals);

  TicketingEndorsement::TicketEndoLines messages;
  TicketingEndorsement tktEndo;

  if (!fallback::fallbackEndorsementsRefactoring(&_trx))
  {
    // this refactoring strongly depends on endorsementExpansion
    // both need to be OFF (value 'Y') to return to Endorse Cutter Limited
    tktEndo.collectEndorsements(_trx, *farePath, messages, EndorseCutter());
  }
  else
  {
    if (!fallback::endorsementExpansion(&_trx))
      tktEndo.collectEndorsements(_trx, *farePath, messages, EndorseCutterUnlimited());
    else
      tktEndo.collectEndorsements(_trx, *farePath, messages,
          EndorseCutterLimited(TicketingEndorsement::MAX_TKT_ENDORSEMENT_LINE_SIZE));
  }
  tktEndo.sortLinesByPrio(_trx, *farePath, messages);

  for (TicketEndorseLine* line : messages)
  {
    Node nodeMSG(_writer, "MSG");
    const char msgType = Message::TYPE_ENDORSE;
    nodeMSG.convertAttr("N06", msgType)
        .convertAttr("Q0K", 0)
        .attr("B00", line->carrier)
        .addSimpleText(line->endorseMessage);
  }

  // Cat35 trailer message/ Cat35 commission msg
  const CollectedNegFareData* cNegFareData = farePath->collectedNegFareData();

  if (!fallback::endorsementExpansion(&_trx))
  {
    if (LIKELY(cNegFareData != 0))
    {
      if (!cNegFareData->trailerMsg().empty())
      {
        Node nodeMSG(_writer, "MSG");
        const char msgType = Message::TYPE_ENDORSE;
        nodeMSG.convertAttr("N06", msgType).convertAttr("Q0K", 0).addSimpleText(
            cNegFareData->trailerMsg());
      }

      if (!cNegFareData->commissionMsg().empty())
      {
        Node nodeMSG(_writer, "MSG");
        const char msgType = Message::TYPE_ENDORSE;
        nodeMSG.convertAttr("N06", msgType).convertAttr("Q0K", 0).addSimpleText(
            cNegFareData->commissionMsg());
      }
    }
  } // fallback
  else
  {
    if (LIKELY(cNegFareData != 0))
    {
      if (!cNegFareData->trailerMsg().empty())
      {
        Node nodeMSG(_writer, "MSG");
        const char msgType = Message::TYPE_ENDORSE;
        nodeMSG.convertAttr("N06", msgType).convertAttr("Q0K", 0).addSimpleText(
            cNegFareData->trailerMsg()
                .substr(0, TicketingEndorsement::MAX_TKT_ENDORSEMENT_LINE_SIZE)
                .c_str());
      }

      if (!cNegFareData->commissionMsg().empty())
      {
        Node nodeMSG(_writer, "MSG");
        const char msgType = Message::TYPE_ENDORSE;
        nodeMSG.convertAttr("N06", msgType).convertAttr("Q0K", 0).addSimpleText(
            cNegFareData->trailerMsg()
                .substr(0, TicketingEndorsement::MAX_TKT_ENDORSEMENT_LINE_SIZE)
                .c_str());
      }
    }
  }

  // populate the warning message
  for (const auto& warningMsg : totals->warningFopMsgs)
  {
    Node nodeMSG(_writer, "MSG");
    const char msgType = Message::TYPE_WARNING;
    nodeMSG.convertAttr("N06", msgType)
        .convertAttr("Q0K", 0)
        .attr("B00", getValidatingCarrier(_trx))
        .addSimpleText(warningMsg);
  }

  for (const auto& fcMsg : totals->fcMessage)
  {
    Node nodeMSG(_writer, "MSG");
    const char msgType = fcMsg.messageType();
    nodeMSG.convertAttr("N06", msgType).convertAttr("Q0K", fcMsg.messageCode());

    if (fcMsg.messageSubType())
    {
      const char msgSubType = fcMsg.messageSubType();
      nodeMSG.convertAttr("Q0M", msgSubType);
    }

    if (UNLIKELY(!fcMsg.airlineCode().empty()))
    {
      nodeMSG.attr("B00", fcMsg.airlineCode());
    }

    nodeMSG.addSimpleText(fcMsg.messageText());
  }
}

void
XMLShoppingResponse::generateSFI(const Itin* itin)
{
  if (!_trx.getRequest()->isCollectOCFees())
  {
    return;
  }

  Node nodeSFI(_writer, "SFI");
  bool foundFees = ServiceFeeUtil::feesFoundForItin(itin);

  if (!foundFees)
  {
    if (itin->timeOutForExceeded())
      nodeSFI.convertAttr("S07", "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER");
    else
      nodeSFI.convertAttr("S07", "AIR EXTRAS NOT APPLICABLE OR ARE UNKNOWN FOR THIS ITINERARY");
  }
  else
  {
    if (itin->moreFeesAvailable())
    {
      nodeSFI.convertAttr("S07", "ADDTL AIR EXTRAS APPLY - REFINE REQUEST");
    }

    bool requestedGroupsNotSpecified = false;

    if (_trx.getOptions()->isProcessAllGroups())
    {
      requestedGroupsNotSpecified = true;
    }

    for (const auto serviceFeesGroup : itin->ocFeesGroup())
    {
      if (requestedGroupsNotSpecified && serviceFeesGroup->ocFeesMap().empty())
      {
        continue;
      }

      generateSFG(itin, serviceFeesGroup);
    }
  }
}

void
XMLShoppingResponse::generateSFG(const Itin* itin, const ServiceFeesGroup* sfg)
{
  Node nodeSFG(_writer, "SFG");
  nodeSFG.convertAttr("SF0", sfg->groupCode());
  nodeSFG.convertAttr("S03", sfg->groupDescription());

  if (sfg->ocFeesMap().empty())
  {
    nodeSFG.convertAttr("S07", "AIR EXTRAS NOT APPLICABLE OR ARE UNKNOWN");
  }
  else
  {
    generateOCF(itin, sfg);
  }
}

void
XMLShoppingResponse::generateOCF(const Itin* itin, const ServiceFeesGroup* sfg)
{
  ServiceFeeUtil::createOCFeesUsages(*sfg, _trx);
  const std::vector<PaxOCFeesUsages>& paxOcFeesUsages = ServiceFeeUtil::getSortedFeesUsages(
      *sfg, _trx.paxType(), (!_trx.getOptions()->isSummaryRequest()));
  std::vector<PaxOCFeesUsages>::const_iterator paxOcFeesUsagesIter = paxOcFeesUsages.begin();
  std::vector<PaxOCFeesUsages>::const_iterator paxOcFeesUsagesIterEnd = paxOcFeesUsages.end();

  ServiceFeeUtil util(_trx);
  CurrencyCode currencyCode = ServiceFeeUtil::getSellingCurrency(_trx);

  for (; paxOcFeesUsagesIter != paxOcFeesUsagesIterEnd; ++paxOcFeesUsagesIter)
  {
    PaxOCFeesUsages paxOcFeesUsages = (*paxOcFeesUsagesIter);
    Node nodeOCF(_writer, "OCF");

    if (paxOcFeesUsages.paxType() == "ALL")
    {
      nodeOCF.convertAttr("B74", paxOcFeesUsages.paxType());
    }
    else
    {
      nodeOCF.convertAttr("B74", paxOcFeesUsages.fees()->farePath()->paxType()->requestedPaxType());
    }

    nodeOCF.convertAttr("SF7", getCommercialName(paxOcFeesUsages.fees()));
    nodeOCF.convertAttr("A01", paxOcFeesUsages.fees()->travelStart()->origin()->loc());
    nodeOCF.convertAttr("A02", paxOcFeesUsages.fees()->travelEnd()->destination()->loc());
    nodeOCF.convertAttr("Q01", itin->segmentOrder(paxOcFeesUsages.fees()->travelStart()));
    nodeOCF.convertAttr("Q02", itin->segmentOrder(paxOcFeesUsages.fees()->travelEnd()));
    nodeOCF.convertAttr("B00", paxOcFeesUsages.fees()->carrierCode());
    Money feeAmount = util.convertOCFeeCurrencyWithTaxes(currencyCode, paxOcFeesUsages.fees());
    nodeOCF.convertAttr("SF1", formatAmount(feeAmount));
    nodeOCF.convertAttr("D01",
                        paxOcFeesUsages.fees()->travelStart()->departureDT().dateToSqlString());
  }
}

void
XMLShoppingResponse::generateTOT(FareCalcCollector& fareCalcCollector,
                                 MoneyAmount totalConstructAmount,
                                 Money constructionCurrency,
                                 std::string constructCurrencyCode,
                                 ItineraryTotals& total,
                                 const uint16_t brandIndex /*= INVALID_BRAND_INDEX*/)
{
  total.construction.currency = constructCurrencyCode;
  total.construction.noDec = constructionCurrency.noDec(_trx.ticketingDate());
  total.construction.amount = totalConstructAmount;
  total.base.amount = fareCalcCollector.getBaseFareTotal(
      _trx, total.base.currency, total.base.noDec, false, brandIndex);
  total.equivalent.amount = fareCalcCollector.getEquivFareAmountTotal(
      _trx, total.equivalent.currency, total.equivalent.noDec, false, brandIndex);
  total.tax.amount =
      fareCalcCollector.getTaxTotal(_trx, total.tax.currency, total.tax.noDec, false, brandIndex);
  total.equivalentAndTax =
      total.equivalent.amount +
      getMoneyAmountInCurrency(total.tax.amount, total.tax.currency, total.equivalent.currency);
  generateTOTbody(total);
}

MoneyAmount
XMLShoppingResponse::getMoneyAmountInCurrency(const MoneyAmount sourceAmount,
                                              const CurrencyCode& sourceCurrency,
                                              const CurrencyCode& targetCurrency)
{
  if (sourceCurrency == targetCurrency)
  {
    return sourceAmount;
  }

  const Money source(sourceAmount, sourceCurrency);
  Money target(targetCurrency);

  if (false == _currencyConverter.convert(target, source, _trx))
  {
    LOG4CXX_ERROR(_logger,
                  "XMLShoppingResponse::getMoneyAmountInCurrency - Currency conversion failed (" +
                      sourceCurrency + " -> " + targetCurrency + ")");
  }

  return target.value();
}

void
XMLShoppingResponse::generateN1U(FareCalcCollector& calc, Itin* itin, Node& node)
{
  std::vector<std::string> privateIndPerPax;
  std::string privateInd = " ";
  generatePrivateInd(*itin, calc, privateIndPerPax);

  for (const auto& pInd : privateIndPerPax)
  {
    if (privateInd == " ")
    {
      privateInd = pInd;
    }
    else if (pInd != " ")
    {
      if (privateInd != pInd)
      {
        privateInd = PILLOW;
      }
    }
  }

  if (privateInd != " ")
  {
    node.convertAttr("N1U", privateInd);
  }

  // end populate private fare
}
void
XMLShoppingResponse::prepareSurcharges(const TravelSeg& tvlSeg, CalcTotals& calcTotal)
{
  // Surcharge Information
  std::map<const TravelSeg*, std::vector<SurchargeData*>>::const_iterator surchargesIter =
      calcTotal.surcharges.find(&tvlSeg);

  if (surchargesIter != calcTotal.surcharges.end())
  {
    std::vector<SurchargeData*>::const_iterator surchargeIter = surchargesIter->second.begin();
    std::vector<SurchargeData*>::const_iterator surchargeIterEnd = surchargesIter->second.end();
    // const tse::CurrencyNoDec numberOfDec = 2;
    Money moneyCalc(calcTotal.farePath->calculationCurrency());
    tse::CurrencyNoDec numberOfDec = moneyCalc.noDec(_trx.ticketingDate());

    for (; surchargeIter != surchargeIterEnd; surchargeIter++)
    {
      SurchargeData& surchargeData = **surchargeIter;

      if (surchargeData.amountNuc() != 0 && surchargeData.selectedTkt() &&
          !surchargeData.isFromOverride())
      {
        Node(_writer, "SUR")
            .convertAttr("A01", surchargeData.brdAirport())
            .convertAttr("A02", surchargeData.offAirport())
            .convertAttr("N0F", surchargeData.surchargeType())
            .convertAttr("C40", calcTotal.farePath->calculationCurrency())
            .convertAttr("C69",
                         formatAmount((surchargeData.amountNuc() * surchargeData.itinItemCount()),
                                      numberOfDec))
            .convertAttr("S03", surchargeData.surchargeDesc());
      }
    }
  }
}

void
XMLShoppingResponse::moveMileageToAmt(std::vector<ISSolution>& paths) const
{
  for (const auto& iSSolution : paths)
  {
    GroupFarePath* gfp = iSSolution.second;

    if (gfp)
    {
      gfp->setTotalNUCAmount(0);
      for (const auto fppqItem : gfp->groupFPPQItem())
      {
        int farePathMileage = 0;

        for (const auto pricingUnit : fppqItem->farePath()->pricingUnit())
        {
          for (const auto fareUsage : pricingUnit->fareUsage())
          {
            gfp->increaseTotalNUCAmount(fareUsage->paxTypeFare()->mileage());
            farePathMileage += fareUsage->paxTypeFare()->mileage();
          }
        }

        fppqItem->farePath()->setTotalNUCAmount(farePathMileage);
      }
    }
  }
}

void
XMLShoppingResponse::generateITNContent(FareCalcCollector& calcCollector,
                                        const Itin* itin,
                                        XMLShoppingResponse::ItineraryTotals& totalAmount)
{
  const FarePath* path = itin->farePath()[0];
  const std::vector<CalcTotals*>& calcTotals = calcCollector.passengerCalcTotals();
  generateSEG(calcTotals, itin);
  std::vector<CalcTotals*>::const_iterator calcItem = calcTotals.begin();

  for (; calcItem != calcTotals.end(); ++calcItem)
  {
    if ((*calcItem)->equivCurrencyCode.empty() == false)
    {
      break;
    }
  }

  TSE_ASSERT(calcItem != calcTotals.end());
  TSE_ASSERT(path != nullptr);
  TSE_ASSERT(_trx.paxType()[0] == path->paxType());
  CalcTotals* totals = calcCollector.findCalcTotals(path);
  TSE_ASSERT(totals != nullptr);
  MoneyAmount totalConstructAmount = 0.0;
  bool minMaxCatFound = false;
  generatePSG(calcCollector, itin, path, totals, minMaxCatFound, totalConstructAmount);
  Money constructionCurrency(itin->calculationCurrency());
  constructionCurrency.setCode(totals->farePath->calculationCurrency());
  std::string constructCurrencyCode(totals->farePath->calculationCurrency());
  generateTOT(calcCollector,
              totalConstructAmount,
              constructionCurrency,
              constructCurrencyCode,
              totalAmount);

  if (_trx.isValidatingCxrGsaApplicable())
  {
    for (const CalcTotals* calcTotal : calcCollector.passengerCalcTotals())
    {
      if (calcTotal->farePath->processed())
      {
        if (!fallback::fallbackValidatingCarrierInItinOrder(&_trx))
          getValidatingCarrierInItinOrder(*(calcTotal->farePath));
        if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
        {
          if (calcTotal->farePath && !calcTotal->farePath->defaultValCxrPerSp().empty())
            prepareValidatingCarrierLists(*(calcTotal->farePath));
          else
            generateValidatingCxrList(*(calcTotal->farePath));
        }
        else
          generateValidatingCxrList(*(calcTotal->farePath));
        // VCL tag is per Itin
        break;
      }
    }
  }

  // Min - Max Stay Indicator
  Node nodeMMS(_writer, "MMS");

  if (minMaxCatFound)
    nodeMMS.attr("I10", "T");
  else
    nodeMMS.attr("I10", "F");
}

void
XMLShoppingResponse::generateNodeADSforCalendarOrAwardAltDates()
{
  if (_trx.calendarSoldOut() || (_trx.awardRequest() && _trx.isAltDates()))
  {
    for (const auto& altDatePair : _trx.altDatePairs())
    {
      if (!altDatePair.second->goodItinForDatePairFound)
      {
        Node nodeADS(_writer, "ADS");

        if (_trx.getRequest()->originBasedRTPricing() &&
            (_trx.outboundDepartureDate() != DateTime::emptyDate()))
        {
          nodeADS.convertAttr("D01", altDatePair.first.second.dateToString(YYYYMMDD, "-"));
        }
        else
        {
          nodeADS.convertAttr("D01", altDatePair.first.first.dateToString(YYYYMMDD, "-"));
        }

        if (_trx.awardRequest() && _trx.isAltDates() &&
            !_trx.getRequest()->originBasedRTPricing() && !altDatePair.first.second.isEmptyDate())
        {
          nodeADS.convertAttr("D02", altDatePair.first.second.dateToString(YYYYMMDD, "-"));
        }
      }
    } // for
  }
}

void
XMLShoppingResponse::generateNodeForExchangeWithBrands()
{
  const uint16_t brandSize = _trx.validBrands().size();
  for (Itin* itin : _trx.itin())
  {
    Node nodeITN(_writer, "ITN");
    generateITNIdAttribute(nodeITN, itin);
    for (uint16_t brandIndex = 0; brandIndex < brandSize; ++brandIndex)
    {
      generateBrandError(itin, brandIndex);
    }
  }
}

void
XMLShoppingResponse::generateNodeForSettlementTypes()
{
  if (_trx.countrySettlementPlanInfos().size())
  {
    for (const auto settlementPlanType : _trx.countrySettlementPlanInfos())
    {
      Node nodeSettlementTypes(_writer, "VCL");
      nodeSettlementTypes.attr("SM0", settlementPlanType->getSettlementPlanTypeCode());
    }
  }
  else
  {
    if (_trx.countrySettlementPlanInfo() != nullptr)
    {
      Node nodeSettlementTypes(_writer, "VCL");
      nodeSettlementTypes.attr("SM0",
                               _trx.countrySettlementPlanInfo()->getSettlementPlanTypeCode());
    }
  }
}

void
XMLShoppingResponse::generateSnapITN(Itin* const primaryItin)
{
  Node nodeSnapITN(_writer, "ITN");
  generateITNIdAttribute(nodeSnapITN, primaryItin);
  generateSID(primaryItin);

  if (_taxSplitter)
  {
    _taxSplitter->clearTaxMaps();
  }

  generateITT(primaryItin, 0);
  generateITT(primaryItin, 1);
}

void
XMLShoppingResponse::generateITT(Itin* const primaryItin, const int legId)
{
  if (!_taxSplitter)
  {
    return;
  }

  Itin* const subItin = _taxSplitter->getSubItin(primaryItin, legId);

  if (!subItin)
  {
    return;
  }

  _taxSplitter->setupTaxes(primaryItin, subItin, legId);
  ItineraryTotals t;
  generateITNbody(subItin, "ITT", boost::bind(&XMLShoppingResponse::generateSID, this, _1), t);
}

void
XMLShoppingResponse::generateSID(const Itin* const itin)
{
  for (const auto& leg : itin->legID())
    Node(_writer, "SID").convertAttr("Q14", leg.first).convertAttr(
        "Q15", ShoppingUtil::findSopId(_trx, leg.first, leg.second));
}

void
XMLShoppingResponse::preparePrimeItins()
{
  // copy all keys (Itin) from primeSubItinMap to_trx.itin()
  _trx.itin().clear();
  for (const auto& primeSubItin : _trx.primeSubItinMap())
  {
    if (false == WnSnapUtil::completeFarePaths(
                     _trx, primeSubItin.second.outboundItin, primeSubItin.second.inboundItin))
    {
      continue;
    }

    _trx.itin().push_back(primeSubItin.first);
  }
}

std::string
XMLShoppingResponse::getSnapKey(const TaxItem& taxItem)
{
  std::stringstream out;
  out << "*" << taxItem.taxCode() << "$" << taxItem.carrierCode() << "&" << taxItem.legId();
  return out.str();
}

void
XMLShoppingResponse::computeItineraryTotals(const Itin* itin,
                                            FareCalcCollector* calc,
                                            uint16_t brandIndex,
                                            bool baseFareDiff,
                                            const Monetary& construction,
                                            ItineraryTotals& total)
{
  total.construction = construction;
  total.base.amount =
      calc->getBaseFareTotal(_trx, total.base.currency, total.base.noDec, false, brandIndex);
  total.equivalent.amount = calc->getEquivFareAmountTotal(
      _trx, total.equivalent.currency, total.equivalent.noDec, false, brandIndex);
  total.tax.amount =
      calc->getTaxTotal(_trx, total.tax.currency, total.tax.noDec, false, brandIndex);

  if (total.equivalent.currency == total.base.currency)
  {
    total.equivalent.amount = total.base.amount;
  }

  if (baseFareDiff)
  {
    total.base = total.equivalent;
  }

  total.equivalentAndTax =
      total.equivalent.amount +
      getMoneyAmountInCurrency(total.tax.amount, total.tax.currency, total.equivalent.currency);

  if (_trx.awardRequest())
  {
    total.psgMileage = _psgMileageTotal;
    _psgMileageTotal = 0;
  }

  if (_trx.getRequest()->isCollectOCFees() && _trx.getOptions()->isSummaryRequest() &&
      ServiceFeeUtil::feesFoundForItin(itin))
  {
    ServiceFeeUtil util(_trx);
    Money feeSummary = util.getOCFeesSummary(itin);
    total.feesAmount = feeSummary.value();
  }
}

void
XMLShoppingResponse::generateTOTbody(const ItineraryTotals& total)
{
  Node nodeTOT(_writer, "TOT");
  nodeTOT.attr("C43", total.construction.currency)
      .attr("C5E", formatAmount(total.construction.amount, total.construction.noDec));

  nodeTOT.attr("C40", total.base.currency)
      .attr("C5A", formatAmount(total.base.amount, total.base.noDec));

  nodeTOT.attr("C45", total.equivalent.currency)
      .attr("C5F", formatAmount(total.equivalent.amount, total.equivalent.noDec));

  nodeTOT.attr("C46", total.tax.currency)
      .attr("C65", formatAmount(total.tax.amount, total.tax.noDec));

  if (!_trx.getRequest()->isCollectTTypeOBFee())
  {
    nodeTOT.attr("C56",
                 formatAmount(total.equivalentAndTax, total.equivalent.noDec)); // the currency
    // for this is
    // the same as
    // "C45"
  }
  else
  {
    nodeTOT.attr("C56",
                 formatAmount(total.equivalentAndTax + total.tTypeFees, total.equivalent.noDec));
    nodeTOT.convertAttr("C82", formatAmount(total.tTypeFees, total.equivalent.noDec));
  }

  if (total.psgMileage)
  {
    nodeTOT.convertAttr("C5L", *total.psgMileage);
  }

  if (total.feesAmount)
  {
    nodeTOT.attr("SF1", formatAmount(*total.feesAmount, total.equivalent.noDec));
  }
}

void
XMLShoppingResponse::generateITNlowerBody(const Itin* itin,
                                          const std::vector<CalcTotals*>& calcTotals,
                                          FareCalcCollector* calc,
                                          const std::vector<std::string>& privateIndPerPax,
                                          const uint16_t brandIndex,
                                          ItineraryTotals& itineraryTotals)
{
  if (fallback::fallbackSegFromGriRemoval(&_trx))
    generateSEG(calcTotals, itin);
  else if (!_trx.getRequest()->isBrandedFaresRequest() && !_trx.isBRAll())
    generateSEG(calcTotals, itin);

  bool minMaxCatFound = false;
  bool minMaxCatFoundInFDC = false;
  MoneyAmount totalConstructAmount = 0.0;
  std::vector<CalcTotals*>::const_iterator calcItem = calcTotals.begin();

  for (; calcItem != calcTotals.end(); ++calcItem)
  {
    if (LIKELY((*calcItem)->equivCurrencyCode.empty() == false))
    {
      break;
    }
  }

  TSE_ASSERT(calcItem != calcTotals.end());
  CurrencyCode equivCurrency;
  int16_t equivDecimals;
  CurrencyCode origCurrency;
  int origDecimals;
  std::string constructCurrencyCode;
  int16_t constructionDecimals = 0;
  CurrencyCode saveOrigCur = " ";
  bool baseFareDiff = false;
  // get the pax types we want to output in the order we want
  // to output them in (same order as the input order)
  std::vector<PaxType*> paxTypes = _trx.paxType();
  std::sort(paxTypes.begin(), paxTypes.end(), comparePaxTypeInputOrder);
  // output information for each pax type
  size_t paxNo = 0;

  for (std::vector<PaxType*>::const_iterator j = paxTypes.begin(); j != paxTypes.end();
       ++j, ++paxNo)
  {
    auto fpctpair = BrandingResponseUtils::findFarePathAndCalcTotalsForPaxTypeAndBrand(
        *itin, *calc, *j, brandIndex, _trx);

    const FarePath* origPath = fpctpair.first;
    CalcTotals* origTotals = fpctpair.second;
    const FarePath* path = origPath;
    CalcTotals* totals = origTotals;

    if (nullptr == totals)
    {
      LOG4CXX_ERROR(_logger, "totals == 0");
      throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED,
                                   "totals == 0");
    }

    if (totals->adjustedCalcTotal && _trx.getOptions() &&
        !_trx.getOptions()->isPDOForFRRule()) // not ORG
    {
      path = origPath->adjustedSellingFarePath();
      totals = origTotals->adjustedCalcTotal;
    }

    uint16_t paxProcOrder;

    for (paxProcOrder = 0; paxProcOrder < _trx.paxType().size(); ++paxProcOrder)
    {
      if (_trx.paxType()[paxProcOrder] == *j)
      {
        break;
      }
    }

    TSE_ASSERT(path != nullptr || _trx.getOptions()->isXoFares() ||
               _trx.getOptions()->forceCorpFares());
    equivCurrency = totals->equivCurrencyCode;
    Money equivCurr(equivCurrency);
    equivDecimals = equivCurr.noDec(_trx.ticketingDate());
    origCurrency = totals->convertedBaseFareCurrencyCode;
    origDecimals = totals->convertedBaseFareNoDec;

    if (saveOrigCur == " ")
    {
      saveOrigCur = origCurrency;
    }
    else if (saveOrigCur != origCurrency)
    {
      baseFareDiff = true;
    }

    Money constructionCurrency(totals->farePath->itin()->calculationCurrency());
    constructionCurrency.setCode(totals->farePath->calculationCurrency());
    constructionDecimals = constructionCurrency.noDec(_trx.ticketingDate());

    if (path == nullptr || totals == nullptr)
    {
      if (totals != nullptr)
      {
        Node nodePSG(_writer, "PSG");
        nodePSG.convertAttr("Q0U", totals->farePath->paxType()->number())
            .convertAttr("B70", totals->requestedPaxType);

        if (_trx.getOptions()->isXoFares())
        {
          nodePSG.attr("S07", "NO RULES VALID FOR PASSENGER TYPE/CLASS OF SERVICE");
        }
        else // _trx.getOptions()->forceCorpFares()  = XC
        {
          nodePSG.attr("S07", "ATTN* DISCOUNT NOT APPLICABLE");
        }
      }

      continue;
    }

    MoneyAmount totalEquivAmountPerPax = 0.0, totalOrigAmountPerPax = 0.0, totalTaxesPerPax = 0.0;
    MoneyAmount commissionPercentPerPax = 0.0, commissionAmountPerPax = 0.0,
                totalConstructAmountPerPax = 0.0;
    std::string usePaxType;
    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(_trx);

    if (LIKELY(fcConfig == nullptr || fcConfig->truePsgrTypeInd() == YES))
    {
      usePaxType = totals->truePaxType;
    }
    else
    {
      usePaxType = totals->requestedPaxType;
    }

    int16_t numPax = totals->farePath->paxType()->number();
    totalEquivAmountPerPax += totals->equivFareAmount;
    totalOrigAmountPerPax += totals->convertedBaseFare;
    totalTaxesPerPax += totals->taxAmount();
    commissionPercentPerPax = totals->farePathInfo.commissionPercent;
    commissionAmountPerPax = totals->farePathInfo.commissionAmount;
    totalConstructAmountPerPax += totals->farePath->getTotalNUCAmount();
    totalConstructAmount += totalConstructAmountPerPax * numPax;
    constructCurrencyCode = totals->farePath->calculationCurrency();
    Node nodePSG(_writer, "PSG");

    if (equivCurrency == origCurrency)
    {
      equivDecimals = origDecimals;
      totalEquivAmountPerPax = totalOrigAmountPerPax;
    }

    if (!_trx.getRequest()->isCollectTTypeOBFee())
    {
      nodePSG.convertAttr("Q0U", path->paxType()->number())
          .convertAttr("B70", usePaxType)
          .attr("C43", constructCurrencyCode)
          .attr("C5E", formatAmount(totalConstructAmountPerPax, constructionDecimals))
          .attr("C40", origCurrency)
          .attr("C5A", formatAmount(totalOrigAmountPerPax, origDecimals))
          .attr("C46", equivCurrency)
          .attr("C66", formatAmount(totalEquivAmountPerPax + totalTaxesPerPax, equivDecimals))
          .attr("C65", formatAmount(totalTaxesPerPax, equivDecimals))
          .attr("S07", totals->fareCalculationLine);
    }
    else
    {
      MoneyAmount totalTTypeFeeAmount = 0.0;
      getTTypeFeePerPax(*totals, path, totalTTypeFeeAmount);
      itineraryTotals.tTypeFees += (totalTTypeFeeAmount * path->paxType()->number());

      nodePSG.convertAttr("Q0U", path->paxType()->number())
          .convertAttr("B70", usePaxType)
          .attr("C43", constructCurrencyCode)
          .attr("C5E", formatAmount(totalConstructAmountPerPax, constructionDecimals))
          .attr("C40", origCurrency)
          .attr("C5A", formatAmount(totalOrigAmountPerPax, origDecimals))
          .attr("C46", equivCurrency)
          .attr("C66",
                formatAmount(totalEquivAmountPerPax + totalTaxesPerPax + totalTTypeFeeAmount,
                             equivDecimals))
          .attr("C65", formatAmount(totalTaxesPerPax, equivDecimals))
          .attr("C82", formatAmount(totalTTypeFeeAmount, equivDecimals))
          .attr("S07", totals->fareCalculationLine);
    }

    if (_trx.hasPriceDynamicallyDeviated())
    {
      const Money& epd = totals->getEffectivePriceDeviation();
      nodePSG.convertAttr("EPD", formatAmount(epd.value(), epd.noDec()));
    }

    // format private fare
    if ((paxTypes.size() > paxNo) && (privateIndPerPax[paxNo] != " "))
    {
      nodePSG.convertAttr("N1U", privateIndPerPax[paxNo]);
    }

    if (_trx.awardRequest())
    {
      int fdcMileageTotal = 0;

      for (const auto pricingUnit : path->pricingUnit())
      {
        for (const auto fareUsage : pricingUnit->fareUsage())
        {
          fdcMileageTotal += fareUsage->paxTypeFare()->mileage();
        }
      }

      std::stringstream psgMileage;
      psgMileage << fdcMileageTotal;
      nodePSG.attr("C5L", psgMileage.str());
      _psgMileageTotal += (fdcMileageTotal * path->paxType()->number());
    }

    // check if there is COP - country of payment minimum fare (LCM) plus up exists
    for (const auto pricingUnit : path->pricingUnit())
    {
      if (pricingUnit->minFarePlusUp().count(COP) != 0)
      {
        nodePSG.attr("C44", origCurrency)
            .attr("C5C", formatAmount(totalOrigAmountPerPax, origDecimals));
        break;
      }
    }

    uint16_t stopoverCount = 0;
    MoneyAmount stopoverCharges = 0;
    CurrencyCode pubCurr = "NUC";
    bool stopoverFlag = totals->getStopoverSummary(stopoverCount, stopoverCharges, pubCurr);

    if (stopoverFlag)
    {
      nodePSG.attr("C63", formatAmount(stopoverCharges, totals->equivNoDec))
          .convertAttr("Q0X", stopoverCount);
    }

    // Calculation ROE
    if (LIKELY((fcConfig->domesticNUC() == YES) ||
               (totals->farePath->itin()->calculationCurrency() !=
                totals->farePath->itin()->originationCurrency())))
    {
      if (LIKELY(totals->roeRate > 0.0))
      {
        //          construct.addAttributeDouble(xml2::ExchangeRateOne, totals->roeRate,
        // exchangeRatePrecision);
        //          construct.addAttributeInteger(xml2::NumberDecimalPlacesExchangeRateOne,
        //                                        total->roeRateNoDec);
        nodePSG.attr("C54", formatAmount(totals->roeRate, totals->roeRateNoDec))
            .convertAttr("Q05", totals->roeRateNoDec);
      }
    }

    // set up Nonrefundable indicator
    if (fallback::fixed::noNraAttrInShoppingResponse())
    {
      // when removing fallback remove also generateP27() function
      if (generateP27(*path))
      {
        nodePSG.convertAttr("P27", "T");
      }
      else
      {
        nodePSG.convertAttr("P27", "F");
      }
    }
    else
    {
      const bool nonRefundable = CommonParserUtils::isNonRefundable(*path);
      nodePSG.convertAttr("P27", nonRefundable ? "T" : "F");

      const Money nonRefAmt = CommonParserUtils::nonRefundableAmount(_trx, *totals, nonRefundable);
      nodePSG.convertAttr("NRA", formatAmount(nonRefAmt.value(), origDecimals));
    }

    // setup S30
    if (LIKELY(true == _trx.posPaxType().empty() || true == _trx.posPaxType()[0].empty()))
    {
      if (JCB == usePaxType)
      {
        nodePSG.convertAttr("S30", "BLK");
      }
    }

    // setup commission percentage and commission amount
    if (commissionPercentPerPax != RuleConst::PERCENT_NO_APPL)
    {
      nodePSG.convertAttr("C5D", formatAmount(commissionPercentPerPax, equivDecimals));
    }

    nodePSG.convertAttr("C5B", formatAmount(commissionAmountPerPax, equivDecimals));

    FuFcIdMap fuFcIdCol;
    if (!fallback::fallbackCommissionManagement(&_trx))
      generateCommissionInfo(nodePSG, totals->farePath, *totals, fuFcIdCol);

    checkDuplicateTvlSeg(*path);
    updateItinCosForSeatRemain(*itin, *path);
    uint16_t segmentOrder = 0;
    uint16_t index = 0;
    bool firstSideTrip = false;
    bool preAvl = true;

    FareCalc::FcTaxInfo::TaxesPerFareUsage taxGrouping;
    if (_trx.getOptions()->isSplitTaxesByFareComponent())
    {
      totals->getMutableFcTaxInfo().getTaxesSplitByFareUsage(taxGrouping);
    }

    for (const auto tvlSeg : itin->travelSeg())
    {
      if (itin->segmentOrder(tvlSeg) < segmentOrder)
      {
        continue;
      }

      for (const auto pricingUnit : path->pricingUnit())
      {
        if (pricingUnit->isSideTripPU() && (!firstSideTrip))
        {
          firstSideTrip = true;
        }
        else
        {
          firstSideTrip = false;
        }
        for (const auto fareUsage : pricingUnit->fareUsage())
        {
          TSE_ASSERT(!fareUsage->travelSeg().empty()); // lint !e530

          uint16_t fcId = 0;
          if (!fallback::fallbackAMCPhase2(&_trx) && !fallback::fallbackAMC2ShoppingChange(&_trx))
          {
            auto it = fuFcIdCol.find(const_cast<const FareUsage*>(fareUsage));
            if (it != fuFcIdCol.end())
              fcId = it->second;
          }

          for (const auto fuTvlSeg : fareUsage->travelSeg())
          {
            if (fuTvlSeg == tvlSeg)
            {
              segmentOrder = itin->segmentOrder(tvlSeg);

              boost::optional<FareCalc::SplitTaxInfo> taxes;
              if (UNLIKELY(_trx.getOptions()->isSplitTaxesByFareComponent()))
              {
                FareCalc::FcTaxInfo::TaxesPerFareUsage::const_iterator taxesIter =
                    taxGrouping.find(fareUsage);
                TSE_ASSERT(taxesIter != taxGrouping.end());
                taxes = taxesIter->second;
              }
              generateFDC(*fareUsage,
                          *totals,
                          itin,
                          &segmentOrder,
                          &index,
                          *pricingUnit,
                          firstSideTrip,
                          &preAvl,
                          paxProcOrder,
                          minMaxCatFoundInFDC,
                          taxes,
                          fcId);

              if (minMaxCatFoundInFDC)
              {
                minMaxCatFound = true;
              }

              break;
            }
          }
        }
      }
    } // end FDC format

    // Loop through the pricing units again to get plus ups
    for (const auto pricingUnit : path->pricingUnit())
    {
      preparePricingUnitPlusUps(*pricingUnit, constructionDecimals, _trx);
    }

    prepareFarePathPlusUps(*path, constructionDecimals);
    const TaxResponse* taxResponse = TaxResponse::findFor(itin, path);

    generateTAD(taxResponse, *totals);
    if (_trx.getOptions()->isSplitTaxesByLeg())
    {
      generateTaxInfoPerLeg(*totals);
    }
    // OB Fees
    generateOBG(*origTotals);
    // currency conversion
    generateCCD(_trx, *totals);
    generateMSG(itin, origTotals);
    generateREX(*totals);
    generateSellingFareData(origTotals);

    if (isMIPResponse() && _trx.getRequest()->isProcessBaggageForMIP())
      _baggageResponse.generateBDI(*path);

    if (const MaxPenaltyResponse* maxPenaltyResponse = path->maxPenaltyResponse())
    {
      generatePEN(*maxPenaltyResponse, _trx.ticketingDate(), *path);
    }
  }

  computeItineraryTotals(
      itin,
      calc,
      brandIndex,
      baseFareDiff,
      Monetary(totalConstructAmount, constructCurrencyCode, constructionDecimals),
      itineraryTotals);
  generateTOTbody(itineraryTotals);

  if (_trx.isValidatingCxrGsaApplicable())
  {
    for (const CalcTotals* calcTotal : calc->passengerCalcTotals())
    {
      if (calcTotal->farePath->processed())
      {
        if (!fallback::fallbackValidatingCarrierInItinOrder(&_trx))
          getValidatingCarrierInItinOrder(*(calcTotal->farePath));
        if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
        {
          if (calcTotal->farePath && !calcTotal->farePath->defaultValCxrPerSp().empty())
            prepareValidatingCarrierLists(*(calcTotal->farePath));
          else
            generateValidatingCxrList(*(calcTotal->farePath));
        }
        else
          generateValidatingCxrList(*(calcTotal->farePath));
        // VCL tag is per Itin
        break;
      }
    }
  }

  // Min - Max Stay Indicator
  {
    Node nodeMMS(_writer, "MMS");

    if (minMaxCatFound)
    {
      nodeMMS.attr("I10", "T");
    }
    else
    {
      nodeMMS.attr("I10", "F");
    }
  }

  writeAltDatePairs(itin);

  generateSFI(itin);
}

void
XMLShoppingResponse::getValidatingCarrierInItinOrder(const FarePath& fp, const SettlementPlanType* sp)
{
  std::vector<CarrierCode> marketingCxrs;
  ValidatingCxrUtil::getMarketingCarriersInItinOrder(*(fp.itin()), marketingCxrs);
  if(!fallback::fallbackValidatingCarrierInItinOrderMultiSp(&_trx) && sp)
    ValidatingCxrUtil::getAlternateCxrInOrder(
          *(fp.itin()), marketingCxrs, const_cast<std::vector<CarrierCode>&>(fp.validatingCarriers()), sp);
  else
    ValidatingCxrUtil::getAlternateCxrInOrder(
      *(fp.itin()), marketingCxrs, const_cast<std::vector<CarrierCode>&>(fp.validatingCarriers()));
}

void
XMLShoppingResponse::generatePEN(const MaxPenaltyResponse& maxPenRes,
                                 const DateTime& ticketingDate,
                                 const FarePath& farePath)
{
  auto prepareMissingData = [&](const std::vector<const FareInfo*> missingDataVec)
  {
    std::vector<const PaxTypeFare*> allPTFs = smp::getAllPaxTypeFares(farePath);

    for (const FareInfo* fareInfo : missingDataVec)
    {
      const std::pair<const PaxTypeFare*, int16_t> ptfFcNumberPair =
          smp::grabMissingDataFareInformationAndCleanUp(*fareInfo, allPTFs);

      Node node(_writer, xml2::PenaltyMissingData);
      node.attr(xml2::FareBasisCode, ptfFcNumberPair.first->createFareBasis(_trx));
      node.convertAttr("Q20", ptfFcNumberPair.second);
    }
  };

  auto preparePenalty = [&](
      const std::string& el, const MaxPenaltyResponse::Fee& fee, const bool cat16)
  {
    Node node(_writer, el);

    if (cat16 && fee.isMissingData())
    {
      node.attr(xml2::IsCategory16, "T");
      prepareMissingData(fee._missingDataVec);
      return;
    }

    if (fee._fee)
    {
      node.attr(xml2::MaximumPenaltyAmount,
                formatAmount(fee._fee->value(), fee._fee->noDec(ticketingDate)));

      node.attr(xml2::MaximumPenaltyCurrency, fee._fee->code());
    }

    if (fee._non)
    {
      node.attr(xml2::NonChangeable, "T");
    }

    if (cat16)
    {
      node.attr(xml2::IsCategory16, "T");
    }
  };

  Node node(_writer, xml2::PenaltyInformation);

  preparePenalty(xml2::ChangePenaltyBefore,
                 maxPenRes._changeFees._before,
                 maxPenRes._changeFees._cat16 & smp::BEFORE);

  preparePenalty(xml2::ChangePenaltyAfter,
                 maxPenRes._changeFees._after,
                 maxPenRes._changeFees._cat16 & smp::AFTER);

  preparePenalty(xml2::RefundPenaltyBefore,
                 maxPenRes._refundFees._before,
                 maxPenRes._refundFees._cat16 & smp::BEFORE);

  preparePenalty(xml2::RefundPenaltyAfter,
                 maxPenRes._refundFees._after,
                 maxPenRes._refundFees._cat16 & smp::AFTER);
}

void
XMLShoppingResponse::generateValidatingCxrList(const FarePath farePath)
{
  if (!fallback::fallbackVCLForISShortcut(&_trx))
  {
    if ((_shoppingTrx != nullptr) && (_shoppingTrx->getTrxType() == ShoppingTrx::IS_TRX) &&
        (_shoppingTrx->startShortCutPricingItin() > 0))
    {
      return;
    }
  }

  Node nodeVCL(_writer, "VCL");
  nodeVCL.attr("SM0", _trx.countrySettlementPlanInfo()->getSettlementPlanTypeCode());
  nodeVCL.attr("VC0", "T");

  {
    // Default Validating Carrier
    Node nodeDCX(_writer, "DCX");
    nodeDCX.attr("B00", farePath.defaultValidatingCarrier());
  }

  if(!fallback::fallbackValidatingCarrierInItinOrderMultiSp(&_trx))
     getValidatingCarrierInItinOrder(farePath);
  // Alternate Validating Carriers
  for (CarrierCode cxrCode : farePath.validatingCarriers())
  {
    Node nodeACX(_writer, "ACX");
    nodeACX.attr("B00", cxrCode);
  }
}

// Builds all VCL elements.
void
XMLShoppingResponse::prepareValidatingCarrierLists(const FarePath& fp)
{
  if (!fallback::fallbackShoppingSPInHierarchyOrder(&_trx))
  {
    for (const auto& sp : vcx::SP_HIERARCHY)
    {
      if (sp == "IPC")
        continue;
      auto spIt = fp.defaultValCxrPerSp().find(sp);
      if (spIt != fp.defaultValCxrPerSp().end())
        generateValidatingCxrList(fp, spIt->first, spIt->second);
    }
  }
  else
  {
    for (const auto& spCxrPair : fp.defaultValCxrPerSp())
      generateValidatingCxrList(fp, spCxrPair.first, spCxrPair.second);
  }
}

void
XMLShoppingResponse::generateValidatingCxrList(const FarePath& fp,
                                               const SettlementPlanType& sp,
                                               const CarrierCode& defaultValCxr)
{
  if (!fp.itin() || !fp.itin()->spValidatingCxrGsaDataMap())
    return;

  auto it = fp.itin()->spValidatingCxrGsaDataMap()->find(sp);
  if (it == fp.itin()->spValidatingCxrGsaDataMap()->end() || !it->second)
    return;

  // VCL per SP
  const ValidatingCxrGSAData& valCxrGsaData = *(it->second);
  Node nodeVCL(_writer, xml2::VcxrInfo);
  nodeVCL.attr(xml2::SettlementMethod, sp);
  nodeVCL.attr(xml2::AtseProcess, "T");

  if (fallback::fallbackRemoveAttrMNVInVCL(&_trx))
  {
    // Indicator set to 'T' when we have no Default Validating Carriers and Multiple Neutral
    // Validating Carriers
    if (defaultValCxr.empty() && fp.itin()->hasNeutralValidatingCarrier(sp))
      nodeVCL.attr(xml2::MultiNeutralValCxr, "T");
  }

  // DCX
  if (!defaultValCxr.empty())
  {
    auto it = valCxrGsaData.validatingCarriersData().find(defaultValCxr);
    if (it != valCxrGsaData.validatingCarriersData().end())
    {
      if((!fallback::fallbackNonBSPVcxrPhase1(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
          && sp == NO_SETTLEMENTPLAN &&
          _trx.getRequest()->spvInd() == tse::spValidator::noSMV_IEV)
      {
        if(!it->second.interlineValidCountries.empty())
          generateValidatingCarrier(defaultValCxr, it->second.interlineValidCountries, true);
      }
      else
        generateValidatingCarrier(defaultValCxr, true);
    }

  }
  else
    generateValidatingCarrier("", true);

  if(!fallback::fallbackValidatingCarrierInItinOrderMultiSp(&_trx))
    getValidatingCarrierInItinOrder(fp, &sp);

  // ACX
  for (const CarrierCode& vcxr : fp.validatingCarriers())
  {
    if (defaultValCxr == vcxr)
      continue;

    auto it = valCxrGsaData.validatingCarriersData().find(vcxr);
    if (it == valCxrGsaData.validatingCarriersData().end())
      continue;
    if((!fallback::fallbackNonBSPVcxrPhase1(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
        && sp == NO_SETTLEMENTPLAN &&
        _trx.getRequest()->spvInd() == tse::spValidator::noSMV_IEV)
    {
      if(!it->second.interlineValidCountries.empty())
        generateValidatingCarrier(vcxr, it->second.interlineValidCountries, false);
    }
    else
      generateValidatingCarrier(vcxr, false);
  }
}

void
XMLShoppingResponse::generateValidatingCarrier(const CarrierCode& vcxr, bool isDefaultValidatingCxr)
{
  Node nodeVC(_writer,
              isDefaultValidatingCxr ? xml2::DefaultValidatingCxr : xml2::AlternateValidatingCxr);

  nodeVC.attr(xml2::ValidatingCxrCode, vcxr);
}

void
XMLShoppingResponse::generateValidatingCarrier(const CarrierCode& vcxr,
                                                const std::vector<NationCode>& interlineValidCountries,
                                                bool isDefaultValidatingCxr)
{
  Node nodeVC(_writer, isDefaultValidatingCxr ? xml2::DefaultValidatingCxr : xml2::AlternateValidatingCxr);

  nodeVC.attr(xml2::ValidatingCxrCode, vcxr);

  std::set<NationCode> sortedIETValidCountries(interlineValidCountries.begin(), interlineValidCountries.end());
  nodeVC.attr(xml2::IETCntryCode, DiagnosticUtil::containerToString(sortedIETValidCountries, true));
}

void
XMLShoppingResponse::writeAltDatePairs(const Itin* itin)
{
  if (!_trx.isAltDates())
    return;

  Node nodeADP(_writer, "ADP");
  nodeADP.convertAttr("D01", itin->datePair()->first.dateToString(YYYYMMDD, "-"));
  nodeADP.convertAttr("D02", itin->datePair()->second.dateToString(YYYYMMDD, "-"));
}

bool
XMLShoppingResponse::itinIsValidBrandSolution(const Itin* itin, const uint16_t brandIndex)
{
  BrandCode* brandCode = nullptr;
  if (!_trx.validBrands().empty())
    brandCode = &(_trx.validBrands()[brandIndex]);

  for (const auto farePath : itin->farePath())
  {
    if (brandCode)
    {
      if (farePath->getBrandCode() == *brandCode)
        return true;
    }
    else if (farePath->brandIndex() == brandIndex)
      return true;
  }

  return false;
}

void
XMLShoppingResponse::generateBrandError(const Itin* itin, const uint16_t brandIndex)
{
  std::string brandCode = ShoppingUtil::getBrandCodeString(_trx, brandIndex);

  // in regular IBF we display soldout statuses for brand parity solutions, valid for given itin
  // in context shopping allow empty brand code for all fixed legs
  if ((brandCode.empty() && !_trx.isContextShopping()) ||
      !ShoppingUtil::isValidItinBrand(itin, brandCode))
    return;

  Node nodeGRI(_writer, "GRI");

  if (!brandCode.empty())
    nodeGRI.convertAttr(xml2::BrandCode, brandCode);

  const IbfErrorMessage ibfErrorMessage =
      itin->getIbfAvailabilityTracker().getStatusForBrand(brandCode);
  nodeGRI.convertAttr("SBL", ShoppingUtil::getIbfErrorCode(ibfErrorMessage));

  const ShoppingUtil::LegBrandQualifiedIndex legBrandQualifiedIndex =
      ShoppingUtil::createLegBrandIndexRelation(_trx, *itin);

  for (const TravelSegPtrVec& tSegs : itin->itinLegs())
  {
    if ((tSegs.front()->segmentType() == Arunk) && (tSegs.size() == 1))
    {
      // do not display ARUNK legs
      continue;
    }

    uint16_t originalLegId = tSegs.front()->legId();

    Node nodeLEG(_writer, "LEG");

    std::string legBrandCode(brandCode);
    if (_trx.isContextShopping() && _trx.getFixedLegs().at(originalLegId))
    {
      // error if request has fixed leg without context (no brand at flight)
      TSE_ASSERT(_trx.getFareComponentShoppingContexts().find(tSegs.front()->pnrSegment()) !=
                 _trx.getFareComponentShoppingContexts().end());
      // for fixed legs we want status of that brand and additionally
      // brand should be printed on LEG
      legBrandCode =
          _trx.getFareComponentShoppingContexts().at(tSegs.front()->pnrSegment())->brandCode;
    }

    TSE_ASSERT(legBrandQualifiedIndex.find(originalLegId) != legBrandQualifiedIndex.end());
    const ShoppingUtil::BrandQualifiedIndex& legBrandIndexMap =
        legBrandQualifiedIndex.at(originalLegId);

    if (!legBrandCode.empty())
    {
      if (_trx.getRequest()->isProcessParityBrandsOverride())
      {
        if (legBrandIndexMap.find(legBrandCode) == legBrandIndexMap.end())
        {
          //TODO(andrzej.fediuk) PBO: Change/correct after expected behavior is defined
          nodeGRI.convertAttr(xml2::BrandCode, "");
          nodeGRI.convertAttr(xml2::BrandDescription, "");
          nodeGRI.convertAttr(xml2::ProgramName, "");
          nodeGRI.convertAttr(xml2::SystemCode, "");
          nodeGRI.convertAttr(xml2::ProgramId, "");
          nodeGRI.convertAttr(xml2::ProgramCodeNew, "");
        }
        else
        {
          TSE_ASSERT(legBrandIndexMap.find(legBrandCode) != legBrandIndexMap.end());
          TSE_ASSERT(legBrandIndexMap.at(legBrandCode) < _trx.brandProgramVec().size());
          const QualifiedBrand& qb = _trx.brandProgramVec()[legBrandIndexMap.at(legBrandCode)];
          if (!xform::formatBrandProgramData(nodeLEG, qb))
            LOG4CXX_ERROR(_logger, "Could not format brand program data");
        }
      }
      else
      {
        TSE_ASSERT(legBrandIndexMap.find(legBrandCode) != legBrandIndexMap.end());
        TSE_ASSERT(legBrandIndexMap.at(legBrandCode) < _trx.brandProgramVec().size());
        const QualifiedBrand& qb = _trx.brandProgramVec()[legBrandIndexMap.at(legBrandCode)];
        if (!xform::formatBrandProgramData(nodeLEG, qb))
          LOG4CXX_ERROR(_logger, "Could not format brand program data");
      }
    }

    IbfErrorMessage legErrorMessage =
        itin->getIbfAvailabilityTracker().getStatusForLeg(legBrandCode, originalLegId);
    nodeLEG.convertAttr("SBL", ShoppingUtil::getIbfErrorCode(legErrorMessage));

    nodeLEG.convertAttr("Q14", originalLegId);
  }
}

bool
XMLShoppingResponse::fillLegToDirectionMap(const Itin* itin)
{
  // this is BRALL. find first not soldout space
  bool gotData = false;

  if (!itin->farePath().empty())
  {
    gotData = true;
    const FarePath* farePath = itin->farePath().front();
    for (const PricingUnit* pu : farePath->pricingUnit())
    {
      TSE_ASSERT(pu != nullptr);
      for (const FareUsage* fu : pu->fareUsage())
      {
        TSE_ASSERT(fu != nullptr);
        const PaxTypeFare* ptf = fu->paxTypeFare();
        Direction dir = ptf->getDirection();
        for (auto& travelSeg : ptf->fareMarket()->travelSeg())
          _legToDirectionMap[travelSeg->legId()] = dir;
      }
    }
  }
  return gotData;
}

void
XMLShoppingResponse::generateBrandCombinationError(Node& nodeGRI,
                                                   Itin* itin,
                                                   uint16_t poptSpaceIndex)
{
  skipper::ItinBranding& itinBranding = itin->getItinBranding();
  for (const TravelSegPtrVec& tSegs : itin->itinLegs())
  {
    if ((tSegs.front()->segmentType() == Arunk) && (tSegs.size() == 1))
      continue; // do not display ARUNK legs

    uint16_t originalLegId = tSegs.front()->legId();

    Node nodeLEG(_writer, "LEG");
    nodeLEG.convertAttr("Q14", originalLegId);
    nodeLEG.convertAttr("SBL",
                        ShoppingUtil::getIbfErrorCode(IbfAvailabilityTools::translateForOutput(
                            itinBranding.getSoldoutStatusForLeg(poptSpaceIndex, originalLegId))));

    bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(_trx);
    if (useDirectionality)
    {
      // To be able to correctly display program for soldout segments a
      // directionality information is stored inside BrandProgram class as
      // <legId, Direction> map during validation in BrandedFaresSelector::processPaxTypeFare.
      // fillLegToDirectionMap returns true if any priced solution was found.
      if (_legToDirectionMap.empty())
        useDirectionality = fillLegToDirectionMap(itin);
    }

    for (const TravelSeg* seg : tSegs)
    {
      const AirSeg* airSeg = seg->toAirSeg();
      if (airSeg == nullptr)
        continue;

      Node nodeSEG(_writer, "SEG");

      const skipper::CarrierBrandPairs& cbpairs =
          itin->getItinBranding().getCarriersBrandsForSegment(poptSpaceIndex, airSeg);
      const CarrierCode& cxr = airSeg->carrier();
      Direction direction = Direction::BOTHWAYS;
      skipper::UnorderedBrandCodes brands;
      skipper::CarrierBrandPairs::const_iterator cbp =
          cbpairs.find(skipper::CarrierDirection(cxr, direction));
      if (cbp != cbpairs.end() && cbp->second != NO_BRAND)
        brands.insert(cbp->second);

      if (useDirectionality)
      {
        TSE_ASSERT(_legToDirectionMap.find(originalLegId) != _legToDirectionMap.end());
        direction = _legToDirectionMap.at(originalLegId);
        cbp = cbpairs.find(skipper::CarrierDirection(cxr, direction));
        if (cbp != cbpairs.end() && cbp->second != NO_BRAND)
          brands.insert(cbp->second);
      }

      if (!brands.empty())
      {
        skipper::QualifiedBrandIndices indices;
        for (const BrandCode& brand : brands)
        {
          const skipper::QualifiedBrandIndices& tmpIndices =
              itin->getItinBranding().getQualifiedBrandIndicesForCarriersBrand(cxr, brand);
          indices.insert(tmpIndices.begin(), tmpIndices.end());
        }
        TSE_ASSERT(!indices.empty());

        int brandProgramIndex = 0;
        if (useDirectionality)
        {
          bool found = false;
          for (int index : indices)
          {
            TSE_ASSERT(static_cast<QualifiedBrandSizeType>(index) < _trx.brandProgramVec().size());
            QualifiedBrand& qb = _trx.brandProgramVec()[index];
            const BrandProgram::LegToDirectionMap& programMap = qb.first->getLegToDirectionMap();
            const BrandProgram::LegToDirectionMap::const_iterator programDirection =
                programMap.find(originalLegId);
            if (programDirection != programMap.end())
            {
              if ((programDirection->second == Direction::BOTHWAYS) ||
                  (direction == Direction::BOTHWAYS) || (direction == programDirection->second))
              {
                BrandProgram::OnDPair ond =
                    std::make_pair(seg->boardMultiCity(), seg->offMultiCity());
                const std::set<BrandProgram::OnDPair>& avaOnOnD = qb.first->getAvailabilityOnOnD();
                if (avaOnOnD.find(ond) != avaOnOnD.end())
                {
                  brandProgramIndex = index;
                  found = true;
                  break;
                }
              }
            }
          }
          if (!found)
            brandProgramIndex = *indices.begin();
        }
        else
        {
          // For now pick any index, as nothing could be priced anyway
          brandProgramIndex = *indices.begin();
        }

        TSE_ASSERT(static_cast<QualifiedBrandSizeType>(brandProgramIndex) <
                   _trx.brandProgramVec().size());

        const QualifiedBrand& qb = _trx.brandProgramVec()[brandProgramIndex];
        if (xform::formatBrandProgramData(nodeSEG, qb))
        {
          // TODO(karol.jurek): remove this attribute (SC0)
          nodeSEG.convertAttr(xml2::ProgramCode,
                              boost::to_upper_copy(std::string(qb.first->programCode())));
        }
        else
        {
          LOG4CXX_ERROR(_logger, "Could not format brand program data");
        }
      }
    }
  }
}

bool
XMLShoppingResponse::isFlexFareGroupValidForItin(const Itin* itin, const uint16_t flexFareGroupId)
    const
{
  if (!itin)
    return false;

  return std::any_of(itin->farePath().cbegin(),
                     itin->farePath().cend(),
                     [flexFareGroupId](const FarePath* const fp)
                     { return fp->getFlexFaresGroupId() == flexFareGroupId; });
}

void
XMLShoppingResponse::generateFlexFareError(const uint16_t flexFareGroupId)
{
  Node nodeGRI(_writer, "GRI");
  nodeGRI.convertAttr("Q17", flexFareGroupId);
  nodeGRI.convertAttr("SGL", FLEX_FARE_GROUP_NOT_OFFERED);
}

void
XMLShoppingResponse::generateHurryLogicFlag(Node& nodeShoppingResponse)
{
  if (const TrxAborter* aborter = _trx.aborter())
  {
    if (aborter->getHurryLogicActivatedFlag())
    {
      nodeShoppingResponse.attr("PHL", "T");
    }
  }
}

bool
XMLShoppingResponse::isItinApplicable(const Itin* itin) const
{
  if (UNLIKELY(_trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_OUTBOUND))
  {
    int legId = itin->legID().at(0).first;
    return legId == AirlineShoppingUtils::FIRST_LEG;
  }
  else if (UNLIKELY(_trx.getRequest()->processingDirection() ==
                    ProcessingDirection::ROUNDTRIP_INBOUND))
  {
    int legId = itin->legID().at(0).first;
    return legId == AirlineShoppingUtils::SECOND_LEG;
  }
  else
    return true;
}

// function object to tell if an itin is valid to be used
// as a solution
bool
XMLShoppingResponse::itinIsValidSolution(const Itin* itin) const
{
  bool isValid = itin->farePath().empty() == false &&
                 itin->errResponseCode() == ErrorResponseException::NO_ERROR &&
                 isItinApplicable(itin);

  return _trx.isBRAll() ? (isValid && !itin->farePath().empty()) : isValid;
}

std::string
XMLShoppingResponse::getCommercialName(const OCFeesUsage* ocFeesUsage)
{
  std::string result = ocFeesUsage->oCFees()->subCodeInfo()->commercialName();

  if (ocFeesUsage && ocFeesUsage->upgradeT198CommercialName() != EMPTY_STRING())
    result = ocFeesUsage->upgradeT198CommercialName();

  return result;
}

bool
XMLShoppingResponse::performVTAValidation(const Itin& itin) const
{
  if (!InterlineTicketCarrier::isPriceInterlineActivated(_trx))
  {
    return false;
  }

  return !itin.isHeadOfFamily() || !_trx.getOptions()->validateTicketingAgreement() ||
         !_trx.getOptions()->MIPWithoutPreviousIS();
}

void
XMLShoppingResponse::generateSpanishDiscountIndicator(Node& nodeITN, const Itin* itin)
{
  const FarePath& farePath = *itin->farePath()[0];

  if (itin == nullptr || isMIPResponse())
    return;

  const std::string indicator = SLFUtil::getIndicator(_trx, *itin, farePath);
  if (!indicator.empty())
    nodeITN.attr("PY5", indicator);
}

size_t
XMLShoppingResponse::generateQ5QForGTC(const SopIdVec& sops,
                                       std::map<std::pair<size_t, bool>, size_t>& q5qMap,
                                       size_t originalFamilyNumber)
{
  size_t newFamilyNumber = 0;

  // Determine if this Itin is 1) all GTC or 2) mixed or non
  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);
  size_t sopsSize = sops.size();
  bool isAllGtc = true;
  for (uint16_t n = 0; n != sopsSize; ++n)
  {
    std::vector<CarrierCode> marketingCxrs;

    Itin* itin = shoppingTrx.legs()[n].sop()[sops[n]].itin();
    ValidatingCxrUtil::getMarketingItinCxrs(*itin, marketingCxrs);
    if (!ValidatingCxrUtil::isGTCCarriers(_trx, marketingCxrs))
    {
      isAllGtc = false;
      break;
    }
  }

  // Based on the above determination, now we can re-map the Itin in the families
  std::map<std::pair<size_t, bool>, size_t>::iterator it =
      q5qMap.find(std::make_pair(originalFamilyNumber, isAllGtc));

  if (it == q5qMap.end()) // Create new family
  {
    q5qMap.insert(std::map<std::pair<int, bool>, int>::value_type(
        std::pair<size_t, bool>(originalFamilyNumber, isAllGtc), q5qMap.size()));
    newFamilyNumber = q5qMap.size() - 1;
  }
  else // Found an existing matching family
  {
    newFamilyNumber = it->second;
  }

  return newFamilyNumber;
}

void
XMLShoppingResponse::calculateQ5Q(std::map<std::pair<int, bool>, int>& q5qJJ,
                                  int* familyNumber,
                                  const std::string& mapKey)
{
  /* The logic of getting the new familyNumber:
     Maintain a map q5qJJ consist of ((old)familyNumber(Q5Q), bool(T(JJ), F(non JJ) Carrier ) as key
     and (new)familyNumber(Q5Q) as value.
     For a (familyNumber,isJJ) key, check if it already exists in q5qJJ.
     - If it doesn't exists, get new familyNumber and insert the key,value into q5qJJ
     - Else, use the familyNumber from q5qJJ as new familyNumber
  */
  bool isJJ = (mapKey == "JJ") ? true : false;

  std::map<std::pair<int, bool>, int>::iterator it =
      q5qJJ.find(std::make_pair(*familyNumber, isJJ));

  if (it == q5qJJ.end()) // (familyNumber,isJJ) is not in q5qJJ, insert with new familyNumber
  {
    q5qJJ.insert(std::map<std::pair<int, bool>, int>::value_type(
        std::pair<int, bool>(*familyNumber, isJJ), q5qJJ.size()));
    *familyNumber = q5qJJ.size() - 1;
  }
  else // (familyNumber,isJJ) is in q5qJJ, get the new familyNumber from q5qJJ
  {
    *familyNumber = it->second;
  }
}

} // END_namespace tse
