//------------------------------------------------------------------
//
//  File:XMLShoppingHandler.cpp
//
//  Copyright Sabre 2005
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipAulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------
#include "Xform/XMLShoppingHandler.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/Config/DynamicConfigurableString.h"
#include "Common/ConfigList.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/ExchangeUtil.h"
#include "Common/ExchShopCalendarUtils.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/LocUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/Billing.h"
#include "DataModel/Diversity.h"
#include "DataModel/ESVOptions.h"
#include "DataModel/ExchangeOverrides.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PosPaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/ReservationData.h"
#include "DataModel/RexNewItin.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/Traveler.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Nation.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DiagManager.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Xform/AvailabilityAdjuster.h"
#include "Xform/AwardPsgTypeChecker.h"
#include "Xform/CommonParserUtils.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/DataModelMap.h"
#include "Xform/DynamicConfig.h"
#include "Xform/ShoppingSchemaNames.h"
#include "Xray/XrayUtil.h"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <sys/resource.h>
#include <sys/syscall.h>

namespace
{
template <size_t n>
void
_getAttr(const IAttributes& attrs, int idx, Code<n>& str) throw()
{
  if (attrs.has(idx))
  {
    const IValueString& attr = attrs.get(idx);
    str.assign(attr.c_str(), attr.length());
  }
}

void
_getAttr(const IAttributes& attrs, int idx, std::string& str) throw()
{
  if (attrs.has(idx))
  {
    const IValueString& attr = attrs.get(idx);
    str.assign(attr.c_str(), attr.length());
  }
}

template <typename T>
void
_getAttr(const IAttributes& attrs, int idx, T& number)
{
  if (attrs.has(idx))
  {
    const IValueString& attr = attrs.get(idx);
    number = std::stoi(std::string(attr.c_str(), attr.length()));
  }
}

template <>
void
_getAttr(const IAttributes& attrs, int idx, double& number)
{
  if (attrs.has(idx))
  {
    const IValueString& attr = attrs.get(idx);
    number = std::stod(std::string(attr.c_str(), attr.length()));
  }
}

bool
isAirLineRequest(const tse::Agent* agent)
{
  return agent->tvlAgencyPCC().size() == 4;
}
}

namespace tse
{
using namespace shopping;

FALLBACK_DECL(purgeBookingCodeOfNonAlpha)
FALLBACK_DECL(fallbackBrandedFaresTNShopping)
FALLBACK_DECL(fallbackISBrandsInCalintl)
FALLBACK_DECL(fallbackNonPreferredVC)
FALLBACK_DECL(fallbackPriceByCabinActivation);
FALLBACK_DECL(fallbackPreferredVC)
FALLBACK_DECL(fallbackMipBForTNShopping);
FALLBACK_DECL(fallbackEnableFamilyLogicInBfa);
FALLBACK_DECL(azPlusUp)
FALLBACK_DECL(azPlusUpExc)
FALLBACK_DECL(fallbackFlexFareGroupNewXCLogic);
FALLBACK_DECL(fallbackFlexFareGroupNewXOLogic);
FALLBACK_DECL(fallbackCorpIDFareBugFixing);
FALLBACK_DECL(fallbackFFGAcctCodeFareFix);
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackFixProcessingRetailerCodeXRS);
FALLBACK_DECL(fallbackCbsForNoFaresFiled);
FALLBACK_DECL(fallbackNoItinParityForShopByLeg);
FALLBACK_DECL(fallbackFlexFareGroupNewJumpCabinLogic);
FALLBACK_DECL(cosExceptionFixDynamic);
FALLBACK_DECL(fallbackJumpCabinExistingLogic);
FIXEDFALLBACK_DECL(failIfIncorrectShoppingRequestType);
FALLBACK_DECL(exsCalendar)
FALLBACK_DECL(fallbackFlexFareGroupXOLogicDeffects);
FALLBACK_DECL(fallbackFFGAffectsMainFare);
FALLBACK_DECL(fallbackVITA4NonBSP);
FALLBACK_DECL(fallbackSci1126)
FALLBACK_DECL(fallbackFFGMaxPenaltyNewLogic);
FALLBACK_DECL(fallbackFFGroupIdTypeChanges);
FALLBACK_DECL(fallbackAgencyRetailerHandlingFees);
FALLBACK_DECL(fallbackPFFAffectsMainFare);

namespace
{
ConfigurableValue<ConfigVector<std::string>>
todRangesStr("SHOPPING_DIVERSITY", "TOD_RANGES", std::string("0-1439"));
ConfigurableValue<bool>
topLevelOnly("METRICS", "RECORD_TOPLINE", false);
ConfigurableValue<bool>
metricsOnly("METRICS", "RECORD_METRICS", false);
ConfigurableValue<bool>
ruleTuning("SHOPPING_OPT", "RULE_TUNING_FOR_IS_PROCESS", false);
ConfigurableValue<int16_t>
maxNumberOfOcFeesPerItin("SERVICE_FEES_SVC", "MAX_NUMBER_OF_OC_FEES_PER_ITIN");
ConfigurableValue<std::string>
groupsSummaryConfig("SERVICE_FEES_SVC", "GROUP_SUMMARY_CONFIG", "NONE");
ConfigurableValue<bool>
solGroupGenerationCfg("SOLO_CARNIVAL_OPT", "GROUP_GENERATION_CONFIG", false);
ConfigurableValue<uint32_t>
numFaresForCat12EstimationCfg("SHOPPING_OPT", "NUM_FARES_FOR_CAT12_ESTIMATION", 0);
ConfigurableValue<std::string>
startEstimatesAtIntl("SHOPPING_OPT", "START_ESTIMATES_AT_INTL");
ConfigurableValue<bool>
esvLogicCfg("SHOPPING_OPT", "ESV_LOGIC", false);
ConfigurableValue<bool>
ffLogicCfg("SHOPPING_OPT", "FF_LOGIC", false);
ConfigurableValue<bool>
bffLogicCfg("SHOPPING_OPT", "BFF_LOGIC", false);
ConfigurableValue<bool>
exchangeLogicCfg("SHOPPING_OPT", "EXCHANGE_LOGIC", false);
ConfigurableValue<int>
timeoutCfg("SHOPPING_SVC", "TIMEOUT", 0);
ConfigurableValue<int>
hurryResponse("SHOPPING_OPT", "HURRY_RESPONSE_THRESHOLD_ALTDATE_PERCENTAGE", 0);
ConfigurableValue<float>
AdditionalNsPercentageUppterLimit("SHOPPING_DIVERSITY",
                                  "ADDITIONAL_NS_PERCENTAGE_UPPER_LIMIT",
                                  std::numeric_limits<float>::max());
ConfigurableValue<int64_t>
minimumConnectTimeDomestic("SHOPPING_OPT", "MINIMUM_CONNECT_TIME_DOMESTIC", -1);
ConfigurableValue<int64_t>
minimumConnectTimeInternational("SHOPPING_OPT", "MINIMUM_CONNECT_TIME_INTERNATIONAL", -1);
//deprecated (cosExceptionFixDynamic)
ConfigurableValue<std::string>
cosExceptionFixEnabled("SHOPPING_OPT", "COS_EXCEPTION_FIX_ENABLED_FOR_PARTITIONIDS");
//^^^
ConfigurableValue<std::string>
excludePaxTypeCfg("SHOPPING_SVC", "IS_EXCLUDE_MULTI_PAXTYPE");
ConfigurableValue<int>
startEstimatesAt("SHOPPING_OPT", "START_ESTIMATES_AT");
ConfigurableValue<int>
accurateSolutions("SHOPPING_OPT", "ACCURATE_SOLUTIONS");
ConfigurableValue<bool>
allRequestsHistorical("SHOPPING_OPT", "ALL_REQUESTS_HISTORICAL", false);

DynamicConfigurableFlagOn
dynamicSoftPassDisabledInTn("TN_PATH", "DISABLE_SOFT_PASS", false);
DynamicConfigurableString
cosExceptionFixEnabledDyn("SHOPPING_OPT", "COS_EXCEPTION_FIX_ENABLED_FOR_PARTITIONIDS", std::string());
ConfigurableValue<ConfigSet<char>>
allowedRequestTypes("SHOPPING_OPT", "ALLOWED_REQUEST_TYPES");
ConfigurableValue<char>
shoppingServerType("SHOPPING_OPT", "SERVER_TYPE");
ConfigurableValue<uint32_t>
maxFcPerLeg("SHOPPING_OPT", "MAX_FC_PER_LEG", 2);
ConfigurableValue<ConfigSet<PseudoCityCode>>
pccSimpleShopping("SHOPPING_OPT", "PCC_SIMPLE_SHOPPING");
ConfigurableValue<bool>
suppressSolForAltDate("SHOPPING_OPT", "ALTDATE_SUPPRESS_SOL", false);
}

Logger
XMLShoppingHandler::_logger("atseintl.Xform.XMLShoppingHandler");
Logger
XMLShoppingHandler::ShoppingRequestParser::_logger("atseintl.Xform.XMLShoppingHandler."
                                                   "ShoppingRequestParser");

flexFares::GroupId
XMLShoppingHandler::_defaultFlexFaresGroupId(0);

flexFares::GroupId
XMLShoppingHandler::_flexFaresGroupId(0);

ConfigurableValue<ConfigSet<PseudoCityCode>>
XMLShoppingHandler::PROTypeParser::_pccsWithoutFamilyLogic(
    "SHOPPING_OPT", "PRICING_WITHOUT_FAMILY_GROUPING_PCC_LIST");

ConfigurableValue<ConfigSet<PaxTypeCode>>
XMLShoppingHandler::ElementParser::_isExcludeMultiPaxType("SHOPPING_SVC",
                                                          "IS_EXCLUDE_MULTI_PAXTYPE");

XMLShoppingHandler::XMLShoppingHandler(DataHandle& dataHandle)
  : _dataHandle(dataHandle), _bookingDT(DateTime::emptyDate())
{
}

bool
XMLShoppingHandler::startElement(int idx, const IAttributes& attrs)
{
  if (_parsers.empty())
  {
    if (_ShoppingRequest == idx)
    {
      _parsers.push(&_shoppingRequestParser);
    }
    else
    {
      // unrecognized element
      _parsers.push(nullptr);
    }
  }
  else if (ElementParser* parser = _parsers.top())
  {
    _parsers.push(parser->newElement(idx, attrs));
  }
  else
  {
    _parsers.push(nullptr);
  }
  if (_parsers.top() != nullptr)
  {
    _parsers.top()->startParsing(attrs);
  }
  return true;
}

bool
XMLShoppingHandler::endElement(int)
{
  if (_parsers.top() != nullptr)
  {
    _parsers.top()->endParsing(_value);
  }
  _parsers.pop();
  return true;
}

void
XMLShoppingHandler::characters(const char* pValue, size_t length)
{
  _value.assign(pValue, length);
}

void
XMLShoppingHandler::createFFinderTrx()
{
  if (_pricingTrx != nullptr)
  {
    return;
  }
  _dataHandle.get(_ffinderTrx);
  _dataHandle.setParentDataHandle(&_ffinderTrx->dataHandle());
  _trxResult = _pricingTrx = _shoppingTrx = _ffinderTrx;
  initTrx();
}

void
XMLShoppingHandler::createShoppingTrx()
{
  if (_pricingTrx != nullptr)
  {
    return;
  }

  _dataHandle.get(_shoppingTrx);
  _dataHandle.setParentDataHandle(&_shoppingTrx->dataHandle());
  _trxResult = _pricingTrx = _shoppingTrx;
  initTrx();
}

void
XMLShoppingHandler::createPricingTrx()
{
  if (_pricingTrx != nullptr)
  {
    return;
  }

  _dataHandle.get(_pricingTrx);
  _dataHandle.setParentDataHandle(&_pricingTrx->dataHandle());
  _trxResult = _pricingTrx;
  _shoppingTrx = nullptr;
  initTrx();
}

void
XMLShoppingHandler::createBrandingTrx()
{
  if (_brandingTrx != nullptr)
  {
    return;
  }

  _dataHandle.get(_brandingTrx);
  _dataHandle.setParentDataHandle(&_brandingTrx->dataHandle());
  _trxResult = _pricingTrx = _brandingTrx;
  _shoppingTrx = nullptr;
  initTrx();
}

void
XMLShoppingHandler::createSettlementTypesTrx()
{
  if (_settlementTypesTrx != nullptr)
  {
    return;
  }

  _dataHandle.get(_settlementTypesTrx);
  _dataHandle.setParentDataHandle(&_settlementTypesTrx->dataHandle());
  _trxResult = _pricingTrx = _settlementTypesTrx;
  _shoppingTrx = nullptr;
  initTrx();
}

void
XMLShoppingHandler::createRexShoppingTrx()
{
  if (_pricingTrx != nullptr)
  {
    return;
  }

  _dataHandle.get(_rexShoppingTrx);
  _dataHandle.setParentDataHandle(&_rexShoppingTrx->dataHandle());
  _trxResult = _pricingTrx = _rexPricingTrx = _rexShoppingTrx;
  _shoppingTrx = nullptr;
  initTrx();
}

void
XMLShoppingHandler::createRexPricingTrx()
{
  _dataHandle.get(_rexPricingTrx);
  _dataHandle.setParentDataHandle(&_rexPricingTrx->dataHandle());
  _trxResult = _pricingTrx = _rexPricingTrx;
  _shoppingTrx = nullptr;
  initTrx();
}

void
XMLShoppingHandler::createRexExchangeTrx()
{
  _dataHandle.get(_rexExchangeTrx);
  _dataHandle.setParentDataHandle(&_rexExchangeTrx->dataHandle());
  _trxResult = _pricingTrx = _rexPricingTrx = _rexExchangeTrx;
  _shoppingTrx = nullptr;
  initTrx();
}

void
XMLShoppingHandler::initTrx()
{
  if (_shoppingTrx != nullptr)
  {
    ESVOptions* esvOptions;
    _dataHandle.get(esvOptions);
    _shoppingTrx->esvOptions() = esvOptions;
    VISOptions* visOptions;
    _dataHandle.get(visOptions);
    _shoppingTrx->visOptions() = visOptions;

    std::vector<std::pair<uint16_t, uint16_t>> todRanges;

    for (const auto todRange : todRangesStr.getValue())
    {
      size_t pos = todRange.find('-');
      todRanges.push_back(
          std::make_pair(atoi(todRange.substr(0, pos).c_str()),
                         atoi(todRange.substr(pos + 1, std::string::npos).c_str())));
    }

    _shoppingTrx->diversity().setTODRanges(todRanges);
    std::vector<float> todDistribution(todRanges.size(), 1.0 / todRanges.size());
    _shoppingTrx->diversity().setTODDistribution(todDistribution);
  }
  // For both exchange shopping and pricing
  if (_rexPricingTrx != nullptr)
  {
    RexPricingOptions* options;
    _dataHandle.get(options);
    _rexPricingTrx->setOptions(options);

    RexPricingRequest* rexPricingRequest;
    _dataHandle.get(rexPricingRequest);
    _rexPricingTrx->setRequest(rexPricingRequest);

    rexPricingRequest->setTrx(_rexPricingTrx);

    _rexPricingTrx->reqType() = AUTOMATED_REISSUE;
    _rexPricingTrx->setFareApplicationDT(rexPricingRequest->getTicketingDT());

    ExcItin* excItin;
    _dataHandle.get(excItin);
    _rexPricingTrx->exchangeItin().push_back(excItin);

    excItin->rexTrx() = _rexPricingTrx;

    Agent* rexAgent;
    _dataHandle.get(rexAgent);
    static_cast<RexPricingRequest*>(_rexPricingTrx->getRequest())->prevTicketIssueAgent() =
        rexAgent;

    Loc* rexAgentLocation;
    _dataHandle.get(rexAgentLocation);
    rexAgent->agentLocation() = rexAgentLocation;
  }
  else
  {
    PricingOptions* options;
    _dataHandle.get(options);
    _pricingTrx->setOptions(options);

    PricingRequest* pricingRequest;
    _dataHandle.get(pricingRequest);
    _pricingTrx->setRequest(pricingRequest);
  }

  PricingRequest* request = _pricingTrx->getRequest();

  request->ticketingDT() = DateTime::localTime();
  request->lowFareRequested() = 'T';

  Agent* agent;
  _dataHandle.get(agent);
  request->ticketingAgent() = agent;

  Loc* agentLocation;
  _dataHandle.get(agentLocation);
  agent->agentLocation() = agentLocation;

  ReservationData* reservData;
  _dataHandle.get(reservData);
  request->reservationData() = reservData;

  if (topLevelOnly.getValue())
  {
    _trxResult->recordTopLevelMetricsOnly() = true;
  }
  else if (!topLevelOnly.isDefault())
  {
    _trxResult->recordTopLevelMetricsOnly() = false;
  }

  if (metricsOnly.getValue())
  {
    _trxResult->recordMetrics() = true;
  }
  else if (!metricsOnly.isDefault())
  {
    _trxResult->recordMetrics() = false;
  }
}

std::string
XMLShoppingHandler::ShoppingRequestParser::serverType() const
{
  // In order to uniquely identify a shopping server we introduced a new configuration
  // variable. We can have only for values: H, M, I, and E.
  using KeyDescription = std::pair<char, std::string>;
  static const std::array<KeyDescription, 4> desc{std::make_pair('H', std::string{"Historical"}),
                                                  std::make_pair('M', std::string{"MIP"}),
                                                  std::make_pair('I', std::string{"IS"}),
                                                  std::make_pair('E', std::string{"ESV"})};

  const auto type = shoppingServerType.getValue();
  const auto result = std::find_if(std::cbegin(desc),
                                   std::cend(desc),
                                   [type](const auto& elem)
                                   { return type == elem.first; });
  return result != std::cend(desc) ? result->second : "Unknown";
}

uint16_t
XMLShoppingHandler::ElementParser::determinePaxAge(PaxTypeCode& work)
{
  if (work.size() >= 3 && isdigit(work[1]) && isdigit(work[2]))
  {
    int age = atoi(work.substr(1, 2).c_str());
    work[1] = 'N';
    work[2] = 'N';
    return age;
  }
  else
  {
    return 0;
  }
}

PricingTrx&
XMLShoppingHandler::ElementParser::trx()
{
  if (UNLIKELY(_parser._pricingTrx == nullptr))
  {
    _parser.createShoppingTrx();
  }

  return *_parser._pricingTrx;
}

ShoppingTrx*
XMLShoppingHandler::ElementParser::shoppingTrx()
{
  if (UNLIKELY(!_parser._pricingTrx))
    _parser.createShoppingTrx();

  return _parser._shoppingTrx;
}

FlightFinderTrx*
XMLShoppingHandler::ElementParser::ffinderTrx()
{
  if (!_parser._ffinderTrx)
    _parser.createFFinderTrx();

  validateInput(_parser._ffinderTrx != nullptr, "Unexpected BFF/FF attributes");
  return _parser._ffinderTrx;
}

std::vector<ShoppingTrx::Leg>&
XMLShoppingHandler::ElementParser::legs()
{
  ShoppingTrx* const trx = shoppingTrx();

  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    if (rexShoppingTrx() != nullptr)
    {
      return rexShoppingTrx()->excLegs();
    }
    else if (rexPricingTrx() != nullptr)
    {
      return _parser._excLegs;
    }
  }

  if (trx != nullptr)
  {
    return trx->legs();
  }

  return _parser._pricingLegs;
}

void
XMLShoppingHandler::ElementParser::setAAFIndex(uint32_t index, uint32_t mapto)
{
  if (UNLIKELY(_parser.parsingExchangeInformation()))
  {
    _parser._aafExcIndices[index] = mapto;
  }
  else
  {
    _parser._aafIndices[index] = mapto;
  }
}

uint32_t
XMLShoppingHandler::ElementParser::getAAFIndex(uint32_t index) const
{
  if (UNLIKELY(_parser.parsingExchangeInformation()))
  {
    const std::map<uint32_t, uint32_t>::const_iterator i = _parser._aafExcIndices.find(index);
    validateInput(i != _parser._aafExcIndices.end(), "Illegal Exchange AAF Index");
    return i->second;
  }

  const std::map<uint32_t, uint32_t>::const_iterator j = _parser._aafIndices.find(index);
  validateInput(j != _parser._aafIndices.end(), "Illegal AAF Index");
  return j->second;
}

void
XMLShoppingHandler::ElementParser::setLEGIndex(uint32_t index, uint32_t mapto)
{
  if (_parser.parsingExchangeInformation())
  {
    validateInput(_parser._legExcIndices.count(index) == 0, "Duplicate Exchange LEG IDs found");
    _parser._legExcIndices[index] = mapto;
    return;
  }

  validateInput(_parser._legIndices.count(index) == 0, "Duplicate LEG IDs found");
  _parser._legIndices[index] = mapto;
}

uint32_t
XMLShoppingHandler::ElementParser::getLEGIndex(uint32_t index) const
{
  if (UNLIKELY(_parser.parsingExchangeInformation()))
  {
    const std::map<uint32_t, uint32_t>::const_iterator i = _parser._legExcIndices.find(index);
    validateInput(i != _parser._legExcIndices.end(), "Illegal Exchange LEG Index");
    return i->second;
  }

  const std::map<uint32_t, uint32_t>::const_iterator j = _parser._legIndices.find(index);
  validateInput(j != _parser._legIndices.end(), "Illegal LEG Index");
  return j->second;
}

void
XMLShoppingHandler::ElementParser::setSOPIndex(uint32_t leg, uint32_t sop, uint32_t mapto)
{
  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    if (rexPricingTrx()->excSchedulingOptionIndices().size() <= static_cast<size_t>(leg))
    {
      rexPricingTrx()->excSchedulingOptionIndices().resize(leg + 1);
    }

    if (rexPricingTrx()->excIndicesToSchedulingOption().size() <= static_cast<size_t>(leg))
    {
      rexPricingTrx()->excIndicesToSchedulingOption().resize(leg + 1);
    }

    validateInput(rexPricingTrx()->excSchedulingOptionIndices()[leg].count(sop) == 0,
                  "Duplicate Exchange SOP IDs found");

    rexPricingTrx()->excSchedulingOptionIndices()[leg][sop] = mapto;
    rexPricingTrx()->excIndicesToSchedulingOption()[leg][mapto] = sop;
    return;
  }

  if (trx().schedulingOptionIndices().size() <= static_cast<size_t>(leg))
  {
    trx().schedulingOptionIndices().resize(leg + 1);
  }

  if (trx().indicesToSchedulingOption().size() <= static_cast<size_t>(leg))
  {
    trx().indicesToSchedulingOption().resize(leg + 1);
  }

  validateInput(trx().schedulingOptionIndices()[leg].count(sop) == 0, "Duplicate SOP IDs found");

  trx().schedulingOptionIndices()[leg][sop] = mapto;

  if (trx().getTrxType() == PricingTrx::MIP_TRX)
  {
    // For IS, this map will be created in ItinAnalyzer
    trx().indicesToSchedulingOption()[leg][mapto] = sop;
  }
}

uint32_t
XMLShoppingHandler::ElementParser::getSOPIndex(uint32_t leg, uint32_t sop)
{
  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    validateInput(leg < static_cast<uint32_t>(rexPricingTrx()->excSchedulingOptionIndices().size()),
                  "Illegal Exchange LEG Index");

    const std::map<uint32_t, uint32_t>::const_iterator i =
        rexPricingTrx()->excSchedulingOptionIndices()[leg].find(sop);

    validateInput(i != rexPricingTrx()->excSchedulingOptionIndices()[leg].end(),
                  "Illegal Exchange SOP Index");
    return i->second;
  }
  validateInput(leg < static_cast<uint32_t>(trx().schedulingOptionIndices().size()),
                "Illegal LEG Index");
  const std::map<uint32_t, uint32_t>::const_iterator i =
      trx().schedulingOptionIndices()[leg].find(sop);
  validateInput(i != trx().schedulingOptionIndices()[leg].end(), "Illegal SOP Index");
  return i->second;
}

void
XMLShoppingHandler::ElementParser::validateInput(bool isValid, const char* msg, bool doAlert) const
{
  if (!isValid)
  {
    if (doAlert)
      TseUtil::alert(msg);
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, msg);
  }
}

void
XMLShoppingHandler::PXITypeParser::startParsing(const IAttributes& attrs)
{
  if (PricingTrx::ESV_TRX == trx().getTrxType())
  {
    return;
  }

  PaxType* pax = nullptr;
  dataHandle().get(pax);
  if (attrs.has(_B70))
  {
    _getAttr(attrs, _B70, pax->paxType());
  }
  if (pax->paxType().empty())
  {
    pax->paxType() = "ADT";
  }

  pax->requestedPaxType() = pax->paxType();

  unsigned short defaultPaxNumber(1);
  unsigned short defaultTotalPaxNumber(1);

  attrs.get(_Q0U, pax->number(), defaultPaxNumber);
  if (0 == pax->number())
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Invalid number of passenger for passenger type, 0");
  }
  attrs.get(_Q0T, pax->age(), pax->age());

  if (trx().getTrxType() == PricingTrx::MIP_TRX && trx().excTrxType() == PricingTrx::AR_EXC_TRX)
    attrs.get(_Q79, pax->totalPaxNumber(), defaultTotalPaxNumber);

  if (trx().excTrxType() == PricingTrx::EXC_IS_TRX)
    attrs.get(_Q79, pax->number(), defaultTotalPaxNumber);

  const uint16_t age = determinePaxAge(pax->paxType());
  if (age)
  {
    pax->age() = age;
  }

  if (attrs.has(_A30))
  {
    _getAttr(attrs, _A30, pax->stateCode());
  }

  if (attrs.get(_MPO) == "I")
  {
    _mpo = smp::INFO;
  }
  else if (attrs.get(_MPO) == "O")
  {
    _mpo = smp::OR;
  }
  else if (attrs.get(_MPO) == "A")
  {
    _mpo = smp::AND;
  }
  else if (attrs.has(_MPO))
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INCORRECT MAXIMUM PENALTY OPERATION");
  }

  if (parser().parsingExchangeInformation())
  {
    if (parser().parsingAccompanyPassenger())
    {
      rexPricingTrx()->accompanyPaxType().push_back(pax);
    }
    else
    {
      rexPricingTrx()->exchangePaxType() = pax;
      rexPricingTrx()->excPaxType().push_back(pax);
    }
  }
  else
  {
    if (parser().parsingAccompanyPassenger())
    {
      validateInput(rexPricingTrx() != nullptr, "Unexpected APX element in non Exchange request");
      rexPricingTrx()->accompanyPaxType().push_back(pax);
    }
    else
    {
      trx().paxType().push_back(pax);
      trx().posPaxType().push_back(PosPaxTypePerPax());
    }
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::PXITypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (PricingTrx::ESV_TRX == trx().getTrxType())
  {
    return nullptr;
  }

  if (_FGG == idx)
  {
    return &parser()._fggParser;
  }

  if (_PEN == idx)
  {
    return &parser()._penParser;
  }

  return nullptr;
}

void
XMLShoppingHandler::ElementParser::paxTypesInitialize()
{
  uint16_t paxInputOrder = 0;
  for (PaxType* pax : trx().paxType())
    PaxTypeUtil::initialize(
        trx(), *pax, pax->paxType(), pax->number(), pax->age(), pax->stateCode(), paxInputOrder++);

  excludePaxTypesForIS(trx().paxType());
}

void
XMLShoppingHandler::PXITypeParser::endParsing(const IValueString& text)
{
  if (PricingTrx::ESV_TRX == trx().getTrxType())
  {
    return;
  }

  if (parser().parsingExchangeInformation())
  {
    if (parser().parsingAccompanyPassenger())
    {
      PaxType* accPax = rexPricingTrx()->accompanyPaxType().back();

      PaxTypeUtil::initialize(trx(),
                              *accPax,
                              accPax->paxType(),
                              accPax->number(),
                              accPax->age(),
                              accPax->stateCode(),
                              0);
    }
    else
    {
      PaxType* rexPax = rexPricingTrx()->excPaxType().back();

      PaxTypeUtil::initialize(trx(),
                              *rexPax,
                              rexPax->paxType(),
                              rexPax->number(),
                              rexPax->age(),
                              rexPax->stateCode(),
                              0);
    }
    return;
  }
  else
  {
    if (parser().parsingAccompanyPassenger())
    {
      PaxType* accPax = rexPricingTrx()->accompanyPaxType().back();

      PaxTypeUtil::initialize(trx(),
                              *accPax,
                              accPax->paxType(),
                              accPax->number(),
                              accPax->age(),
                              accPax->stateCode(),
                              0);
      return;
    }
  }

  PosPaxTypePerPax& posPaxType = trx().posPaxType().back();
  PaxType* pax = trx().paxType().back();

  if (!posPaxType.empty())
  {
    PaxType* paxType;
    dataHandle().get(paxType);
    paxType->paxType() = pax->paxType();

    if (posPaxType.size() == 1 && posPaxType[0]->paxType() == "JCB")
    {
      paxType->paxType() = "JCB";
    }
    else if (pax->paxType() == "ADT" || (posPaxType[0]->paxType() == "ADT"))
    {
      paxType->paxType() = "NEG";
    }
    paxType->number() = pax->number();
    paxType->age() = pax->age();
    trx().paxType().pop_back();
    trx().paxType().push_back(paxType);
    pax = paxType;
  }

  pax->maxPenaltyInfo() =
      CommonParserUtils::validateMaxPenaltyInput(trx(),
                                                 std::move(_mpo),
                                                 std::move(parser()._penParser._changeFilter),
                                                 std::move(parser()._penParser._refundFilter));

  if (parser().isTagPROProcessed())
  {
    PaxTypeUtil::initialize(
        trx(), *pax, pax->paxType(), pax->number(), pax->age(), pax->stateCode(), _paxInputOrder);
    if (trx().paxType().size() > 1)
      excludePaxTypeForIS(trx().paxType().back());

    const flexFares::GroupId defaultFlexFaresGroupID = parser().defaultFlexFaresGroupID(trx());
    if (!fallback::fallbackFFGMaxPenaltyNewLogic(&trx()) && defaultFlexFaresGroupID > 0)
    {//Flex fare group: save maxPenaltyInfo
      PricingRequest& request = *trx().getRequest();
      if (!request.getMutableFlexFaresGroupsData().isFlexFareGroup(defaultFlexFaresGroupID))
      {
        request.getMutableFlexFaresGroupsData().createNewGroup(defaultFlexFaresGroupID);
        request.getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true,defaultFlexFaresGroupID);
      }
      request.getMutableFlexFaresGroupsData().setFFGMaxPenaltyInfo(pax->maxPenaltyInfo(), defaultFlexFaresGroupID);
    }
  }

  if (!fallback::fallbackFFGMaxPenaltyNewLogic(&trx()))
  {//Reset MaxPenaltyInfo for _changeFilter/_refundFilter
    parser()._penParser._changeFilter = MaxPenaltyInfo::Filter{smp::BOTH, {}, {}};
    parser()._penParser._refundFilter = MaxPenaltyInfo::Filter{smp::BOTH, {}, {}};
    parser().setTagPXIProcessed();
  }
  ++_paxInputOrder;
}

void
XMLShoppingHandler::ElementParser::excludePaxTypeForIS(PaxType* paxType)
{
  if ((trx().getTrxType() != PricingTrx::IS_TRX) || !parser()._shoppingTrx)
  {
    return;
  }

  if (_isExcludeMultiPaxType.getValue().has(paxType->paxType()))
  {
    parser()._shoppingTrx->excludedPaxType().push_back(paxType);
    trx().paxType().erase(std::remove(trx().paxType().begin(), trx().paxType().end(), paxType));
  }
}

void
XMLShoppingHandler::ElementParser::excludePaxTypesForIS(std::vector<PaxType*>& paxTypes)
{
  if ((trx().getTrxType() != PricingTrx::IS_TRX) || (paxTypes.size() <= 1) ||
      !parser()._shoppingTrx)
  {
    return;
  }

  for (auto paxTypeIt = paxTypes.begin(); paxTypeIt != paxTypes.end();)
  {
    if (_isExcludeMultiPaxType.getValue().has((*paxTypeIt)->paxType()))
    {
      parser()._shoppingTrx->excludedPaxType().push_back(*paxTypeIt);
      paxTypeIt = paxTypes.erase(paxTypeIt);
    }
    else
      ++paxTypeIt;
  }
}

void
XMLShoppingHandler::PENTypeParser::startParsing(const IAttributes& attrs)
{
  if (trx().isExchangeTrx())
  {
    return;
  }

  if (!attrs.has(_MPT))
  {
    _changeFilter = getPenaltyInfo(trx(), attrs);
    _refundFilter = getPenaltyInfo(trx(), attrs);
  }
  else if (attrs.get(_MPT) == "C")
  {
    _changeFilter = getPenaltyInfo(trx(), attrs);
  }
  else if (attrs.get(_MPT) == "R")
  {
    _refundFilter = getPenaltyInfo(trx(), attrs);
  }
  else
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INCORRECT MAXIMUM PENALTY APPLICATION INDICATOR");
  }
}

MaxPenaltyInfo::Filter
XMLShoppingHandler::PENTypeParser::getPenaltyInfo(const PricingTrx& trx, const IAttributes& attrs)
    const
{
  smp::RecordApplication abd = smp::BOTH;
  boost::optional<smp::ChangeQuery> mpi;
  boost::optional<MoneyAmount> mpa;
  boost::optional<CurrencyCode> mpc;

  if (attrs.get(_ABD) == "A")
  {
    abd = smp::AFTER;
  }
  else if (attrs.get(_ABD) == "B")
  {
    abd = smp::BEFORE;
  }
  else if (attrs.has(_ABD))
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INCORRECT FORMAT BEFORE/AFTER DEPARTURE");
  }

  if (attrs.get(_MPI) == "A")
  {
    mpi = boost::make_optional(smp::CHANGEABLE);
  }
  else if (attrs.get(_MPI) == "N")
  {
    mpi = boost::make_optional(smp::NONCHANGEABLE);
  }
  else if (attrs.has(_MPI))
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INCORRECT MAXIMUM PENALTY CHANGE IDENTIFICATOR");
  }

  if (attrs.has(_MPA))
  {
    try
    {
      mpa = std::stod(attrs.get(_MPA).c_str());
    }
    catch (...)
    {
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "INVALID MAXIMUM PENALTY CHANGE AMOUNT - MODIFY AND REENTER");
    }
  }

  if (attrs.has(_MPC))
    mpc = attrs.get(_MPC).c_str();

  return CommonParserUtils::validateMaxPenaltyFilter(trx, abd, mpi, mpa, mpc);
}

void
XMLShoppingHandler::OPTTypeParser::startParsing(const IAttributes& attrs)
{
  if(attrs.has(_SFM))
  {
    if (attrs.get(_SFM, false))
    {
      trx().getRequest()->setSimpleShoppingRQ(true);
      trx().getRequest()->setMaxFCsPerLeg(maxFcPerLeg.getValue());
    }
    else
    {
      trx().getRequest()->setSimpleShoppingRQ(false);
      trx().getRequest()->setMaxFCsPerLeg(0);
    }
  }

  if (attrs.has(_FPL))
  {
    uint32_t maxFcPerLeg = trx().getRequest()->getMaxFCsPerLeg();
    attrs.get(_FPL, maxFcPerLeg, maxFcPerLeg);
    trx().getRequest()->setMaxFCsPerLeg(maxFcPerLeg);
  }

}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::FGGTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_FGI == idx)
  {
    parseFGIType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::FGGTypeParser::parseFGIType(const IAttributes& attrs)
{
  if (parser().parsingExchangeInformation())
  {
    return;
  }
  PosPaxType* posPaxType = nullptr;
  dataHandle().get(posPaxType);
  if (attrs.has(_B70))
  {
    _getAttr(attrs, _B70, posPaxType->paxType());
    posPaxType->age() = determinePaxAge(posPaxType->paxType());
  }
  if (posPaxType->paxType().empty())
  {
    return;
  }
  if (attrs.has(_A01))
  {
    _getAttr(attrs, _A01, posPaxType->pcc());
  }
  if (attrs.get(_P33) == "F")
  {
    posPaxType->positive() = false;
  }
  else
  {
    posPaxType->positive() = true;
  }
  attrs.get(_AC0, posPaxType->corpID(), posPaxType->corpID());
  attrs.get(_Q18, posPaxType->fgNumber(), posPaxType->fgNumber());
  attrs.get(_Q0C, posPaxType->priority(), posPaxType->priority());

  trx().posPaxType().back().push_back(posPaxType);
}

namespace
{
bool
parseDate(const IValueString& str, int32_t& year, int32_t& month, int32_t& day)
{
  const bool valid = str.length() == 10 && str[4] == '-' && str[7] == '-' &&
                     std::count_if(str.begin(), str.end(), isdigit) == 8;
  if (UNLIKELY(valid == false))
  {
    return false;
  }
  const char* ptr = str.c_str();
  char* endptr = nullptr;
  year = strtol(ptr, &endptr, 10);
  month = strtol(ptr + 5, &endptr, 10);
  day = atoi(ptr + 8);
  return true;
}
}
// parse date in YYYY-MM-DD format, and time in HHMM format

DateTime
XMLShoppingHandler::ElementParser::parseDateTime(const IValueString& date, const IValueString& time)
{
  if (date.empty())
  {
    return DateTime();
  }
  int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0;
  const bool res(parseDate(date, year, month, day));
  validateInput(res, "Dates must be in YYYY-MM-DD format");

  validateInput(time.empty() || (4 == time.length() &&
                                 std::count_if(time.begin(), time.end(), isdigit) ==
                                     static_cast<int>(time.length())),
                "Times must be in HHMM format");
  if (LIKELY(4 == time.length()))
  {
    int dummy(atoi(time.c_str()));
    hour = dummy / 100;
    minute = dummy % 100;
  }
  return DateTime(year, month, day, hour, minute, 0);
}
// parse date in YYYY-MM-DD format

DateTime
XMLShoppingHandler::ElementParser::parseDateTime(const IValueString& date)
{
  if (date.empty())
  {
    return DateTime();
  }
  int32_t year = 0, month = 0, day = 0;
  const bool res = parseDate(date, year, month, day);
  validateInput(res, "Dates must be in YYYY-MM-DD format");

  return DateTime(year, month, day);
}

void
XMLShoppingHandler::ElementParser::setTimeInHistoricalDate(const IAttributes& attrs,
                                                           DateTime& histDate,
                                                           _AttributeNameIdx_ timeSource)
{
  if (attrs.has(timeSource))
  {
    unsigned minutesCount = attrs.get<unsigned>(timeSource, 0);
    histDate.addMinutes(minutesCount);
    histDate.setHistoricalIncludesTime();
  }
}

void
XMLShoppingHandler::PROTypeParser::parseESVOptions(const IAttributes& attrs)
{
  ESVOptions*& esvOptions = shoppingTrx()->esvOptions();
  if (esvOptions == nullptr)
  {
    dataHandle().get(esvOptions);
  }

  if (attrs.has(_P80))
  {
    const IValueString& mustPriceFlag = attrs.get(_P80);
    esvOptions->mustPriceFlag() = mustPriceFlag == "T" || mustPriceFlag == "t";
  }
  if (attrs.has(_Q0S))
  {
    esvOptions->setRequestedNumberOfSolutions(attrs.get<int>(_Q0S));
  }
  if (attrs.has(_Q0E))
  {
    attrs.get(_Q0E, esvOptions->noOfESVLowFareSolutionsReq());
  }
  if (attrs.has(_Q60))
  {
    attrs.get(_Q60, esvOptions->noOfMustPriceOnlineSolutions());
  }
  if (attrs.has(_Q61))
  {
    attrs.get(_Q61, esvOptions->noOfMustPriceInterlineSolutions());
  }
  if (attrs.has(_Q62))
  {
    attrs.get(_Q62, esvOptions->noOfMustPriceNonstopOnlineSolutions());
  }
  if (attrs.has(_Q63))
  {
    attrs.get(_Q63, esvOptions->noOfMustPriceNonstopInterlineSolutions());
  }
  if (attrs.has(_Q64))
  {
    attrs.get(_Q64, esvOptions->noOfMustPriceSingleStopOnlineSolutions());
  }
  if (attrs.has(_Q65))
  {
    attrs.get(_Q65, esvOptions->perStopPenalty());
  }
  if (attrs.has(_Q66))
  {
    attrs.get(_Q66, esvOptions->travelDurationPenalty());
  }
  if (attrs.has(_Q67))
  {
    attrs.get(_Q67, esvOptions->departureTimeDeviationPenalty());
  }
  if (attrs.has(_Q68))
  {
    attrs.get(_Q68, esvOptions->percentFactor());
  }
  if (attrs.has(_Q69))
  {
    attrs.get(_Q69, esvOptions->flightOptionReuseLimit());
  }
  if (attrs.has(_Q6A))
  {
    attrs.get(_Q6A, esvOptions->upperBoundFactorForNotNonstop());
  }
  if (attrs.has(_Q6Q))
  {
    attrs.get(_Q6Q, esvOptions->noOfMustPriceNonAndOneStopOnlineSolutions());
  }
  if (attrs.has(_Q6R))
  {
    attrs.get(_Q6R, esvOptions->noOfMustPriceNonAndOneStopInterlineSolutions());
  }
  if (attrs.has(_Q6S))
  {
    attrs.get(_Q6S, esvOptions->upperBoundFactorForNonstop());
  }
  // LFS
  if (attrs.has(_Q5T))
  {
    attrs.get(_Q5T, esvOptions->esvPercent());
  }
  if (attrs.has(_Q5U))
  {
    attrs.get(_Q5U, esvOptions->noOfMinOnlinePerCarrier());
  }
  if (attrs.has(_Q5V))
  {
    attrs.get(_Q5V, esvOptions->onlinePercent());
  }
  if (attrs.has(_Q5W))
  {
    attrs.get(_Q5W, esvOptions->loMaximumPerOption());
  }
  if (attrs.has(_Q5X))
  {
    attrs.get(_Q5X, esvOptions->hiMaximumPerOption());
  }
  if (attrs.has(_Q6U))
  {
    attrs.get(_Q6U, esvOptions->upperBoundFactorForLFS());
  }
  if (attrs.has(_S5A))
  {
    attrs.get(_S5A, esvOptions->avsCarriersString());
  }
}

void
XMLShoppingHandler::PROTypeParser::parseBrandingOptions(const IAttributes& attrs)
{
  parseIBFOptions(attrs);
  parseTNBrandOptions(attrs);

  const bool isBrandsForTnShopping = trx().isBrandsForTnShopping();
  const bool isIbf = trx().getRequest()->isBrandedFaresRequest();
  const bool isMipB = (dynamic_cast<const BrandingTrx*>(&trx()) != nullptr);

  validateInput(!(trx().isBrandsForTnShopping() && trx().getRequest()->isBrandedFaresRequest()),
                "Brands for Tn Shopping and IBF cannot be enabled together");

  if (fallback::fallbackMipBForTNShopping(&trx()))
    return;

  // logic previous to introduction on MIP-B for TN shopping was to assume MIP-B is IBF by default.
  if (isMipB && (!isBrandsForTnShopping && !isIbf))
    parser().markBrandedFaresRequest(*(trx().getRequest()));
}

namespace
{
void
parseBool(const IAttributes& attrs, unsigned int attrIdx, std::function<void(void)> actionWhenTrue)
{
  if (attrs.has(attrIdx))
  {
    char ch = 0;
    attrs.get(attrIdx, ch);
    if ('T' == ch)
    {
      actionWhenTrue();
    }
  }
};

void
parseUInt(const IAttributes& attrs, unsigned int attrIdx, std::function<void(uint16_t)> action)
{
  if (attrs.has(attrIdx))
  {
    uint16_t value = attrs.get<uint16_t>(attrIdx);
    action(value);
  }
};

bool
isTrueBoolean(const IAttributes& attrs, unsigned int attrIdx)
{
  if (attrs.has(attrIdx))
  {
    char ch = 0;
    attrs.get(attrIdx, ch);
    if ('T' == ch)
      return true;
  }
  return false;
}

// returns unitialized variable if attribute is absent or set to false
// returns numeric value if set to a number
// returns zero if "T"
boost::optional<size_t>
isTrueOrNumeric(const IAttributes& attrs, unsigned int attrIdx)
{
  if (attrs.has(attrIdx))
  {
    char ch = attrs.get<char>(attrIdx, 'X');
    if (ch == 'T')
    {
      // is set but without a numerical value
      return boost::optional<size_t>(0);
    }
    else if (ch != 'F')
    {
      size_t value = attrs.get<size_t>(attrIdx);
      return boost::optional<size_t>(value);
    }
  }
  return boost::optional<size_t>();
}

} // namespace

void
XMLShoppingHandler::PROTypeParser::parseIBFOptions(const IAttributes& attrs)
{
  PricingRequest* const request = trx().getRequest();

  parseBool(attrs, _BFR, [&]()
            { parser().markBrandedFaresRequest(*request); });
  parseBool(attrs, _CAB, [&]()
            { parser().setCatchAllBucket(*request, true); });

  if (trx().getTrxType() == PricingTrx::MIP_TRX || !fallback::fallbackISBrandsInCalintl(&trx()))
    parseBool(attrs, _BPL, [&]()
              { parser().enableCheapestWithLegParityPath(*request); });

  if (fallback::fallbackCbsForNoFaresFiled(&trx()))
  {
    parseBool(attrs, _CBS, [&]()
              { parser().markChangeSoldoutBrand(*request); });
  }
  else
  {
    if (attrs.has(_CBS))
    {
      char ch = 0;
      attrs.get(_CBS, ch);
      if ('T' == ch || 'X' == ch)
      {
        parser().markChangeSoldoutBrand(*request);
        if ('X' == ch)
          request->setUseCbsForNoFares(true);
      }
    }
  }

  if (!fallback::fallbackNoItinParityForShopByLeg(&trx()))
  {
    parseBool(attrs, _PBO, [&]()
              { request->setProcessParityBrandsOverride(true); });
  }

  parseBool(attrs, _UAF, [&]()
            { parser().markAllFlightsRepresented(*request); });
  parseBool(attrs, _IMT, [&]()
            { parser().markReturnIllogicalFlights(*request); });
  parseBool(attrs, _AFD, [&]()
            { parser().setAllFlightsData(*request); });
  parseUInt(attrs, _SRL, [&](uint16_t x)
            { parser().setScheduleRepeatLimit(*request, x); });
}

void
XMLShoppingHandler::PROTypeParser::parseTNBrandOptions(const IAttributes& attrs)
{
  if (fallback::fallbackBrandedFaresTNShopping(&trx()))
    return;

  if ((trx().getTrxType() != PricingTrx::MIP_TRX) && (trx().getTrxType() != PricingTrx::IS_TRX))
    return;

  if (trx().getTrxType() == PricingTrx::MIP_TRX)
    parseBool(attrs,
              _BFS,
              [&]()
              {
      trx().setSoftPassDisabled(dynamicSoftPassDisabledInTn.isValid(&trx()));
      trx().setTnShoppingBrandingMode(TnShoppingBrandingMode::SINGLE_BRAND);
    });

  if (isTrueBoolean(attrs, _P1F)) // alt dates are only supported in BFS mode in MIP
    return;

  boost::optional<size_t> bfa = isTrueOrNumeric(attrs, _BFA);

  if (bfa)
  {
    // in shopping in TN path disable soft passes for BFA/BFS (if option set to true)
    trx().setSoftPassDisabled(dynamicSoftPassDisabledInTn.isValid(&trx()));
    size_t value = bfa.get();

    if (value == 1)
    {
      // SINGLE BRAND mode after all. It's only valid for MIP
      if (trx().getTrxType() == PricingTrx::IS_TRX)
        return;

      trx().setTnShoppingBrandingMode(TnShoppingBrandingMode::SINGLE_BRAND);
      return;
    }

    // MULTIPLE_BRANDS mode
    if (trx().getTrxType() == PricingTrx::MIP_TRX)
    {
      trx().setTnShoppingBrandingMode(TnShoppingBrandingMode::MULTIPLE_BRANDS);
      trx().setNumberOfBrands(value);
      if (fallback::fallbackEnableFamilyLogicInBfa(&trx()))
        parser().setDisableFamilyLogic(true);

      return;
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseGSAOptions(const IAttributes& attrs)
{
  PricingRequest* const request = trx().getRequest();
  bool parseSM0andDVL = true;

  if (attrs.has(_VCX))
  {
    char ch = 0;
    attrs.get(_VCX, ch);
    if ('T' == ch)
    {
      parseSM0andDVL = false;
      parser().setValidatingCarrierRequest(*request, true);
      if (attrs.has(_DVL))
      {
        ch = 0;
        attrs.get(_DVL, ch);
        if ('T' == ch)
        {
          parser().setAlternateValidatingCarrierRequest(*request, true);
        }
        else
        {
          parser().setAlternateValidatingCarrierRequest(*request, false);
        }
      }
      else
      {
        parser().setAlternateValidatingCarrierRequest(*request, true);
      }
      if (attrs.has(_SM0))
      {
        std::string smoValue = attrs.get<std::string>(_SM0);
        parser().setSettlementMethodOverride(*request, smoValue);
      }
    }
    else
    {
      parser().setValidatingCarrierRequest(*request, false);
      parser().setAlternateValidatingCarrierRequest(*request, false);
    }
  }
  else
  {
    parser().setValidatingCarrierRequest(*request, false);
    parser().setAlternateValidatingCarrierRequest(*request, false);
  }

  if (parseSM0andDVL)
  {
    if (attrs.has(_DVL))
    {
      char ch = 0;
      attrs.get(_DVL, ch);
      if ('F' == ch)
      {
        parser().setAlternateValidatingCarrierRequest(*request, false);
      }
    }

    if (attrs.has(_SM0))
    {
      std::string smoValue = attrs.get<std::string>(_SM0);
      parser().setSettlementMethodOverride(*request, smoValue);
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseGSAOptionsNew(const IAttributes& attrs)
{
  PricingRequest* const request = trx().getRequest();
  bool parseSM0andDVL = true;
  if (attrs.has(_VCX))
  {
    char ch = 0;
    attrs.get(_VCX, ch);
    if ('F' == ch)
    {
      parser().setValidatingCarrierRequest(*request, false);
      parser().setAlternateValidatingCarrierRequest(*request, false);
    }
    else
    {
      parseSM0andDVL = false;
      if (attrs.has(_DVL))
      {
        ch = 0;
        attrs.get(_DVL, ch);
        if ('F' == ch)
        {
          parser().setAlternateValidatingCarrierRequest(*request, false);
        }
      }

      if (attrs.has(_SM0))
      {
        std::string smoValue = attrs.get<std::string>(_SM0);
        parser().setSettlementMethodOverride(*request, smoValue);
      }
    }
  }

  if (parseSM0andDVL)
  {
    if (attrs.has(_DVL))
    {
      char ch = 0;
      attrs.get(_DVL, ch);
      if ('F' == ch)
      {
        parser().setAlternateValidatingCarrierRequest(*request, false);
      }
    }

    if (attrs.has(_SM0))
    {
      std::string smoValue = attrs.get<std::string>(_SM0);
      parser().setSettlementMethodOverride(*request, smoValue);
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseCalendarShoppingOptions(const IAttributes& attrs)
{
  PricingOptions* const options = trx().getOptions();

  if (attrs.has(_PA0))
    options->setEnableCalendarForInterlines(attrs.get(_PA0, false));

  if (attrs.has(_PA1))
    options->setSplitTaxesByLeg(attrs.get(_PA1, false));

  if (attrs.has(_PA2))
    options->setSplitTaxesByFareComponent(attrs.get(_PA2, false));

  if (options->isSplitTaxesByLeg())
  {
    options->setForceFareBreaksAtLegPoints(attrs.get(_PA3, true));
  }
}

void
XMLShoppingHandler::PROTypeParser::parseExchangeOptions(const IAttributes& attrs)
{
  PricingRequest* request = trx().getRequest();
  RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(trx().getRequest());
  if (rexBaseRequest != nullptr)
  {
    rexBaseRequest->setTicketingDT(request->ticketingDT());
    if (rexBaseRequest->getTicketingDT().isValid())
    {
      rexPricingTrx()->dataHandle().setTicketDate(rexBaseRequest->getTicketingDT());
      rexPricingTrx()->currentTicketingDT() = rexBaseRequest->getTicketingDT();
      rexPricingTrx()->setFareApplicationDT(rexBaseRequest->getTicketingDT());
    }
  }

  if (attrs.has(_C6Y))
  {
    _getAttr(attrs, _C6Y, parser().newItinCurrency());
  }

  RexPricingOptions* rexOptions = dynamic_cast<RexPricingOptions*>(rexPricingTrx()->getOptions());

  if ((rexOptions != nullptr) && attrs.has(_C6P))
  {
    _getAttr(attrs, _C6P, rexOptions->baseFareCurrencyOverride());
  }

  if (attrs.has(_D92))
  {
    const IValueString& originalDateTime = attrs.get(_D92);
    rexPricingTrx()->setOriginalTktIssueDT() = parseDateTime(originalDateTime);
    setTimeInHistoricalDate(attrs, rexPricingTrx()->setOriginalTktIssueDT(), _T92);
  }

  if (attrs.has(_D94))
  {
    const IValueString& reIssueDateTime = attrs.get(_D94);
    rexPricingTrx()->lastTktReIssueDT() = parseDateTime(reIssueDateTime);
    setTimeInHistoricalDate(attrs, rexPricingTrx()->lastTktReIssueDT(), _T94);
  }

  if (attrs.has(_AC0))
  {
    if (rexBaseRequest != nullptr)
      attrs.get(_AC0, rexBaseRequest->newCorporateID(), rexBaseRequest->newCorporateID());
  }

  if (attrs.has(_Q16))
  {
    attrs.get(_Q16, request->numberTaxBoxes(), request->numberTaxBoxes());
  }

  if (attrs.has(_Q6B))
  {
    attrs.get(_Q6B, request->roeOverride());

    std::ostringstream stream;
    stream << request->roeOverride();
    std::string tmpVal(stream.str());

    std::string::size_type decPos = tmpVal.find(".");
    if (decPos != std::string::npos)
      request->roeOverrideNoDec() = tmpVal.size() - decPos - 1;
  }

  if (trx().getTrxType() == PricingTrx::MIP_TRX)
  {
    if (attrs.has(_MXP))
    {
      int32_t maxPriceJump = -1;
      attrs.get(_MXP, maxPriceJump);
      rexExchangeTrx()->setMaxPriceJump(maxPriceJump);
    }

    bool keepStrategy = false;
    attrs.get(_KOB, keepStrategy);
    rexExchangeTrx()->setKeepOriginal(keepStrategy);

    keepStrategy = false;
    attrs.get(_KOF, keepStrategy);

    if (keepStrategy)
    {
      rexExchangeTrx()->setKeepOriginal(true);
      rexExchangeTrx()->setKeepOriginalStrategy(RexExchangeTrx::KEEP_FARE);
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parsePFFAttribute(const IAttributes& attrs,
                                                     PricingOptions* const options,
                                                     PricingRequest* const request)
{
  if (attrs.has(_PFF))
  {
    char ch(0);
    attrs.get(_PFF, ch);
    if ('T' == ch)
    {
      trx().setFlexFarePhase1(true);
      if (fallback::fallbackFlexFareGroupXOLogicDeffects(&trx())) //Remove below code
        options->xoFares() = 'T';
      if (!fallback::fallbackFlexFareGroupNewXOLogic(&trx()) &&
           fallback::fallbackFlexFareGroupXOLogicDeffects(&trx()))//remove below code
        options->mainGroupXOFares() = 'T';
      if (fallback::fallbackJumpCabinExistingLogic(&trx()))
        request->setJumpCabinLogic(JumpCabinLogic::ONLY_MIXED);
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseQ17Attribute(const IAttributes& attrs)
{
  if (attrs.has(_Q17) && trx().isFlexFarePhase1())
  {
    trx().setFlexFare(true);
    flexFares::GroupId& defaultFlexFaresGroupID = parser().defaultFlexFaresGroupID(trx());
    attrs.get(_Q17, defaultFlexFaresGroupID);
    if (defaultFlexFaresGroupID == 0 && !fallback::fallbackFFGAffectsMainFare(&trx()))
      trx().setFlexFare(false);
    if (defaultFlexFaresGroupID == 0 && !fallback::fallbackJumpCabinExistingLogic(&trx()))
      trx().setMainFareFFGGroupIDZero(true);

    if (!fallback::fallbackFFGMaxPenaltyNewLogic(&trx()) &&
                   defaultFlexFaresGroupID > 0 && parser().isTagPXIProcessed())
    {
      PaxType* pax = trx().paxType().back();
      PricingRequest& request = *trx().getRequest();
      if (!request.getMutableFlexFaresGroupsData().isFlexFareGroup())
      {
        request.getMutableFlexFaresGroupsData().createNewGroup(defaultFlexFaresGroupID);
        request.getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true,defaultFlexFaresGroupID);
      }
      request.getMutableFlexFaresGroupsData().setFFGMaxPenaltyInfo(pax->maxPenaltyInfo(), defaultFlexFaresGroupID);
    }

    if (trx().paxType().size() > 0)
    {
      parser().updateDefaultFFGData(flexFares::PASSENGER_TYPE,
                                    static_cast<std::string>((trx().paxType()[0])->paxType()));
    }
    else
    {
      parser().updateDefaultFFGData(flexFares::PASSENGER_TYPE, ADULT);
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseQ6FAttribute(const IAttributes& attrs,
                                                     PricingOptions* const options,
                                                     PricingRequest* const request)
{
  double lngCnxSolutionsPercent(0);
  attrs.get(_Q6F, lngCnxSolutionsPercent, lngCnxSolutionsPercent);
  if (lngCnxSolutionsPercent > 0)
  {
    if (lngCnxSolutionsPercent > 1.0)
      lngCnxSolutionsPercent /= 100.0;

    request->percentOfLngCnxSolutions() = lngCnxSolutionsPercent;
    uint32_t lngCnxSolutions =
        static_cast<uint32_t>(lngCnxSolutionsPercent * options->getRequestedNumberOfSolutions());
    if (shoppingTrx() != nullptr)
      shoppingTrx()->maxNumOfLngCnxSolutions() = std::max(lngCnxSolutions, 1u);
  }
}

void
XMLShoppingHandler::PROTypeParser::parseOutboundDepartureDate(const IAttributes& attrs)
{
  if (attrs.has(_P2D))
  {
    const IValueString& p2d = attrs.get(_P2D);
    if (attrs.has(_D96))
    {
      const IValueString& d96 = attrs.get(_D96);
      trx().outboundDepartureDate() = parseDateTime(p2d, d96);
    }
    else
    {
      if (parseDateTime(p2d).date() == DateTime::localTime().date())
      {
        DateTime currentDate(DateTime::localTime().date(),
                             boost::posix_time::time_duration(23, 59, 0));
        trx().outboundDepartureDate() = currentDate;
      }
      else
      {
        trx().outboundDepartureDate() = parseDateTime(p2d);
      }
    }
    DateTime currentDate(DateTime::localTime().date(), boost::posix_time::time_duration(0, 0, 0));
    if (trx().outboundDepartureDate() < currentDate)
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Incorrect Outbound/Inbound Date");
    }
  }
  else
  {
    trx().outboundDepartureDate() = DateTime::emptyDate();
  }
}

void
XMLShoppingHandler::PROTypeParser::parseInboundDepartureDate(const IAttributes& attrs)
{
  if (attrs.has(_D12))
  {
    const IValueString& d12 = attrs.get(_D12);
    if (attrs.has(_D96))
    {
      const IValueString& d96 = attrs.get(_D96);
      trx().inboundDepartureDate() = parseDateTime(d12, d96);
    }
    else
    {
      trx().inboundDepartureDate() = parseDateTime(d12);
    }
    DateTime currentDate(DateTime::localTime().date(), boost::posix_time::time_duration(0, 0, 0));
    if (trx().inboundDepartureDate() < currentDate)
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Incorrect Outbound/Inbound Date");
    }
  }
  else
  {
    trx().inboundDepartureDate() = DateTime::emptyDate();
  }
}

void
XMLShoppingHandler::PROTypeParser::parseCollectOCFees(const IAttributes& attrs,
                                                      PricingOptions* const options,
                                                      PricingRequest* const request)
{
  attrs.get(_SEY, request->collectOCFees(), request->collectOCFees());

  if (request->isCollectOCFees())
  {
    attrs.get(_SEU, options->isSummaryRequest(), options->isSummaryRequest());

    if (options->isSummaryRequest())
    {
      std::string parsingResult = "OK";
      parsingResult = OcFeeGroupConfig::parseOCFeesSummaryConfiguration(
          options->groupsSummaryConfig(), options->groupsSummaryConfigVec());

      if (parsingResult != "OK")
      {
        LOG4CXX_ERROR(_logger,
                      "XMLShoppingHandler::PROTypeParser::startParsing - "
                      "Incorrect SERVICE_FEES_SVC\\GROUP_SUMMARY_CONFIG "
                      "configuration parameter. Error description: " +
                          parsingResult);
      }
    }
  }
}
void
XMLShoppingHandler::PROTypeParser::parseTicketingDateTimeOverride(const IAttributes& attrs,
                                                                  PricingOptions* const options,
                                                                  PricingRequest* const request)
{
  if (attrs.has(_D07))
  {
    const IValueString& dateTime = attrs.get(_D07);
    const DateTime ticketingDT = parseDateTime(dateTime);
    validateTicketingDate(ticketingDT);
    request->ticketingDT() = ticketingDT;
    if (attrs.has(_D54))
    {
      attrs.get(_D54, options->ticketTimeOverride());
      request->ticketingDT() = request->ticketingDT() +
                               tse::Hours(options->ticketTimeOverride() / 60) +
                               tse::Minutes(options->ticketTimeOverride() % 60) + tse::Seconds(0);
    }
    else
    {
      request->ticketingDT() = request->ticketingDT() + tse::Minutes(1);
    }

    if (request->ticketingDT().isValid())
    {
      trx().dataHandle().setTicketDate(request->ticketingDT());
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseCorporateIDs(const IAttributes& attrs,
                                                     PricingRequest* const request)
{
  attrs.get(_AC0, request->corporateID(), request->corporateID());
  if (request->corporateID().size() != 0)
    parser().updateDefaultFFGData(flexFares::CORP_IDS, request->corporateID());

  // Multi corpID/Account code
  _AttributeNameIdx_ mac[] = {_AC1, _AC2, _AC3, _AC4};
  int size = sizeof(mac) / sizeof(_AttributeNameIdx_);
  for (int idx = 0; idx < size; ++idx)
  {
    std::string corpId;
    _getAttr(attrs, mac[idx], corpId);
    if (!corpId.empty())
    {
      parser().updateDefaultFFGData(flexFares::CORP_IDS, corpId);
      if (trx().dataHandle().corpIdExists(corpId.c_str(), trx().ticketingDate()))
      {
        std::vector<std::string>& corpIdVec = request->corpIdVec();
        if (std::find(corpIdVec.begin(), corpIdVec.end(), corpId.c_str()) == corpIdVec.end())
          corpIdVec.push_back(corpId.c_str());
      }
      else
      {
        std::vector<std::string>& incorrectCorpIdVec = request->incorrectCorpIdVec();
        if (std::find(incorrectCorpIdVec.begin(), incorrectCorpIdVec.end(), corpId.c_str()) ==
            incorrectCorpIdVec.end())
          incorrectCorpIdVec.push_back(corpId.c_str());
      }
    }
  }

  if (attrs.has(_ACX))
  {
    std::string stringACX;
    attrs.get(_ACX, stringACX);

    if (!stringACX.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringACX, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currCorpId = ((std::string)tokenI->data());

        parser().updateDefaultFFGData(flexFares::CORP_IDS, currCorpId);
        if (trx().dataHandle().corpIdExists(currCorpId.c_str(), trx().ticketingDate()))
        {
          std::vector<std::string>& corpIdVec = request->corpIdVec();
          if (std::find(corpIdVec.begin(), corpIdVec.end(), currCorpId.c_str()) == corpIdVec.end())
            corpIdVec.push_back(currCorpId.c_str());
        }
        else
        {
          std::vector<std::string>& incorrectCorpIdVec = request->incorrectCorpIdVec();
          if (std::find(incorrectCorpIdVec.begin(), incorrectCorpIdVec.end(), currCorpId.c_str()) ==
              incorrectCorpIdVec.end())
            incorrectCorpIdVec.push_back(currCorpId.c_str());
        }
      }
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseAccountCodes(const IAttributes& attrs,
                                                     PricingRequest* const request)
{
  _AttributeNameIdx_ acc[] = {_SM1, _SM2, _SM3, _SM4};
  int size = 4;
  for (int idx = 0; idx < size; ++idx)
  {
    std::string acct;
    _getAttr(attrs, acc[idx], acct);
    if (!acct.empty())
    {
      parser().updateDefaultFFGData(flexFares::ACC_CODES, acct);
      std::vector<std::string>& accCodeVec = request->accCodeVec();
      if (std::find(accCodeVec.begin(), accCodeVec.end(), acct.c_str()) == accCodeVec.end())
        accCodeVec.push_back(acct.c_str());
    }
  }

  if (attrs.has(_SMX))
  {
    std::string stringSMX;
    attrs.get(_SMX, stringSMX);

    if (!stringSMX.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringSMX, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currAcctCode = ((std::string)tokenI->data());

        parser().updateDefaultFFGData(flexFares::ACC_CODES, currAcctCode);
        std::vector<std::string>& accCodeVec = request->accCodeVec();
        if (std::find(accCodeVec.begin(), accCodeVec.end(), currAcctCode.c_str()) ==
            accCodeVec.end())
          accCodeVec.push_back(currAcctCode.c_str());
      }
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseDynamicPriceDeviation(const IAttributes& attrs)
{
  if (attrs.has(_DDP))
  {
    trx().assignPriceDeviator(
        PriceDeviatorPtr(new DiscountPercentageDeviator(-attrs.get<Percent>(_DDP))));
  }

  if (attrs.has(_DMP))
  {
    trx().assignPriceDeviator(
        PriceDeviatorPtr(new MarkUpPercentageDeviator(attrs.get<Percent>(_DMP))));
  }

  if (attrs.has(_DDA))
  {
    CurrencyCode dynamicDiscountCurrency;
    _getAttr(attrs, _DDC, dynamicDiscountCurrency);
    MoneyAmount amount = -attrs.get<MoneyAmount>(_DDA);
    trx().assignPriceDeviator(PriceDeviatorPtr(
        new DiscountAmountDeviator(std::make_pair(amount, dynamicDiscountCurrency))));
  }

  if (attrs.has(_DMA))
  {
    CurrencyCode dynamicMarkUpCurrency;
    _getAttr(attrs, _DDC, dynamicMarkUpCurrency);
    MoneyAmount amount = attrs.get<MoneyAmount>(_DMA);
    trx().assignPriceDeviator(
        PriceDeviatorPtr(new MarkUpAmountDeviator(std::make_pair(amount, dynamicMarkUpCurrency))));
  }
  trx().configurePriceDeviator();
}

void
XMLShoppingHandler::PROTypeParser::parseFLPAttribute(const IAttributes& attrs)
{
  if (attrs.has(_FLP))
  {
    std::string stringFLP;
    attrs.get(_FLP, stringFLP);

    if (!stringFLP.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringFLP, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        trx().flpVector().push_back(boost::lexical_cast<uint16_t>(tokenI->data()));
      }

      // fill in the default values if its size is not equal to TOTAL_FLP_INDEX
      switch (trx().flpVector().size())
      {
      case PricingTrx::NUM_FAMILY_AVAIL_SPLITS: // only 1 element, need to push
        trx().flpVector().push_back(
            PricingTrx::NUM_FAMILY_AVAIL_SPLITS_VAL); // 3 default values. No break!
      case PricingTrx::NUM_FAMILY_BOOKED_SPLITS: // only 2 elements, need to push
        trx().flpVector().push_back(
            PricingTrx::NUM_FAMILY_BOOKED_SPLITS_VAL); // 2 default values, No break!
      case PricingTrx::NUM_FAMILY_MOTHER_BOOKED_SPLITS: // only 3 elements, need to push
        trx().flpVector().push_back(
            PricingTrx::NUM_FAMILY_MOTHER_BOOKED_SPLITS_VAL); // 1 default value.
        break;
      default:
        break;
      }

      // If 2nd, 3rd and 4th parameters of FLP are all 0, it's no split so
      // the FLP vector needs to be emptied so that no EFL code is executed
      if (!trx().flpVector()[PricingTrx::NUM_FAMILY_AVAIL_SPLITS] &&
          !trx().flpVector()[PricingTrx::NUM_FAMILY_BOOKED_SPLITS] &&
          !trx().flpVector()[PricingTrx::NUM_FAMILY_MOTHER_BOOKED_SPLITS])
      {
        trx().flpVector().clear();
      }
    }
  }
}

void
XMLShoppingHandler::SPVTypeParser::startParsing(const IAttributes& attrs)
{
  if (!fallback::fallbackVITA4NonBSP(&trx()) &&
       (trx().getTrxType()==PricingTrx::MIP_TRX || trx().getTrxType()==PricingTrx::IS_TRX))
  {
    trx().overrideFallbackValidationCXRMultiSP() = true;
  }

  if (fallback::fallbackNonBSPVcxrPhase1(&trx()) && !trx().overrideFallbackValidationCXRMultiSP())
    return;

  PricingRequest* const request = trx().getRequest();
  std::string strAttr;
  TSE_ASSERT(request != NULL);
  char chSMV('T');
  char chIEV('T');

  if (attrs.has(_SMV))
      attrs.get(_SMV, chSMV);

  if (attrs.has(_IEV))
        attrs.get(_IEV, chIEV);

  if(chSMV == 'F')
  {
    if(chIEV == 'F')
      request->spvInd() = tse::spValidator::noSMV_noIEV;
    else
      request->spvInd() = tse::spValidator::noSMV_IEV;
  }
  else if(chSMV == 'T' && chIEV == 'T')
    request->spvInd() = tse::spValidator::SMV_IEV;
  else
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "INVALID SETTLEMENT TYPE INDICATOR");
  if (attrs.has(_CRC))
  {
    _getAttr(attrs, _CRC, strAttr);
    parseSPVCodes(strAttr, _CRC);
  }
  if (attrs.has(_CTC) && request->spvInd() == tse::spValidator::noSMV_IEV)
  {
    strAttr.clear();
    _getAttr(attrs, _CTC, strAttr);
    parseSPVCodes(strAttr, _CTC);

    const Loc* posLoc = ValidatingCxrUtil::getPOSLoc(trx(), 0);
    const NationCode nation(ValidatingCxrUtil::getNation(trx(), posLoc));
    if(std::find(request->spvCntyCode().begin(), request->spvCntyCode().end(), nation)
                == request->spvCntyCode().end())
      request->spvCntyCode().push_back(nation);

    std::sort(request->spvCntyCode().begin(), request->spvCntyCode().end());
  }
}

void
XMLShoppingHandler::SPVTypeParser::parseSPVCodes(const std::string& strAttr, int attrType)
{
  PricingRequest* const request = trx().getRequest();
  TSE_ASSERT(request != NULL);

  if (!strAttr.empty())
  {
    std::istringstream ss(strAttr);
    std::string token;
    while (std::getline(ss, token, '|'))
    {
      if (_CRC == attrType)
        request->spvCxrsCode().push_back(token);
      if (_CTC == attrType)
      {
        DataHandle dataHandle(trx().ticketingDate());
        const tse::Nation* nation = dataHandle.getNation(token, trx().ticketingDate());
        if (nation &&
            std::find(request->spvCntyCode().begin(), request->spvCntyCode().end(), token)
            == request->spvCntyCode().end())
          request->spvCntyCode().push_back(token);
      }
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(trx().travelSeg().empty(), "PRO element must come before AAF");
  PricingOptions* const options = trx().getOptions();
  PricingRequest* const request = trx().getRequest();

  // PRO parser should only have been created from a parser that
  // initialized options and request
  TSE_ASSERT(options != NULL && request != NULL);

  if (attrs.has(_TEP)) // Flag for shopping activation - used for various shopping projects
    trx().setProjectActivationFlag(attrs.get<int>(_TEP));

  parsePFFAttribute(attrs, options, request);

  parseQ17Attribute(attrs);

  parser().setNumberOfSolutions(attrs.get<int16_t>(_Q0S, parser().getNumberOfSolutions()));
  options->setRequestedNumberOfSolutions(parser().getNumberOfSolutions());

  parseQ6FAttribute(attrs, options, request);

  //enable dropping results on timeout (PRO/@DRT)
  parseBool(attrs, _DRT, [&]()
            { parser().enableDropResultsOnTimeout(*request); });

  parseBrandingOptions(attrs);

  parseGSAOptionsNew(attrs);

  parseCalendarShoppingOptions(attrs);

  if (trx().isMip() && TrxUtil::newDiscountLogic(trx()))
    parseDynamicPriceDeviation(attrs);

  if (shoppingTrx() != nullptr)
  {
    validateInput(options->getRequestedNumberOfSolutions() > 0, "Invalid number of solutions");

    // Default one ADT passenger type for ESV/VIS request
    if (trx().getTrxType() == PricingTrx::ESV_TRX)
    {
      PaxType* pax = nullptr;
      trx().dataHandle().get(pax);

      pax->paxType() = "ADT";
      pax->number() = 1;

      trx().paxType().push_back(pax);
      trx().posPaxType().push_back(PosPaxTypePerPax());

      PaxTypeUtil::initialize(
          trx(), *pax, pax->paxType(), pax->number(), pax->age(), pax->stateCode(), 0);
    }

    char ch(0);
    attrs.get(_P1W, ch);
    shoppingTrx()->onlineSolutionsOnly() = 'T' == ch;
    attrs.get(_P41, ch);
    shoppingTrx()->interlineSolutionsOnly() = 'T' == ch;
    attrs.get(_PAE, ch);
    shoppingTrx()->noDiversity() = 'T' == ch;
    attrs.get(_Q19, shoppingTrx()->interlineWeightFactor());
    attrs.get(_N1T, shoppingTrx()->groupMethodForMip(), shoppingTrx()->groupMethodForMip());

    validateInput(shoppingTrx()->onlineSolutionsOnly() == false ||
                      shoppingTrx()->interlineSolutionsOnly() == false,
                  "MAY ONLY SPECIFY ONE OF *ONLINE SOLUTIONS ONLY* and "
                  "*INTERLINE SOLUTIONS ONLY*");

    if (isFamilyGroupingDisable(*shoppingTrx()) && attrs.has(_Q0S))
    {
      shoppingTrx()->setRequestedNumOfEstimatedSolutions(0);
    }
    else
    {
      if (attrs.has(_Q1S))
      {
        shoppingTrx()->setRequestedNumOfEstimatedSolutions(attrs.get<int>(_Q1S));
      }
      else
      {
        // if the request just contains the number of options,
        // see if we should price all of them fully, or if we
        // should estimate some of them, according to what is
        // specified in the config file.
        setNumEstimatesFromConfig(true);
        PricingOptions* options = trx().getOptions();
        int estimatesStartAt = startEstimatesAt.getValue();
        if (!startEstimatesAt.isDefault() &&
            options->getRequestedNumberOfSolutions() > estimatesStartAt)
        {
          shoppingTrx()->setRequestedNumOfEstimatedSolutions(
              options->getRequestedNumberOfSolutions());
          options->setRequestedNumberOfSolutions(estimatesStartAt);
          int numAccurateSolutions = accurateSolutions.getValue();
          if (!accurateSolutions.isDefault() &&
              options->getRequestedNumberOfSolutions() > numAccurateSolutions)
          {
            options->setRequestedNumberOfSolutions(numAccurateSolutions);
          }
        }
      }
    }

    parseESVOptions(attrs);

    if (attrs.has(_QCU))
    {
      int16_t qcu = parser().getNumberOfSolutions() > attrs.get<int>(_QCU)
                        ? attrs.get<int>(_QCU)
                        : parser().getNumberOfSolutions();
      shoppingTrx()->setNumOfCustomSolutions(qcu);
    }
    // Parse VIS options
    parseVISParameters(attrs);
  }

  char ch(0);
  attrs.get(_P1F, ch);
  trx().setAltDates('T' == ch);
  attrs.get(_P1G, ch);
  trx().calendarSoldOut() = 'T' == ch;

  attrs.get(_PBD, ch);
  trx().budgetShopping() = ch == 'T';

  attrs.get(_Q6W, ch);
  trx().awardRequest() = 'T' == ch;

  attrs.get(_Q6C, ch);
  trx().snapRequest() = 'T' == ch;

  if (shoppingTrx() != nullptr)
  {
    shoppingTrx()->setRuleTuningISProcess(false);
    if (trx().isAltDates())
    {
      shoppingTrx()->noDiversity() = false;

      if (ruleTuning.getValue())
      {
        shoppingTrx()->setRuleTuningISProcess(true);
      }
    }

    if (attrs.has(_SPT))
    {
      uint32_t spt = 0;
      attrs.get(_SPT, spt);
      shoppingTrx()->startShortCutPricingItin(spt);
    }
  }

  parseOutboundDepartureDate(attrs);

  parseInboundDepartureDate(attrs);

  attrs.get(_P1V, request->lowFareNoAvailability(), request->lowFareNoAvailability());
  attrs.get(_Q4U, options->puShortCKTTimeout(), options->puShortCKTTimeout());
  attrs.get(_P2F, options->altDateMIPCutOffRequest(), options->altDateMIPCutOffRequest());

  parseCorporateIDs(attrs, request);

  attrs.get(_P23, options->resTicketRestr(), options->resTicketRestr());
  if (options->isResTicketRestr())
  {
    parser().updateDefaultFFGData(flexFares::NO_ADVANCE_PURCHASE);
  }
  attrs.get(_P49, options->noMinMaxStayRestr(), options->noMinMaxStayRestr());
  if (options->isNoMinMaxStayRestr())
  {
    parser().updateDefaultFFGData(flexFares::NO_MIN_MAX_STAY);
  }
  attrs.get(_P0J, request->electronicTicket(), request->electronicTicket());
  attrs.get(_P0L, request->eTktOffAndNoOverride(), request->eTktOffAndNoOverride());
  attrs.get(_P0F, options->web(), options->web());
  attrs.get(_P1Y, options->publishedFares(), options->publishedFares());
  if (options->isPublishedFares())
  {
    parser().updateDefaultFFGData(flexFares::PUBLIC_FARES);
  }
  attrs.get(_P1Z, options->privateFares(), options->privateFares());
  if (options->isPrivateFares())
  {
    parser().updateDefaultFFGData(flexFares::PRIVATE_FARES);
  }
  attrs.get(_P20, options->xoFares(), options->xoFares());
  if (!fallback::fallbackFlexFareGroupNewXOLogic(&trx()) &&
       fallback::fallbackFlexFareGroupXOLogicDeffects(&trx()))//remove below code
    options->mainGroupXOFares() = options->xoFares();

  if (!fallback::fallbackFlexFareGroupXOLogicDeffects(&trx()))
  {
    const flexFares::GroupId defaultFlexFaresGroupID = parser().defaultFlexFaresGroupID(trx());

    if (defaultFlexFaresGroupID == 0) //If it is main group
      options->mainGroupXOFares() = options->xoFares();
    else
    {
      if (!fallback::fallbackFFGMaxPenaltyNewLogic(&trx()) &&
          !request->getMutableFlexFaresGroupsData().isFlexFareGroup(defaultFlexFaresGroupID))
      {
        request->getMutableFlexFaresGroupsData().createNewGroup(defaultFlexFaresGroupID);
        request->getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, defaultFlexFaresGroupID);
      }
      request->getMutableFlexFaresGroupsData().setFlexFareXOFares(options->xoFares(),
                                                            defaultFlexFaresGroupID);
    }
  }

  attrs.get(_P21, options->iataFares(), options->iataFares());
  attrs.get(_P1W, options->onlineFares(), options->onlineFares());
  attrs.get(_P53, request->exemptSpecificTaxes(), request->exemptSpecificTaxes());
  attrs.get(_P54, request->exemptAllTaxes(), request->exemptAllTaxes());
  attrs.get(_PBG, options->jpsEntered(), options->jpsEntered());
  attrs.get(_SEZ, request->collectOBFee(), request->collectOBFee());
  attrs.get(_SE2, request->getCollectRTypeOBFee(), request->getCollectRTypeOBFee());
  attrs.get(_SE3, request->getCollectTTypeOBFee(), request->getCollectTTypeOBFee());
  if (trx().diagnostic().diagnosticType() == Diagnostic872)
  {
    DiagManager diag872(trx(), Diagnostic872);
    std::stringstream diag872Str;

    diag872Str << "R type OB fee = " << (request->isCollectRTypeOBFee() ? "T" : "F") << std::endl;
    diag872Str << "T type OB fee = " << (request->isCollectTTypeOBFee() ? "T" : "F") << std::endl;
    diag872 << diag872Str.str();
  }

  if (attrs.has(_SEV))
  {
    attrs.get(_SEV, request->processVITAData(), request->processVITAData());
  }

  const int16_t maxNumberOfOcFeesForItin = maxNumberOfOcFeesPerItin.getValue();
  if (maxNumberOfOcFeesForItin != 0)
  {
    options->maxNumberOfOcFeesForItin() = maxNumberOfOcFeesForItin;
  }

  if (!groupsSummaryConfig.isDefault())
  {
    options->groupsSummaryConfig() = groupsSummaryConfig.getValue();
  }

  parseCollectOCFees(attrs, options, request);

  if (options->jpsEntered() == 'T')
  {
    options->jpsEntered() = 'Y';
  }
  else if (options->jpsEntered() == 'F')
  {
    options->jpsEntered() = 'N';
  }
  if (attrs.has(_C48))
  {
    _getAttr(attrs, _C48, options->currencyOverride());
  }
  attrs.get(_P69, options->mOverride(), options->mOverride());
  if (attrs.has(_C47))
  {
    _getAttr(attrs, _C47, options->alternateCurrency());
  }

  if (attrs.has(_VTI))
  {
    attrs.get(_VTI, options->validateTicketingAgreement(), options->validateTicketingAgreement());
  }

  if ((trx().getTrxType() == PricingTrx::MIP_TRX) && (attrs.has(_MWI)))
  {
    attrs.get(_MWI, options->MIPWithoutPreviousIS(), options->MIPWithoutPreviousIS());
  }

  if (attrs.has(_AF0))
  {
    _getAttr(attrs, _AF0, request->salePointOverride());
  }
  validateInput(request->salePointOverride().empty() || request->salePointOverride().size() == 3,
                "INCORRECT LENGTH OF AF0 FIELD");
  validateInput(request->salePointOverride().empty() ||
                    nullptr !=
                        dataHandle().getLoc(request->salePointOverride(), DateTime::localTime()),
                "INVALID CITY IN AF0");
  if (attrs.has(_AG0))
  {
    _getAttr(attrs, _AG0, request->ticketPointOverride());
  }
  validateInput(request->ticketPointOverride().empty() ||
                    request->ticketPointOverride().size() == 3,
                "INCORRECT LENGTH OF AG0 FIELD");
  validateInput(request->ticketPointOverride().empty() ||
                    nullptr !=
                        dataHandle().getLoc(request->ticketPointOverride(), DateTime::localTime()),
                "INVALID CITY IN AG0");

  if (options && !options->isMOverride() && !request->salePointOverride().empty())
  {
    CurrencyUtil::getSaleLocOverrideCurrency(
        request->salePointOverride(), options->currencyOverride(), request->ticketingDT());
  }

  attrs.get(_P23, options->noAdvPurchRestr(), options->noAdvPurchRestr());
  attrs.get(_S11, request->accountCode(), request->accountCode());
  if (request->accountCode().size() != 0)
    parser().updateDefaultFFGData(flexFares::ACC_CODES, request->accountCode());
  attrs.get(_P43, options->thruFares(), options->thruFares());
  if (attrs.has(_B05))
  {
    _getAttr(attrs, _B05, request->validatingCarrier());
  }
  if (!fallback::fallbackNonPreferredVC(&trx()) && attrs.has(_B12))
  {
    parseB12B13Attribute(attrs, request->nonPreferredVCs(), _B12);
  }

  if (!fallback::fallbackPreferredVC(&trx()) && attrs.has(_B13))
  {
    parseB12B13Attribute(attrs, request->preferredVCs(), _B13);
  }

  attrs.get(_PXD, ch);
  request->brandedFareEntry() = 'T' == ch;

  attrs.get(_PYA, ch);
  request->originBasedRTPricing() = 'T' == ch;

  ShoppingRequestParser::saveCountryAndStateRegionCodes(
      trx(), attrs, options->employment(), options->nationality(), options->residency());

  parseTicketingDateTimeOverride(attrs, options, request);

  parseAccountCodes(attrs, request);

  if (rexPricingTrx() != nullptr)
  {
    parseExchangeOptions(attrs);
  }

  if (trx().isExchangeTrx())
  {
    parser().setDisableFamilyLogic(true);
  }

  bool contextExchangeShoppingRQ = false;
  attrs.get(_CES, contextExchangeShoppingRQ, contextExchangeShoppingRQ);

  if (contextExchangeShoppingRQ)
  {
    trx().getRequest()->setContextShoppingRequest();
  }

  bookingDT() = request->ticketingDT();

  if (bookingDT().totalSeconds() == 60)
  {
    // hard-coded here, do not want to but it is the only way of
    // knowing we are having ticketing date override, in which case
    // 'D-01OCT05 ticket date would be 01/10/2005-00:01:00
    DateTime today = DateTime::localTime();
    if (bookingDT() > today)
    {
      // means that we intended to ticketing at time in future
      bookingDT() = today; // w/o knowing agent local time zone,
      // this is best we can do
    }
  }
  attrs.get(_PDG, options->AdvancePurchaseOption(), options->AdvancePurchaseOption());
  attrs.get(_PAF, options->returnFareCalc(), options->returnFareCalc());
  attrs.get(_PBM, options->callToAvailability(), options->callToAvailability());
  attrs.get(_CS0, ch);
  options->cacheProxyMIP() = 'T' == ch;

  attrs.get(_PXC, ch);
  options->forceCorpFares() = 'T' == ch;
  if (!fallback::fallbackFlexFareGroupNewXCLogic(&trx()))
    options->initialForceCorpFares() = options->forceCorpFares();

  if (!trx().isMainFareFFGGroupIDZero() && !fallback::fallbackFFGAffectsMainFare(&trx()) &&
      trx().isFlexFarePhase1())
  {
    const flexFares::GroupId defaultFlexFaresGroupID = parser().defaultFlexFaresGroupID(trx());
    request->getMutableFlexFaresGroupsData().createNewGroup(defaultFlexFaresGroupID);
    request->getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, defaultFlexFaresGroupID);
    request->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(options->forceCorpFares(),
                                             defaultFlexFaresGroupID);
  }

  attrs.get(_PXE, ch);
  options->inhibitSplitPNR() = 'T' == ch;

  // Flex fare trx forces jumpCabinLogic, so, we should ignore PXS&PXU tag
  if (!trx().isFlexFarePhase1())
  {
    attrs.get(_PXS, ch);

    if (TypeConvert::pssCharToBool(ch))
    {
      request->setJumpCabinLogic(JumpCabinLogic::ONLY_MIXED);

      char ch2;
      attrs.get(_PXU, ch2);

      if (TypeConvert::pssCharToBool(ch2))
        request->setJumpCabinLogic(JumpCabinLogic::DISABLED);
    }
  }
  //Main Fare JumpCabinLogic should not be overridden by Flex Fare JumpCabin Logic
  else if (!fallback::fallbackJumpCabinExistingLogic(&trx()))
  {
    attrs.get(_PXS, ch);
    flexFares::JumpCabinLogic jcl = flexFares::JumpCabinLogic::ENABLED;
    if (TypeConvert::pssCharToBool(ch))
    {
      request->setJumpCabinLogic(JumpCabinLogic::ONLY_MIXED);
      jcl = flexFares::JumpCabinLogic::ONLY_MIXED;
    }

    attrs.get(_PXU, ch);
    if (TypeConvert::pssCharToBool(ch))
    {
      request->setJumpCabinLogic(JumpCabinLogic::DISABLED);
      jcl = flexFares::JumpCabinLogic::DISABLED;
    }

    if(!trx().isMainFareFFGGroupIDZero())
    {
      const flexFares::GroupId defaultFlexFaresGroupID = parser().defaultFlexFaresGroupID(trx());
      if (!fallback::fallbackFFGMaxPenaltyNewLogic(&trx()) &&
          !request->getMutableFlexFaresGroupsData().isFlexFareGroup(defaultFlexFaresGroupID))
      {
        request->getMutableFlexFaresGroupsData().createNewGroup(defaultFlexFaresGroupID);
        request->getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true,
                                                     defaultFlexFaresGroupID);
      }
      request->getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(jcl,
                                               defaultFlexFaresGroupID);
    }
  }

  if ((shoppingTrx() != nullptr) && (trx().getTrxType() == PricingTrx::ESV_TRX))
  {
    if (false == request->processVITAData())
    {
      options->validateTicketingAgreement() = false;
    }
  }

  if (attrs.has(_PHR))
  {
    attrs.get(_PHR, ch);
    request->owPricingRTTaxProcess() = 'T' == ch;
  }

  if (attrs.has(_AE0))
  {
    if (trx().getRequest()->ticketingAgent() != nullptr)
    {
      attrs.get(_AE0, trx().getRequest()->ticketingAgent()->hostCarrier());
    }
  }

  paxTypesInitialize();

  if (shoppingTrx() != nullptr)
  {
    // When there is PRO/P43=.T., we want to look at thru fare only in SOL.
    bool suppressSumOfLocals =
        options->isThruFares() || attrs.get<bool>(_PTF, request->isSuppressedSumOfLocals());
    // For altDate trx, allow setting suppress SOL via configuration
    if (trx().isAltDates() && !suppressSumOfLocals)
    {
      if (suppressSolForAltDate.getValue())
      {
        suppressSumOfLocals = true;
      }
    }
    request->setSuppressSumOfLocals(suppressSumOfLocals);
  }
  if (attrs.has(_SLC))
  {
    options->setCarnivalSumOfLocal(attrs.get<bool>(_SLC));
  }

  if (options->isCarnivalSumOfLocal())
    options->setSolGroupGenerationConfig() = solGroupGenerationCfg.getValue();

  if (attrs.has(_B11))
  {
    CarrierCode cxrOverride = BLANK_CODE;
    _getAttr(attrs, _B11, cxrOverride);
    if (trx().getTrxType() == PricingTrx::IS_TRX)
      request->cxrOverride() = cxrOverride;
    else
      request->governingCarrierOverrides().insert(std::make_pair(0, cxrOverride));
  }

  if (attrs.has(_QD1))
  {
    options->setAdditionalItinsRequestedForOWFares(attrs.get<int16_t>(_QD1));
  }

  if (attrs.has(_QD2))
  {
    options->setMaxAllowedUsesOfFareCombination(attrs.get<int16_t>(_QD2));
  }

  if (attrs.has(_DFL))
  {
    attrs.get(_DFL, ch);
    parser().setDisableFamilyLogic('T' == ch);
  }

  if (attrs.has(_DFN))
  {
    int16_t value = 0;
    attrs.get(_DFN, value);

    const auto discountLevel = SLFUtil::getDiscountLevelFromInt(value);
    trx().getOptions()->setSpanishLargeFamilyDiscountLevel(discountLevel);
  }

  if (attrs.has(_PZF))
  {
    if (trx().getTrxType() == PricingTrx::MIP_TRX &&
        (trx().isAltDates() || options->isEnableCalendarForInterlines()))
    {
      attrs.get(_PZF, ch);
      options->setZeroFareLogic('T' == ch);
    }
  }

  parseFLPAttribute(attrs);

  if ((PricingTrx::MIP_TRX == trx().getTrxType()) && trx().isAltDates())
  {
    uint32_t numFaresForCat12Estimation = 0;
    if (attrs.has(_QFL))
      attrs.get(_QFL, numFaresForCat12Estimation);
    else
    {
      numFaresForCat12Estimation = numFaresForCat12EstimationCfg.getValue();
    }
    trx().getOptions()->setNumFaresForCat12Estimation(numFaresForCat12Estimation);
  }

  if (TrxUtil::isBaggageChargesInMipActivated(trx()))
    request->setBaggageForMIP(true);
  else if ((trx().getTrxType() == PricingTrx::MIP_TRX) && attrs.has(_FBS))
  {
    attrs.get(_FBS, ch);
    request->setBaggageForMIP(ch == 'T' || ch == 'Y');
  }

  if (TrxUtil::isBaggageInPQEnabled(trx()))
    trx().mutableBaggagePolicy().setRequestedBagPieces(attrs.get<uint32_t>(_NBP, 2u));

  if ((trx().getTrxType() == PricingTrx::MIP_TRX) && attrs.has(_URC))
  {
    attrs.get(_URC, ch);
    request->setUseReducedConstructions(ch == 'T' || ch == 'Y');
  }

  parseS15XFFOptionsForFareFocus(attrs);
  restrictExcludeFareFocusRule();

  parseS15PDOOptions(attrs);
  checkEprForPDOorXRS();

  parseS15PDROptions(attrs);
  checkEprForPDR();

  parseS15XRSOptions(attrs);
  checkEprForPDOorXRS();

  if (!fallback::fallbackAgencyRetailerHandlingFees(&trx()))
  {
    checkCominationPDOPDRXRSOptionalParameters();
  }
  parseRetailerCode(attrs);
  parseBI0CabinOptions(attrs);

  parser().setTagPROProcessed();

  restrictNumberOfSolutions();

  const auto& pccSet = pccSimpleShopping.getValue();

  const Agent* agent = trx().getRequest()->ticketingAgent();
  if (pccSet.has(agent->tvlAgencyPCC()) || pccSet.has(agent->mainTvlAgencyPCC()) || pccSet.has("*"))
  {
    trx().getRequest()->setSimpleShoppingRQ(true);
    trx().getRequest()->setMaxFCsPerLeg(maxFcPerLeg.getValue());
  }
}

// PreferredVC , Non-PreferredVC
void
XMLShoppingHandler::PROTypeParser::parseB12B13Attribute(const IAttributes& attrs,
                                                        std::vector<CarrierCode>& carriers,
                                                        int attrType)
{
  std::string strAttr;

  _getAttr(attrs, attrType, strAttr);

  if (!strAttr.empty())
  {
    std::istringstream ss(strAttr);
    std::string token;
    while (std::getline(ss, token, '|'))
    {
      carriers.push_back(token);
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::restrictNumberOfSolutions()
{
  int maxSolutions = TrxUtil::getMaxNumberOfSolutions(trx());
  PricingOptions* const options = trx().getOptions();

  if (maxSolutions && trx().isAltDates() && trx().getTrxType() == PricingTrx::IS_TRX &&
      !options->isEnableCalendarForInterlines())
  {
    if (options->getRequestedNumberOfSolutions() > maxSolutions)
    {
      options->setRequestedNumberOfSolutions(maxSolutions);
      LOG4CXX_WARN(_logger, "Number of solutions restricted to: " << maxSolutions);
    }
  }
}

void
XMLShoppingHandler::PROTypeParser::parseS15XFFOptionsForFareFocus(const IAttributes& attrs)
{
  if (attrs.has(_XFF))
  {
    char ch = 0;
    attrs.get(_XFF, ch);
    if ('T' == ch)
      trx().getOptions()->setExcludeFareFocusRule(true);
  }
  if (!parser().isS15PROProcessed())
  {
    if (attrs.has(_S15))
    {
      std::string xmlS15;
      attrs.get(_S15, xmlS15);
      std::set<std::string>& theEprSet = trx().getOptions()->eprKeywords();
      char* pHolder = nullptr;
      for (char* token = strtok_r((char*)xmlS15.c_str(), ",", &pHolder); token != nullptr;
           token = strtok_r(nullptr, ",", &pHolder))
      {
        theEprSet.insert(token);
      }
    }
    parser().setS15PROProcessed();
  }
}
void
XMLShoppingHandler::PROTypeParser::parseS15PDOOptions(const IAttributes& attrs)
{
  if (attrs.has(_PDO))
  {
    char ch = 0;
    attrs.get(_PDO, ch);
    if ('T' == ch)
      trx().getOptions()->setPDOForFRRule(true);
  }
  if (!parser().isS15PROProcessed())
  {
    if (attrs.has(_S15))
    {
      std::string xmlS15;
      attrs.get(_S15, xmlS15);
      std::set<std::string>& theEprSet = trx().getOptions()->eprKeywords();
      char* pHolder = nullptr;
      for (char* token = strtok_r((char*)xmlS15.c_str(), ",", &pHolder); token != nullptr;
           token = strtok_r(nullptr, ",", &pHolder))
      {
        theEprSet.insert(token);
      }
    }
    parser().setS15PROProcessed();
  }
}

void
XMLShoppingHandler::PROTypeParser::parseS15PDROptions(const IAttributes& attrs)
{
  if (attrs.has(_PDR))
  {
    char ch = 0;
    attrs.get(_PDR, ch);
    if ('T' == ch)
      trx().getOptions()->setPDRForFRRule(true);
  }
  if (!parser().isS15PROProcessed())
  {
    if (attrs.has(_S15))
    {
      std::string xmlS15;
      attrs.get(_S15, xmlS15);
      std::set<std::string>& theEprSet = trx().getOptions()->eprKeywords();
      char* pHolder = nullptr;
      for (char* token = strtok_r((char*)xmlS15.c_str(), ",", &pHolder); token != nullptr;
           token = strtok_r(nullptr, ",", &pHolder))
      {
        theEprSet.insert(token);
      }
    }
    parser().setS15PROProcessed();
  }
}

void
XMLShoppingHandler::PROTypeParser::parseS15XRSOptions(const IAttributes& attrs)
{
  if (attrs.has(_XRS))
  {
    char ch = 0;
    attrs.get(_XRS, ch);
    if ('T' == ch)
      trx().getOptions()->setXRSForFRRule(true);
  }
  if (!parser().isS15PROProcessed())
  {
    if (attrs.has(_S15))
    {
      std::string xmlS15;
      attrs.get(_S15, xmlS15);
      std::set<std::string>& theEprSet = trx().getOptions()->eprKeywords();
      char* pHolder = nullptr;
      for (char* token = strtok_r((char*)xmlS15.c_str(), ",", &pHolder); token != nullptr;
           token = strtok_r(nullptr, ",", &pHolder))
      {
        theEprSet.insert(token);
      }
    }
    parser().setS15PROProcessed();
  }
}

void
XMLShoppingHandler::PROTypeParser::restrictExcludeFareFocusRule()
{
  static std::string EPR_FFOCUS = "FFOCUS";
  if (trx().getOptions()->isExcludeFareFocusRule() &&
      !trx().getOptions()->isKeywordPresent(EPR_FFOCUS))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "NEED KEYWORD FFOCUS");
}

void
XMLShoppingHandler::PROTypeParser::checkEprForPDOorXRS()
{
  static std::string EPR_ORDFQD = "ORGFQD";
  if ((trx().getOptions()->isPDOForFRRule() || trx().getOptions()->isXRSForFRRule()) &&
      !trx().getOptions()->isKeywordPresent(EPR_ORDFQD))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "NEED KEYWORD ORGFQD");
}

void
XMLShoppingHandler::PROTypeParser::checkEprForPDR()
{
  static std::string EPR_AGYRET = "AGYRET";
  if (trx().getOptions()->isPDRForFRRule() && !trx().getOptions()->isKeywordPresent(EPR_AGYRET))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "NEED KEYWORD AGYRET");
}

void
XMLShoppingHandler::PROTypeParser::checkCominationPDOPDRXRSOptionalParameters()
{
  if (trx().getOptions()->isPDOForFRRule() && trx().getOptions()->isXRSForFRRule())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE ORG AND XRS");

  else if (trx().getOptions()->isPDRForFRRule() && trx().getOptions()->isXRSForFRRule())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE RET AND XRS");

  else if (trx().getOptions()->isPDOForFRRule() &&
           trx().getOptions()->isPDRForFRRule())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE RET AND ORG");
}

void
XMLShoppingHandler::PROTypeParser::parseBI0CabinOptions(const IAttributes& attrs)
{
  if (UNLIKELY(!attrs.has(_BI0)))
  {
    return;
  }
  PricingOptions* const options = trx().getOptions();
  const IValueString& str = attrs.get(_BI0);
  const char* reqCabin = str.begin();

  if (str.length() > 2)
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID CABIN CODE");

  if (strncmp(reqCabin, "PB", 2) == 0)
  {
    options->cabin().setPremiumFirstClass();
  }
  else if (strncmp(reqCabin, "FB", 2) == 0)
  {
    options->cabin().setFirstClass();
  }
  else if (strncmp(reqCabin, "JB", 2) == 0)
  {
    options->cabin().setPremiumBusinessClass();
  }
  else if (strncmp(reqCabin, "BB", 2) == 0)
  {
    options->cabin().setBusinessClass();
    ;
  }
  else if (strncmp(reqCabin, "SB", 2) == 0)
  {
    options->cabin().setPremiumEconomyClass();
  }
  else if (strncmp(reqCabin, "YB", 2) == 0)
  {
    options->cabin().setEconomyClass();
  }
  else if (strncmp(reqCabin, "AB", 2) == 0)
  {
    options->cabin().setAllCabin();
  }
  else
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID CABIN CODE");

  if (!fallback::fallbackPriceByCabinActivation(&trx()) && !options->cabin().isUndefinedClass() &&
      !options->cabin().isAllCabin() && PricingTrx::MIP_TRX == trx().getTrxType())
  {
    trx().getRequest()->setjumpUpCabinAllowed();
  }
}

void
XMLShoppingHandler::PROTypeParser::parseVISParameters(const IAttributes& attrs)
{
  VISOptions*& visOptions = shoppingTrx()->visOptions();

  if (nullptr == visOptions)
  {
    dataHandle().get(visOptions);
  }

  // Value-based Itinerary Selection
  if (attrs.has(_PVI))
  {
    attrs.get(_PVI, visOptions->valueBasedItinSelection().enableVIS());
  }
  if (attrs.has(_Q80))
  {
    attrs.get(_Q80, visOptions->valueBasedItinSelection().noOfOutboundsRT());
  }
  if (attrs.has(_Q81))
  {
    attrs.get(_Q81, visOptions->valueBasedItinSelection().noOfInboundsRT());
  }
  if (attrs.has(_QC3))
  {
    attrs.get(_QC3, visOptions->valueBasedItinSelection().noOfOutboundsOW());
  }

  // VIS outbound selection (Round Trip)
  if (attrs.has(_Q82))
  {
    attrs.get(_Q82, visOptions->visOutboundSelectionRT().carrierPriority());
  }
  if (attrs.has(_Q83))
  {
    attrs.get(_Q83, visOptions->visOutboundSelectionRT().timeOfDayPriority());
  }
  if (attrs.has(_Q84))
  {
    attrs.get(_Q84, visOptions->visOutboundSelectionRT().elapsedTimePriority());
  }
  if (attrs.has(_Q85))
  {
    attrs.get(_Q85, visOptions->visOutboundSelectionRT().utilityValuePriority());
  }
  if (attrs.has(_Q86))
  {
    attrs.get(_Q86, visOptions->visOutboundSelectionRT().nonStopPriority());
  }
  if (attrs.has(_Q87))
  {
    attrs.get(_Q87, visOptions->visOutboundSelectionRT().noOfCarriers());
  }
  if (attrs.has(_Q88))
  {
    attrs.get(_Q88, visOptions->visOutboundSelectionRT().noOfOptionsPerCarrier());
  }
  if (attrs.has(_SG1))
  {
    parseTimeBins(attrs, _SG1, visOptions->visOutboundSelectionRT().timeOfDayBins());
  }
  if (attrs.has(_Q89))
  {
    attrs.get(_Q89, visOptions->visOutboundSelectionRT().noOfOptionsPerTimeBin());
  }
  if (attrs.has(_Q90))
  {
    attrs.get(_Q90, visOptions->visOutboundSelectionRT().noOfElapsedTimeOptions());
  }
  if (attrs.has(_Q91))
  {
    attrs.get(_Q91, visOptions->visOutboundSelectionRT().noOfUtilityValueOptions());
  }
  if (attrs.has(_Q92))
  {
    attrs.get(_Q92, visOptions->visOutboundSelectionRT().nonStopFareMultiplier());
  }
  if (attrs.has(_Q93))
  {
    attrs.get(_Q93, visOptions->visOutboundSelectionRT().noOfNonStopOptions());
  }

  // VIS outbound selection (Round Trip)
  if (attrs.has(_Q94))
  {
    attrs.get(_Q94, visOptions->visInboundSelectionRT().lowestFarePriority());
  }
  if (attrs.has(_Q95))
  {
    attrs.get(_Q95, visOptions->visInboundSelectionRT().timeOfDayPriority());
  }
  if (attrs.has(_Q96))
  {
    attrs.get(_Q96, visOptions->visInboundSelectionRT().elapsedTimePriority());
  }
  if (attrs.has(_Q97))
  {
    attrs.get(_Q97, visOptions->visInboundSelectionRT().utilityValuePriority());
  }
  if (attrs.has(_Q98))
  {
    attrs.get(_Q98, visOptions->visInboundSelectionRT().nonStopPriority());
  }
  if (attrs.has(_Q99))
  {
    attrs.get(_Q99, visOptions->visInboundSelectionRT().simpleInterlinePriority());
  }
  if (attrs.has(_QA0))
  {
    attrs.get(_QA0, visOptions->visInboundSelectionRT().noOfLFSOptions());
  }
  if (attrs.has(_SG2))
  {
    parseTimeBins(attrs, _SG2, visOptions->visInboundSelectionRT().timeOfDayBins());
  }
  if (attrs.has(_QA1))
  {
    attrs.get(_QA1, visOptions->visInboundSelectionRT().noOfOptionsPerTimeBin());
  }
  if (attrs.has(_QA2))
  {
    attrs.get(_QA2, visOptions->visInboundSelectionRT().noOfElapsedTimeOptions());
  }
  if (attrs.has(_QA3))
  {
    attrs.get(_QA3, visOptions->visInboundSelectionRT().noOfUtilityValueOptions());
  }
  if (attrs.has(_QA4))
  {
    attrs.get(_QA4, visOptions->visInboundSelectionRT().nonStopFareMultiplier());
  }
  if (attrs.has(_QA5))
  {
    attrs.get(_QA5, visOptions->visInboundSelectionRT().noOfNonStopOptions());
  }
  if (attrs.has(_QA6))
  {
    attrs.get(_QA6, visOptions->visInboundSelectionRT().noOfSimpleInterlineOptions());
  }

  // VIS outbound selection (One Way)
  if (attrs.has(_QA7))
  {
    attrs.get(_QA7, visOptions->visOutboundSelectionOW().carrierPriority());
  }
  if (attrs.has(_QA8))
  {
    attrs.get(_QA8, visOptions->visOutboundSelectionOW().timeOfDayPriority());
  }
  if (attrs.has(_QA9))
  {
    attrs.get(_QA9, visOptions->visOutboundSelectionOW().elapsedTimePriority());
  }
  if (attrs.has(_QB0))
  {
    attrs.get(_QB0, visOptions->visOutboundSelectionOW().utilityValuePriority());
  }
  if (attrs.has(_QB1))
  {
    attrs.get(_QB1, visOptions->visOutboundSelectionOW().nonStopPriority());
  }
  if (attrs.has(_QB2))
  {
    attrs.get(_QB2, visOptions->visOutboundSelectionOW().noOfCarriers());
  }
  if (attrs.has(_QB3))
  {
    attrs.get(_QB3, visOptions->visOutboundSelectionOW().noOfOptionsPerCarrier());
  }
  if (attrs.has(_SG3))
  {
    parseTimeBins(attrs, _SG3, visOptions->visOutboundSelectionOW().timeOfDayBins());
  }
  if (attrs.has(_QB4))
  {
    attrs.get(_QB4, visOptions->visOutboundSelectionOW().noOfOptionsPerTimeBin());
  }
  if (attrs.has(_QB5))
  {
    attrs.get(_QB5, visOptions->visOutboundSelectionOW().noOfElapsedTimeOptions());
  }
  if (attrs.has(_QB6))
  {
    attrs.get(_QB6, visOptions->visOutboundSelectionOW().noOfUtilityValueOptions());
  }
  if (attrs.has(_QB7))
  {
    attrs.get(_QB7, visOptions->visOutboundSelectionOW().nonStopFareMultiplier());
  }
  if (attrs.has(_QB8))
  {
    attrs.get(_QB8, visOptions->visOutboundSelectionOW().noOfNonStopOptions());
  }

  // VIS Low Fare Search
  if (attrs.has(_QB9))
  {
    attrs.get(_QB9, visOptions->visLowFareSearch().noOfLFSItineraries());
  }
  if (attrs.has(_QC0))
  {
    attrs.get(_QC0, visOptions->visLowFareSearch().noOfAdditionalOutboundsRT());
  }
  if (attrs.has(_QC1))
  {
    attrs.get(_QC1, visOptions->visLowFareSearch().noOfAdditionalInboundsRT());
  }
  if (attrs.has(_QC2))
  {
    attrs.get(_QC2, visOptions->visLowFareSearch().noOfAdditionalOutboundsOW());
  }
}

void
XMLShoppingHandler::PROTypeParser::parseTimeBins(const IAttributes& attrs,
                                                 int attributeName,
                                                 std::vector<VISTimeBin>& timeOfDayBinsVec)
{
  const IValueString& binsString = attrs.get(attributeName);

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  boost::char_separator<char> separator(",");
  tokenizer tokens(binsString, separator);
  tokenizer::iterator tokenI;

  std::vector<short> binsVector;

  for (tokenI = tokens.begin(); tokenI != tokens.end(); ++tokenI)
  {
    std::string token = ((std::string)tokenI->data());

    short bin = atoi(token.c_str());

    if ((bin > 0) && (bin < 2359))
    {
      binsVector.push_back(bin);
    }
  }

  if (!binsVector.empty())
  {
    timeOfDayBinsVec.clear();

    short startTime = 0;
    short endTime = 2359;

    for (uint32_t i = 0; i < binsVector.size(); ++i)
    {
      VISTimeBin tb1((i == 0) ? startTime : startTime + 1, binsVector[i]);

      timeOfDayBinsVec.push_back(tb1);

      startTime = binsVector[i];

      if (i == (binsVector.size() - 1))
      {
        VISTimeBin tb2(startTime + 1, endTime);

        timeOfDayBinsVec.push_back(tb2);
      }
    }
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::PROTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_BH0 == idx) // exempt tax (/TE)
  {
    return &parser()._bh0Parser;
  }
  else if (_MTY == idx) // specify tax (/TX)
  {
    return &parser()._mtyParser;
  }
  else if (idx == _XPN) // WPNI'XP no penalty
  {
    return &parser()._xpnParser;
  }
  else if (idx == _OPT)
  {
    return &parser()._optParser;
  }
  return nullptr;
}

void
XMLShoppingHandler::BH0TypeParser::endParsing(const IValueString& text)
{
  // Get the request object from the trx
  PricingRequest* request = trx().getRequest();

  std::vector<std::string>& taxIdExemptedVector = request->taxIdExempted();
  std::string taxIdExempted(text.c_str(), text.length());
  taxIdExemptedVector.push_back(taxIdExempted);
  validateInput(taxIdExempted != "XF", "FORMAT - PFC/XF CANNOT BE EXEMPTED-0027");
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::MTYTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_TXS == idx)
  {
    parseTXSType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::MTYTypeParser::parseTXSType(const IAttributes& attrs)
{
  PricingRequest* request = trx().getRequest();

  // Create tax override object
  TaxOverride* taxOverride;
  dataHandle().get(taxOverride);

  // Insert a tax override instance into the vector
  std::vector<TaxOverride*>& taxOverrideVector = request->taxOverride();
  taxOverrideVector.push_back(taxOverride);

  // Set the tax override object amount to the attribute value
  attrs.get(_C6B, taxOverride->taxAmt());
  // taxOverride->taxAmt() = static_cast<MoneyAmount>(atof(attr["C6B"].c_str()));

  // Set the tax override object code to the text pulled in from this element
  _getAttr(attrs, _BH0A, taxOverride->taxCode());
}

void
XMLShoppingHandler::XPNTypeParser::startParsing(const IAttributes& attrs)
{
  PricingOptions* const options = trx().getOptions();
  attrs.get(_P47, options->noPenalties(), options->noPenalties());
  if (options->isNoPenalties())
  {
    parser().updateDefaultFFGData(flexFares::NO_PENALTIES);
  }
}

XMLShoppingHandler::AAFTypeParser::AAFTypeParser(XMLShoppingHandler& parser)
  : XMLShoppingHandler::ElementParser(parser),
    _isInternational(false),
    _interItin(false),
    _sideTripStart(false),
    _sideTripEnd(false),
    _sideTripNumber(0),
    _prevFareCompInfo(nullptr),
    _pssOpenDateBase("1966-01")
{
}

void
XMLShoppingHandler::AAFTypeParser::startParsing(const IAttributes& attrs)
{
  if (UNLIKELY(isExcArunkSegment(attrs)))
  {
    parseArunkSegment(attrs);
    return;
  }

  AirSeg* seg = nullptr;
  dataHandle().get(seg);
  _currentSeg = seg;

  uint32_t aafIndex = attrs.get<uint32_t>(_Q1K, 0);

  seg->originalId() = aafIndex + 1;

  int pnrSegment;
  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    setAAFIndex(aafIndex, rexPricingTrx()->excTravelSeg().size());
    pnrSegment = rexPricingTrx()->excTravelSeg().size() + 1;
  }
  else
  {
    setAAFIndex(aafIndex, trx().travelSeg().size());
    pnrSegment = trx().travelSeg().size() + 1;
  }

  seg->pnrSegment() = pnrSegment;

  _getAttr(attrs, _B00, seg->carrier());

  // const IValueString &operatingCarrier = attrs.get(_B01);
  if (LIKELY(attrs.has(_B01)))
  {
    std::string operatingCarrier;
    attrs.get(_B01, operatingCarrier);
    seg->setOperatingCarrierCode(operatingCarrier);
  }
  attrs.get(_Q0B, seg->flightNumber());
  _getAttr(attrs, _S05, seg->equipmentType());
  char ch(0);
  attrs.get(_P0Z, ch);
  seg->eticket() = 'T' == ch;
  attrs.get(_PCR, ch);
  seg->bbrCarrier() = 'T' == ch;

  seg->bookedCabin().setEconomyClass(); // make it the lowest cabin
  seg->setBookingCode(DUMMY_BOOKING); // dummy booking code so it will not match ""
  _getAttr(attrs, _BB0, seg->resStatus());
  if (seg->resStatus() == EMPTY_STRING())
  {
    seg->resStatus() = "OK"; // defaut
  }
  _getAttr(attrs, _BB2, seg->realResStatus());
  attrs.get(_BB3, seg->marriageStatus(), seg->marriageStatus());
  seg->bookingDT() = parseDateTime(attrs.get(_D00), attrs.get(_D30));

  if (UNLIKELY(trx().getRequest()->isBrandedFaresRequest()))
  {
    startParsingContextShoppingAAF(attrs, seg);
  }

  if (UNLIKELY(rexPricingTrx() != nullptr))
  {
    startParsingRexAAF(attrs, seg);
    return;
  }

  trx().travelSeg().push_back(seg);
}

void
XMLShoppingHandler::AAFTypeParser::startParsingRexAAF(const IAttributes& attrs,
                                                      TravelSeg* const seg)
{
  if (attrs.has(_P72))
  {
    attrs.get(_P72, seg->forcedConx());
  }
  if (attrs.has(_P73))
  {
    attrs.get(_P73, seg->forcedStopOver());
  }

  bool segFlown = false;

  if (attrs.has(_PCI))
  {
    attrs.get(_PCI, segFlown);
  }
  seg->unflown() = !segFlown;

  seg->isShopped() = false;

  if (parser().parsingExchangeInformation() == false && attrs.has(_SSH))
  {
    attrs.get(_SSH, seg->isShopped());
  }

  if (parser().parsingExchangeInformation())
  {
    if (attrs.has(_B50))
    {
      _getAttr(attrs, _B50, seg->fareBasisCode());
    }

    if (attrs.has(_C50))
    {
      _getAttr(attrs, _C50, seg->fareCalcFareAmt());
    }

    if (attrs.has(_S07))
    {
      attrs.get(_S07, _sideTripStart, _sideTripStart);
    }
    if (attrs.has(_S08))
    {
      attrs.get(_S08, _sideTripEnd, _sideTripEnd);
    }
    if (attrs.has(_Q6E))
    {
      attrs.get(_Q6E, _sideTripNumber, _sideTripNumber);
    }

    if (attrs.has(_SB2))
    {
      BrandCode brandCode;
      attrs.get(_SB2, brandCode);
      seg->setBrandCode(brandCode);
    }

    // Setup Fare component
    saveFareComponent(attrs, seg);

    rexPricingTrx()->excTravelSeg().push_back(seg);
  }
  else
  {
    trx().travelSeg().push_back(seg);
  }
}

void
XMLShoppingHandler::AAFTypeParser::saveFareComponent(const IAttributes& attrs, TravelSeg* const seg)
{
  int fareCompNo = 0;
  attrs.get(_Q6D, fareCompNo);

  if (_sideTripStart && _prevFareCompInfo)
  {
    _prevFareCompInfo->fareMarket()->sideTripTravelSeg().push_back(std::vector<TravelSeg*>());
    _fareCompInfoStack.push(_prevFareCompInfo);
  }

  if (_fareCompInfoStack.empty() && (_sideTripNumber > 0 || _sideTripEnd))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Side trip data error");

  if (_sideTripNumber > 0)
  {
    seg->forcedSideTrip() = 'T';
    if (fareCompNo > 0)
      _fareCompInfoStack.top()->fareMarket()->sideTripTravelSeg().back().push_back(seg);
  }
  if (_sideTripEnd)
  {
    _prevFareCompInfo = _fareCompInfoStack.top();
    _fareCompInfoStack.pop();
  }

  if (fareCompNo <= 0)
  {
    return;
  }

  // Setup Fare component
  std::vector<FareCompInfo*>::reverse_iterator rIter =
      rexPricingTrx()->exchangeItin().back()->fareComponent().rbegin();
  FareCompInfo* fareComponent = nullptr;

  for (; rIter != rexPricingTrx()->exchangeItin().back()->fareComponent().rend(); ++rIter)
  {
    if ((**rIter).fareCompNumber() == fareCompNo)
      fareComponent = *rIter;
  }

  if (fareComponent == nullptr)
  {
    dataHandle().get(fareComponent);
    rexPricingTrx()->exchangeItin().back()->fareComponent().push_back(fareComponent);

    FareMarket* fareMarket = nullptr;
    dataHandle().get(fareMarket);
    fareComponent->fareMarket() = fareMarket;
    fareMarket->fareCompInfo() = fareComponent;
    fareComponent->fareCompNumber() = fareCompNo;
  }

  //----------------------------
  double discountAmount = 0;
  double discPercentage = 0;

  if (!fallback::azPlusUp(&trx()) && !fallback::azPlusUpExc(&trx()))
  {
    RexBaseRequest& request = dynamic_cast<RexBaseRequest&>(*trx().getRequest());
    ExcItin* excItin = rexPricingTrx()->exchangeItin().front();

    uint32_t discountGroupNum = attrs.get<uint32_t>(_Q12, 0);
    CurrencyCode currencyCode = excItin->calcCurrencyOverride().empty()
                                    ? excItin->calculationCurrency()
                                    : excItin->calcCurrencyOverride();
    if (attrs.has(_DMA))
    {
      discountAmount = -attrs.get<double>(_DMA);
    }
    else if (attrs.has(_C6I))
    {
      discountAmount = attrs.get<double>(_C6I);
    }

    if (attrs.has(_DMP))
    {
      discPercentage = -attrs.get<double>(_DMP);
    }
    else if (attrs.has(_Q17))
    {
      discPercentage = attrs.get<double>(_Q17);
    }

    if (attrs.has(_DMA) || attrs.has(_C6I))
    {
      request.excDiscounts().addAmount(
          discountGroupNum, seg->pnrSegment(), discountAmount, currencyCode);
    }
    if (attrs.has(_DMP) || attrs.has(_Q17))
    {
      request.excDiscounts().addPercentage(seg->pnrSegment(), discPercentage);
    }
  }
  else
  {
    attrs.get(_C6I, discountAmount);
    attrs.get(_Q17, discPercentage);
  }

  fareComponent->discounted() = (discPercentage > 0.0 || discountAmount > 0.0);
  fareComponent->fareBasisCode() = seg->fareBasisCode();
  tse::VCTR VTCR;

  if (attrs.has(_S37) && attrs.has(_B09) && attrs.has(_S89) && attrs.has(_S90))
  {
    _getAttr(attrs, _S37, VTCR.vendor());
    _getAttr(attrs, _B09, VTCR.carrier());
    attrs.get(_S89, VTCR.tariff());
    _getAttr(attrs, _S90, VTCR.rule());

    fareComponent->hasVCTR() = true;
    fareComponent->VCTR() = VTCR;
  }

  MoneyAmount fareComponentAmount = 0;
  fareComponentAmount = strtod(seg->fareCalcFareAmt().c_str(), nullptr);
  if (fareComponentAmount > 0)
  {
    fareComponent->fareCalcFareAmt() = fareComponent->tktFareCalcFareAmt() = fareComponentAmount;
  }

  fareComponent->fareMarket()->travelSeg().push_back(seg);

  if (!_sideTripEnd)
  {
    _prevFareCompInfo = fareComponent;
  }
}

void
XMLShoppingHandler::AAFTypeParser::startParsingContextShoppingAAF(const IAttributes& attrs,
                                                                  TravelSeg* const seg)
{
  int fareComponentNo = seg->pnrSegment(); // Assume default value, slim chance client will send it
  if (attrs.has(_Q6D))
  {
    attrs.get(_Q6D, fareComponentNo);
  }

  skipper::FareComponentShoppingContext* context = nullptr;

  auto it = parser()._fcShoppingContexts.find(fareComponentNo);
  if (it == parser()._fcShoppingContexts.end())
  {
    dataHandle().get(context);
  }
  else
  {
    context = it->second;
  }

  _getAttr(attrs, _B50, context->fareBasisCode);
  _getAttr(attrs, _C50, context->fareCalcFareAmt);
  _getAttr(attrs, _SB2, context->brandCode);
  seg->setBrandCode(context->brandCode);

  if (attrs.has(_S37) && attrs.has(_B09) && attrs.has(_S89) && attrs.has(_S90))
  {
    VCTR vctr;
    _getAttr(attrs, _S37, vctr.vendor());
    _getAttr(attrs, _B09, vctr.carrier());
    attrs.get(_S89, vctr.tariff());
    _getAttr(attrs, _S90, vctr.rule());
    context->vctr = vctr;
  }

  if (context->isValid())
  {
    parser()._fcShoppingContexts[fareComponentNo] = context;
    trx().getMutableFareComponentShoppingContexts()[seg->pnrSegment()] = context;
  }
}

bool
XMLShoppingHandler::AAFTypeParser::isExcArunkSegment(const IAttributes& attrs)
{
  if (LIKELY(!parser().parsingExchangeInformation() || !attrs.has(_N03)))
    return false;

  char segType(0);
  attrs.get(_N03, segType, segType);
  return (segType == 'K');
}

void
XMLShoppingHandler::AAFTypeParser::parseArunkSegment(const IAttributes& attrs)
{
  ArunkSeg* const seg = dataHandle().create<ArunkSeg>();

  seg->segmentType() = Arunk;

  seg->pnrSegment() = rexPricingTrx()->excTravelSeg().size() + 1;

  uint32_t aafIndex = attrs.get<uint32_t>(_Q1K, 0);

  seg->originalId() = aafIndex + 1;

  setAAFIndex(aafIndex, rexPricingTrx()->excTravelSeg().size());

  seg->bookedCabin().setEconomyClass(); // make it the lowest cabin

  seg->setBookingCode(DUMMY_BOOKING); // dummy booking code so it will not match ""

  _getAttr(attrs, _BB0, seg->resStatus());

  if (seg->resStatus() == EMPTY_STRING())
    seg->resStatus() = "OK";

  _getAttr(attrs, _BB2, seg->realResStatus());

  seg->bookingDT() = parseDateTime(attrs.get(_D00), attrs.get(_D30));

  if (rexPricingTrx() != nullptr)
  {
    startParsingRexAAF(attrs, seg);
  }
}

void
XMLShoppingHandler::AAFTypeParser::endParsing(const IValueString& text)
{
  AirSeg* seg;

  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    if (_sideTripEnd)
    {
      _sideTripStart = false;
      _sideTripEnd = false;
      _sideTripNumber = 0;
    }

    if (rexPricingTrx()->excTravelSeg().empty() || !rexPricingTrx()->excTravelSeg().back()->isAir())
    {
      return;
    }
    seg = static_cast<AirSeg*>(rexPricingTrx()->excTravelSeg().back());
  }
  else
  {
    if (UNLIKELY(trx().travelSeg().empty() || !trx().travelSeg().back()->isAir()))
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingHandler::AAFTypeParser::endParsing - "
                    "Travel segment empty or not of Air type.");
      return;
    }
    seg = static_cast<AirSeg*>(trx().travelSeg().back());
  }

  // update MultiCity information
  if (LIKELY(seg->segmentType() != Arunk))
  {
    const Loc* originLoc = seg->origin();
    const Loc* destinationLoc = seg->destination();
    // Set GEO travel type
    if (LIKELY(originLoc != nullptr && destinationLoc != nullptr))
    {
      if (LocUtil::isDomestic(*originLoc, *destinationLoc))
        seg->geoTravelType() = GeoTravelType::Domestic;
      else if (LocUtil::isInternational(*originLoc, *destinationLoc))
      {
        seg->geoTravelType() = GeoTravelType::International;
        _interItin = true;
        if (!_isInternational)
        {
          setEstimatedSolutionsForInternational();
          if (trx().travelSeg().size() > 1)
          {
            AirSeg* airSeg;
            std::vector<TravelSeg*>::iterator tvlSegIter = trx().travelSeg().begin();
            std::vector<TravelSeg*>::iterator tvlSegIterEnd = trx().travelSeg().end() - 1;
            for (; tvlSegIter < tvlSegIterEnd; ++tvlSegIter)
            {
              airSeg = static_cast<AirSeg*>(*tvlSegIter);
              setUpBoardOffMultiCities(airSeg);
            }
          }
        }
      }
      else if (LocUtil::isTransBorder(*originLoc, *destinationLoc))
        seg->geoTravelType() = GeoTravelType::Transborder;
      else if (LIKELY(LocUtil::isForeignDomestic(*originLoc, *destinationLoc)))
        seg->geoTravelType() = GeoTravelType::ForeignDomestic;
    }
    setUpBoardOffMultiCities(seg);
  }
}

void
XMLShoppingHandler::AAFTypeParser::setEstimatedSolutionsForInternational()
{
  _isInternational = true;

  if (rexPricingTrx() != nullptr)
    return;

  if (getNumEstimatesFromConfig())
  {
    setEstimatedSolutions("START_ESTIMATES_AT_INTL", "ACCURATE_SOLUTIONS_INTL");
  }
}

void
XMLShoppingHandler::ElementParser::setEstimatedSolutions(const char* estimatedConfigName,
                                                         const char* accuredConfigName)
{
  PricingOptions* options = trx().getOptions();
  int estimatesStartAt = 0;
  if (Global::config().getValue(estimatedConfigName, estimatesStartAt, "SHOPPING_OPT") &&
      options->getRequestedNumberOfSolutions() > estimatesStartAt)
  {
    shoppingTrx()->setRequestedNumOfEstimatedSolutions(options->getRequestedNumberOfSolutions());
    options->setRequestedNumberOfSolutions(estimatesStartAt);
    int numAccurateSolutions = 0;
    if (Global::config().getValue(accuredConfigName, numAccurateSolutions, "SHOPPING_OPT") &&
        options->getRequestedNumberOfSolutions() > numAccurateSolutions)
    {
      options->setRequestedNumberOfSolutions(numAccurateSolutions);
    }
  }
}

void
XMLShoppingHandler::AAFTypeParser::setUpBoardOffMultiCities(AirSeg* seg)
{
  // see if the origin is an airport in a multi-airport city
  const std::vector<tse::MultiTransport*>& boardMAC = dataHandle().getMultiTransportCity(
      seg->origAirport(),
      seg->carrier(),
      (_interItin ? GeoTravelType::International : seg->geoTravelType()),
      seg->departureDT());
  if (boardMAC.empty() == false)
  {
    seg->boardMultiCity() = boardMAC.front()->multitranscity();
  }
  else
  {
    seg->boardMultiCity() = seg->origAirport();
  }

  // see if the destination is an airport in a multi-airport city

  const std::vector<tse::MultiTransport*>& offMAC = dataHandle().getMultiTransportCity(
      seg->destAirport(),
      seg->carrier(),
      (_interItin ? GeoTravelType::International : seg->geoTravelType()),
      seg->departureDT());

  if (offMAC.empty() == false)
  {
    seg->offMultiCity() = offMAC.front()->multitranscity();
  }
  else
  {
    seg->offMultiCity() = seg->destAirport();
  }

  return;
}

const Loc*
XMLShoppingHandler::AAFTypeParser::getLoc(const LocCode& city, const DateTime& dt)
{
  const LocCode key(city);

  LocationMap& m = _locCache[dt.get64BitRepDateOnly()];
  LocationMap::const_iterator itor = m.find(key);
  if (itor != m.end())
  {
    return itor->second;
  }
  else
  {
    const Loc* res = dataHandle().getLoc(city, dt);
    m[key] = res;
    return res;
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::AAFTypeParser::newElement(int idx, const IAttributes& attrs)
{
  AirSeg* seg;
  if (UNLIKELY(parser().parsingExchangeInformation() && !rexPricingTrx()->excTravelSeg().empty()))
  {
    TravelSeg* segment = rexPricingTrx()->excTravelSeg().back();

    if (segment->segmentType() == Arunk)
    {
      if (_BRD == idx)
        _getAttr(attrs, _A01, segment->origAirport());

      else if (_OFF == idx)
        _getAttr(attrs, _A02, segment->destAirport());
    }

    if (!segment->isAir())
    {
      return nullptr;
    }

    seg = static_cast<AirSeg*>(segment);
  }
  else
  {
    if (UNLIKELY(trx().travelSeg().empty() || !trx().travelSeg().back()->isAir()))
    {
      return nullptr;
    }
    seg = static_cast<AirSeg*>(trx().travelSeg().back());
  }

  if (_HSP == idx)
  {
    parseHSPType(attrs, seg);
  }
  else if (_BRD == idx)
  {
    parseBRDType(attrs, seg);
  }
  else if (LIKELY(_OFF == idx))
  {
    parseOFFType(attrs, seg);
  }
  else if (_MIL == idx)
  {
    parseMILType(attrs);
  }
  else if (_PUP == idx)
  {
    parsePUPType(attrs);
  }
  else if (_SUR == idx)
  {
    parseSURType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::AAFTypeParser::parsePUPType(const IAttributes& attrs)
{
  MinimumFareOverride* minFareOverride = nullptr;
  dataHandle().get(minFareOverride);

  if (!rexPricingTrx() || rexPricingTrx()->exchangeItin().empty())
  {
    return;
  }

  if (attrs.has(_C6L))
  {
    attrs.get(_C6L, minFareOverride->plusUpAmount);
  }

  // A11 - Plus Up Origin City
  _getAttr(attrs, _A11, minFareOverride->boardPoint);

  // A12 - Plus Up Destination City
  _getAttr(attrs, _A12, minFareOverride->offPoint);

  // A13 - Plus Up Fare Origin City
  _getAttr(attrs, _A13, minFareOverride->fareBoardPoint);

  // A14 - Plus Up Fare Destination City
  _getAttr(attrs, _A14, minFareOverride->fareOffPoint);

  // A18 - Plus Up Fare Destination City
  _getAttr(attrs, _A18, minFareOverride->constructPoint);

  // A19 - Plus Up Second Via City
  _getAttr(attrs, _A19, minFareOverride->constructPoint2);

  ExcItin* excItin = rexPricingTrx()->exchangeItin().back();

  if (excItin && !excItin->fareComponent().empty())
  {
    excItin->fareComponent().back()->hip() = minFareOverride;
  }
}

void
XMLShoppingHandler::AAFTypeParser::parseSURType(const IAttributes& attrs)
{
  if (trx().isNotExchangeTrx())
    return;

  Indicator surchargeOverrideType = 'Q';
  MoneyAmount surchargeOverrideAmt = 0.0;
  std::string surchargeOverrideCur;
  std::string fcBrdCity;
  std::string fcOffCity;
  Indicator surchargeSurchargeType = 'S';

  attrs.get(_N0F, surchargeOverrideType, surchargeOverrideType);
  attrs.get(_C69, surchargeOverrideAmt, surchargeOverrideAmt);
  attrs.get(_C46, surchargeOverrideCur, surchargeOverrideCur);
  attrs.get(_A11, fcBrdCity, fcBrdCity);
  attrs.get(_A12, fcOffCity, fcOffCity);
  attrs.get(_N28, surchargeSurchargeType, surchargeSurchargeType);

  if (surchargeOverrideAmt == 0.0)
    return;

  SurchargeOverride* surOverride = nullptr;
  trx().dataHandle().get(surOverride);
  if (surOverride == nullptr)
    return;

  if (parser().parsingExchangeInformation())
    surOverride->fromExchange() = true;

  surOverride->type() = surchargeOverrideType;
  surOverride->amount() = surchargeOverrideAmt;
  surOverride->currency() = CurrencyCode(surchargeOverrideCur);

  if (surchargeSurchargeType != 'S')
    surOverride->singleSector() = false;

  surOverride->fcBrdCity() = LocCode(fcBrdCity);
  surOverride->fcOffCity() = LocCode(fcOffCity);

  if (surchargeSurchargeType == 'J')
    surOverride->fcFpLevel() = true;

  surOverride->travelSeg() = _currentSeg;

  static_cast<RexBaseTrx*>(&trx())->exchangeOverrides().surchargeOverride().push_back(surOverride);
}

void
XMLShoppingHandler::AAFTypeParser::parseMILType(const IAttributes& attrs)
{
  if (trx().getTrxType() == PricingTrx::MIP_TRX && trx().excTrxType() == PricingTrx::AR_EXC_TRX &&
      parser().parsingExchangeInformation() && !rexPricingTrx()->exchangeItin().empty() &&
      !rexPricingTrx()->exchangeItin().back()->fareComponent().empty())
  {
    FareCompInfo* fareComponent = rexPricingTrx()->exchangeItin().back()->fareComponent().back();
    int mileageSurchargePctg = 0;
    // Indicator mileageSurchargeType = ' ';
    LocCode mileageSurchargeCity = "";

    validateInput(attrs.has(_AP2), "MIL/AP2 (MILEAGE DISPLAY TYPE) is required");

    //_getAttr(attrs, _AP2, mileageSurchargeType);
    _getAttr(attrs, _AP3, mileageSurchargeCity);
    attrs.get(_Q48, mileageSurchargePctg);

    if (fareComponent != nullptr)
    {
      if (mileageSurchargePctg > 0)
        fareComponent->mileageSurchargePctg() = mileageSurchargePctg;

      if (!mileageSurchargeCity.empty())
        fareComponent->mileageSurchargeCity() = mileageSurchargeCity;
    }
  }
}

XMLShoppingHandler::AVLTypeParser::AVLTypeParser(XMLShoppingHandler& parser)
  : XMLShoppingHandler::ElementParser(parser),
    _classOfServiceKey(0),
    _classOfServiceListVect(nullptr),
    _q16(0)
{
}

void
XMLShoppingHandler::AVLTypeParser::startParsing(const IAttributes& attrs)
{
  // Clear out the availability vectors
  _classOfServiceKey.clear();
  dataHandle().get(_classOfServiceListVect);

  if (UNLIKELY(PricingTrx::ESV_TRX == trx().getTrxType()))
  {
    attrs.get(_Q16, _q16);
    _availabilityStatusESV.clear();
  }
}

void
XMLShoppingHandler::AVLTypeParser::addThruFareClassOfService(TravelSeg* seg, ClassOfService* cos)
{
  // add a new class of service (booking code) object for this travel seg.
  // If a cos with the same booking code for this seg already exists then
  // we use the cos with more seats.
  // If no cos with the same booking code already exists, then we add it.
  ClassOfServiceList& list = trx().maxThruFareAvailabilityMap()[seg];
  ClassOfServiceList::iterator i = list.begin();
  for (; i != list.end(); ++i)
  {
    if ((*i)->bookingCode() == cos->bookingCode())
    {
      break;
    }
  }

  if (i == list.end())
  {
    list.push_back(cos);
  }
  else if (cos->numSeats() > (*i)->numSeats())
  {
    *i = cos;
  }
}

void
XMLShoppingHandler::AVLTypeParser::endParsing(const IValueString& text)
{
  if (UNLIKELY(PricingTrx::ESV_TRX == trx().getTrxType()))
  {
    endParsingESV(text);
  }
  else
  {
    trx().availabilityMap().insert(
        AvailabilityMap::value_type(ShoppingUtil::buildAvlKey(key()), value()));

    // if it's a thru-fare, then set the thru fare class of service
    if (key().size() > 1)
    {
      TSE_ASSERT(key().size() == value()->size());

      for (size_t n = 0; n != key().size(); ++n)
      {
        TravelSeg* const seg = key()[n];
        ClassOfServiceList& list = value()->at(n);

        for (ClassOfServiceList::iterator i = list.begin(); i != list.end(); ++i)
        {
          addThruFareClassOfService(seg, *i);
        }
      }
    }

    // if it's sum-of-local, then set the TravelSeg's class of service
    if (key().size() == 1)
    {
      key().front()->classOfService() = value()->front();
    }
  }
}

void
XMLShoppingHandler::AVLTypeParser::endParsingESV(const IValueString& text)
{
  if ((key().size() != value()->size()) || (key().size() != availabilityStatusESV().size()))
  {
    LOG4CXX_ERROR(_logger,
                  "XMLShoppingHandler::AVLTypeParser::endParsingESV - "
                  "Incorrect size of key and value vectors.");
    return;
  }

  CarrierCode carrier = "";
  bool haveAvailability = true;

  for (uint32_t i = 0; i < key().size(); ++i)
  {
    TravelSeg* travelSeg = key().at(i);
    AirSeg& airSeg = dynamic_cast<AirSeg&>(*travelSeg);

    if (carrier.empty())
    {
      carrier = airSeg.carrier();
    }
    else
    {
      if (carrier != airSeg.carrier())
      {
        haveAvailability = false;
        break;
      }
    }

    if (false == availabilityStatusESV().at(i))
    {
      haveAvailability = false;
      break;
    }
  }

  if (true == haveAvailability)
  {
    for (uint32_t i = 0; i < key().size(); ++i)
    {
      TravelSeg* travelSeg = key().at(i);
      AirSeg& airSeg = dynamic_cast<AirSeg&>(*travelSeg);

      airSeg.validAvailabilityIds().push_back(_q16);
    }
  }

  ((ShoppingTrx&)trx()).availabilityMap()[ShoppingUtil::buildAvlKey(key())] = value();
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::AVLTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (LIKELY(_FBK == idx))
  {
    return &parser()._fbkParser;
  }
  return nullptr;
}

const Cabin*
XMLShoppingHandler::FBKTypeParser::getCabin(const CarrierCode& cxr,
                                            const BookingCode& bkc,
                                            const DateTime& dt,
                                            const AirSeg* seg)
{
  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx()))
  {
    RBDByCabinUtil rbdUtil(trx(), SHP_HANDLER);
    return rbdUtil.getCabinByRBD(cxr, bkc, *seg, false, dt);
  }
  else
  {
    CabinMap& m = _cabinCache[dt.get64BitRepDateOnly()];
    const CabinKey key(cxr, bkc);
    CabinMap::const_iterator itor = m.find(key);
    if (itor != m.end())
    {
      return itor->second;
    }
    else
    {
      const Cabin* res = dataHandle().getCabin(cxr, bkc, dt);
      m[key] = res;
      return res;
    }
  }
}

void
XMLShoppingHandler::FBKTypeParser::startParsing(const IAttributes& attrs)
{
  if (UNLIKELY(PricingTrx::ESV_TRX == trx().getTrxType()))
  {
    startParsingESV(attrs);
  }
  else
  {
    validateInput(!trx().travelSeg().empty(), "No valid AAF entries found in request");

    // index into travel segments
    size_t q1k(0);
    attrs.get(_Q1K, q1k);
    size_t aafIndex(getAAFIndex(q1k));
    if (UNLIKELY(aafIndex >= trx().travelSeg().size()))
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Invalid AAF index found");
    }
    // get the corresponding travel seg with the index.
    // Add it to ClassOfServiceKey.
    TravelSeg* travelSeg = trx().travelSeg()[aafIndex];
    _avlParser.key().push_back(travelSeg);

    // add a corresponding empty ClassOfServiceList to the vector
    // that we can access with back() when parsing BKC's.
    _avlParser.value()->push_back(ClassOfServiceList());
    // availability and bookingcode
    if (UNLIKELY(!attrs.has(_Q17)))
    {
      //  setFBK (false);
      return;
    }
    const IValueString& str = attrs.get(_Q17);

    // work out how many class of service tokens there will be
    const int ntokens = std::count(str.begin(), str.end(), '|') + 1;
    if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx()))
    {
      std::vector<ClassOfService*>* cosVec = nullptr;
      dataHandle().get(cosVec);
      std::vector<BookingCode> bookingCodes;
      BookingCode bc;
      std::vector<int> numSeats;
      std::string numberOfSeats;
      int numSeatsInteger = 0;

      IValueString token;
      for (const char* pos = str.begin(); pos != str.end();)
      {
        if (*pos == '|')
        {
          ++pos;
        }
        const char* end = std::find(pos, str.end(), '|');
        token.assign(pos, end - pos);
        pos = end;
        const char* startNumber = std::find_if(token.begin(), token.end(), isdigit);
        validateInput(startNumber != token.begin() && startNumber != token.end(),
                      "Invalid Booking Code");
        size_t index = startNumber - token.begin();
        bc.assign(token.c_str(), index);
        bookingCodes.push_back(bc);

        numSeatsInteger = atoi(token.c_str() + index);
        numSeats.push_back(numSeatsInteger);
      }
      AirSeg& seg = *static_cast<AirSeg*>(_avlParser.key().back());
      RBDByCabinUtil rbdUtil(trx(), SHP_HANDLER);
      rbdUtil.getCabinsByRbd(seg, bookingCodes, cosVec);
      for (uint16_t j = 0; j < cosVec->size(); ++j)
      {
        ((*cosVec)[j])->numSeats() = numSeats[j];
        _avlParser.value()->back().push_back((*cosVec)[j]);
      }
    }
    else // old path
    {
      // rather than allocate the ClassOfService objects one at a time,
      // we will allocate them all together in one big vector for
      // efficiency
      std::vector<ClassOfService>* cosBuf;
      dataHandle().get(cosBuf);
      cosBuf->resize(ntokens);
      std::vector<ClassOfService>::iterator nextClassOfService = cosBuf->begin();

      IValueString token;
      for (const char* pos = str.begin(); pos != str.end();)
      {
        if (*pos == '|')
        {
          ++pos;
        }
        const char* end = std::find(pos, str.end(), '|');
        token.assign(pos, end - pos);
        pos = end;

        ClassOfService* classOfService = &*nextClassOfService;
        ++nextClassOfService;

        const char* startNumber = std::find_if(token.begin(), token.end(), isdigit);
        validateInput(startNumber != token.begin() && startNumber != token.end(),
                      "Invalid Booking Code");

        size_t index = startNumber - token.begin();
        classOfService->bookingCode().assign(token.c_str(), index);
        classOfService->numSeats() = atoi(token.c_str() + index);

        TSE_ASSERT(_avlParser.key().empty() == false);
        TSE_ASSERT(_avlParser.key().back()->isAir());
        const AirSeg& seg = *static_cast<const AirSeg*>(_avlParser.key().back());
        const Cabin* cabin =
            getCabin(seg.carrier(), classOfService->bookingCode(), seg.departureDT());
        if (LIKELY(cabin != nullptr))
        {
          classOfService->cabin() = cabin->cabin();
        }

        _avlParser.value()->back().push_back(classOfService);
      }
    }
    setFBK();
  }
}

void
XMLShoppingHandler::FBKTypeParser::startParsingESV(const IAttributes& attrs)
{
  if (trx().travelSeg().empty())
  {
    LOG4CXX_ERROR(_logger,
                  "XMLShoppingHandler::FBKTypeParser::startParsingESV - "
                  "Travel segments vector is empty.");
    return;
  }

  // Get apropriate travel segment
  size_t q1k = 0;
  attrs.get(_Q1K, q1k);
  size_t aafIndex = getAAFIndex(q1k);

  if (aafIndex >= trx().travelSeg().size())
  {
    LOG4CXX_ERROR(_logger,
                  "XMLShoppingHandler::FBKTypeParser::startParsingESV - "
                  "Incorrect index of travel segment.");
    return;
  }

  bool availabilityFound = false;

  TravelSeg* travelSeg = trx().travelSeg()[aafIndex];
  AirSeg& airSeg = *static_cast<AirSeg*>(travelSeg);

  // Get availability, booking codes and cabin
  if (!attrs.has(_Q17))
  {
    LOG4CXX_ERROR(_logger,
                  "XMLShoppingHandler::FBKTypeParser::startParsingESV - "
                  "Availability is empty.");
    return;
  }

  const IValueString& cosString = attrs.get(_Q17);

  std::vector<ClassOfService*> cosVec;
  cosVec.assign(26, nullptr);

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  boost::char_separator<char> separator("|");
  tokenizer tokens(cosString, separator);
  tokenizer::iterator tokenI;

  for (tokenI = tokens.begin(); tokenI != tokens.end(); ++tokenI)
  {
    ClassOfService* cos = nullptr;
    dataHandle().get(cos);

    if (2 != ((std::string)tokenI->data()).size())
    {
      std::string token = (std::string)tokenI->data();

      std::string::iterator startNumber = std::find_if(token.begin(), token.end(), isdigit);

      size_t index = startNumber - token.begin();
      size_t end = token.end() - startNumber;

      cos->bookingCode() = token.substr(0, index);
      cos->numSeats() = atoi(token.substr(index, end).c_str());
    }
    else
    {
      if (isdigit(((std::string)tokenI->data()).at(1)))
      {
        cos->bookingCode() = ((std::string)tokenI->data()).at(0);
        cos->numSeats() = atoi(((std::string)tokenI->data()).substr(1, 1).c_str());
      }
      else
      {
        LOG4CXX_ERROR(_logger,
                      "XMLShoppingHandler::FBKTypeParser::startParsingESV - "
                      "Incorrect seats number.");
        continue;
      }
    }

    if ((cos->numSeats() > 0) && (!cos->bookingCode().empty()))
    {
      // Get cabin
      const Cabin* cabin =
          getCabin(airSeg.carrier(), cos->bookingCode(), airSeg.departureDT(), &airSeg);

      if (nullptr != cabin)
      {
        cos->cabin() = cabin->cabin();
      }
      else
      {
        cos->cabin().setEconomyClass();
      }

      // Update class of service
      int index = ((int)(cos->bookingCode()[0])) - 65;

      if ((index >= 0) && (index <= 25))
      {
        cosVec[index] = (ClassOfService*)cos;

        availabilityFound = true;
      }
      else
      {
        LOG4CXX_ERROR(_logger,
                      "XMLShoppingHandler::FBKTypeParser::startParsingESV - "
                      "Incorrect booking code.");
        continue;
      }
    }
  }

  _avlParser.key().push_back(travelSeg);
  _avlParser.value()->push_back(cosVec);
  _avlParser.availabilityStatusESV().push_back(availabilityFound);

  setFBK();
}

void
XMLShoppingHandler::FBKTypeParser::endParsing(const IValueString& text)
{
  if (UNLIKELY(PricingTrx::ESV_TRX == trx().getTrxType()))
    return;

  if (LIKELY(!trx().getRequest()->brandedFareEntry()))
    return;

  if (trx().getRequest()->isAdvancedBrandProcessing())
  {
    if (!fallback::fallbackSci1126(&trx()))
      return;

    DiagManager diag499(trx(), Diagnostic499);
    AvailabilityAdjuster adjuster(trx().getRequest()->brandedFareBookingCode(),
                                  trx().getRequest()->brandedFareSecondaryBookingCode());
    adjuster.process(_avlParser.value()->back(), diag499);
    return;
  }

  const PricingRequest* request = trx().getRequest();
  const uint16_t brandSize = request->getBrandedFareSize();

  for (uint16_t brandIndex = 0; brandIndex < brandSize; brandIndex++)
  {
    if (request->brandedFareBookingCode(brandIndex).empty() &&
        request->brandedFareBookingCodeExclude(brandIndex).empty())
      return;
  }

  typedef std::map<ClassOfService*, std::set<uint16_t>> BkgPendingToRemove;
  BkgPendingToRemove bkgPendingToRemove;
  std::stringstream diag499Str;
  for (uint16_t brandIndex = 0; brandIndex < brandSize; brandIndex++)
  {
    bool requestedBrandHasOnlyBookingCodes = (request->brandedFareFamily(brandIndex).empty() &&
                                              request->brandedFareBasisCode(brandIndex).empty());

    std::vector<ClassOfService*>& cosVec = _avlParser.value()->back();
    const std::vector<BookingCode>& brandedFareExclude =
        request->brandedFareBookingCodeExclude(brandIndex);
    const std::vector<BookingCode>& brandedFare = request->brandedFareBookingCode(brandIndex);
    std::vector<BookingCode>& brandedFBCDispDiag499 =
        trx().getRequest()->brandedFBCDispDiag499(brandIndex);
    std::vector<ClassOfService*>::iterator it;
    for (it = cosVec.begin(); it != cosVec.end(); it++)
    {
      BookingCode bkc = (*it)->bookingCode();
      std::vector<BookingCode>::const_iterator bkcIt =
          std::find(brandedFare.begin(), brandedFare.end(), bkc);
      std::vector<BookingCode>::const_iterator bkcExIt =
          std::find(brandedFareExclude.begin(), brandedFareExclude.end(), bkc);
      // The brand has only Booking Codes defined
      // and current Booking Code is not requested or the Booking Code is excluded
      if ((requestedBrandHasOnlyBookingCodes && (bkcIt == brandedFare.end())) ||
          (bkcExIt != brandedFareExclude.end()))
      {
        std::vector<BookingCode>::iterator displayedIt =
            std::find(brandedFBCDispDiag499.begin(), brandedFBCDispDiag499.end(), bkc);
        if (displayedIt == brandedFBCDispDiag499.end())
        {
          diag499Str << "BRANDED CHK FOR " << request->brandId(brandIndex)
                     << " - PENDING FOR SETTING THE AVAILABILITY OF BKCODE " << bkc << " TO 0.\n";
        }
        brandedFBCDispDiag499.push_back(bkc);
        bkgPendingToRemove[*it].insert(brandIndex);
      }
    }
  }

  BkgPendingToRemove::const_iterator cos = bkgPendingToRemove.begin();
  for (; cos != bkgPendingToRemove.end(); cos++)
  {
    if (cos->second.size() == brandSize)
    {
      diag499Str << "BRANDED CHK - SETTING THE AVAILABILITY OF BKCODE " << cos->first->bookingCode()
                 << " TO 0.\n";
      cos->first->numSeats() = 0;
    }
  }

  if (trx().diagnostic().diagnosticType() == Diagnostic499)
  {
    DiagManager diag499(trx(), Diagnostic499);
    diag499 << diag499Str.str();
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::FBKTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_BKC == idx)
  {
    if (isSetFBK() || (PricingTrx::ESV_TRX == trx().getTrxType()))
    {
      return nullptr;
    }
    parseBKCType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::FBKTypeParser::parseBKCType(const IAttributes& attrs)
{
  _avlParser.value()->back().push_back(parseClassOfService(attrs));
}

ClassOfService*
XMLShoppingHandler::FBKTypeParser::parseClassOfService(const IAttributes& attrs)
{
  ClassOfService* classOfService = dataHandle().create<ClassOfService>();

  _getAttr(attrs, _B30, classOfService->bookingCode());

  const std::string& bkc(classOfService->bookingCode());
  classOfService->bookingCode() = fallback::purgeBookingCodeOfNonAlpha(shoppingTrx())
                                      ? bkc
                                      : DataModelMap::purgeBookingCodeOfNonAlpha(bkc);

  attrs.get(_Q17, classOfService->numSeats(), classOfService->numSeats());

  TSE_ASSERT(!_avlParser.key().empty());
  TSE_ASSERT(_avlParser.key().back()->isAir());
  const AirSeg& seg = *static_cast<const AirSeg*>(_avlParser.key().back());
  const Cabin* cabin =
      getCabin(seg.carrier(), classOfService->bookingCode(), seg.departureDT(), &seg);
  if (cabin != nullptr)
  {
    classOfService->cabin() = cabin->cabin();
  }

  return classOfService;
}

void
XMLShoppingHandler::AAFTypeParser::parseHSPType(const IAttributes& attrs, AirSeg* seg)
{
  if (seg == nullptr)
  {
    LOG4CXX_ERROR(_logger, "XMLShoppingHandler::AAFTypeParser::parseHSPType - Travel is NULL");
    return;
  }

  LocCode hiddenCity;
  _getAttr(attrs, _A03, hiddenCity);

  const Loc* loc = getLoc(hiddenCity, seg->departureDT());
  if (loc == nullptr)
  {
    loc = getLoc(hiddenCity, DateTime::localTime());
  }

  if (loc != nullptr)
  {
    seg->hiddenStops().push_back(loc);
  }
}

void
XMLShoppingHandler::AAFTypeParser::parseBRDType(const IAttributes& attrs, AirSeg* seg)
{
  if (UNLIKELY(seg == nullptr))
  {
    return;
  }

  _getAttr(attrs, _A01, seg->origAirport());
  // attribute to warm the server
  if (UNLIKELY(attrs.has(_Q0R)))
  {
    int numDateFromToday = attrs.get<int>(_Q0R, 0);
    DateTime today = DateTime::localTime();
    seg->departureDT() = today.addDays(numDateFromToday);
  }
  else
  {
    seg->departureDT() = parseDateTime(attrs.get(_D01), attrs.get(_D31));
  }

  if (LIKELY(_pssOpenDateBase != attrs.get(_D01)))
    seg->pssDepartureDate() = seg->departureDT().dateToString(YYYYMMDD, "-");
  seg->pssDepartureTime() = seg->departureDT().timeToString(HHMM, "");

  if (seg->bookingDT() == DateTime::emptyDate())
  {
    seg->bookingDT() = bookingDT();
  }
  seg->origin() = getLoc(seg->origAirport(), seg->departureDT());

  // Catch invalid date (1966-01-*)
  if (UNLIKELY(!seg->origin()))
  {
    seg->origin() = getLoc(seg->origAirport(), DateTime::localTime());
  }
}

void
XMLShoppingHandler::AAFTypeParser::parseOFFType(const IAttributes& attrs, AirSeg* seg)
{
  if (UNLIKELY(seg == nullptr))
    return;

  _getAttr(attrs, _A02, seg->destAirport());

  if (UNLIKELY(attrs.has(_Q0R)))
  {
    int numDateFromToday = attrs.get<int>(_Q0R, 0);
    DateTime today = DateTime::localTime();
    seg->arrivalDT() = today.addDays(numDateFromToday);
  }
  else
  {
    seg->arrivalDT() = parseDateTime(attrs.get(_D02), attrs.get(_D32));
  }

  if (LIKELY(_pssOpenDateBase != attrs.get(_D02)))
    seg->pssArrivalDate() = seg->arrivalDT().dateToString(YYYYMMDD, "-");
  seg->pssArrivalTime() = seg->arrivalDT().timeToString(HHMM, "");

  seg->destination() = getLoc(seg->destAirport(), seg->arrivalDT());
  if (UNLIKELY(!seg->destination()))
  {
    seg->destination() = getLoc(seg->destAirport(), DateTime::localTime());
  }
}

void
XMLShoppingHandler::ITNTypeParser::startParsing(const IAttributes& attrs)
{
  if (UNLIKELY(rexPricingTrx() != nullptr))
  {
    startParsingRexITN(attrs);
    return;
  }
  Itin* itin = nullptr;
  dataHandle().get(itin);
  if (!parser().isDisableFamilyLogic())
  {
    if (attrs.has(_Q5Q))
    {
      int family(0);
      attrs.get(_Q5Q, family);
      itin->setItinFamily(family);

      // group similar Itins into base by family number
      Itin*& baseItin = _itinFamily[family];
      if (baseItin == nullptr)
      {
        baseItin = itin;
      }
      else
      {
        validateInput(!trx().isAltDates(), "Family logic is not supported for Alt Dates");

        baseItin->addSimilarItin(itin);
      }
    }
  }

  if (LIKELY(attrs.has(_NUM)))
  {
    attrs.get(_NUM, itin->itinNum());
  }
  char ch(0);
  attrs.get(_PCA, ch);
  itin->dcaSecondCall() = 'T' == ch;

  ch = 0;
  attrs.get(_TFO, ch);
  itin->setThruFareOnly('T' == ch);

  trx().itin().push_back(itin);
  itinDateTime().clear();

  MoneyAmount estTax(0);
  if (attrs.has(_C65))
  {
    attrs.get(_C65, estTax);
    itin->setEstimatedTax(estTax);
  }
}

void
XMLShoppingHandler::ITNTypeParser::startParsingRexITN(const IAttributes& attrs)
{
  Itin* itin = nullptr;
  if (parser().parsingExchangeInformation())
  {
    parser().numberOfEXCITNTypes()++;
    validateInput(parser().numberOfEXCITNTypes() == 1,
                  "To many EXC/ITN elements in RexShopping request");

    if (rexPricingTrx()->exchangeItin().size() != 1)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingHandler::ITNTypeParser::startParsingRexITN  - "
                    "There Should be only one Exchange Itin");
    }
    // exchangeItin created in initTrx
    itin = dynamic_cast<Itin*>(rexPricingTrx()->exchangeItin().back());
  }
  else
  {
    RexNewItin* newItin = dataHandle().create<RexNewItin>();
    newItin->rexTrx() = rexPricingTrx();
    itin = newItin;

    if (trx().getTrxType() == PricingTrx::RESHOP_TRX)
    {
      parser().numberOfNEWITNTypes()++;
      validateInput(parser().numberOfNEWITNTypes() == 1,
                    "To many ITN elements in RexShopping request");
    }

    // newItin and itin should point to _itin element
    rexPricingTrx()->newItin().push_back(newItin);
  }

  if (itin == nullptr)
  {
    return;
  }

  char ch(0);
  attrs.get(_PCA, ch);
  itin->dcaSecondCall() = 'T' == ch;

  ch = 0;
  attrs.get(_TFO, ch);
  itin->setThruFareOnly('T' == ch);

  if (attrs.has(_NUM))
  {
    attrs.get(_NUM, itin->itinNum());
  }

  MoneyAmount estTax(0);
  if (attrs.has(_C65))
  {
    attrs.get(_C65, estTax);
    itin->setEstimatedTax(estTax);
  }

  if (attrs.has(_Q5Q) && !parser().isDisableFamilyLogic())
  {
    int family(0);
    attrs.get(_Q5Q, family);
    Itin*& baseItin = _itinFamily[family];

    if (baseItin == nullptr)
    {
      baseItin = itin;
    }
    else
    {
      baseItin->addSimilarItin(itin);
    }
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::ITNTypeParser::newElement(int idx, const IAttributes& attrs)
{
  validateInput(shoppingTrx() == nullptr, "unexpected ITN element in IS request");

  if (LIKELY(_SID == idx))
  {
    return &parser()._sidParser;
  }
  else if (_BRI == idx)
  {
    return &parser()._briParser;
  }

  return nullptr;
}

void
XMLShoppingHandler::ITNTypeParser::endParsing(const IValueString& text)
{
  if (UNLIKELY(rexPricingTrx() != nullptr))
  {
    if (parser().parsingExchangeInformation())
    {
      // exchange segments are not shared across  itins - there is just one exc itin
      ExcItin* excItin = rexPricingTrx()->exchangeItin().back();
      std::vector<TravelSeg*>::iterator segIter = excItin->travelSeg().begin();
      for (size_t segOrder = 1; segIter != excItin->travelSeg().end(); ++segOrder, ++segIter)
      {
        TravelSeg& segment = **segIter;
        segment.segmentOrder() = segOrder;
      }

      // setup origin and destination characteristics for ARUNK segment
      for (size_t i = 1; i < excItin->travelSeg().size(); i++)
      {
        TravelSeg* tvlSeg = excItin->travelSeg()[i];
        if (tvlSeg->segmentType() == Arunk)
        {
          TravelSeg* prevSeg = excItin->travelSeg()[i - 1];

          tvlSeg->origAirport() = prevSeg->destAirport();
          tvlSeg->origin() = prevSeg->destination();
          tvlSeg->boardMultiCity() = prevSeg->offMultiCity();

          if (i + 1 < excItin->travelSeg().size())
          {
            TravelSeg* nextSeg = excItin->travelSeg()[i + 1];

            tvlSeg->destAirport() = nextSeg->origAirport();
            tvlSeg->destination() = nextSeg->origin();
            tvlSeg->offMultiCity() = nextSeg->boardMultiCity();
            //   tvlSeg->departureDT() = nextSeg->departureDT();
            //  tvlSeg->bookingDT() = nextSeg->bookingDT();
          }
        }
      }
    }
    else
    {
      std::vector<Itin*>::iterator itinIter = rexPricingTrx()->newItin().begin();
      std::vector<Itin*>::const_iterator itinIterE = rexPricingTrx()->newItin().end();
      for (; itinIter != itinIterE; ++itinIter)
      {
        Itin& newItin = **itinIter;
        newItin.calcCurrencyOverride() = parser().newItinCurrency();
      }
    }
  }

  if (!trx().isBRAll() || !fallback::fallbackEnableFamilyLogicInBfa(&trx()))
  {
    // see if this itin was made a child of another
    // itin, in which case it shouldn't appear in the main vector
    for (std::vector<Itin*>::const_iterator i = trx().itin().begin(); i != trx().itin().end(); ++i)
    {
      if (std::count_if((*i)->getSimilarItins().begin(),
                        (*i)->getSimilarItins().end(),
                        [&](const SimilarItinData& data)
                        { return data.itin == trx().itin().back(); }))
      {
        trx().itin().pop_back();
        return;
      }
    }
  }

  if (LIKELY(trx().getTrxType() == PricingTrx::MIP_TRX))
    cloneStopOverSeg(trx().itin().back()->travelSeg());

  if (!trx().isAltDates())
  {
    return;
  }

  if (parser()._pricingLegs.size() > 2)
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "INVALID NUMBER OF LEGS FOR ALT DATE REQUEST");

  PricingTrx::AltDateInfo* altDateInfo = trx().dataHandle().create<PricingTrx::AltDateInfo>();
  altDateInfo->numOfSolutionNeeded = 1;
  if (itinDateTime().size() == 2)
  {
    DatePair datePair(itinDateTime()[0], itinDateTime()[1]);

    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    dataHandle().get(trx().itin().back()->datePair());
    *(trx().itin().back()->datePair()) = datePair;
    trx().altDatePairs().insert(mapItem);
  }
  else if (itinDateTime().size() == 1)
  {
    DatePair datePair(itinDateTime()[0], DateTime::emptyDate());

    if (trx().getRequest()->originBasedRTPricing())
    {
      if (trx().outboundDepartureDate() != DateTime::emptyDate())
      {
        datePair.second = datePair.first;
        datePair.first = trx().outboundDepartureDate();
      }
      else
      {
        datePair.second = trx().inboundDepartureDate();
      }
    }

    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    dataHandle().get(trx().itin().back()->datePair());
    *(trx().itin().back()->datePair()) = datePair;
    trx().altDatePairs().insert(mapItem);
  }
}

void
XMLShoppingHandler::ITNTypeParser::cloneStopOverSeg(std::vector<TravelSeg*>& travelSeg)
{
  GeoTravelType geoTravelType = TravelSegUtil::getGeoTravelType(travelSeg, dataHandle());
  std::vector<bool> stopOver = TravelSegUtil::calculateStopOvers(travelSeg, geoTravelType);

  unsigned int segmentsSize = travelSeg.size();
  for (unsigned int i = 0; i < segmentsSize - 1; i++) // - 1 last seg is always stopOver
  {
    TravelSeg* firstSeg = travelSeg[i];
    std::map<const TravelSeg*, bool>::const_iterator it = travelSegStopOver.find(firstSeg);
    if (it != travelSegStopOver.end())
    {
      if (it->second != stopOver[i])
      {
        std::map<const TravelSeg*, TravelSeg*>::const_iterator clonedIt =
            clonedTravelSeg.find(firstSeg);
        if (clonedIt == clonedTravelSeg.end())
        {
          AirSeg* newAirSeg = nullptr;
          AirSeg* oldAirSeg = dynamic_cast<AirSeg*>(firstSeg);
          if (oldAirSeg)
          {
            newAirSeg = dataHandle().create<AirSeg>();
            *newAirSeg = *oldAirSeg;
            travelSeg[i] = newAirSeg;
            clonedTravelSeg[firstSeg] = newAirSeg;
          }
        }
        else
          travelSeg[i] = clonedIt->second;
      }
    }
    else
      travelSegStopOver[firstSeg] = stopOver[i];
  }
}

void
XMLShoppingHandler::SIDTypeParser::startParsing(const IAttributes& attrs)
{
  Itin* target;
  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    if (rexPricingTrx()->exchangeItin().size() != 1)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingHandler::SIDTypeParser::startParsing - "
                    "Incorrect size of Exchange Itin");
      return;
    }
    target = rexPricingTrx()->exchangeItin().back();
  }
  else
  {
    if (UNLIKELY(trx().itin().empty()))
    {
      LOG4CXX_ERROR(_logger, "XMLShoppingHandler::SIDTypeParser::startParsing - Itin is empty");
      return;
    }

    target = trx().itin().back();
  }

  uint32_t legIdxAttr(0);
  attrs.get(_Q14, legIdxAttr);
  const uint32_t legID(getLEGIndex(legIdxAttr));
  validateInput(legID < legs().size(), "Invalid LEG index found");

  size_t q15(0);
  attrs.get(_Q15, q15);
  const uint32_t sopID = getSOPIndex(legID, q15);
  validateInput(sopID < legs()[legID].sop().size(), "Invalid SOP index found");

  const Itin* const source = legs()[legID].sop()[sopID].itin();

  if (!target->travelSeg().empty() && !source->travelSeg().empty() &&
      !parser().parsingExchangeInformation())
  {
    // see if we need to insert an arunk segment
    TravelSeg* const seg1 = target->travelSeg().back();
    TravelSeg* const seg2 = source->travelSeg().front();

    if ((seg1->offMultiCity() != seg2->boardMultiCity()) &&
        (seg1->destAirport() != seg2->origAirport()))
    {
      ArunkSeg* const arunk = TravelSegUtil::buildArunk(trx().dataHandle(), seg1, seg2);
      arunk->legId() = legID;
      target->travelSeg().push_back(arunk);
    }
  }

  _travelSegNumber = target->travelSeg().size();
  target->travelSeg().insert(
      target->travelSeg().end(), source->travelSeg().begin(), source->travelSeg().end());
  target->legID().push_back(std::pair<int, int>(legID, sopID));
  const DateTime& dep1 = source->travelSeg().front()->departureDT();
  DateTime val1(dep1.year(), dep1.month(), dep1.day());
  itinDateTime().push_back(val1);
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::SIDTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (LIKELY(_BKK == idx))
  {
    parseBKKType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::SIDTypeParser::parseBKKType(const IAttributes& attrs)
{
  Itin* target;

  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    if (rexPricingTrx()->exchangeItin().size() != 1)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingHandler::SIDTypeParser::parseBKKType - "
                    "Incorrect size of Exchange Itin");
      return;
    }
    target = rexPricingTrx()->exchangeItin().back();
  }
  else
  {
    if (UNLIKELY(trx().itin().empty()))
    {
      LOG4CXX_ERROR(_logger, "XMLShoppingHandler::SIDTypeParser::parseBKKType - Itin is empty");
      return;
    }
    target = trx().itin().back();
  }

  validateInput(_travelSegNumber < target->travelSeg().size(),
                "booking code is exceed number of travelSeg");

  TravelSeg* const seg = target->travelSeg()[_travelSegNumber];

  BookingCode bkc(DUMMY_BOOKING);

  CabinType cabin;
  cabin.setEconomyClass();

  Indicator cab = cabin.getCabinIndicator();
  _getAttr(attrs, _B30, bkc);

  if (UNLIKELY(attrs.has(_Q13)))
  {
    attrs.get(_Q13, cab, cab);
    cabin.setClass(cab);
  }

  if (attrs.has(_B31))
  {
    const char alfaNumCabin = attrs.get<char>(_B31, 0);
    cabin.setClassFromAlphaNum(alfaNumCabin);
  }

  // only update the booking code information if this is the first
  // time it is being set, or if this itin has a higher number for
  // the cabin field.
  if (seg->getBookingCode() == DUMMY_BOOKING || cabin > seg->bookedCabin())
  {
    seg->setBookingCode(bkc);
    seg->bookedCabin() = cabin;
  }

  // ASSUMPTION: MIP req have BKK element for ARUNK segment
  if (seg->segmentType() == Arunk && !parser().parsingExchangeInformation())
  {
    TravelSeg* const seg = target->travelSeg()[_travelSegNumber + 1];
    if (((_travelSegNumber + 1) < uint16_t(target->travelSeg().size())) &&
        (seg->getBookingCode() == DUMMY_BOOKING || cabin > seg->bookedCabin()))
    {
      seg->setBookingCode(bkc);
      seg->bookedCabin() = cabin;

      ClassOfService* classOfService = &dataHandle().safe_create<ClassOfService>();

      classOfService->bookingCode() = bkc;
      classOfService->cabin() = cabin;

      target->origBooking().push_back(classOfService);

      ++_travelSegNumber;
    }
  }

  ClassOfService* classOfService = &dataHandle().safe_create<ClassOfService>();

  classOfService->bookingCode() = bkc;
  classOfService->cabin() = cabin;

  target->origBooking().push_back(classOfService);

  ++_travelSegNumber;
}

void
XMLShoppingHandler::BRITypeParser::startParsing(const IAttributes& attrs)
{
  Itin* target;

  if (trx().getTrxType() == PricingTrx::IS_TRX)
  {
    _brandCode.clear();
    if (attrs.has(_SB2))
      _getAttr(attrs, _SB2, _brandCode);

    if (!_brandCode.empty())
      trx().getMutableBrandsFilterForIS().insert(_brandCode);
  }
  else
  {
    if (trx().itin().empty())
    {
      LOG4CXX_ERROR(_logger, "XMLShoppingHandler::BRITypeParser::startParsing - No itins");
      return;
    }

    target = trx().itin().back();

    if (attrs.has(_SB2))
      _getAttr(attrs, _SB2, _brandCode);

    if (!_brandCode.empty())
    {
      target->brandFilterMap()[_brandCode];
    }
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::BRITypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (trx().getTrxType() != PricingTrx::IS_TRX)
  {
    if (_PRG == idx)
      parsePRGType(attrs);
  }

  return nullptr;
}

void
XMLShoppingHandler::BRITypeParser::parsePRGType(const IAttributes& attrs)
{
  Itin* target;

  if (trx().itin().empty())
  {
    LOG4CXX_ERROR(_logger, "XMLShoppingHandler::BRITypeParser::parsePRGType - Empty itin");
    return;
  }

  target = trx().itin().back();

  ProgramID programId;
  if (attrs.has(_SC0))
  {
    _getAttr(attrs, _SC0, programId);
    if (!programId.empty())
    {
      try
      {
        target->brandFilterMap().at(_brandCode).insert(programId);
      }
      catch (std::out_of_range& outOfRange)
      {
        LOG4CXX_ERROR(_logger, "XMLShoppingHandler::BRITypeParser::parsePRGType - No filter");
        return;
      }
    }
  }
}

void
XMLShoppingHandler::SOPTypeParser::startParsing(const IAttributes& attrs)
{
  Itin* itin;
  dataHandle().get(itin);

  if (shoppingTrx() != nullptr)
  {
    trx().itin().push_back(itin);
  }

  TSE_ASSERT(legs().empty() == false);

  validateInput(attrs.has(_Q15), "SOP/Q15 is required");
  uint32_t sopIndex(0);
  attrs.get(_Q15, sopIndex);

  setSOPIndex(legs().size() - 1, sopIndex, legs().back().sop().size());

  legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itin, sopIndex));
  legs().back().requestSops() = legs().back().sop().size();
  char ch(0);
  attrs.get(_PBX, ch);
  legs().back().sop().back().combineSameCxr() = 'T' == ch;

  ch = 0;
  attrs.get(_UGL, ch);
  legs().back().sop().back().isLngCnxSop() = 'T' == ch;

  attrs.get(_PCU, ch);
  legs().back().sop().back().setCustomSop('T' == ch);
  legs().back().setCustomLeg((legs().back().isCustomLeg() || 'T' == ch));

  // Add to lookup map in shopping transaction for external sop id
  if (shoppingTrx() != nullptr)
  {
    std::map<uint32_t, IndexPair>& extSopMap = shoppingTrx()->externalSopToLegMap();
    IndexPair iPair(legs().size() - 1, legs().back().sop().size() - 1);
    extSopMap[sopIndex] = iPair;
  }
}

namespace
{
bool
mergeClassOfService(std::vector<ClassOfService*>& cos1, const std::vector<ClassOfService*>& cos2)
{
  bool res = false;
  for (std::vector<ClassOfService*>::const_iterator i = cos2.begin(); i != cos2.end(); ++i)
  {
    std::vector<ClassOfService*>::iterator j = cos1.begin();
    for (; j != cos1.end(); ++j)
    {
      if ((*i)->bookingCode() == (*j)->bookingCode())
      {
        if ((*i)->numSeats() > (*j)->numSeats())
        {
          *j = *i;
          res = true;
        }

        break;
      }
    }

    if (j == cos1.end())
    {
      cos1.push_back(*i);
      res = true;
    }
  }

  return res;
}
}

void
XMLShoppingHandler::SOPTypeParser::endParsing(const IValueString& text)
{
  TSE_ASSERT(legs().empty() == false && legs().back().sop().empty() == false);
  Itin* const itin = legs().back().sop().back().itin();
  TSE_ASSERT(itin);

  validateInput(itin->travelSeg().empty() == false, "SOP has no travel segs");

  // last travel seg is a stop over
  itin->travelSeg().back()->stopOver() = true;
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::SOPTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_FID == idx)
  {
    parseFIDType(attrs);
  }
  else if (LIKELY(_AID == idx))
  {
    parseAIDType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::SOPTypeParser::parseAIDType(const IAttributes& attrs)
{
  if (UNLIKELY(PricingTrx::ESV_TRX == trx().getTrxType()))
  {
    uint32_t _q16 = 0;
    attrs.get(_Q16, _q16);

    legs().back().sop().back().availabilityIds().push_back(_q16);
  }
}

void
XMLShoppingHandler::SOPTypeParser::parseFIDType(const IAttributes& attrs)
{
  size_t q1k(0);
  attrs.get(_Q1K, q1k);
  const size_t aafIndex = getAAFIndex(q1k);

  TravelSeg* travelSeg = nullptr;
  Itin* itin = legs().back().sop().back().itin();
  int16_t legId = legs().back().originalId();

  if (UNLIKELY(parser().parsingExchangeInformation()))
  {
    TSE_ASSERT(rexPricingTrx()->excTravelSeg().empty() == false);

    if (aafIndex >= rexPricingTrx()->excTravelSeg().size())
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Invalid Exchange AAF index found");
    }

    // Get the corresponding travel seg with the index.
    travelSeg = rexPricingTrx()->excTravelSeg()[aafIndex];
    travelSeg->legId() = legId;

    itin->travelSeg().push_back(travelSeg);
    return; // EXC itin use AAF/N03 to create Arunk
  }

  TSE_ASSERT(trx().travelSeg().empty() == false);

  if (UNLIKELY(aafIndex >= trx().travelSeg().size()))
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Invalid AAF index found");
  }

  // Get the corresponding travel seg with the index.
  travelSeg = trx().travelSeg()[aafIndex];

  if (!itin->travelSeg().empty())
  {
    // see if we need to insert an arunk segment
    TravelSeg* const seg = itin->travelSeg().back();
    if ((trx().getTrxType() == PricingTrx::MIP_TRX ||
         trx().getTrxType() == PricingTrx::RESHOP_TRX) &&
        (seg->offMultiCity() != travelSeg->boardMultiCity()) &&
        (seg->destAirport() != travelSeg->origAirport()))
    {
      ArunkSeg* const arunk = TravelSegUtil::buildArunk(trx().dataHandle(), seg, travelSeg);
      arunk->legId() = legId;
      adjustLegIdForTaxEnhancement(*arunk);
      itin->travelSeg().push_back(arunk);
    }
  }

  // If travel segment is used on multiple legs then clone it to ensure
  // that its legId() is properly initialized.
  if (!parser().isLegIdAdjusted() && travelSeg->legId() >= 0 && travelSeg->legId() != legId)
  {
    travelSeg = travelSeg->clone(trx().dataHandle());
    setAAFIndex(q1k, trx().travelSeg().size());
    trx().travelSeg().push_back(travelSeg);
  }

  travelSeg->legId() = legId;
  adjustLegIdForTaxEnhancement(*travelSeg);
  itin->travelSeg().push_back(travelSeg);
}

namespace
{
const uint16_t INBOUND_LEG_ID = 1;
}

void
XMLShoppingHandler::SOPTypeParser::adjustLegIdForTaxEnhancement(TravelSeg& seg)
{
  if (TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
  {
    if (UNLIKELY(trx().getRequest()->originBasedRTPricing() &&
                 !trx().outboundDepartureDate().isEmptyDate()))
    {
      parser().setLegIdAdjusted(true);
      // Set Leg Id to inbound
      seg.legId() = INBOUND_LEG_ID;
    }
  }
}

void
XMLShoppingHandler::LEGTypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(attrs.has(_Q14), "LEG/Q14 (LEG ID) is required");

  _cabinSelector.setup(&trx());

  parser().setLegIdAdjusted(false);

  uint32_t q14(0);
  attrs.get(_Q14, q14);
  setLEGIndex(q14, legs().size());
  _legId = q14;
  _cabinSelector.setLegId(_legId);

  legs().push_back(ShoppingTrx::Leg());
  legs().back().originalId() = q14;

  const size_t maxLegs = 10;

  validateInput(shoppingTrx() == nullptr || legs().size() <= maxLegs,
                "Too many legs specified in request");

  if (attrs.has(_Q13))
  {
    Indicator preferredCabinInd = attrs.get<Indicator>(_Q13, 0);
    _cabinSelector.setClass(preferredCabinInd);
  }

  if (attrs.has(_B31))
  {
    char alfaNumCabin = attrs.get<char>(_B31, 0);

    if (!fallback::fallbackPriceByCabinActivation(&trx()) &&
        !trx().getOptions()->cabin().isUndefinedClass() &&
        trx().billing()->actionCode().compare(0, 4, "WPNI") == 0)
    {
      if (trx().getOptions()->cabin().isAllCabin())
      {
        alfaNumCabin = 'Y'; // force ECONOMY_CLASS
      }
      else
      {
        alfaNumCabin = trx().getOptions()->cabin().getClassAlphaNum();
      }
    }

    if (trx().isBRAll())
    {
      // in TN Multiple brands mode we don't filter fares but whole brands
      // that's why regular cabin selector is not set
      CabinType cabinForBrandFiltering;
      cabinForBrandFiltering.setClassFromAlphaNum(alfaNumCabin);
      trx().setCabinForLeg(q14, cabinForBrandFiltering);
    }
    _cabinSelector.setClassFromAlphaNum(alfaNumCabin);
  }

  if (!parser().parsingExchangeInformation())
  {
    if (trx().getRequest()->isBrandedFaresRequest())
    {
      bool isFixedLeg = false;
      attrs.get(_FIX, isFixedLeg, false);

      if (isFixedLeg)
      {
        trx().getRequest()->setContextShoppingRequest();
        // according to BRD catch all bucket should be disabled in Context Shopping
        parser().setCatchAllBucket(*(trx().getRequest()), false);
      }
      trx().getMutableFixedLegs().push_back(isFixedLeg);
    }

    bool returnAllFlights = false;
    attrs.get(_RAF, returnAllFlights, false);

    if (returnAllFlights)
    {
      legs().back().setReturnAllFlights();
    }
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::LEGTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_SOP == idx)
  {
    return &parser()._sopParser;
  }
  else if (_BKK == idx)
  {
    parseLEGBKKType(attrs);
  }
  else if (_PCL == idx)
  {
    return &parser()._pclParser;
  }
  return nullptr;
}

void
XMLShoppingHandler::LEGTypeParser::endParsing(const IValueString& text)
{
  if (trx().getOptions()->cabin().isUndefinedClass())
  {
    _cabinSelector.selectCabin();
  }

  legs().back().preferredCabinClass() = _cabinSelector.getPreferredCabin();
  trx().legPreferredCabinClass().push_back(_cabinSelector.getPreferredCabin());

  if (parser().isLegIdAdjusted())
  {
    if (trx().legPreferredCabinClass().size() < INBOUND_LEG_ID + 1)
      trx().legPreferredCabinClass().resize(INBOUND_LEG_ID + 1);
    trx().legPreferredCabinClass()[INBOUND_LEG_ID] = _cabinSelector.getPreferredCabin();
  }

  if (trx().calendarSoldOut() || (trx().awardRequest() && trx().isAltDates()))
  {
    trx().calendarRequestedCabin() = _cabinSelector.getPreferredCabin();
  }

  if (trx().getRequest()->isContextShoppingRequest() && !trx().getFixedLegs().empty() &&
      trx().getFixedLegs().back())
  {
    Itin* itin = legs().back().sop().back().itin();

    for (TravelSeg* travelSeg : itin->travelSeg())
    {
      travelSeg->isShopped() = true;
    }
  }
}

void
XMLShoppingHandler::LEGTypeParser::parseLEGBKKType(const IAttributes& attrs)
{
  if (attrs.has(_Q1K) && TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx()))
  {
    parseLEGBKKForRBDAnswerTable(attrs);
    return;
  }

  BookingCode bookingCode = BLANK_CODE;
  CarrierCode carrier = BLANK_CODE;
  LocCode origin = BLANK_CODE;
  LocCode destination = BLANK_CODE;

  if (attrs.has(_B30))
  {
    _getAttr(attrs, _B30, bookingCode);
  }

  if (attrs.has(_B00))
  {
    _getAttr(attrs, _B00, carrier);
  }

  if (attrs.has(_A01))
  {
    _getAttr(attrs, _A01, origin);
  }

  if (attrs.has(_A02))
  {
    _getAttr(attrs, _A02, destination);
  }

  if (bookingCode != BLANK_CODE && carrier != BLANK_CODE)
  {
    DateTime dt;

    if (origin == BLANK_CODE)
    {
      if (trx().getRequest()->ticketingDT().isValid())
      {
        dt = trx().getRequest()->ticketingDT();
      }
      else
      {
        dt = DateTime::localTime();
      }

      const Cabin* cabin = dataHandle().getCabin(carrier, bookingCode, dt);

      if ((cabin != nullptr) &&
          ((_cabinSelector.getBkkPreferredCabinInd() == CabinType::UNDEFINED_CLASS) ||
           (cabin->cabin().getCabinIndicator() < _cabinSelector.getBkkPreferredCabinInd())))
      {
        _cabinSelector.setBkkPreferredCabinInd(cabin->cabin().getCabinIndicator());
      }
    }
    else
    {
      _cabinSelector.addBkkNode(bookingCode, carrier, origin, destination);
    }
  }
}

void
XMLShoppingHandler::LEGTypeParser::parseLEGBKKForRBDAnswerTable(const IAttributes& attrs)
{
  BookingCode bookingCode = BLANK_CODE;

  if (trx().travelSeg().empty() || !TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx()) ||
      !attrs.has(_B30) || !attrs.has(_Q1K))
  {
    return;
  }

  _getAttr(attrs, _B30, bookingCode);

  uint32_t originalId = attrs.get<uint32_t>(_Q1K, 0) + 1;
  const AirSeg* airSeg = 0;

  for (const TravelSeg* tvlSeg : trx().travelSeg())
  {
    if (tvlSeg->isAir() && (tvlSeg->originalId() == originalId))
    {
      airSeg = tvlSeg->toAirSeg();
      break;
    }
  }

  if (airSeg != nullptr)
  {
    const LocCode& origin = airSeg->origin()->loc();
    const LocCode& destination = airSeg->destination()->loc();
    _cabinSelector.addBkkNode(bookingCode, airSeg->carrier(), origin, destination);
  }
}

void
XMLShoppingHandler::BFFTypeParser::startParsing(const IAttributes& attrs)
{
  AirSeg* outSeg;
  dataHandle().get(outSeg);

  outSeg->departureDT() = DateTime::emptyDate();

  if (attrs.has(_D01))
  {
    const IValueString& d01 = attrs.get(_D01);
    outSeg->departureDT() = parseDateTime(d01);
  }

  /* Board city parsing */
  validateInput(attrs.has(_A11), "BFF/A11 - Board City is required");
  LocCode& boardCityOut = outSeg->origAirport();
  _getAttr(attrs, _A11, boardCityOut);

  /* Off city parsing */
  validateInput(attrs.has(_A12), "BFF/A12 - Off City is required");
  LocCode& offCityOut = outSeg->destAirport();
  _getAttr(attrs, _A12, offCityOut);

  /* Carrier parsing */
  validateInput(attrs.has(_B00), "BFF/B00 - Carrier is required");
  CarrierCode& cxrCode = outSeg->carrier();
  _getAttr(attrs, _B00, cxrCode);

  /* Setup first segment */
  Itin* itin;
  dataHandle().get(itin);
  itin->travelSeg().push_back(outSeg);

  attrs.get(_N23, ffinderTrx()->owrt(), ffinderTrx()->owrt());
  int q6t(0);
  attrs.get(_Q6T, q6t);
  ffinderTrx()->bffStep() = static_cast<FlightFinderTrx::BffStep>(q6t);

  /* Availability needed flag parsing */
  if (attrs.has(_NAA))
  {
    if (ffinderTrx()->isBffReq() && ((FlightFinderTrx::STEP_1 == ffinderTrx()->bffStep()) ||
                                     (FlightFinderTrx::STEP_3 == ffinderTrx()->bffStep())))
    {
      char ch(0);
      attrs.get(_NAA, ch);
      ffinderTrx()->avlInS1S3Request() = ('V' == ch);
    }
  }

  /* If RT requested , create second segment */
  AirSeg* inSeg = nullptr;

  if ((attrs.has(_D02) || (!attrs.has(_D01) && !attrs.has(_D02))) ||
      (ffinderTrx()->isBffReq() && ffinderTrx()->isRT()))
  {
    dataHandle().get(inSeg);
    inSeg->departureDT() = DateTime::emptyDate();

    if (attrs.has(_D02))
    {
      const IValueString& d02 = attrs.get(_D02);
      inSeg->departureDT() = parseDateTime(d02);
    }

    inSeg->origAirport() = offCityOut;
    inSeg->destAirport() = boardCityOut;
    inSeg->carrier() = cxrCode;

    // Need to parse DTL type to determine if RT requested
    // when both dates are empty and FF request
    _optInboundSeg = inSeg;

    if (attrs.has(_D02) || (ffinderTrx()->isBffReq() && ffinderTrx()->isRT()))
    {
      _isRoundTripRequested = true;
    }
  } // End of second segment processing

  if (shoppingTrx() != nullptr)
  {
    shoppingTrx()->journeyItin() = itin;
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::BFFTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (idx == _DTL)
  {
    return &parser()._dtlParser;
  }
  else if (idx == _RQF)
  {
    parseRQFType(attrs);
  }
  else if (idx == _PSG)
  {
    parsePSGType(attrs);
  }
  else if (idx == _DRG)
  {
    parseDRGType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::BFFTypeParser::endParsing(const IValueString& text)
{
  if (ffinderTrx()->isBffReq())
  {
    // Setup Bff alt dates, journey itin, and Fare Market
    createBffAltDatesAndJourneyItin();
  }

  AirSeg* outSeg = dynamic_cast<AirSeg*>(shoppingTrx()->journeyItin()->travelSeg().front());

  // Setup journey itin departure date in case of alt dates
  if (trx().isAltDates())
  {
    ShoppingTrx::AltDatePairs::const_iterator altDateItem = trx().altDatePairs().begin();
    if (outSeg->departureDT().isEmptyDate())
    {
      outSeg->departureDT() = altDateItem->first.first;
    }
    if (_optInboundSeg)
    {
      _optInboundSeg->departureDT() = altDateItem->first.second;
    }
  }

  /*Use existing function from DTL parser*/
  TravelSegUtil::setupItinerarySegment(dataHandle(),
                                       outSeg,
                                       outSeg->departureDT(),
                                       outSeg->origAirport(),
                                       outSeg->destAirport(),
                                       outSeg->carrier(),
                                       1 // pnrSegment
                                       );

  FareMarket* journeyFM;
  dataHandle().get(journeyFM);

  ShoppingAltDateUtil::setJrnItinFM(*journeyFM, outSeg);

  shoppingTrx()->journeyItin()->fareMarket().push_back(journeyFM);

  if (_isRoundTripRequested && _optInboundSeg)
  {
    TravelSegUtil::setupItinerarySegment(dataHandle(),
                                         _optInboundSeg,
                                         _optInboundSeg->departureDT(),
                                         _optInboundSeg->origAirport(),
                                         _optInboundSeg->destAirport(),
                                         _optInboundSeg->carrier(),
                                         2 // pnrSegment
                                         );

    // Add second segment to journey Itinerary
    shoppingTrx()->journeyItin()->travelSeg().push_back(_optInboundSeg);

    FareMarket* journeyFM;
    dataHandle().get(journeyFM);

    ShoppingAltDateUtil::setJrnItinFM(*journeyFM, _optInboundSeg);
    shoppingTrx()->journeyItin()->fareMarket().push_back(journeyFM);
  }
  // This flag ensures that FCO get fare the same direction as fare market direction
  shoppingTrx()->journeyItin()->simpleTrip() = true;
}

void
XMLShoppingHandler::BFFTypeParser::parseRQFType(const IAttributes& attrs)
{
  FlightFinderTrx::FareBasisCodeInfo fareInfo;

  validateInput(attrs.has(_B50), "BFF/B50 Fare basis code is required");
  _getAttr(attrs, _B50, fareInfo.fareBasiscode);

  validateInput(attrs.has(_C75), "BFF/C75 Fare Monetary amount required");
  attrs.get(_C75, fareInfo.fareAmount);

  validateInput(attrs.has(_C76), "BFF/C76 Currency code required");
  _getAttr(attrs, _C76, fareInfo.currencyCode);

  if (ffinderTrx() != nullptr)
  {
    ffinderTrx()->fareBasisCodesFF().push_back(fareInfo);
  }
}

void
XMLShoppingHandler::BFFTypeParser::parsePSGType(const IAttributes& attrs)
{
  validateInput(attrs.has(_B70), "BFF/B70 Pax Type code is required");
  PaxTypeCode psgType;
  _getAttr(attrs, _B70, psgType);
  ffinderTrx()->reqPaxType().push_back(psgType);
}

void
XMLShoppingHandler::BFFTypeParser::parseDRGType(const IAttributes& attrs)
{
  // validateInput(attr["D01"].empty() == false,"DRG/D01 Start date is required");
  const IValueString& d01 = attrs.get(_D01);
  ffinderTrx()->departureDT() = parseDateTime(d01);

  validateInput(attrs.has(_Q4V), "DRG/Q4V Number of days is required");
  attrs.get(_Q4V, ffinderTrx()->numDaysFwd());
}

void
XMLShoppingHandler::BFFTypeParser::createBffAltDatesAndJourneyItin()
{
  AirSeg* firstJrnTrvSeg = dynamic_cast<AirSeg*>(ffinderTrx()->journeyItin()->travelSeg().front());

  LocCode boardCityOut = firstJrnTrvSeg->origAirport();
  LocCode offCityOut = firstJrnTrvSeg->destAirport();
  CarrierCode cxrCode = firstJrnTrvSeg->carrier();

  // Generating dates
  if (_bffOutboundDates.empty())
  { // for step_1 this will be empty

    _bffOutboundDates.push_back(firstJrnTrvSeg->departureDT());
  }

  std::vector<DateTime>::const_iterator outboundIter = _bffOutboundDates.begin();

  for (; outboundIter != _bffOutboundDates.end(); ++outboundIter)
  {
    DateTime outboundDate = *outboundIter;
    DateTime startDate = outboundDate;

    if (!ffinderTrx()->departureDT().isEmptyDate())
    { // for step_1

      startDate = ffinderTrx()->departureDT();
    }

    for (uint32_t dayIter = 0; dayIter < ffinderTrx()->numDaysFwd();
         ++dayIter, startDate = startDate.nextDay())
    {
      DatePair datePair;

      if (ffinderTrx()->bffStep() == FlightFinderTrx::STEP_1)
      {
        datePair = DatePair(startDate, DateTime::emptyDate());
      }
      else
      {
        if (outboundDate.get64BitRepDateOnly() > startDate.get64BitRepDateOnly())
        {
          continue;
        }

        datePair = DatePair(outboundDate, startDate);
      }

      PricingTrx::AltDateInfo* altInfo;
      ffinderTrx()->dataHandle().get(altInfo);
      int16_t pnrSegment = 1;

      ffinderTrx()->dataHandle().get(altInfo->journeyItin);

      DatePair*& myPair = altInfo->journeyItin->datePair();
      dataHandle().get(myPair);
      *myPair = datePair;

      // Setup first segment
      ShoppingAltDateUtil::generateJourneySegAndFM(trx().dataHandle(),
                                                   *altInfo->journeyItin,
                                                   datePair.first,
                                                   boardCityOut,
                                                   offCityOut,
                                                   cxrCode,
                                                   pnrSegment);

      if (ffinderTrx()->isRT())
      {
        _isRoundTripRequested = true;

        // Setup second segment
        ShoppingAltDateUtil::generateJourneySegAndFM(trx().dataHandle(),
                                                     *altInfo->journeyItin,
                                                     datePair.second,
                                                     offCityOut,
                                                     boardCityOut,
                                                     cxrCode,
                                                     ++pnrSegment);
      }

      altInfo->journeyItin->setTravelDate(
          TseUtil::getTravelDate(altInfo->journeyItin->travelSeg()));
      altInfo->journeyItin->bookingDate() =
          TseUtil::getBookingDate(altInfo->journeyItin->travelSeg());

      //  Setup alt dates map with new data pair
      trx().altDatePairs()[datePair] = altInfo;

      // For promotional shopping create alternate dates duration map
      if (ffinderTrx()->avlInS1S3Request())
      {
        parser()._dtlParser.setAltDatesDuration(datePair, altInfo);
      }

      // startDate = startDate.nextDay();
    }

  } // end for _bffOutboundDates

  if (!trx().altDatePairs().empty())
  {
    trx().setAltDates(true);
  }
}

void
XMLShoppingHandler::DTLTypeParser::startParsing(const IAttributes& attrs)
{
  if (!attrs.has(_D17))
  {
    return;
  }
  const IValueString& d17 = attrs.get(_D17);
  DateTime outboundDate = parseDateTime(d17);
  _outboundDateList.push_back(outboundDate);
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::DTLTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_IBL == idx)
  {
    parseIBLType(attrs);
  }

  return nullptr;
}

void
XMLShoppingHandler::DTLTypeParser::parseIBLType(const IAttributes& attrs)
{
  if (!attrs.has(_D18))
  {
    return;
  }
  const IValueString& d18 = attrs.get(_D18);
  DateTime inboundDate = parseDateTime(d18);
  _inboundDateList.push_back(inboundDate);
}

void
XMLShoppingHandler::DTLTypeParser::setAltDatesDuration(DatePair& inOutDatePair,
                                                       PricingTrx::AltDateInfo* altInfo)
{
  std::pair<DatePair, PricingTrx::AltDateInfo*> altDatePairElem(inOutDatePair, altInfo);

  uint64_t duration =
      (inOutDatePair.second.get64BitRepDateOnly()) - (inOutDatePair.first.get64BitRepDateOnly());

  PricingTrx::DurationAltDatePairs::iterator foundElemIt =
      trx().durationAltDatePairs().find(duration);

  if (foundElemIt != trx().durationAltDatePairs().end())
  {
    foundElemIt->second.insert(altDatePairElem);
  }
  else
  {
    PricingTrx::AltDatePairs datePairsMap;
    datePairsMap.insert(altDatePairElem);
    trx().durationAltDatePairs()[duration] = datePairsMap;
  }
}

/*
 *This function contains the logic to setup alt dates journey itin for Flight Finder
 **/
void
XMLShoppingHandler::DTLTypeParser::endParsing(const IValueString& text)
{
  if (_outboundDateList.empty())
  {
    return;
  }
  // For Bff step_3
  // alt Date journey itins are created once BFF element
  // is fully parsed (BffTypeParser::endParsing method)
  // since need to know all outbound dates and date range for inbound dates

  else if (ffinderTrx()->isBffReq() && ffinderTrx()->bffStep() == FlightFinderTrx::STEP_3)
  {
    parser()._bffParser._bffOutboundDates.push_back(_outboundDateList.back());
    return;
  }

  DateTime outboundDate = _outboundDateList.back();

  AirSeg* firstJrnTrvSeg = dynamic_cast<AirSeg*>(shoppingTrx()->journeyItin()->travelSeg().front());

  LocCode boardCityOut = firstJrnTrvSeg->origAirport();
  LocCode offCityOut = firstJrnTrvSeg->destAirport();
  CarrierCode cxrCode = firstJrnTrvSeg->carrier();

  if ((ffinderTrx()->isFFReq() && _inboundDateList.empty()) ||
      (ffinderTrx()->isBffReq() && ffinderTrx()->isOW()))
  {
    DateTime inboundDate(DateTime::emptyDate());
    int16_t pnrSegment = 1;

    DatePair inOutDatePair(outboundDate, inboundDate);
    PricingTrx::AltDateInfo* altInfo;
    dataHandle().get(altInfo);

    dataHandle().get(altInfo->journeyItin);

    DatePair*& myPair = altInfo->journeyItin->datePair();
    dataHandle().get(myPair);
    *myPair = inOutDatePair;

    /* Setup first segment */
    ShoppingAltDateUtil::generateJourneySegAndFM(dataHandle(),
                                                 *altInfo->journeyItin,
                                                 inOutDatePair.first,
                                                 boardCityOut,
                                                 offCityOut,
                                                 cxrCode,
                                                 pnrSegment);

    altInfo->journeyItin->setTravelDate(TseUtil::getTravelDate(altInfo->journeyItin->travelSeg()));

    altInfo->journeyItin->bookingDate() =
        TseUtil::getBookingDate(altInfo->journeyItin->travelSeg());

    /*  Setup alt dates map with new data pair */
    trx().altDatePairs()[inOutDatePair] = altInfo;

    setAltDatesDuration(inOutDatePair, altInfo);
  }
  else
  {
    // Need this flag to add second segment to main journeyItin
    parser()._bffParser._isRoundTripRequested = true;

    // Since bff request STEP_2 doesn't contain
    // inbound dates add empty one to create journey segs
    if (ffinderTrx()->isBffReq() && ffinderTrx()->bffStep() == FlightFinderTrx::STEP_2)
    {
      _inboundDateList.push_back(DateTime::emptyDate());
    }

    std::vector<DateTime>::const_iterator inbIter = _inboundDateList.begin();
    for (; inbIter != _inboundDateList.end(); ++inbIter)
    {
      DateTime inboundDate = *inbIter;

      DatePair inOutDatePair(outboundDate, inboundDate);
      int16_t pnrSegment = 1;

      PricingTrx::AltDateInfo* altInfo;
      dataHandle().get(altInfo);

      dataHandle().get(altInfo->journeyItin);

      DatePair*& myPair = altInfo->journeyItin->datePair();
      dataHandle().get(myPair);
      *myPair = inOutDatePair;

      /* Setup first segment */
      ShoppingAltDateUtil::generateJourneySegAndFM(dataHandle(),
                                                   *altInfo->journeyItin,
                                                   inOutDatePair.first,
                                                   boardCityOut,
                                                   offCityOut,
                                                   cxrCode,
                                                   pnrSegment);

      /* Setup second segment */
      pnrSegment++;

      ShoppingAltDateUtil::generateJourneySegAndFM(dataHandle(),
                                                   *altInfo->journeyItin,
                                                   inOutDatePair.second,
                                                   offCityOut,
                                                   boardCityOut,
                                                   cxrCode,
                                                   pnrSegment);

      altInfo->journeyItin->setTravelDate(
          TseUtil::getTravelDate(altInfo->journeyItin->travelSeg()));

      altInfo->journeyItin->bookingDate() =
          TseUtil::getBookingDate(altInfo->journeyItin->travelSeg());

      /*  Setup alt dates map with new data pair */
      trx().altDatePairs()[inOutDatePair] = altInfo;

      setAltDatesDuration(inOutDatePair, altInfo);

    } // END_OF_INB_ITER
  } // END_ELSE

  if (trx().altDatePairs().empty() == false)
  {
    trx().setAltDates(true);
  }

  /* Clear list for next parsing */
  _outboundDateList.clear();
  _inboundDateList.clear();
}

void
XMLShoppingHandler::DIATypeParser::startParsing(const IAttributes& attrs)
{
  int diag(-1);
  attrs.get(_Q0A, diag);

  if (diag == 927)
    diag = 187;

  trx().getRequest()->diagnosticNumber() = static_cast<int>(diag);

  trx().diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diag);
  trx().diagnostic().activate();

  char pas(0);
  attrs.get(_PAS, pas);
  if (pas == 'T')
    trx().diagnostic().processAllServices() = true;
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::DIATypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_ARG == idx)
  {
    parseARGType(attrs);
  }

  return nullptr;
}

void
XMLShoppingHandler::DIATypeParser::parseARGType(const IAttributes& attrs)
{
  const IValueString& nam = attrs.get(_NAM), &val = attrs.get(_VAL);
  std::string namStr(nam.c_str(), nam.length()), valStr(val.c_str(), val.length());
  trx().diagnostic().diagParamMap().insert(std::make_pair(namStr, valStr));
}

void
XMLShoppingHandler::DIATypeParser::endParsing(const IValueString&)
{
  if (trx().diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::TOPLINE_METRICS))
  {
    trx().recordTopLevelMetricsOnly() = true;
    trx().recordMetrics() = true;
  }

  if (trx().diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::FULL_METRICS))
  {
    trx().recordTopLevelMetricsOnly() = false;
    trx().recordMetrics() = true;
  }
}

void
XMLShoppingHandler::ShoppingRequestParser::rejectUnsupportedRequest(char requestType) const
{
  // N06 should be set by default to "I", however when a shopping request
  // doesn't set explicitely N06 we get it empty.
  if (0 == requestType)
    requestType = 'I';

  if (!allowedRequestTypes.getValue().has(requestType))
  {
    std::ostringstream msg{"Incorrect Request Type for ", std::ios_base::ate};
    msg << serverType() << " Server: N06 = " << requestType << " is unsupported.";
    validateInput(false, msg.str().c_str(), true);
  }
}

void
XMLShoppingHandler::ShoppingRequestParser::startParsing(const IAttributes& attrs)
{
  char requestType(0);
  attrs.get(_N06, requestType);

  if (!fallback::fixed::failIfIncorrectShoppingRequestType())
  {
    rejectUnsupportedRequest(requestType);
  }

  switch (requestType)
  {
  case 'E':
    validateInput(esvLogicCfg.getValue(), "ESV LOGIC OFF", true);
    parser().createShoppingTrx();
    trx().setTrxType(PricingTrx::ESV_TRX);
    break;

  case 'M':
  case 'P':
    if (attrs.has(_N0W))
    {
      char type = attrs.get<char>(_N0W, 0);

      if (type == 'R')
      {
        parser().createRexExchangeTrx();
      }
      else
      {
        parser().createPricingTrx();
      }
    }
    else
    {
      parser().createPricingTrx();
    }

    if (requestType == 'M')
    {
      trx().setTrxType(PricingTrx::MIP_TRX);
    }
    break;

  case 'C':
  {
    parser().createSettlementTypesTrx();
    trx().setTrxType(PricingTrx::MIP_TRX);
    parser().markSettlementTypesRequest(*(trx().getRequest()));

    // Billing is not necessary for MIP_Brand request
    // and is not included in the request at all
    // but it's not safe to leave it uninitialized
    Billing*& billing = trx().billing();
    if (billing == nullptr)
      dataHandle().get(billing);
  }
  break;
  case 'N':
  {
    // MIP request from Intelisell for cache update
    // asking for all valid branded fares for supplied itins
    parser().createBrandingTrx();
    trx().setTrxType(PricingTrx::MIP_TRX);
    if (fallback::fallbackMipBForTNShopping(&trx()))
    {
      // It is assumed that N06='N' request is also an IBF request
      parser().markBrandedFaresRequest(*(trx().getRequest()));
    }
    // Billing is not necessary for MIP_Brand request
    // and is not included in the request at all
    // but it's not safe to leave it uninitialized
    Billing*& billing = trx().billing();
    if (billing == nullptr)
    {
      dataHandle().get(billing);
    }
    break;
  }

  case 'F':
    validateInput(ffLogicCfg.getValue(), "FF LOGIC OFF");
    parser().createFFinderTrx();
    trx().setTrxType(PricingTrx::FF_TRX);
    break;

  case 'B':
    validateInput(bffLogicCfg.getValue(), "BFF LOGIC OFF");
    parser().createFFinderTrx();
    trx().setTrxType(PricingTrx::FF_TRX);
    break;

  case 'R':
    validateInput(exchangeLogicCfg.getValue(), "EXCHANGE LOGIC OFF");
    parser().createRexShoppingTrx();
    trx().setTrxType(PricingTrx::RESHOP_TRX);
    break;

  default:
    parser().createShoppingTrx();
    trx().setTrxType(PricingTrx::IS_TRX);
    break;
  }

  parser().readShoppingConfigParameters(trx());

  if (attrs.has(_TST))
  {
    char ch(0);
    attrs.get(_TST, ch);
    trx().isTestRequest() = ('Y' == ch || 'T' == ch);
  }

  if (attrs.has(_NTO))
  {
    char ch(0);
    attrs.get(_NTO, ch);
    trx().setForceNoTimeout('Y' == ch || 'T' == ch);
  }

  int timeout = 0;
  if (attrs.has(_D70))
  {
    attrs.get(_D70, timeout);
  }

  else
  {
    timeout = timeoutCfg.getValue();
  }

  if (timeout > 0)
  {
    TrxAborter* const aborter = &dataHandle().safe_create<TrxAborter>(&trx());
    trx().aborter() = aborter;
    ShoppingUtil::setTimeout(trx(), timeout);
  }

  if (attrs.has(_N25))
  {
    Indicator rexPrimaryProcessType;
    attrs.get(_N25, rexPrimaryProcessType, rexPrimaryProcessType);
    if (trx().getTrxType() == PricingTrx::MIP_TRX && attrs.has(_N0W))
    {
      BaseExchangeTrx* baseTrx = static_cast<BaseExchangeTrx*>(rexExchangeTrx());
      if (baseTrx)
        baseTrx->setRexPrimaryProcessType(rexPrimaryProcessType);
    }
    else if (trx().getTrxType() == PricingTrx::RESHOP_TRX)
    {
      BaseExchangeTrx* baseTrx = static_cast<BaseExchangeTrx*>(rexShoppingTrx());
      if (baseTrx)
        baseTrx->setRexPrimaryProcessType(rexPrimaryProcessType);
    }
  }

  if (attrs.has(_STK))
  {
    attrs.get(_STK, trx().token());
  }
  if (attrs.has(_BSL))
  {
    char bsl('F');
    attrs.get(_BSL, bsl);
    if (bsl == 'T')
      trx().getRequest()->setDisplayBaseline(true);
  }

  if (shoppingTrx() == nullptr)
  {
    return;
  }

  if (attrs.has(_AS0))
  {
    char noASOLegs(0);
    attrs.get(_AS0, noASOLegs);
    switch (noASOLegs)
    {
    case 'T':
    case 't':
      shoppingTrx()->noASOLegs() = true;
      break;
    case 'Y':
    case 'y':
      shoppingTrx()->noASOLegs() = true;
      break;
    case '1':
      shoppingTrx()->noASOLegs() = true;
      break;
    default:
      shoppingTrx()->noASOLegs() = false;
    }
  }

  if (attrs.has(_AS1))
  {
    attrs.get(_AS1, shoppingTrx()->asoBitmapThreshold());
    validateInput(shoppingTrx()->asoBitmapThreshold() > 0,
                  "If specifying ASO bitmap size threshold, it must be greater than zero");
  }

  if (attrs.has(_AS2))
  {
    attrs.get(_AS2, shoppingTrx()->asoConxnPointLimit());
    validateInput(shoppingTrx()->asoConxnPointLimit() > 0,
                  "If specifying ASO connection point limit, it must be greater than zero");
  }

  if (attrs.has(_AS3))
  {
    attrs.get(_AS3, shoppingTrx()->asoMaxValidations());
  }

  if (trx().getTrxType() == PricingTrx::IS_TRX && attrs.has(_N0W))

  {
    char reqType = char(0);
    attrs.get(_N0W, reqType);
    if (reqType == 'R')
    {
      trx().setExcTrxType(PricingTrx::EXC_IS_TRX);
    }
  }
}

void
XMLShoppingHandler::ShoppingRequestParser::saveCountryAndStateRegionCodes(const PricingTrx& trx,
                                                                          const IAttributes& attrs,
                                                                          NationCode& employment,
                                                                          NationCode& nationality,
                                                                          LocCode& residency)
{
  _getAttr(attrs, _A50, residency);

  const char n0m = attrs.get<char>(_N0M);
  const auto codes = {attrs.get<std::string>(_A40), attrs.get<std::string>(_AH0)};

  for (const std::string& code : codes)
  {
    if (code.empty())
      continue;

    switch (n0m)
    {
    case 'E':
      employment += code;
      break;
    case 'N':
      nationality = code;
      break;
    case 'R':
      residency += code;
      break;
    default:
      break;
    }
  }
}

namespace
{
bool
paxTypeCompare(const PaxType* p1, const PaxType* p2)
{
  const bool adult1 = PaxTypeUtil::isAdult(p1->paxType(), p1->vendorCode());
  const bool adult2 = PaxTypeUtil::isAdult(p2->paxType(), p2->vendorCode());

  if (adult1 && !adult2)
  {
    return true;
  }
  else if (adult2)
  {
    return false;
  }

  // neither pax type is an adult
  const bool child1 = PaxTypeUtil::isChild(p1->paxType(), p1->vendorCode());
  const bool child2 = PaxTypeUtil::isChild(p2->paxType(), p2->vendorCode());

  // if the first pax type is a child and the other is an infant, then
  // the child comes first.

  return child1 && !child2;
}

// function which merges availability for IS.
// Local availability also includes thru availability in IS.

void
mergeAvailability(PricingTrx& trx, Itin* itin)
{
  AvailabilityMap::const_iterator avail =
      trx.availabilityMap().find(ShoppingUtil::buildAvlKey(itin->travelSeg()));

  if (UNLIKELY(avail == trx.availabilityMap().end()))
  {
    return;
  }

  std::vector<TravelSeg*> oldTravelSeg = itin->travelSeg();

  const std::vector<std::vector<ClassOfService*>>* cos = avail->second;
  TSE_ASSERT(cos->size() == itin->travelSeg().size());

  for (size_t n = 0; n != cos->size(); ++n)
  {
    // create a new, duplicate air segment, with modified
    // class of service information specific to this SOP.
    AirSeg* oldAirSeg = dynamic_cast<AirSeg*>(itin->travelSeg()[n]);

    if (LIKELY(oldAirSeg))
    {
      std::vector<ClassOfService*> mergedCos = oldAirSeg->classOfService();

      if (mergeClassOfService(mergedCos, cos->at(n)))
      {
        AirSeg* newAirSeg = trx.dataHandle().create<AirSeg>();
        *newAirSeg = *oldAirSeg;
        newAirSeg->classOfService() = mergedCos;
        itin->travelSeg()[n] = newAirSeg;
      }
    }
  }
}

void
mergeAvailabilityForCalendar(PricingTrx& trx,
                             Itin* itin,
                             AvailabilityMap& availabilityMap)
{
  AvailabilityMap::const_iterator availIter =
      availabilityMap.find(ShoppingUtil::buildAvlKey(itin->travelSeg()));
  if (availIter == availabilityMap.end())
    return;

  const std::vector<std::vector<ClassOfService*>>* cos = availIter->second;
  TSE_ASSERT(cos->size() == itin->travelSeg().size());

  for (size_t segmentIndex = 0; segmentIndex != cos->size(); ++segmentIndex)
  {
    AirSeg* airSegment = dynamic_cast<AirSeg*>(itin->travelSeg()[segmentIndex]);
    if (airSegment)
    {
      std::vector<ClassOfService*> mergedCos = airSegment->classOfService();
      if (mergeClassOfService(mergedCos, (*cos)[segmentIndex]))
      {
        std::vector<TravelSeg*> key;
        key.push_back(airSegment);

        AvailabilityMap::iterator availIter =
            availabilityMap.find(ShoppingUtil::buildAvlKey(key));
        if (availIter != availabilityMap.end())
        {
          std::vector<ClassOfServiceList> cosListVec;
          cosListVec.push_back(mergedCos);

          *(availIter->second) = cosListVec;
        }
      }
    }
  }
}
}

void
XMLShoppingHandler::ShoppingRequestParser::endParsing(const IValueString& text)
{
  if (PricingTrx::ESV_TRX == trx().getTrxType())
  {
    return;
  }

  if ((trx().diagnostic().diagnosticType() == Diagnostic187) &&
      (trx().diagnostic().diagParamMapItem("DD") == "ISPARSE"))
  {
    printTravelSegsCOS();
  }

  if ((PricingTrx::MIP_TRX == trx().getTrxType()) && trx().getRequest()->isCollectOCFees() &&
      trx().getOptions()->isSummaryRequest())
  {
    if (trx().getOptions()->serviceGroupsVec().empty())
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "AIR EXTRAS SUMMARY REQUEST REQUIRES AT LEAST ONE GROUP CODE");
    }
  }

  if ((PricingTrx::MIP_TRX == trx().getTrxType()) && (trx().getRequest()->isCollectOCFees()) &&
      (false == trx().getOptions()->isSummaryRequest()))
  {
    bool aeRequested = false;

    std::vector<RequestedOcFeeGroup>::iterator groupsIter =
                                                   trx().getOptions()->serviceGroupsVec().begin(),
                                               groupsIterEnd =
                                                   trx().getOptions()->serviceGroupsVec().end();

    for (; groupsIter != groupsIterEnd; ++groupsIter)
    {
      if (groupsIter->groupCode() == "AE")
      {
        aeRequested = true;
        break;
      }
    }

    if (aeRequested)
    {
      trx().getOptions()->isProcessAllGroups() = true;
      trx().getOptions()->serviceGroupsVec().clear();
    }
  }

  if (PricingTrx::FF_TRX == trx().getTrxType())
  {
    // Prevent from processing of flights for step 1/3
    if (ffinderTrx()->isBffReq() && !ffinderTrx()->avlInS1S3Request())
    {
      if ((FlightFinderTrx::STEP_1 == ffinderTrx()->bffStep()) ||
          (FlightFinderTrx::STEP_3 == ffinderTrx()->bffStep()))
      {
        // Check if flight components exist in request
        validateInput(ffinderTrx()->travelSeg().empty() ||
                          ffinderTrx()->availabilityMap().empty() || ffinderTrx()->legs().empty(),
                      "BFF Step 1/3 request with NAA not equal to V "
                      "should not contain any AAF, AVL or LEG elements");
      }
    }
  }

  // Now we know if the request is alternate date or calendar shopping
  // So we can set specific hurry time out for these kind of requests
  if ((trx().getTrxType() == PricingTrx::IS_TRX) && (trx().isAltDates()))
  {
    int hurryAt = 0;
    int hurryPercentDefault = 75;
    TrxAborter* aborter = trx().aborter();
    int hurry = hurryResponse.getValue();
    if (!hurryResponse.isDefault())
    {
      if (aborter != nullptr)
      {
        try
        {
          hurryAt = ((aborter->getTimeOutAt() - time(nullptr)) * hurry) / 100;
        }
        catch (boost::bad_lexical_cast& exception)
        {
          hurryAt = ((aborter->getTimeOutAt() - time(nullptr)) * hurryPercentDefault) / 100;
        }
      }
    }
    else
    {
      hurryAt = ((aborter->getTimeOutAt() - time(nullptr)) * hurryPercentDefault) / 100;
      CONFIG_MAN_LOG_KEY_ERROR(
          _logger, "HURRY_RESPONSE_THRESHOLD_ALTDATE_PERCENTAGE", "SHOPPING_OPT");
    }
    if (hurryAt < 1)
    {
      hurryAt = 1;
    }
    aborter->setHurry(hurryAt);
  }

  // we must sort trx().paxType(), but we may also keep
  // PosPaxTypePerPax in sync as it is a 'parallel vector'
  std::map<const PaxType*, size_t> oldPaxTypePos;
  for (size_t n = 0; n != trx().paxType().size(); ++n)
  {
    oldPaxTypePos[trx().paxType()[n]] = n;
  }
  std::sort(trx().paxType().begin(), trx().paxType().end(), paxTypeCompare);

  std::vector<PosPaxTypePerPax> newPosPaxType;
  newPosPaxType.resize(trx().paxType().size());
  for (size_t n = 0; n != trx().paxType().size(); ++n)
  {
    const size_t index = oldPaxTypePos[trx().paxType()[n]];
    newPosPaxType[n].swap(trx().posPaxType()[index]);
  }

  trx().posPaxType().swap(newPosPaxType);

  if (trx().getTrxType() == PricingTrx::MIP_TRX && trx().calendarSoldOut())
  {
    AvailabilityMap newAvailMap = trx().availabilityMap();

    std::vector<Itin*>::iterator itinIt = trx().itin().begin();
    std::vector<Itin*>::iterator itinItEnd = trx().itin().end();
    for (; itinIt != itinItEnd; ++itinIt)
    {
      mergeAvailabilityForCalendar(trx(), *itinIt, newAvailMap);
    }

    trx().availabilityMap().swap(newAvailMap);
  }

  // In IS processing, we merge thru availability and local availability into
  // local availability. We don't do this for MIP, FF, or if we are running
  // diag 902 with DD=AVAILABILITY
  if ((shoppingTrx() != nullptr) && (trx().getTrxType() != PricingTrx::FF_TRX) &&
      ((trx().diagnostic().diagnosticType() != Diagnostic902) ||
       (trx().diagnostic().diagParamMapItem("DD") != "AVAILABILITY")))
  {
    std::vector<ShoppingTrx::Leg>::iterator legIter(shoppingTrx()->legs().begin());

    for (; legIter != shoppingTrx()->legs().end(); ++legIter)
    {
      std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter(legIter->sop().begin());

      for (; sopIter != legIter->sop().end(); ++sopIter)
      {
        mergeAvailability(trx(), sopIter->itin());
        legIter->setSopTravelSegs(*sopIter);
      }
    }
  }

  if (trx().getTrxType() == PricingTrx::FF_TRX)
  {
    if (trx().diagnostic().diagParamMapItemPresent("AR") && !trx().isAltDates())
    {
      ffinderTrx()->ignoreDiagReq() = true;
    }
  }

  if (trx().awardRequest())
  {
    AwardPsgTypeChecker awardPsgChecker;
    awardPsgChecker.checkPsgType(trx());
  }

  if (trx().getRequest()->owPricingRTTaxProcess())
  {
    if (trx().inboundDepartureDate() != DateTime::emptyDate())
      trx().getRequest()->processingDirection() = ProcessingDirection::ROUNDTRIP_OUTBOUND;
    else if (trx().outboundDepartureDate() != DateTime::emptyDate())
      trx().getRequest()->processingDirection() = ProcessingDirection::ROUNDTRIP_INBOUND;
    else if (legs().size() == 2)
      trx().getRequest()->processingDirection() = ProcessingDirection::ROUNDTRIP;
    else
      trx().getRequest()->processingDirection() = ProcessingDirection::ONEWAY;
  }

  bool sumOfLocalsRequest = false;

  if (shoppingTrx())
  {
    if (shoppingTrx()->legs().size() == 2 && !ShoppingUtil::isOpenJaw(*shoppingTrx()))
    {
      shoppingTrx()->setSimpleTrip(true);
    }
    else if (shoppingTrx()->legs().size() == 1)
    {
      shoppingTrx()->setSimpleTrip(true);
    }

    sumOfLocalsRequest = shoppingTrx()->isSumOfLocalsProcessingEnabled();
  }

  if (sumOfLocalsRequest)
  {
    // In case the numberOfSolutions was overwritten by estimated value from config
    // START_ESTIMATES_AT, ACCURATE_SOLUTIONS or START_ESTIMATES_AT_INTL, ACCURATE_SOLUTIONS_INTL
    trx().getOptions()->setRequestedNumberOfSolutions(parser().getNumberOfSolutions());
  }

  // we're checking if it's possible to produce requested number of custom solutions
  // very straightforward check - number of possible permutations of sops
  if (shoppingTrx() && shoppingTrx()->getNumOfCustomSolutions())
  {
    uint32_t numOfPossibleCustom = 1;
    bool atLeastOneCustomLeg = false;
    std::vector<ShoppingTrx::Leg>::const_iterator itLeg = legs().begin();
    for (; itLeg != legs().end(); ++itLeg)
    {
      uint32_t numOfSops = 0;
      if (itLeg->isCustomLeg())
      {
        atLeastOneCustomLeg = true;
        std::vector<ShoppingTrx::SchedulingOption>::const_iterator itSop = itLeg->sop().begin();
        for (; itSop != itLeg->sop().end(); ++itSop)
        {
          if (itSop->isCustomSop())
            ++numOfSops;
        }
      }
      else
      {
        numOfSops = itLeg->sop().size();
      }
      numOfPossibleCustom *= numOfSops;
    }
    if (atLeastOneCustomLeg)
    {
      if (static_cast<uint32_t>(shoppingTrx()->getNumOfCustomSolutions()) > numOfPossibleCustom)
      {
        shoppingTrx()->setNumOfCustomSolutions(numOfPossibleCustom);
      }
    }
    else
    {
      shoppingTrx()->setNumOfCustomSolutions(0);
    }
  }

  if (smp::isPenaltyCalculationRequired(trx()))
  {
    smp::validatePenaltyInputInformation(trx());
  }

  if (trx().isExchangeTrx())
  {
    if (trx().getRequest()->isContextShoppingRequest())
    {
      if (!ExchangeUtil::validateFixedLegsInCEXS(trx(), legs()))
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "Shopped/Fixed part of the journey must be continuous");
      }
    }

    const RexBaseRequest* rexRequest = dynamic_cast<RexBaseRequest*>(trx().getRequest());
    if (rexRequest && !fallback::azPlusUpExc(&trx()) && trx().isMip() &&
        (rexRequest->excDiscounts().isPAEntry() || rexRequest->excDiscounts().isPPEntry()))
    {
      trx().getRequest()->discountsNew().clearAmountDiscounts();
      trx().getRequest()->discountsNew().clearPercentageDiscounts();
    }
  }

  //Update trx().isFlexFare() flag based on Flex Fare Groups
  if (!fallback::fallbackPFFAffectsMainFare(&trx()) && trx().isFlexFare() &&
      trx().getRequest()->getFlexFaresGroupsData().getSize() == 1 && _defaultFlexFaresGroupId == 0)
    trx().setFlexFare(false);
}

void
XMLShoppingHandler::ShoppingRequestParser::printTravelSegsCOS()
{
  DiagManager diag187(trx(), Diagnostic187);
  std::stringstream diag187Str;
  diag187Str.setf(std::ios::left, std::ios::adjustfield);

  diag187Str << "\nSHOPPING TRAVEL SEGMENTS DETAILS: BKCODE-CABIN\n";
  diag187Str << "==================================================\n";
  diag187Str << "1 = FIRST_CLASS_PREMIUM\n";
  diag187Str << "2 = FIRST_CLASS\n";
  diag187Str << "4 = BUSINESS_CLASS_PREMIUM\n";
  diag187Str << "5 = BUSINESS_CLASS\n";
  diag187Str << "7 = ECONOMY_CLASS_PREMIUM\n";
  diag187Str << "8 = ECONOMY_CLASS\n";
  diag187Str << "9 = INVALID_CLASS\n";
  diag187Str << "- = UNKNOWN_CLASS\n\n";

  diag187Str << "TOTAL TRAVEL SEGMENTS: " << trx().travelSeg().size() << "\n";
  for (const TravelSeg* ts : trx().travelSeg())
  {
    diag187Str << std::setw(4) << ts->originalId() << "[" << ts->origin()->loc()
               << ts->destination()->loc() << "] ";

    if (ts->isAir())
    {
      const AirSeg* as = ts->toAirSeg();

      diag187Str << as->carrier() << std::setw(4) << as->flightNumber() << " ";
    }

    for (const ClassOfService* cos : ts->classOfService())
    {
      diag187Str << cos->bookingCode() << cos->cabin();
    }
    diag187Str << "\n";
  }
  diag187 << diag187Str.str();
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::ShoppingRequestParser::newElement(int idx, const IAttributes& attrs)
{
  if (_AGI == idx)
  {
    parseAGIType(attrs);
  }
  else if (_DUR == idx)
  {
    parseDURType(attrs);
  }
  else if (_PDT == idx)
  {
    parsePDTType(attrs);
  }
  else if (_BIL == idx)
  {
    parseBILType(attrs);
  }
  else if (UNLIKELY(_ODD == idx))
  {
    parseODDType(attrs);
  }
  else if (_PXI == idx)
  {
    parser().parsingAccompanyPassenger() = false;
    return &parser()._pxiParser;
  }
  else if (UNLIKELY(_APX == idx))
  {
    parser().parsingAccompanyPassenger() = true;
    return &parser()._pxiParser;
  }
  else if (_SPV == idx)
  {
    return &parser()._spvParser;
  }
  else if (_PRO == idx)
  {
    return &parser()._proParser;
  }
  else if (_BRI == idx)
  {
    if (trx().getTrxType() == PricingTrx::IS_TRX)
    {
      return &parser()._briParser;
    }
  }
  else if (UNLIKELY(_BRN == idx))
  {
    return &parser()._brnParser;
  }
  else if (_AAF == idx)
  {
    return &parser()._aafParser;
  }
  else if (_AVL == idx)
  {
    return &parser()._avlParser;
  }
  else if (_LEG == idx)
  {
    return &parser()._legParser;
  }
  else if (_OND == idx)
  {
    parseONDType(attrs);
  }
  else if (UNLIKELY(_FFG == idx))
  {
    if (!fallback::fallbackFFGMaxPenaltyNewLogic(&trx()))
      return &parser()._ffgParser;
    else
      parseFFGType(attrs);
  }
  else if (_ITN == idx)
  {
    return &parser()._itnParser;
  }
  else if (_DIA == idx)
  {
    return &parser()._diaParser;
  }
  else if (_CMD == idx)
  {
    return &parser()._cmdParser;
  }
  else if (_BFF == idx)
  {
    return &parser()._bffParser;
  }
  else if (_FCL == idx)
  {
    return &parser()._fclParser;
  }
  else if (_EXC == idx)
  {
    return &parser()._excParser;
  }
  else if (_FFY == idx)
  {
    parseFFYType(attrs);
  }
  else if (_RFG == idx)
  {
    parseRFGType(attrs);
  }
  else if (_BRS == idx)
  {
    return &parser()._brsParser;
  }
  else if (_DIV == idx)
  {
    return &parser()._divParser;
  }
  else if (_XRA == idx)
  {
    if (xray::xrayEnabledCfg.isValid(&trx()))
      parseXRAType(attrs);
    else
      LOG4CXX_WARN(_logger, "Xray tracking is disabled");
  }
  else if (_DynamicConfig == idx)
  {
    parseDynamicConfigType(attrs);
  }
  return nullptr;
}

void
XMLShoppingHandler::ShoppingRequestParser::parseAGIType(const IAttributes& attrs)
{
  Agent* agent;
  if (parser().parsingExchangeInformation())
    agent = dynamic_cast<RexPricingRequest*>(rexPricingTrx()->getRequest())->prevTicketIssueAgent();
  else
    agent = trx().getRequest()->ticketingAgent();

  if (agent == nullptr)
  {
    LOG4CXX_ERROR(_logger, "XMLShoppingHandler::agent is NULL");
    return;
  }

  _getAttr(attrs, _A10, agent->agentCity());
  _getAttr(attrs, _A20, agent->tvlAgencyPCC());
  _getAttr(attrs, _A21, agent->mainTvlAgencyPCC());
  _getAttr(attrs, _AB0, agent->tvlAgencyIATA());
  _getAttr(attrs, _AB1, agent->homeAgencyIATA());
  _getAttr(attrs, _AE1, agent->officeDesignator());
  _getAttr(attrs, _AE2, agent->officeStationCode());
  _getAttr(attrs, _AE3, agent->defaultTicketingCarrier());
  _getAttr(attrs, _AE4, agent->airlineChannelCode());
  if (trx().diagnostic().diagnosticType() == Diagnostic872)
  {
    DiagManager diag872(trx(), Diagnostic872);
    diag872 << "AIRLINE CHANNEL CODE: " << agent->airlineChannelCode() << "\n";
  }

  _getAttr(attrs, _AB2, agent->airlineIATA());
  _getAttr(attrs, _A90, agent->agentFunctions());
  _getAttr(attrs, _A80, agent->airlineDept());
  _getAttr(attrs, _N0G, agent->agentDuty());
  _getAttr(attrs, _B00, agent->cxrCode());
  _getAttr(attrs, _C40, agent->currencyCodeAgent());
  attrs.get(_Q01, agent->coHostID(), agent->coHostID());

  _getAttr(attrs, _AE0, agent->vendorCrsCode());

  ReservationData& reservData = *trx().getRequest()->reservationData();

  if (parser().parsingExchangeInformation())
  {
    _getAttr(attrs, _N0L, agent->agentCommissionType());
    attrs.get(_C6C, agent->agentCommissionAmount());
  }
  else
  {
    _getAttr(attrs, _A40, reservData.agentNation());
  }

  // do not parse PAV, follow pricing by using AE0 to identify Abacus

  //
  // Per marketing requirement the Customer Record can ONLY use agent city and tvlAgencyPCC
  // Home Psuedo City(Z360) is not Valid for branch(PCC-2N72)
  //
  // Change to use multiTranport table after the conversatioreservData.agentNation()n
  // with Vladi, Darrin and Gary on 9-27 due to PL 10328
  //
  // For PCC-TP4A located near EAP airport that serves two nations
  // the POS needs to be correct nation
  //

  if (parser().parsingExchangeInformation() == false && rexPricingTrx() &&
      rexPricingTrx()->excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    rexPricingTrx()->setUpSkipSecurityForExcItin();
  }

  LocCode agentCity;

  const Loc* agentLocation = dataHandle().getLoc(agent->agentCity(), time(nullptr));

  std::vector<Customer*> custList = DataHandle().getCustomer(agent->tvlAgencyPCC());

  if (custList.empty())
  {
    if (isAirLineRequest(agent))
    {
      std::stringstream msg;
      msg << "Customer record: '" << agent->tvlAgencyPCC() << "' AGENT_PCC_NON_EXISTENT!";
      LOG4CXX_ERROR(_logger, msg.str());
    }
    agent->agentLocation() = agentLocation;
    if (!agent->agentLocation())
      throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);

    if (agent->currencyCodeAgent().empty())
    {
      DateTime today = DateTime::localTime();
      NationCode nationCode;
      // get the currency for the nation
      if (reservData.agentNation().empty() == false)
      {
        nationCode = reservData.agentNation();
      }
      else if (agent->agentLocation()->nation().empty() == false)
      {
        nationCode = agent->agentLocation()->nation();
      }

      const Nation* nation = dataHandle().getNation(nationCode, today);
      if ((nation == nullptr) || (nation->primeCur().empty()))
      {
        throw ErrorResponseException(ErrorResponseException::AGENT_CURRENCY_CODE_MISSING);
      }
      else
      {
        bool rc = false;
        CurrencyCode nationCurrency;
        rc = CurrencyUtil::getNationCurrency(nationCode, nationCurrency, today);
        if (rc)
        {
          agent->currencyCodeAgent() = nationCurrency;
        }
      }
    }
    return;
  }

  agent->agentTJR() = custList.front();

  agentCity = dataHandle().getMultiTransportCity(agent->agentCity());
  agent->agentLocation() = dataHandle().getLoc(agentCity, time(nullptr));

  if (agent->agentLocation() && agentLocation)
  {
    if (agent->agentLocation()->nation() != agentLocation->nation())
      agent->agentLocation() = agentLocation;
  }
  if (!agent->agentLocation())
    agent->agentLocation() = agentLocation;
  if (agent->currencyCodeAgent().empty())
  {
    agent->currencyCodeAgent() = custList.front()->defaultCur();
  }
  if (!agent->agentLocation())
  {
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
  }
  if (agent->currencyCodeAgent().empty())
  {
    DateTime today = DateTime::localTime();
    NationCode nationCode;
    // get the currency for the nation
    if (reservData.agentNation().empty() == false)
    {
      nationCode = reservData.agentNation();
    }
    else if (agent->agentLocation()->nation().empty() == false)
    {
      nationCode = agent->agentLocation()->nation();
    }
    const Nation* nation = dataHandle().getNation(nationCode, today);

    if ((nation == nullptr) || (nation->primeCur().empty()))
    {
      throw ErrorResponseException(ErrorResponseException::AGENT_CURRENCY_CODE_MISSING);
    }
    else
    {
      bool rc = false;
      CurrencyCode nationCurrency;
      rc = CurrencyUtil::getNationCurrency(nationCode, nationCurrency, today);
      if (rc)
      {
        agent->currencyCodeAgent() = nationCurrency;
      }
    }
  }
}

void
XMLShoppingHandler::ShoppingRequestParser::parseDURType(const IAttributes& attrs)
{
  if (shoppingTrx() != nullptr)
  {
    attrs.get(_MIN, shoppingTrx()->minDuration(), shoppingTrx()->minDuration());
    attrs.get(_MAX, shoppingTrx()->maxDuration(), shoppingTrx()->maxDuration());
  }
}

void
XMLShoppingHandler::ShoppingRequestParser::parsePDTType(const IAttributes& attrs)
{
  if (shoppingTrx() != nullptr)
  {
    const IValueString& d01 = attrs.get(_D01);
    shoppingTrx()->setTravelDate(parseDateTime(d01));
    const IValueString& d02 = attrs.get(_D02);
    shoppingTrx()->setReturnDate(parseDateTime(d02));
  }
}

namespace
{
bool
isDomesticClientServiceName(const PricingTrx& trx, const std::string& clientServiceName)
{
  return (boost::starts_with(clientServiceName, "SDSXX") ||
          boost::starts_with(clientServiceName, "SDSBA") ||
          boost::starts_with(clientServiceName, "SDSBS") ||
          boost::starts_with(clientServiceName, "SDSCS"));
}
}

void
XMLShoppingHandler::ShoppingRequestParser::parseBILType(const IAttributes& attrs)
{
  Billing*& billing = trx().billing();
  if (billing == nullptr)
  {
    dataHandle().get(billing);
  }

  _getAttr(attrs, _A20, billing->userPseudoCityCode());
  _getAttr(attrs, _A22, billing->aaaCity());
  _getAttr(attrs, _A70, billing->actionCode());
  _getAttr(attrs, _Q03, billing->userStation());
  _getAttr(attrs, _Q02, billing->userBranch());
  _getAttr(attrs, _AE0, billing->partitionID());
  _getAttr(attrs, _AD0, billing->userSetAddress());
  _getAttr(attrs, _AA0, billing->aaaSine());
  attrs.get(_C00, billing->parentTransactionID(), billing->parentTransactionID());
  attrs.get(_C01, billing->clientTransactionID(), billing->clientTransactionID());
  _getAttr(attrs, _C20, billing->parentServiceName());
  _getAttr(attrs, _C21, billing->clientServiceName());
  // put service name into parent service name
  // service name contains 'ATSEILF1' for IS requests and 'ATSEIMIP' for MIP
  if (PricingTrx::MIP_TRX == trx().getTrxType())
  {
    billing->serviceName().assign("ATSEIMIP");
    if (isDomesticClientServiceName(trx(), billing->clientServiceName()))
    {
      trx().setMipDomestic();
    }
    else if (boost::starts_with(billing->clientServiceName(), "SDSVI"))
    {
      trx().setVisMip();
    }

    parser().determineCosExceptionFixEnabled(trx());
  }
  else
  {
    billing->serviceName().assign("ATSEILF1");
  }
  billing->updateTransactionIds(trx().transactionId());
  billing->updateServiceNames(Billing::SVC_SHOPPING);
}

void
XMLShoppingHandler::ShoppingRequestParser::parseODDType(const IAttributes& attrs)
{
  if (shoppingTrx() == nullptr)
    return;

  PricingRequest* const request = shoppingTrx()->getRequest();

  if (attrs.has(_D07))
  {
    const IValueString& d07 = attrs.get(_D07);
    if (attrs.has(_D37))
    {
      const IValueString& d37 = attrs.get(_D37);
      request->requestedDepartureDT() = parseDateTime(d07, d37);
    }
    else
    {
      request->requestedDepartureDT() = parseDateTime(d07);
    }
  }
}

void
XMLShoppingHandler::ShoppingRequestParser::parseFFYType(const IAttributes& attrs)
{
  if ((trx().getTrxType() != PricingTrx::MIP_TRX) ||
      (trx().getRequest()->isCollectOCFees() != true))
  {
    return;
  }

  PaxType::FreqFlyerTierWithCarrier* freqFlyerStatusData = nullptr;
  dataHandle().get(freqFlyerStatusData);

  if (attrs.has(_Q7D))
  {
    FreqFlyerTierLevel newTierLevel;
    attrs.get(_Q7D, newTierLevel, freqFlyerStatusData->freqFlyerTierLevel());
    freqFlyerStatusData->setFreqFlyerTierLevel(newTierLevel);
  }
  else
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Attribute Q7D (Frequent Flyer Status) is required.");
  }
  CarrierCode cxr;
  _getAttr(attrs, _B00, cxr);
  freqFlyerStatusData->setCxr(cxr);
  for (PaxType* paxType : trx().paxType())
    paxType->freqFlyerTierWithCarrier().push_back(freqFlyerStatusData);
}

void
XMLShoppingHandler::ShoppingRequestParser::parseRFGType(const IAttributes& attrs)
{
  if ((trx().getTrxType() != PricingTrx::MIP_TRX) ||
      (trx().getRequest()->isCollectOCFees() != true))
  {
    return;
  }

  RequestedOcFeeGroup requestedOcFeeGroup;
  const OcFeeGroupConfig* feeGroupConfigPtr = nullptr;

  if (attrs.has(_S01))
  {
    _getAttr(attrs, _S01, requestedOcFeeGroup.groupCode());

    if (requestedOcFeeGroup.groupCode().size() > 3)
    {
      std::string error = ("Attribute RFG->S01 (Group code): " + requestedOcFeeGroup.groupCode() +
                           "have more than 3 characters");
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, error.c_str());
    }

    if (trx().getOptions()->isSummaryRequest())
    {
      feeGroupConfigPtr = ServiceFeeUtil::getGroupConfigurationForCode(
          trx().getOptions()->groupsSummaryConfigVec(), requestedOcFeeGroup.groupCode());

      // Skip group codes not defined in configuration
      if (feeGroupConfigPtr == nullptr)
      {
        return;
      }
    }
  }
  else
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Attribute RFG->S01 (Group code) is required.");
  }

  if (attrs.has(_Q0A))
  {
    if (trx().getOptions()->isSummaryRequest())
    {
      if (feeGroupConfigPtr->startRange() == -1)
      {
        std::string error = ("Attribute RFG->Q0A (Number of items) - "
                             "Quantity is not allowed for this group: " +
                             requestedOcFeeGroup.groupCode());
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, error.c_str());
      }

      std::string numberOfItems = "";
      attrs.get(_Q0A, numberOfItems, numberOfItems);

      try
      {
        requestedOcFeeGroup.numberOfItems() = atoi(numberOfItems.c_str());

        if ((requestedOcFeeGroup.numberOfItems() < feeGroupConfigPtr->startRange()) ||
            (requestedOcFeeGroup.numberOfItems() > feeGroupConfigPtr->endRange()))
        {
          std::string error =
              ("Attribute RFG->Q0A (Number of items) - Incorrect quantity value: " + numberOfItems);
          throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, error.c_str());
        }
      }
      catch (const std::exception& e)
      {
        std::string error =
            ("Attribute RFG->Q0A (Number of items): " + numberOfItems + " - Error while parsing.");
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, error.c_str());
      }
    }
    else
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Attribute RFG->Q0A (Number of items) - "
                                   "Quantity is allowed only for summary requests.");
    }
  }
  else
  {
    requestedOcFeeGroup.numberOfItems() = 1;
  }

  trx().getOptions()->serviceGroupsVec().push_back(requestedOcFeeGroup);
}

void
XMLShoppingHandler::ShoppingRequestParser::parseXRAType(const IAttributes& attrs)
{
  std::string xrayMessageId;
  std::string xrayConversationId;

  LOG4CXX_DEBUG(_logger,"In parseXRAType");

  if (attrs.has(_MID))
  {
    _getAttr(attrs, _MID, xrayMessageId);
    LOG4CXX_DEBUG(_logger,"MID = " << xrayMessageId);
  }
  if (attrs.has(_CID))
  {
    _getAttr(attrs, _CID, xrayConversationId);
    LOG4CXX_DEBUG(_logger,"CID = " << xrayConversationId);
  }

  trx().assignXrayJsonMessage(xray::JsonMessagePtr(
      new xray::JsonMessage(xrayMessageId, xrayConversationId)));
}

void
XMLShoppingHandler::ShoppingRequestParser::parseDynamicConfigType(const IAttributes& attrs)
{
  DynamicConfigHandler handler(trx());
  if (!handler.check())
    return;

  DynamicConfigInput input;

  if (attrs.has(_Name))
    input.setName(attrs.get<std::string>(_Name));
  if (attrs.has(_Value))
    input.setValue(attrs.get<std::string>(_Value));
  if (attrs.has(_Substitute))
    input.setSubstitute(attrs.get<std::string>(_Substitute));
  if (attrs.has(_Optional))
    input.setOptional(attrs.get<std::string>(_Optional));

  handler.process(input);
}

void
XMLShoppingHandler::ShoppingRequestParser::parseONDType(const IAttributes& attrs)
{
  PricingTrx::OriginDestination thruFMForMip;

  _getAttr(attrs, _A01, thruFMForMip.boardMultiCity);
  if (attrs.has(_A02))
  {
    _getAttr(attrs, _A02, thruFMForMip.offMultiCity);
  }
  if (attrs.has(_D01))
  {
    const IValueString& d01 = attrs.get(_D01);
    thruFMForMip.travelDate = parseDateTime(d01);
  }
  if (attrs.has(_D02))
  {
    const IValueString& d02 = attrs.get(_D02);
    thruFMForMip.travelEndDate = parseDateTime(d02);
  }
  if (attrs.has(_Q14))
  {
    attrs.get(_Q14, thruFMForMip.legID);
  }
  else
  {
    if (trx().getRequest()->owPricingRTTaxProcess())
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Origin destination must have leg id specified");
    }
  }
  if (attrs.has(_C57))
  {
    if (attrs.has(_P44))
    {
      attrs.get(_C57, thruFMForMip.minBudgetAmount);
    }
    else
    {
      if (trx().getRequest()->owPricingRTTaxProcess())
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "Min Budget Amount must have Currency Code specified");
      }
    }
  }
  if (attrs.has(_C58))
  {
    if (attrs.has(_P44))
    {
      attrs.get(_C58, thruFMForMip.maxBudgetAmount);
    }
    else
    {
      if (trx().getRequest()->owPricingRTTaxProcess())
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "Max Budget Amount must have Currency Code specified");
      }
    }
  }
  _getAttr(attrs, _P44, thruFMForMip.currencyCode);
  if (!fallback::exsCalendar(&trx()))
  {
    _getAttr(attrs, _D41, thruFMForMip.calDaysBefore);
    _getAttr(attrs, _D42, thruFMForMip.calDaysAfter);
  }

  if (!ExchShopCalendar::validateInputParams(thruFMForMip, trx().isTestRequest()))
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Invalid calendar option");
  }

  for (auto& ond : trx().orgDest)
  {
    if ((thruFMForMip.boardMultiCity == ond.boardMultiCity) &&
        (thruFMForMip.offMultiCity == ond.offMultiCity) &&
        (thruFMForMip.travelDate.date() == ond.travelDate.date()))
    {
      ++(ond.skippedOND);
      return;
    }
  }
  trx().orgDest.push_back(thruFMForMip);
}

void
XMLShoppingHandler::ShoppingRequestParser::parseFFGType(const IAttributes& attrs)
{
  PricingRequest& request = *trx().getRequest();
  PricingOptions& option = *trx().getOptions();
  flexFares::GroupId flexFaresGroupId = 0;

  if (trx().isMainFareFFGGroupIDZero() && !fallback::fallbackFFGAffectsMainFare(&trx()))
    trx().setFlexFare(true);
  if (trx().getTrxType() != PricingTrx::MIP_TRX || !trx().isFlexFare())
  {
    return;
  }

  // Q17 - Flex Fare Group Id
  if (!attrs.has(_Q17))
  { // It doesn't have the flex fare group ID so it's invalid
    return;
  }
  else
  {
    attrs.get(_Q17, flexFaresGroupId);
    request.getMutableFlexFaresGroupsData().createNewGroup(flexFaresGroupId);
    if (!fallback::fallbackFlexFareGroupNewXCLogic(&trx()))
      request.getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, flexFaresGroupId);
  }

  //The group in <PRO> is to be considered as Flex Group for JumpCabin Validation
  if (!fallback::fallbackJumpCabinExistingLogic(&trx()) && trx().isMainFareFFGGroupIDZero())
  {
    trx().setMainFareFFGGroupIDZero(false);
    if (request.getJumpCabinLogic() != JumpCabinLogic::ENABLED)
    {
      const flexFares::GroupId defaultFlexFaresGroupID = parser().defaultFlexFaresGroupID(trx());
      request.getMutableFlexFaresGroupsData().createNewGroup(defaultFlexFaresGroupID);
      request.getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, defaultFlexFaresGroupID);
      if (request.getJumpCabinLogic() == JumpCabinLogic::DISABLED)
        request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
                flexFares::JumpCabinLogic::DISABLED, defaultFlexFaresGroupID);
      else
        request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
                flexFares::JumpCabinLogic::ONLY_MIXED, defaultFlexFaresGroupID);
    }
  }

  // PXC - Flex Fare Phase III XC indicator
  if (attrs.has(_PXC) && !fallback::fallbackFlexFareGroupNewXCLogic(&trx()))
  {
    char xcIndicator = attrs.get<char>(_PXC, 0);
    request.getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(
        TypeConvert::pssCharToBool(xcIndicator), flexFaresGroupId);
  }

  // P20 - Flex Fare Group XO indicator
  if (attrs.has(_P20) && !fallback::fallbackFlexFareGroupNewXOLogic(&trx()))
  {
    char xoIndicator = attrs.get<char>(_P20, 0);
    request.getMutableFlexFaresGroupsData().setFlexFareXOFares(xoIndicator, flexFaresGroupId);
  }

  // PTC - Passenger Type
  if (attrs.has(_PTC))
  {
    PaxTypeCode psgType;
    _getAttr(attrs, _PTC, psgType);
    request.getMutableFlexFaresGroupsData().setPaxTypeCode(psgType, flexFaresGroupId);
  }

  // B31 - Requested Flex Fare Group Cabin
  if (attrs.has(_B31))
  {
    char alfaNumCabin = attrs.get<char>(_B31, 0);
    request.getMutableFlexFaresGroupsData().setRequestedCabinFromAlphaNum(alfaNumCabin,
                                                                          flexFaresGroupId);
  }

  //Update setJumpCabinLogic status for the Flex Fare Groups
  if (!fallback::fallbackFlexFareGroupNewJumpCabinLogic(&trx()))
  {
    //PXS Expand Jump Cabin Indicator for Flex Fare Group
    if (attrs.has(_PXS))
    {
      if(TypeConvert::pssCharToBool(attrs.get<char>(_PXS, 0)))
        request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
          flexFares::JumpCabinLogic::ONLY_MIXED, flexFaresGroupId);
    }

    //PXU Disable Jump Cabin Indicator for Flex Fare Group
    if (attrs.has(_PXU))
    {
      if(TypeConvert::pssCharToBool(attrs.get<char>(_PXU, 0)))
        request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
          flexFares::JumpCabinLogic::DISABLED, flexFaresGroupId);
      else
      {
        request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
          flexFares::JumpCabinLogic::ENABLED, flexFaresGroupId);
      }
    }

    //updating the jumpCabinLogic status if <PRO> and <FFG> have different jumpcabinlogic
    if (!fallback::fallbackJumpCabinExistingLogic(&trx()))
    {
      JumpCabinLogic jcl = JumpCabinLogic::ENABLED;
      flexFares::JumpCabinLogic ffgJCL =
        request.getMutableFlexFaresGroupsData().getFFGJumpCabinLogic(flexFaresGroupId);
      if (ffgJCL == flexFares::JumpCabinLogic::ONLY_MIXED)
        jcl = JumpCabinLogic::ONLY_MIXED;
      else if (ffgJCL == flexFares::JumpCabinLogic::DISABLED)
          jcl = JumpCabinLogic::DISABLED;
      if (jcl < request.getJumpCabinLogic())
        request.setJumpCabinLogic(jcl);
    }
  }

  // P1Y - Published/Public Fares
  if (attrs.has(_P1Y))
  {
    char publicFare = attrs.get<char>(_P1Y, 0);
    request.getMutableFlexFaresGroupsData().setPublicFares(TypeConvert::pssCharToBool(publicFare),
                                                           flexFaresGroupId);
    if (publicFare == 'T')
      option.privateFares() = 'F';
  }

  // P1Z - Private Fares
  if (attrs.has(_P1Z))
  {
    char privateFare = attrs.get<char>(_P1Z, 0);
    request.getMutableFlexFaresGroupsData().setPrivateFares(TypeConvert::pssCharToBool(privateFare),
                                                            flexFaresGroupId);
    if (privateFare == 'T')
      option.publishedFares() = 'F';
  }

  // FFR - Flex Fare Restrictions
  if (attrs.has(_FFR))
  {
    std::string stringFFR;
    attrs.get(_FFR, stringFFR);

    if (!stringFFR.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringFFR, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currFFR = ((std::string)tokenI->data());

        if (currFFR == "XA")
        {
          request.getMutableFlexFaresGroupsData().requireNoAdvancePurchase(flexFaresGroupId);
        }
        else if (currFFR == "XP")
        {
          request.getMutableFlexFaresGroupsData().requireNoPenalties(flexFaresGroupId);
        }
        else if (currFFR == "XS")
        {
          request.getMutableFlexFaresGroupsData().requireNoMinMaxStay(flexFaresGroupId);
        }
        else if (currFFR == "XR")
        {
          request.getMutableFlexFaresGroupsData().requireNoRestrictions(flexFaresGroupId);
        }
      }
    }
  }

  // ACX - Corp Ids
  if (attrs.has(_ACX))
  {
    std::string stringACX;
    attrs.get(_ACX, stringACX);

    if (!stringACX.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringACX, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currCorpId = ((std::string)tokenI->data());
        request.getMutableFlexFaresGroupsData().addCorpId(currCorpId, flexFaresGroupId);

        if (!fallback::fallbackCorpIDFareBugFixing(&trx()))
        {
          if (!fallback::fallbackFFGAcctCodeFareFix(&trx()))
          {
            std::vector<std::string>& corpIdVec = request.corpIdVec();
            if (std::find(corpIdVec.begin(), corpIdVec.end(), currCorpId.c_str()) ==
                corpIdVec.end())
              corpIdVec.push_back(currCorpId.c_str());
          }
          else
          {
            std::vector<std::string>& accCodeVec = request.accCodeVec();
            if (std::find(accCodeVec.begin(), accCodeVec.end(), currCorpId.c_str()) ==
                accCodeVec.end())
              accCodeVec.push_back(currCorpId.c_str());
          }
        }
      }
    }
  }

  // SMX - Accont codes
  if (attrs.has(_SMX))
  {
    std::string stringSMX;
    attrs.get(_SMX, stringSMX);

    if (!stringSMX.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringSMX, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currAcctCode = ((std::string)tokenI->data());
        request.getMutableFlexFaresGroupsData().addAccCode(currAcctCode, flexFaresGroupId);
        if (!fallback::fallbackFFGAcctCodeFareFix(&trx()))
        {
          std::vector<std::string>& accCodeVec = request.accCodeVec();
          if (std::find(accCodeVec.begin(), accCodeVec.end(), currAcctCode.c_str()) ==
              accCodeVec.end())
            accCodeVec.push_back(currAcctCode.c_str());
        }
      }
    }
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::DIVTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (idx == _CXP)
    parseCXPType(attrs);
  return nullptr;
}

void
XMLShoppingHandler::DIVTypeParser::parseCXPType(const IAttributes& attrs)
{
  if (attrs.has(_OPC) && attrs.has(_PCP))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Setting absolute and relative number of options "
                                 "per carrier at the same time is not allowed");
  tse::Diversity& diversity = shoppingTrx()->diversity();
  std::string value;
  CarrierCode cxr;
  if (attrs.has(_B00))
  {
    _getAttr(attrs, _B00, value);
    cxr = value;
    if (diversity.getOptionsPerCarrier(cxr) != 0.0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Multiple CXP tags with same carrier detected");
  }
  else
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "B00 attribute is required in CXP tag");
  if (attrs.has(_OPC))
  {
    _getAttr(attrs, _OPC, value);
    int pValue = atoi(value.c_str());
    if (pValue <= 0)
      throw ErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "Absolute number of options per carrier should be positive value");
    diversity.setOptionsPerCarrier(cxr, pValue);
  }
  if (attrs.has(_PCP))
  {
    _getAttr(attrs, _PCP, value);
    float pValue = atof(value.c_str());
    if (pValue <= 0.0 || pValue > 1.0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Relative number of options per carrier "
                                   "should be between 0.0 and 1.0");
    diversity.setOptionsPerCarrier(cxr, -pValue);
  }
}

void
XMLShoppingHandler::PROTypeParser::parseRetailerCode(const IAttributes& attrs)
{
  if (!fallback::fallbackFRRProcessingRetailerCode(&trx()))
  {
    tse::PricingRequest* const request = trx().getRequest();
    std::string value;

    if (attrs.has(_PRM) && request != nullptr)
    {
      value = attrs.get<std::string>(_PRM);
      if (value == "T" || value == "t")
        request->setPRM(true);
      else if (value == "F" || value == "f")
        request->setPRM(false);
    }

    if (attrs.has(_RCQ) && request != nullptr)
    {
      _getAttr(attrs, _RCQ, value);
      request->setRCQValues(value);
    }

    if (!fallback::fallbackFixProcessingRetailerCodeXRS(&trx())
        && trx().getOptions()->isXRSForFRRule()
        && (request->rcqValues().size() > 0) )
    {
      std::string sMsg = "UNABLE TO COMBINE ";
      if (request->prmValue())
        sMsg+= "RRM*";
      else
        sMsg+= "RR*";

      sMsg += " WITH XRS";
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, sMsg.c_str());
    }
  }
}

void
XMLShoppingHandler::DIVTypeParser::startParsing(const IAttributes& attrs)
{
  if (shoppingTrx() == nullptr)
    return;

  if (attrs.has(_OPC) && attrs.has(_PCP))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Setting 'Minimal number of options per carrier' "
                                 "and 'Percent of options per carrier' together are not allowed");

  tse::Diversity& diversity = shoppingTrx()->diversity();
  diversity.setEnabled();

  std::string value;
  if (attrs.has(_MDL))
  {
    value = attrs.get<std::string>(_MDL);
    diversity.setModel(DiversityModelType::parse(value));
  }

  if (!shoppingTrx()->isAltDates())
  {
    if (attrs.has(_DCL))
    {
      _getAttr(attrs, _DCL, value);
      boost::char_separator<char> sep("|");
      boost::tokenizer<boost::char_separator<char>> tknizer(value, sep);
      for (boost::tokenizer<boost::char_separator<char>>::iterator tkIt = tknizer.begin();
           tkIt != tknizer.end();
           ++tkIt)
      {
        const std::string& currentStr = *tkIt;
        const std::string delimit(",");
        std::string::size_type lastPosition = 0;
        std::string::size_type position = currentStr.find_first_of(delimit, lastPosition);
        CarrierCode cxr = currentStr.substr(lastPosition, position - lastPosition);
        lastPosition = position + 1;
        position = currentStr.length();
        int requestNum = atoi((currentStr.substr(lastPosition, position - lastPosition)).c_str());
        diversity.setDiversityCarrierList(cxr, requestNum);
      }
    }

    if (attrs.has(_OCO))
    {
      value = attrs.get<std::string>(_OCO);
      if (value == "Y" || value == "y")
        diversity.setOCO();
    }

    if (attrs.has(_NSD))
    {
      float ValueNSD = attrs.get<float>(_NSD);
      diversity.setOptionsPerCarrierDefault(ValueNSD);
    }
  }

  if (attrs.has(_NSV))
  {
    std::size_t pValue = attrs.get<std::size_t>(_NSV);
    diversity.setNonStopOptionsCount(pValue);
  }

  if (diversity.getModel() == DiversityModelType::V2 || diversity.isExchangeForAirlines())
  {
    return; // Ignore other attributes in case of V2 model
  }

  if (attrs.has(_TOD))
  {
    _getAttr(attrs, _TOD, value);
    std::vector<std::pair<uint16_t, uint16_t>> todRanges;
    boost::char_separator<char> sep("|");
    boost::tokenizer<boost::char_separator<char>> tknizer(value, sep);
    for (boost::tokenizer<boost::char_separator<char>>::iterator tkIt = tknizer.begin();
         tkIt != tknizer.end();
         ++tkIt)
    {
      const std::string& curStr = *tkIt;
      size_t pos = curStr.find('-');
      todRanges.push_back(std::make_pair(atoi(curStr.substr(0, pos).c_str()),
                                         atoi(curStr.substr(pos + 1, std::string::npos).c_str())));
    }
    uint16_t trStart = 0, trEnd = 1439;
    for (size_t i = 0; i < todRanges.size(); ++i)
    {
      if (todRanges[i].first != trStart || todRanges[i].second <= todRanges[i].first ||
          todRanges[i].second > trEnd)
        break;
      trStart = todRanges[i].second + 1;
    }
    if (trStart != trEnd + 1)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Time of day ranges should cover whole day "
                                   "sequentially without holes");
    diversity.setTODRanges(todRanges);
  }
  if (attrs.has(_OPC))
  {
    int pValue = attrs.get<int>(_OPC);
    if (pValue <= 0)
      throw ErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "Minimal number of options per carrier should be positive value");
    diversity.setOptionsPerCarrierDefault(pValue);
  }
  if (attrs.has(_FAC))
  {
    float pValue = attrs.get<float>(_FAC);
    if (pValue <= 1.0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Maximal fare amount coefficient should be greater than 1.0");
    diversity.setFareCutoffCoef(pValue);
  }
  if (attrs.has(_NSO))
  {
    float pValue = attrs.get<float>(_NSO);
    float upperLimit = AdditionalNsPercentageUppterLimit.getValue();

    if (pValue < EPSILON || pValue > upperLimit)
    {
      char errorMsg[70];
      snprintf(
          errorMsg, 70, "Non stop options percentage should be between 0.0 and %f", upperLimit);
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, errorMsg);
    }
    diversity.setNonStopOptionsPercentage(pValue);
  }
  if (attrs.has(_IOP))
  {
    int pValue = attrs.get<int>(_IOP);
    if (pValue <= 0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Inbound/outbound pairing should be positive value");
    diversity.setInboundOutboundPairing(pValue);
  }
  if (attrs.has(_FLN))
  {
    int pValue = attrs.get<int>(_FLN);
    if (pValue <= 0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Fare level number should be positive value");
    diversity.setFareLevelNumber(pValue);
  }
  if (attrs.has(_FAS))
  {
    float pValue = attrs.get<float>(_FAS);
    if (pValue <= 1.0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Fare amount separator should be greater than 1.0");
    diversity.setFareAmountSeparatorCoef(pValue);
  }
  if (attrs.has(_TTS))
  {
    float pValue = attrs.get<float>(_TTS);
    if (pValue <= 1.0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Travel time separator should be greater than 1.0");
    diversity.setTravelTimeSeparatorCoef(pValue);
  }
  if (attrs.has(_TTD))
  {
    _getAttr(attrs, _TTD, value);
    std::vector<float> todDistribution;
    float tddSum = 0.0;
    boost::char_separator<char> sep("|");
    boost::tokenizer<boost::char_separator<char>> tknizer(value, sep);
    for (boost::tokenizer<boost::char_separator<char>>::iterator tkIt = tknizer.begin();
         tkIt != tknizer.end();
         ++tkIt)
    {
      const std::string& curStr = *tkIt;
      float pValue = atof(curStr.c_str());
      if (pValue < 0.0 || pValue > 1.0)
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "Distribution value for any TOD bucket "
                                     "should be between 0.0 and 1.0");
      todDistribution.push_back(pValue);
      tddSum += pValue;
    }
    if (todDistribution.size() != diversity.getTODRanges().size())
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Distribution value for each defined TOD "
                                   "bucket should be provided");
    if (fabsf(tddSum - 1.0) > 0.0001)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Sum of distribution values for all TOD "
                                   "buckets should be equal to 1.0");
    diversity.setTODDistribution(todDistribution);
  }
  else
    diversity.setTODDistribution(
        std::vector<float>(diversity.getTODRanges().size(), 1.0 / diversity.getTODRanges().size()));
  if (attrs.has(_PCP))
  {
    float pValue = attrs.get<float>(_PCP);
    if (pValue <= 0.0 || pValue > 1.0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Percent of options per carrier should be between 0.0 and 1.0");
    diversity.setOptionsPerCarrierDefault(-pValue);
  }
  if (attrs.has(_BKD))
  {
    _getAttr(attrs, _BKD, value);
    float bktDistribution[Diversity::BUCKET_COUNT];
    float bktSum = 0.0;
    size_t bktCount = 0;
    boost::char_separator<char> sep("|");
    boost::tokenizer<boost::char_separator<char>> tknizer(value, sep);
    boost::tokenizer<boost::char_separator<char>>::iterator tkIt;
    for (tkIt = tknizer.begin(); tkIt != tknizer.end() && bktCount < Diversity::BUCKET_COUNT;
         ++tkIt, ++bktCount)
    {
      const std::string& curStr = *tkIt;
      float pValue = atof(curStr.c_str());
      if (pValue < 0.0 || pValue > 1.0)
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "Distribution value for any diversity bucket "
                                     "should be between 0.0 and 1.0");
      bktDistribution[bktCount] = pValue;
      bktSum += pValue;
    }
    if (bktCount != Diversity::BUCKET_COUNT || tkIt != tknizer.end())
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Too many/few distribution values for diversity buckets");
    if (fabsf(bktSum - 1.0) > 0.0001)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Sum of distribution values for all diversity buckets "
                                   "should be equal to 1.0");
    for (size_t i = 0; i < Diversity::BUCKET_COUNT; ++i)
      diversity.setBucketDistribution((Diversity::BucketType)i, bktDistribution[i]);
  }
  if (attrs.has(_PFC))
  {
    _getAttr(attrs, _PFC, value);
    std::vector<CarrierCode> preferredCarriers;
    boost::char_separator<char> sep("|");
    boost::tokenizer<boost::char_separator<char>> tknizer(value, sep);
    boost::tokenizer<boost::char_separator<char>>::iterator tkIt;
    for (tkIt = tknizer.begin(); tkIt != tknizer.end(); ++tkIt)
    {
      CarrierCode cxr;
      cxr.assign(tkIt->c_str(), tkIt->size());
      preferredCarriers.push_back(cxr);
    }
    diversity.setPreferredCarriers(preferredCarriers);
  }
  if (attrs.has(_FRL))
  {
    int pValue = attrs.get<int>(_FRL);
    if (pValue < 0)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Flight repeat limit pairing should be non-negative value");
    diversity.setFlightRepeatLimit(pValue);
  }

  if (attrs.has(_HDM))
  {
    diversity.setHighDensityMarket(attrs.get<bool>(_HDM));
  }

//  parseRetailerCode(attrs);
}

void
XMLShoppingHandler::CMDTypeParser::endParsing(const IValueString& text)
{
  PricingOptions* options = trx().getOptions();
  options->lineEntry().assign(text.c_str(), text.length());
}

// branded fare

void
XMLShoppingHandler::BRNTypeParser::startParsing(const IAttributes& attrs)
{
  BookingCode bkc = BLANK_CODE;
  BookingCode secondaryBkc = BLANK_CODE;
  IValueString fbf;
  char indicator = BLANK;
  bool excludeFlag = false;

  if (attrs.has(_B30))
  {
    _getAttr(attrs, _B30, bkc);
  }

  if (attrs.has(_B31))
  {
    _getAttr(attrs, _B31, secondaryBkc);
  }

  if (attrs.has(_N24))
  {
    fbf = attrs.get(_N24);
  }

  validateInput(bkc != BLANK_CODE || secondaryBkc != BLANK_CODE || !fbf.empty(),
                "Branded Data : One of Booking Code, Fare Basis/Family must not be blank");

  bool bkcBlank = (secondaryBkc == BLANK_CODE && bkc == BLANK_CODE);
  validateInput((!bkcBlank && fbf.empty()) || (bkcBlank && !fbf.empty()),
                "Branded Data : Only one of Booking Code, Fare Basis/Family should not be blank");

  if (attrs.has(_N22))
  {
    attrs.get(_N22, indicator);
  }

  if (attrs.has(_PXY))
  {
    char ch(0);
    attrs.get(_PXY, ch);
    excludeFlag = 'T' == ch;
  }

  if (bkc != BLANK_CODE || secondaryBkc != BLANK_CODE)
  {
    parseBookingCode(bkc, secondaryBkc, indicator, excludeFlag);
  }
  else
  {
    parseFareBasisOrFamily(fbf, indicator, excludeFlag);
  }
}

template <typename T>
bool
XMLShoppingHandler::BRNTypeParser::isUniqe(const std::vector<T>& codes, const T& code) const
{
  return std::find(codes.begin(), codes.end(), code) == codes.end();
}

template <typename T>
void
XMLShoppingHandler::BRNTypeParser::collect(const T& code,
                                           bool isExclude,
                                           char indicator,
                                           const char* msg,
                                           std::vector<T>& codes,
                                           std::vector<T>& excludeCodes,
                                           std::map<T, char>& codeData) const
{
  validateInput(isUniqe(codes, code), msg);
  validateInput(isUniqe(excludeCodes, code), msg);

  if (isExclude)
  {
    excludeCodes.push_back(code);
  }
  else
  {
    codes.push_back(code);
    codeData[code] = indicator;
  }
}

void
XMLShoppingHandler::BRNTypeParser::parseBookingCode(const BookingCode& bkc,
                                                    const BookingCode& secondaryBkc,
                                                    char indicator,
                                                    bool excludeFlag)
{
  PricingRequest& request = *trx().getRequest();
  const uint16_t brandIndex = (request.getBrandedFareSize() ? request.getBrandedFareSize() - 1 : 0);

  if (bkc != BLANK_CODE)
    collect(bkc,
            excludeFlag,
            indicator,
            "Branded Data : duplicate booking code",
            request.brandedFareBookingCode(brandIndex),
            request.brandedFareBookingCodeExclude(brandIndex),
            request.brandedFareBookingCodeData(brandIndex));

  if (secondaryBkc != BLANK_CODE)
    collect(secondaryBkc,
            excludeFlag,
            indicator,
            "Branded Data : duplicate secondary booking code",
            request.brandedFareSecondaryBookingCode(brandIndex),
            request.brandedFareSecondaryBookingCodeExclude(brandIndex),
            request.brandedFareSecondaryBookingCodeData(brandIndex));
}

void
XMLShoppingHandler::BRNTypeParser::parseFareBasisOrFamily(const IValueString& fbf,
                                                          char indicator,
                                                          bool excludeFlag)
{
  PricingRequest* request = trx().getRequest();
  const uint16_t brandIndex = request->getBrandedFareSize() ? request->getBrandedFareSize() - 1 : 0;

  const int ndashes = std::count(fbf.begin(), fbf.end(), '-');
  validateInput(ndashes <= 1, "Branded Data : incorect Fare Basis/Family");
  FareClassCode fcc(fbf.c_str(), fbf.length());

  if (ndashes > 0) // means we have Fare Family Specified
  {
    collect(fcc,
            excludeFlag,
            indicator,
            "Branded Data : duplicate fare family",
            request->brandedFareFamily(brandIndex),
            request->brandedFareFamilyExclude(brandIndex),
            request->brandedFareFamilyData(brandIndex));
  }
  else // Fare Basis Code specified
  {
    collect(fcc,
            excludeFlag,
            indicator,
            "Branded Data : duplicate fare basis code",
            request->brandedFareBasisCode(brandIndex),
            request->brandedFareBasisCodeExclude(brandIndex),
            request->brandedFareBasisCodeData(brandIndex));
  }
}

void
XMLShoppingHandler::EXCTypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(rexPricingTrx() != nullptr,
                "XMLShoppingHandler::EXCTypeParser::newElement - "
                "EXC element in non - Exchange Request .");
  parser().parsingExchangeInformation() = true;
  rexPricingTrx()->setAnalyzingExcItin(true);
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::EXCTypeParser::newElement(int idx, const IAttributes& attrs)
{
  validateInput(rexPricingTrx() != nullptr,
                "XMLShoppingHandler::EXCTypeParser::newElement - "
                "EXC element in non - Exchange Request .");
  if (_AGI == idx)
  {
    parseAGIType(attrs);
  }
  else if (_PRO == idx)
  {
    parseEXCPROType(attrs);
  }
  else if (_PXI == idx)
  {
    parser().parsingAccompanyPassenger() = false;
    return &parser()._pxiParser;
  }
  else if (_APX == idx)
  {
    parser().parsingAccompanyPassenger() = true;
    return &parser()._pxiParser;
  }
  else if (_AAF == idx)
  {
    return &parser()._aafParser;
  }
  else if (_LEG == idx)
  {
    return &parser()._legParser;
  }
  else if (_ITN == idx)
  {
    return &parser()._itnParser;
  }
  else if (_FCI == idx)
  {
    return &parser()._fciParser;
  }
  return nullptr;
}

void
XMLShoppingHandler::EXCTypeParser::parseEXCPROType(const IAttributes& attrs)
{
  RexPricingRequest* rexRequest = dynamic_cast<RexPricingRequest*>(rexPricingTrx()->getRequest());

  if (attrs.has(_ACX))
  {
    attrs.get(_ACX, rexRequest->excCorporateID(), rexRequest->excCorporateID());

    if (!rexPricingTrx()->dataHandle().corpIdExists(rexRequest->excCorporateID(),
                                                    rexPricingTrx()->ticketingDate()))
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_CORP_ID);
    }
  }

  if (attrs.has(_B05))
  {
    _getAttr(attrs, _B05, rexRequest->excValidatingCarrier());
  }
  if (attrs.has(_AF0))
  {
    _getAttr(attrs, _AF0, rexRequest->salePointOverride());
  }
  if (attrs.has(_AG0))
  {
    _getAttr(attrs, _AG0, rexRequest->ticketPointOverride());
  }

  attrs.get(_SMX, rexRequest->excAccountCode(), rexRequest->excAccountCode());

  RexPricingOptions* rexOptions = dynamic_cast<RexPricingOptions*>(rexPricingTrx()->getOptions());

  if (rexOptions == nullptr)
  {
    LOG4CXX_ERROR(_logger,
                  "XMLShoppingHandler::EXCTypeParser::parseEXCPROType  - "
                  "RexPricingOptions must not be NULL");
  }
  ShoppingRequestParser::saveCountryAndStateRegionCodes(trx(),
                                                        attrs,
                                                        rexOptions->excEmployment(),
                                                        rexOptions->excNationality(),
                                                        rexOptions->excResidency());
  if (attrs.has(_S94))
  {
    _getAttr(attrs, _S94, rexPricingTrx()->waiverCode());
  }

  CurrencyCode fareCalcCurrency;

  _getAttr(attrs, _C6Y, fareCalcCurrency);

  if (attrs.has(_D95))
  {
    const IValueString& d95 = attrs.get(_D95);
    if (trx().getTrxType() == PricingTrx::RESHOP_TRX)
    {
      BaseExchangeTrx* baseTrx = static_cast<BaseExchangeTrx*>(rexShoppingTrx());
      if (baseTrx)
      {
        baseTrx->previousExchangeDT() = parseDateTime(d95);
        setTimeInHistoricalDate(attrs, baseTrx->previousExchangeDT(), _T95);
      }
    }
    else
    {
      BaseExchangeTrx* baseTrx = static_cast<BaseExchangeTrx*>(rexExchangeTrx());
      if (baseTrx)
      {
        baseTrx->previousExchangeDT() = parseDateTime(d95);
        setTimeInHistoricalDate(attrs, baseTrx->previousExchangeDT(), _T95);
      }
    }
  }

  if (attrs.has(_C5A))
  {
    attrs.get(_C5A, rexOptions->excTotalFareAmt());
  }

  if (rexPricingTrx())
  {
    _getAttr(attrs, _C6P, rexOptions->excBaseFareCurrency());

    validateInput(!rexOptions->excBaseFareCurrency().empty(), "Missing ExcTkt currency - C6P");
    validateInput(!rexOptions->excTotalFareAmt().empty(), "Missing ExcTkt amount - C5A");

    rexPricingTrx()->setTotalBaseFareAmount(Money(
        strtod(rexOptions->excTotalFareAmt().c_str(), nullptr), rexOptions->excBaseFareCurrency()));

    if (attrs.has(_NRA))
    {
      MoneyAmount excNonRefundableAmt = -EPSILON;
      attrs.get(_NRA, excNonRefundableAmt);
      validateInput(!rexOptions->excBaseFareCurrency().empty(), "Missing ExcTkt currency - C6P");

      rexPricingTrx()->setExcNonRefAmount(
          Money(excNonRefundableAmt, rexOptions->excBaseFareCurrency()));
    }

    rexOptions->setNetSellingIndicator(attrs.get(_PY6, 'F') == 'T');
  }

  if (!fareCalcCurrency.empty() && !rexPricingTrx()->exchangeItin().empty())
  {
    ExcItin* exchangeItin = rexPricingTrx()->exchangeItin().back();
    if (exchangeItin == nullptr)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingHandler::EXCTypeParser::parseEXCPROType  - "
                    "Exchange Itin must not be NULL");
    }
    exchangeItin->calcCurrencyOverride() = fareCalcCurrency;
    exchangeItin->calculationCurrency() = fareCalcCurrency;
  }
}

void
XMLShoppingHandler::EXCTypeParser::endParsing(const IValueString& text)
{
  RexBaseRequest* rexBaseRequest = static_cast<RexBaseRequest*>(trx().getRequest());
  ExcItin* itin = rexPricingTrx()->exchangeItin().at(0);
  CommonParserUtils::initShoppingDiscountForSegsWithNoDiscountIfReqHasDiscount(*rexBaseRequest,
                                                                               itin);
  parser().parsingExchangeInformation() = false;
  parser().parsingAccompanyPassenger() = false;
  rexPricingTrx()->setAnalyzingExcItin(false);
}

void
XMLShoppingHandler::PCLTypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(attrs.has(_B00), "PCL/B00 (CARRIER CODE) is required");

  CarrierCode cxr;
  _getAttr(attrs, _B00, cxr);
  rexShoppingTrx()->cxrListFromPSS().cxrList.insert(cxr);
  if (attrs.has(_EXL))
  {
    bool excluded;
    attrs.get(_EXL, excluded);
    rexShoppingTrx()->cxrListFromPSS().excluded = excluded;
  }
}

void
XMLShoppingHandler::FCLTypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(attrs.has(_AJ0), "FCL/AJ0 (LOCATION CODE) is required");

  LocCode loc;
  _getAttr(attrs, _AJ0, loc);
  shoppingTrx()->forcedConnection().insert(loc);
}

void
XMLShoppingHandler::FCITypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(attrs.has(_Q6D), "FCI/Q6D (FARE COMPONENT NUMBER) is required");

  FareComponentInfo* fareComponentInfo = nullptr;
  dataHandle().get(fareComponentInfo);
  attrs.get(_Q6D, fareComponentInfo->fareCompNumber());
  rexPricingTrx()->excFareCompInfo().push_back(fareComponentInfo);
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::FCITypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_VCT == idx)
  {
    return &parser()._vctParser;
  }
  else if (_R3I == idx)
  {
    return &parser()._r3iParser;
  }

  return nullptr;
}

void
XMLShoppingHandler::VCTTypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(attrs.has(_S37), "VCT/S37 (VENDOR) is required");
  validateInput(attrs.has(_B09), "VCT/B09 (CARRIER) is required");
  validateInput(attrs.has(_S89), "VCT/S89 (FARE TARIFF) is required");
  validateInput(attrs.has(_S90), "VCT/S90 (RULE NUMBER) is required");
  validateInput(attrs.has(_RTD), "VCT/RTD (FARE RETRIEVAL DATE TIME) is required");

  VCTRInfo* vctrInfo = nullptr;
  dataHandle().get(vctrInfo);
  _getAttr(attrs, _S37, vctrInfo->vendor());
  _getAttr(attrs, _B09, vctrInfo->carrier());
  attrs.get(_S89, vctrInfo->fareTariff());
  _getAttr(attrs, _S90, vctrInfo->ruleNumber());
  if (attrs.has(_RTD))
  {
    const IValueString& rtd = attrs.get(_RTD);
    vctrInfo->retrievalDate() = parseDateTime(rtd);
  }
  else
  {
    vctrInfo->retrievalDate() = DateTime::emptyDate();
  }
  rexPricingTrx()->excFareCompInfo().back()->vctrInfo() = vctrInfo;
}

void
XMLShoppingHandler::R3ITypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(attrs.has(_Q5Y), "R3I/Q5Y (CAT31 RECORD3 ITEM NUMBER) is required");

  Cat31Info* cat31Info = nullptr;
  dataHandle().get(cat31Info);
  attrs.get(_Q5Y, cat31Info->rec3ItemNo);
  rexPricingTrx()->excFareCompInfo().back()->cat31Info().push_back(cat31Info);

  if (!fallback::exsCalendar(&trx()) && attrs.has(_D41) && attrs.has(_D42))
  {
    cat31Info->rec3DateRange.firstDate = parseDateTime(attrs.get(_D41));
    cat31Info->rec3DateRange.lastDate = parseDateTime(attrs.get(_D42));
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::R3ITypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_SEQ == idx)
  {
    return &parser()._seqParser;
  }

  return nullptr;
}

void
XMLShoppingHandler::SEQTypeParser::startParsing(const IAttributes& attrs)
{
  validateInput(attrs.has(_Q5Z), "SEQ/Q5Z (SEQ TAB988 ITEM NUMBER) is required");

  int tab988SeqNo = -1;
  attrs.get(_Q5Z, tab988SeqNo);
  auto& cat31Info = *rexPricingTrx()->excFareCompInfo().back()->cat31Info().back();
  cat31Info.tab988SeqNo.insert(tab988SeqNo);

  if (!fallback::exsCalendar(&trx()) && attrs.has(_D41) && attrs.has(_D42))
  {
    const IValueString& d41 = attrs.get(_D41), &d42 = attrs.get(_D42);

    cat31Info.tab988SeqNoToDateRangeMap.emplace(
        tab988SeqNo, ExchShopCalendar::DateRange{parseDateTime(d41), parseDateTime(d42)});
  }
}

// brand info

void
XMLShoppingHandler::BRSTypeParser::startParsing(const IAttributes& attrs)
{
  PricingRequest* request = trx().getRequest();
  const uint16_t newBrandIndex = request->getBrandedFareSize();
  if (attrs.has(_SB2))
  {
    _getAttr(attrs, _SB2, request->brandId(newBrandIndex));
  }
  else
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Attribute SB2 (Brand ID) is required.");
  }

  if (attrs.has(_SC0))
  {
    _getAttr(attrs, _SC0, request->programId(newBrandIndex));
  }
  else
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Attribute SC0 (Program ID) is required.");
  }
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::BRSTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (_BRN == idx)
  {
    return &parser()._brnParser;
  }

  return nullptr;
}

void
XMLShoppingHandler::PROTypeParser::validateTicketingDate(const DateTime& ticketingDT)
{
  if (Global::allowHistorical() && allRequestsHistorical.getValue())
    return;

  if (!((trx().excTrxType() == PricingTrx::EXC_IS_TRX) ||
        (trx().getTrxType() == PricingTrx::RESHOP_TRX) ||
        (trx().getTrxType() == PricingTrx::MIP_TRX &&
         trx().excTrxType() == PricingTrx::AR_EXC_TRX)))
  {
    const DateTime currentDT = DateTime::localTime().subtractDays(1);
    const DateTime currentDTnoTime(currentDT.year(), currentDT.month(), currentDT.day());
    const bool validDT = ticketingDT >= currentDTnoTime;
    validateInput(validDT, "Ticketing date is from the past");
  }
}

void
XMLShoppingHandler::readShoppingConfigParameters(PricingTrx& pricingTrx)
{
  const int64_t minConnectionTimeDomestic = minimumConnectTimeDomestic.getValue();
  if (minConnectionTimeDomestic > 0)
  {
    pricingTrx.getOptions()->setMinConnectionTimeDomestic(minConnectionTimeDomestic *
                                                          SECONDS_IN_MINUTE);
  }
  const int64_t minConnectionTimeInternational = minimumConnectTimeInternational.getValue();
  if (minConnectionTimeInternational > 0)
  {
    pricingTrx.getOptions()->setMinConnectionTimeInternational(minConnectionTimeInternational *
                                                               SECONDS_IN_MINUTE);
  }
}

void
XMLShoppingHandler::determineCosExceptionFixEnabled(PricingTrx& pricingTrx)
{
  if (pricingTrx.getTrxType() != PricingTrx::MIP_TRX)
    return;

  if (!pricingTrx.billing() || pricingTrx.billing()->partitionID().empty())
    return;

  if (!fallback::cosExceptionFixDynamic(&pricingTrx))
  {
    const ConfigSet<std::string> partitionIDs(cosExceptionFixEnabledDyn.getValue(&pricingTrx));
    pricingTrx.setCosExceptionFixEnabled(partitionIDs.has("*") ||
                                         partitionIDs.has(pricingTrx.billing()->partitionID()));
    return;
  }

  const std::string& partitionID = pricingTrx.billing()->partitionID();
  const std::string configPartitionIDs = cosExceptionFixEnabled.getValue();

  if (configPartitionIDs == "*")
  {
    pricingTrx.setCosExceptionFixEnabled(true);
    return;
  }

  boost::char_separator<char> separator("|");
  boost::tokenizer<boost::char_separator<char>> tokenizer(configPartitionIDs, separator);

  for (const std::string& configPartID : tokenizer)
  {
    if (configPartID == partitionID)
    {
      pricingTrx.setCosExceptionFixEnabled(true);
      return;
    }
  }
}

bool
XMLShoppingHandler::PROTypeParser::isFamilyGroupingDisable(const ShoppingTrx& trx)
{
  return isFamilyGroupingDisableForPCC(trx.getRequest()->ticketingAgent()->tvlAgencyPCC()) ||
         trx.isExchangeTrx() || trx.getRequest()->isBrandedFaresRequest();
}

bool
XMLShoppingHandler::PROTypeParser::isFamilyGroupingDisableForPCC(const std::string& pcc)
{
  if (trx().getTrxType() != PricingTrx::IS_TRX || pcc.empty())
    return false;

  return _pccsWithoutFamilyLogic.getValue().has(pcc);
}

void
XMLShoppingHandler::updateDefaultFFGData(const flexFares::Attribute& aValue,
                                         const std::string& sValue)
{
  if (trx()->getTrxType() != PricingTrx::MIP_TRX || !trx()->isFlexFare())
    return;

  PricingRequest& request = *trx()->getRequest();
  switch (aValue)
  {
  case flexFares::CORP_IDS:
    if (!fallback::fallbackCorpIDFareBugFixing(trx()))
    {
      request.getMutableFlexFaresGroupsData().addCorpId(sValue, defaultFlexFaresGroupID(*trx()));
    }
    else if (_defaultFlexFaresGroupId != 0)
    {
      request.getMutableFlexFaresGroupsData().addCorpId(sValue, defaultFlexFaresGroupID(*trx()));
    }
    break;
  case flexFares::ACC_CODES:
    if (!fallback::fallbackCorpIDFareBugFixing(trx()))
    {
      request.getMutableFlexFaresGroupsData().addAccCode(sValue, defaultFlexFaresGroupID(*trx()));
    }
    else if (defaultFlexFaresGroupID(*trx()) != 0)
    {
      request.getMutableFlexFaresGroupsData().addAccCode(sValue, defaultFlexFaresGroupID(*trx()));
    }
    break;
  case flexFares::PUBLIC_FARES:
    request.getMutableFlexFaresGroupsData().setPublicFares(true, defaultFlexFaresGroupID(*trx()));
    break;
  case flexFares::PRIVATE_FARES:
    request.getMutableFlexFaresGroupsData().setPrivateFares(true, defaultFlexFaresGroupID(*trx()));
    break;
  case flexFares::PASSENGER_TYPE:
    request.getMutableFlexFaresGroupsData().setPaxTypeCode(static_cast<PaxTypeCode>(sValue),
                                                           defaultFlexFaresGroupID(*trx()));
    break;
  case flexFares::NO_ADVANCE_PURCHASE:
    request.getMutableFlexFaresGroupsData().requireNoAdvancePurchase(defaultFlexFaresGroupID(*trx()));
    break;
  case flexFares::NO_PENALTIES:
    request.getMutableFlexFaresGroupsData().requireNoPenalties(defaultFlexFaresGroupID(*trx()));
    break;
  case flexFares::NO_MIN_MAX_STAY:
    request.getMutableFlexFaresGroupsData().requireNoMinMaxStay(defaultFlexFaresGroupID(*trx()));
    break;
  default:
    break;
  }
}

void
XMLShoppingHandler::FFGTypeParser::startParsing(const IAttributes& attrs)
{
  PricingRequest& request = *trx().getRequest();
  PricingOptions& option = *trx().getOptions();
  parser().setValidFlexFareGroupCreated(false);

  if (trx().isMainFareFFGGroupIDZero())
    trx().setFlexFare(true);
  if (trx().getTrxType() != PricingTrx::MIP_TRX || !trx().isFlexFare())
  {
    return;
  }

  flexFares::GroupId& flexFaresGroupID = parser().flexFaresGroupID(trx());
  // Q17 - Flex Fare Group Id
  if (!attrs.has(_Q17))
  { // It doesn't have the flex fare group ID so it's invalid
    return;
  }
  else
  {
    attrs.get(_Q17, flexFaresGroupID);
    if (!request.getMutableFlexFaresGroupsData().isFlexFareGroup(flexFaresGroupID))
      request.getMutableFlexFaresGroupsData().createNewGroup(flexFaresGroupID);
    request.getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, flexFaresGroupID);
    parser().setValidFlexFareGroupCreated(true);
  }

  //The group in <PRO> is to be considered as Flex Group for JumpCabin Validation
  if (trx().isMainFareFFGGroupIDZero())
  {
    trx().setMainFareFFGGroupIDZero(false);
    if (request.getJumpCabinLogic() != JumpCabinLogic::ENABLED)
    {
      const flexFares::GroupId defaultFlexFaresGroupID = parser().defaultFlexFaresGroupID(trx());
      if (!request.getMutableFlexFaresGroupsData().isFlexFareGroup(defaultFlexFaresGroupID))
      {
        request.getMutableFlexFaresGroupsData().createNewGroup(defaultFlexFaresGroupID);
        request.getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, defaultFlexFaresGroupID);
      }
      if (request.getJumpCabinLogic() == JumpCabinLogic::DISABLED)
        request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
                flexFares::JumpCabinLogic::DISABLED, defaultFlexFaresGroupID);
      else
        request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
                flexFares::JumpCabinLogic::ONLY_MIXED, defaultFlexFaresGroupID);
    }
  }

  // PXC - Flex Fare Phase III XC indicator
  if (attrs.has(_PXC))
  {
    char xcIndicator = attrs.get<char>(_PXC, 0);
    request.getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(
        TypeConvert::pssCharToBool(xcIndicator), flexFaresGroupID);
  }

  // P20 - Flex Fare Group XO indicator
  if (attrs.has(_P20))
  {
    char xoIndicator = attrs.get<char>(_P20, 0);
    request.getMutableFlexFaresGroupsData().setFlexFareXOFares(xoIndicator, flexFaresGroupID);
  }

  // PTC - Passenger Type
  if (attrs.has(_PTC))
  {
    PaxTypeCode psgType;
    _getAttr(attrs, _PTC, psgType);
    request.getMutableFlexFaresGroupsData().setPaxTypeCode(psgType, flexFaresGroupID);
  }

  // B31 - Requested Flex Fare Group Cabin
  if (attrs.has(_B31))
  {
    char alfaNumCabin = attrs.get<char>(_B31, 0);
    request.getMutableFlexFaresGroupsData().setRequestedCabinFromAlphaNum(alfaNumCabin,
                                                                          flexFaresGroupID);
  }

  //Update setJumpCabinLogic status for the Flex Fare Groups
  //PXS Expand Jump Cabin Indicator for Flex Fare Group
  if (attrs.has(_PXS))
  {
    if(TypeConvert::pssCharToBool(attrs.get<char>(_PXS, 0)))
      request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
              flexFares::JumpCabinLogic::ONLY_MIXED, flexFaresGroupID);
  }

  //PXU Disable Jump Cabin Indicator for Flex Fare Group
  if (attrs.has(_PXU))
  {
    if(TypeConvert::pssCharToBool(attrs.get<char>(_PXU, 0)))
      request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
              flexFares::JumpCabinLogic::DISABLED, flexFaresGroupID);
    else
      request.getMutableFlexFaresGroupsData().setFFGJumpCabinLogic(
              flexFares::JumpCabinLogic::ENABLED, flexFaresGroupID);
  }

  //updating the jumpCabinLogic status if <PRO> and <FFG> have different jumpcabinlogic
  JumpCabinLogic jcl = JumpCabinLogic::ENABLED;
  flexFares::JumpCabinLogic ffgJCL =
             request.getMutableFlexFaresGroupsData().getFFGJumpCabinLogic(flexFaresGroupID);
  if (ffgJCL == flexFares::JumpCabinLogic::ONLY_MIXED)
    jcl = JumpCabinLogic::ONLY_MIXED;
  else if (ffgJCL == flexFares::JumpCabinLogic::DISABLED)
    jcl = JumpCabinLogic::DISABLED;
  if (jcl < request.getJumpCabinLogic())
    request.setJumpCabinLogic(jcl);

  // P1Y - Published/Public Fares
  if (attrs.has(_P1Y))
  {
    char publicFare = attrs.get<char>(_P1Y, 0);
    request.getMutableFlexFaresGroupsData().setPublicFares(TypeConvert::pssCharToBool(publicFare),
                                                           flexFaresGroupID);
    if (publicFare == 'T')
      option.privateFares() = 'F';
  }

  // P1Z - Private Fares
  if (attrs.has(_P1Z))
  {
    char privateFare = attrs.get<char>(_P1Z, 0);
    request.getMutableFlexFaresGroupsData().setPrivateFares(TypeConvert::pssCharToBool(privateFare),
                                                            flexFaresGroupID);
    if (privateFare == 'T')
      option.publishedFares() = 'F';
  }

  // FFR - Flex Fare Restrictions
  if (attrs.has(_FFR))
  {
    std::string stringFFR;
    attrs.get(_FFR, stringFFR);

    if (!stringFFR.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringFFR, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currFFR = ((std::string)tokenI->data());

        if (currFFR == "XA")
        {
          request.getMutableFlexFaresGroupsData().requireNoAdvancePurchase(flexFaresGroupID);
        }
        else if (currFFR == "XP")
        {
          request.getMutableFlexFaresGroupsData().requireNoPenalties(flexFaresGroupID);
        }
        else if (currFFR == "XS")
        {
          request.getMutableFlexFaresGroupsData().requireNoMinMaxStay(flexFaresGroupID);
        }
        else if (currFFR == "XR")
        {
          request.getMutableFlexFaresGroupsData().requireNoRestrictions(flexFaresGroupID);
        }
      }
    }
  }

  // ACX - Corp Ids
  if (attrs.has(_ACX))
  {
    std::string stringACX;
    attrs.get(_ACX, stringACX);

    if (!stringACX.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringACX, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currCorpId = ((std::string)tokenI->data());
        request.getMutableFlexFaresGroupsData().addCorpId(currCorpId, flexFaresGroupID);

        std::vector<std::string>& corpIdVec = request.corpIdVec();
        if (std::find(corpIdVec.begin(), corpIdVec.end(), currCorpId.c_str()) ==corpIdVec.end())
           corpIdVec.push_back(currCorpId.c_str());
      }
    }
  }

  // SMX - Accont codes
  if (attrs.has(_SMX))
  {
    std::string stringSMX;
    attrs.get(_SMX, stringSMX);

    if (!stringSMX.empty())
    {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      boost::char_separator<char> separator("|");
      tokenizer tokens(stringSMX, separator);
      tokenizer::iterator tokenI = tokens.begin();

      for (; tokenI != tokens.end(); ++tokenI)
      {
        std::string currAcctCode = ((std::string)tokenI->data());
        request.getMutableFlexFaresGroupsData().addAccCode(currAcctCode, flexFaresGroupID);
        std::vector<std::string>& accCodeVec = request.accCodeVec();
        if (std::find(accCodeVec.begin(), accCodeVec.end(), currAcctCode.c_str()) == accCodeVec.end())
          accCodeVec.push_back(currAcctCode.c_str());
      }
    }
  }

  if (attrs.get(_MPO) == "I")
    _ffgMPO = smp::INFO;
  else if (attrs.get(_MPO) == "O")
    _ffgMPO = smp::OR;
  else if (attrs.get(_MPO) == "A")
    _ffgMPO = smp::AND;
  else if (attrs.has(_MPO))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INCORRECT MAXIMUM PENALTY OPERATION");
}

XMLShoppingHandler::ElementParser*
XMLShoppingHandler::FFGTypeParser::newElement(int idx, const IAttributes& attrs)
{
  if (parser().isValidFlexFareGroupCreated() && _PEN == idx)
    return &parser()._penParser;

  return nullptr;
}

void
XMLShoppingHandler::FFGTypeParser::endParsing(const IValueString& text)
{
  if (parser().isValidFlexFareGroupCreated())
  {
    PricingRequest& request = *trx().getRequest();
    MaxPenaltyInfo* maxPenalty = CommonParserUtils::validateMaxPenaltyInput(trx(), std::move(_ffgMPO),
           std::move(parser()._penParser._changeFilter),std::move(parser()._penParser._refundFilter));
    request.getMutableFlexFaresGroupsData().setFFGMaxPenaltyInfo(maxPenalty, parser().flexFaresGroupID(trx()));

    parser()._penParser._changeFilter = MaxPenaltyInfo::Filter{smp::BOTH, {}, {}};
    parser()._penParser._refundFilter = MaxPenaltyInfo::Filter{smp::BOTH, {}, {}};
  }
}

flexFares::GroupId&
XMLShoppingHandler::defaultFlexFaresGroupID(PricingTrx& trx)
{
  if (fallback::fallbackFFGroupIdTypeChanges(&trx))
    return _defaultFlexFaresGroupId;

  return _defaultFlexFaresGroupID;
}

const flexFares::GroupId&
XMLShoppingHandler::defaultFlexFaresGroupID(PricingTrx& trx) const
{
  if (fallback::fallbackFFGroupIdTypeChanges(&trx))
    return _defaultFlexFaresGroupId;

  return _defaultFlexFaresGroupID;
}

flexFares::GroupId&
XMLShoppingHandler::flexFaresGroupID(PricingTrx& trx)
{
  if (fallback::fallbackFFGroupIdTypeChanges(&trx))
    return _flexFaresGroupId;

  return _flexFaresGroupID;
}

const flexFares::GroupId&
XMLShoppingHandler::flexFaresGroupID(PricingTrx& trx) const
{
  if (fallback::fallbackFFGroupIdTypeChanges(&trx))
    return _flexFaresGroupId;

  return _flexFaresGroupID;
}

} // end namespace tse);
