//----------------------------------------------------------------------------
//  File:        TaxShoppingRequestHandlerOld.cpp
//  Created:     2012-05-28
//
//  Description: Shopping handler for charger tax requests
//
//  Updates:
//
//  Copyright Sabre 2012
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

#include "Xform/TaxShoppingRequestHandlerOld.h"

#include "Common/ClassOfService.h"
#include "Common/Config/ConfigMan.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingTaxUtil.h"
#include "Common/TaxShoppingConfig.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Nation.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Taxes/LegacyTaxes/TaxGB.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/DynamicConfig.h"
#include "Xform/TaxShoppingSchemaNames.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackTaxShoppingAmountGrouping);
FIXEDFALLBACK_DECL(fallbackSplitYQYR_TaxShop);
FIXEDFALLBACK_DECL(tmpFixForRequestThrottling);
FIXEDFALLBACK_DECL(fallbackTaxShoppingTransitTimes);
FIXEDFALLBACK_DECL(fallbackTaxShoppingStopoverGrouping);
FIXEDFALLBACK_DECL(fallbackTaxShoppingFirstTvlDateGrouping);
FIXEDFALLBACK_DECL(taxShoppingPfcInfant);
FIXEDFALLBACK_DECL(firstTvlDateRages);
FIXEDFALLBACK_DECL(repricingForTaxShopping);
FALLBACK_DECL(fallbackGBTaxEquipmentException);
FALLBACK_DECL(markupAnyFareOptimization);
FALLBACK_DECL(fallbackTaxTrxTimeout);
FALLBACK_DECL(Taxes_BulkCharger_PassengerWithAge);
}

namespace tse
{
using namespace taxshopping;

namespace
{

TaxItem* getFakeYQItem(TaxTrx* trx, MoneyAmount money, const TaxCode& taxCode,
                       const uint16_t segStartId, const uint16_t segEndId)
{
  TaxItem* item(nullptr);
  trx->dataHandle().get(item);
  if(item)
  {
    item->taxAmount() = money;
    item->taxCode() = taxCode;
    item->serviceFee() = true;
    item->setTravelSegStartIndex(segStartId);
    item->setTravelSegEndIndex(segEndId);
    return item;
  }

  return nullptr;
}

void setDiscountData(TaxTrx* trx, PaxTypeFare* ptFare,
                     uint8_t discountCategory,
                     MoneyAmount discountPercent)
{
  if (discountCategory)
  {
    DiscountInfo* discountInfo(nullptr);
    trx->dataHandle().get(discountInfo);
    discountInfo->category() = discountCategory;
    discountInfo->discPercent() = discountPercent;

    PaxTypeFareRuleData* ruleData(nullptr);
    trx->dataHandle().get(ruleData);
    ruleData->ruleItemInfo() = discountInfo;

    ptFare->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, trx->dataHandle(), ruleData);
    ptFare->status().set(PaxTypeFare::PTF_Discounted);

    // turn off revalidation
    ptFare->setCategoryProcessed(RuleConst::CHILDREN_DISCOUNT_RULE, true);
    ptFare->setCategoryProcessed(RuleConst::TOUR_DISCOUNT_RULE, true);
    ptFare->setCategoryProcessed(RuleConst::AGENTS_DISCOUNT_RULE, true);
    ptFare->setCategoryProcessed(RuleConst::OTHER_DISCOUNT_RULE, true);
  }
}


ILookupMap _elemLookupMapShopping, _attrLookupMapShopping;

bool is24HStopover(const std::vector<TravelSeg*>& segments)
{
  for (auto iSeg = segments.begin(); iSeg != segments.end() - 1; ++iSeg)
  {
    if ((*std::next(iSeg))->departureDT() - (*iSeg)->arrivalDT() > Hours(HOURS_PER_DAY))
      return true;
  }
  return false;
}

inline bool hasEquipmentException(const std::vector<TravelSeg*>& segments)
{
  bool nationGBFound = false;
  bool specialEquipmentFound = false;
  for (const auto* seg : segments)
  {
    if (!nationGBFound &&
        (seg->origin()->nation() == UNITED_KINGDOM || seg->destination()->nation() == UNITED_KINGDOM))
      nationGBFound = true;

    if (!specialEquipmentFound && TaxGB::isSpecialEquipment(seg->equipmentType()))
      specialEquipmentFound = true;
    if (nationGBFound && specialEquipmentFound)
      return true;
  }
  return false;
}
}

TaxShoppingRequestHandlerOld::TaxShoppingRequestHandlerOld(Trx*& trx, const TaxShoppingConfig& taxCfg, bool throttled)
  : CommonRequestHandler(trx),
    _taxTrx(nullptr),
    _firstTvlDate(boost::gregorian::not_a_date_time),
    _lastTvlDate(boost::gregorian::not_a_date_time),
    _currentItin(nullptr),
    _currentFarePath(nullptr),
    _currentFli(nullptr),
    _taxClassId(0),
    _currentItinId(0),
    _applyUSCAgrouping(true),
    _useFlightRanges(false),
    _taxShoppingCfg(taxCfg),
    _throttled(throttled)
{
}

TaxShoppingRequestHandlerOld::~TaxShoppingRequestHandlerOld() {}

void
TaxShoppingRequestHandlerOld::parse(DataHandle& dataHandle, const std::string& content)
{
  parse(dataHandle, content.c_str());
}

void
TaxShoppingRequestHandlerOld::parse(DataHandle& dataHandle, const char*& content)
{
  std::string emptyContent;
  createTransaction(dataHandle, emptyContent);

  IXMLUtils::initLookupMaps(_TaxShoppingElementNames,
                            _NoOfElementNames_,
                            _elemLookupMapShopping,
                            _TaxShoppingAttributeNames,
                            _NoOfAttributeNames_,
                            _attrLookupMapShopping);

  IValueString attrValueArray[_NoOfAttributeNames_];
  int attrRefArray[_NoOfAttributeNames_];
  IXMLSchema schema(_elemLookupMapShopping,
                    _attrLookupMapShopping,
                    _NoOfAttributeNames_,
                    attrValueArray,
                    attrRefArray,
                    true);

  if (!fallback::fixed::tmpFixForRequestThrottling() && _throttled)
    return;

  IParser parser(content, strlen(content), *this, schema);
  parser.parse();
  //  std::cout << "REDUCTION FACTOR: " << (float)_taxTrx->allItins().size()/_taxTrx->itin().size()
  // << std::endl;
}

void
TaxShoppingRequestHandlerOld::createTransaction(DataHandle& dataHandle, const std::string& content)
{
  _trx = _taxTrx = dataHandle.create<TaxTrx>();

  _taxTrx->requestType() = OTA_REQUEST;
  _taxTrx->otaXmlVersion() = "2.0.0";
  _taxTrx->setShoppingPath(true);
  _taxTrx->otaRequestRootElementType() = "TRQ";

  TaxRequest* request = nullptr;
  _taxTrx->dataHandle().get(request);
  request->ticketingDT() = DateTime::localTime();
  _taxTrx->setRequest(request);

  PricingOptions* options = nullptr;
  _taxTrx->dataHandle().get(options);
  _taxTrx->setOptions(options);

  Agent* agent = nullptr;
  _taxTrx->dataHandle().get(agent);
  _taxTrx->getRequest()->ticketingAgent() = agent;

  Billing* billing = nullptr;
  _taxTrx->dataHandle().get(billing);
  _taxTrx->billing() = billing;

  _taxTrx->billing()->parentServiceName() = _taxTrx->billing()->getServiceName(Billing::SVC_TAX);
  _taxTrx->billing()->updateTransactionIds(_taxTrx->transactionId());
  _taxTrx->billing()->updateServiceNames(Billing::SVC_TAX);
}

bool
TaxShoppingRequestHandlerOld::startElement(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case _TRQ:
    onStartTRQ(attrs);
    break;
  case _TRQ_AGI:
    onStartAGI(attrs);
    break;
  case _TRQ_BIL:
    onStartBIL(attrs);
    break;
  case _TRQ_PRO:
    onStartPRO(attrs);
    break;
  case _TRQ_CAL:
    onStartCAL(attrs);
    break;
  case _TRQ_FLI:
    onStartFLI(attrs);
    break;
  case _TRQ_DAT:
    onStartDAT(attrs);
    break;
  case _TRQ_HSP:
    onStartHSP(attrs);
    break;
  case _TRQ_COM:
    onStartCOM(attrs);
    break;
  case _TRQ_PXI:
    onStartPXI(attrs);
    break;
  case _TRQ_CID:
    onStartCID(attrs);
    break;
  case _TRQ_FID:
    onStartFID(attrs);
    break;
  case _TRQ_DIA:
    onStartDIA(attrs);
    break;
  case _TRQ_ARG:
    onStartARG(attrs);
    break;
  case _TRQ_TAX:
    onStartTAX(attrs);
    break;
  case _TRQ_DynamicConfig:
    onStartDynamicConfig(attrs);
    break;

  default:
    break;
  }

  return true;
}

void
TaxShoppingRequestHandlerOld::characters(const char* value, size_t length)
{
}

bool
TaxShoppingRequestHandlerOld::endElement(int idx)
{
  switch (idx)
  {
  case _TRQ:
    onEndTRQ();
    break;
  case _TRQ_COM:
    onEndCOM();
    break;
  case _TRQ_PXI:
    onEndPXI();
    break;
  case _TRQ_FLI:
    onEndFLI();
    break;
  default:
    break;
  }

  return true;
}

bool
TaxShoppingRequestHandlerOld::startElement(const IKeyString& name, const IAttributes& attrs)
{
  return true;
}

bool
TaxShoppingRequestHandlerOld::endElement(const IKeyString& name)
{
  return true;
}

void
TaxShoppingRequestHandlerOld::onStartAGI(const IAttributes& attrs)
{
  Agent* agent = _taxTrx->getRequest()->ticketingAgent();

  if (!agent)
  {
    return;
  }

  getAttr(attrs, _TRQ_A10, agent->agentCity());
  getAttr(attrs, _TRQ_A20, agent->tvlAgencyPCC());
  getAttr(attrs, _TRQ_A21, agent->mainTvlAgencyPCC());
  getAttr(attrs, _TRQ_C40, agent->currencyCodeAgent());
  getAttr(attrs, _TRQ_AB0, agent->tvlAgencyIATA());
  getAttr(attrs, _TRQ_AB1, agent->homeAgencyIATA());
  getAttr(attrs, _TRQ_A80, agent->airlineDept());
  getAttr(attrs, _TRQ_A90, agent->agentFunctions());
  getAttr(attrs, _TRQ_N0G, agent->agentDuty());
  getAttr(attrs, _TRQ_B00, agent->cxrCode());
  attrs.get(_TRQ_Q01, agent->coHostID(), agent->coHostID());

  const Loc* agentLocation = _taxTrx->dataHandle().getLoc(agent->agentCity(), time(nullptr));

  std::vector<Customer*> custList = _taxTrx->dataHandle().getCustomer(agent->tvlAgencyPCC());

  if (custList.empty())
  {
    agent->agentLocation() = agentLocation;

    if (!agent->agentLocation())
    {
      throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
    }

    return;
  }

  agent->agentTJR() = custList.front();

  LocCode agentCity;
  agentCity = _taxTrx->dataHandle().getMultiTransportCity(agent->agentCity());
  agent->agentLocation() = _taxTrx->dataHandle().getLoc(agentCity, time(nullptr));

  if (agent->agentLocation() && agentLocation)
  {
    if (agent->agentLocation()->nation() != agentLocation->nation())
    {
      agent->agentLocation() = agentLocation;
    }
  }

  if (!agent->agentLocation())
  {
    agent->agentLocation() = agentLocation;
  }

  if (!agent->agentLocation())
  {
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
  }
}

void
TaxShoppingRequestHandlerOld::onStartBIL(const IAttributes& attrs)
{
  std::string tempString("");

  getAttr(attrs, _TRQ_A20, _taxTrx->billing()->userPseudoCityCode());
  getAttr(attrs, _TRQ_A22, _taxTrx->billing()->aaaCity());
  getAttr(attrs, _TRQ_AA0, _taxTrx->billing()->aaaSine());
  getAttr(attrs, _TRQ_AE0, _taxTrx->billing()->partitionID());

  if (attrs.has(_TRQ_C00))
  {
    getAttr(attrs, _TRQ_C00, tempString);
    _taxTrx->billing()->parentTransactionID() = std::atoll(tempString.c_str());
  }

  if (attrs.has(_TRQ_C01))
  {
    getAttr(attrs, _TRQ_C01, tempString);
    _taxTrx->billing()->clientTransactionID() = std::atoll(tempString.c_str());
  }

  _taxTrx->billing()->parentServiceName() = "V2TAX";
}

void
TaxShoppingRequestHandlerOld::onStartPRO(const IAttributes& attrs)
{

  _taxTrx->setGroupItins(TrxUtil::isHpsDomGroupingEnabled(*_taxTrx) ||
                         TrxUtil::isHpsIntlGroupingEnabled(*_taxTrx));

  _useFlightRanges = TrxUtil::useHPSFlightRanges(*_taxTrx);

  if (attrs.has(_TRQ_D07))
  {
    std::string date = "";
    getAttr(attrs, _TRQ_D07, date);

    std::string minutes = "1";

    if (attrs.has(_TRQ_D54))
    {
      getAttr(attrs, _TRQ_D54, minutes);
    }

    _taxTrx->getRequest()->ticketingDT() = DateTime(date, atoi(minutes.c_str()));
  }

  if (attrs.has(_TRQ_Q0S))
  {
    std::string noOfItinerariesStr = "0";
    getAttr(attrs, _TRQ_Q0S, noOfItinerariesStr);

    size_t noOfItineraries = std::atol(noOfItinerariesStr.c_str());
    _taxTrx->itin().reserve(noOfItineraries);
    if (_taxTrx->isGroupItins())
    {
      _taxTrx->allItins().reserve(noOfItineraries);
      _taxTrx->shoppingItinMapping().resize(noOfItineraries, -1);
      _keyToTaxClassId.rehash(noOfItineraries);
    }
  }

  attrs.get(_TRQ_P0J,
            _taxTrx->getRequest()->electronicTicket(),
            _taxTrx->getRequest()->electronicTicket());

  if (attrs.has(_TRQ_PFC))
  {
    bool calcPFC = true;
    attrs.get<bool>(_TRQ_PFC, calcPFC);

    _taxTrx->getOptions()->setCalcPfc(calcPFC);
  }

  if (_taxTrx->isGroupItins())
  {
    if (attrs.has(_TRQ_HGI))
    {
      bool applyUSCAgrouping = true;
      attrs.get<bool>(_TRQ_HGI, applyUSCAgrouping);
      _applyUSCAgrouping = applyUSCAgrouping;
    }
    if (_applyUSCAgrouping)
    {
      if (!TrxUtil::isHpsDomGroupingEnabled(*_taxTrx))
        _taxTrx->setGroupItins(false);
    }
    else
    {
      if (!TrxUtil::isHpsIntlGroupingEnabled(*_taxTrx))
        _taxTrx->setGroupItins(false);
    }
  }
  if (attrs.has(_TRQ_AF0))
  {
    std::string af0Val;
    attrs.get<std::string>(_TRQ_AF0, af0Val);
    _taxTrx->getRequest()->salePointOverride() = af0Val;
  }
  if (attrs.has(_TRQ_AG0))
  {
    std::string ag0Val;
    attrs.get<std::string>(_TRQ_AG0, ag0Val);
    _taxTrx->getRequest()->ticketPointOverride() = ag0Val;
  }

  if (attrs.has(_TRQ_C48))
  {
    std::string c48Val;
    attrs.get<std::string>(_TRQ_C48, c48Val);
    _taxTrx->getOptions()->currencyOverride() = c48Val;
    _taxTrx->getOptions()->mOverride() = 'T';
  }

  if (!fallback::fixed::repricingForTaxShopping())
  {
    if (attrs.has(_TRQ_RPC))
    {
      bool enableRepricing(true);
      attrs.get<bool>(_TRQ_RPC, enableRepricing);

      _taxTrx->setRepricingForTaxShopping(enableRepricing);
    }
  }
}

void
TaxShoppingRequestHandlerOld::onStartDIA(const IAttributes& attrs)
{
  int diag(-1);
  attrs.get(_TRQ_Q0A, diag);

  _taxTrx->getRequest()->diagnosticNumber() = static_cast<int>(diag);

  _taxTrx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diag);
  _taxTrx->diagnostic().activate();
}

void
TaxShoppingRequestHandlerOld::onStartARG(const IAttributes& attrs)
{
  const IValueString& nam = attrs.get(_TRQ_NAM), &val = attrs.get(_TRQ_VAL);
  std::string namStr(nam.c_str(), nam.length()), valStr(val.c_str(), val.length());

  _taxTrx->diagnostic().diagParamMap().insert(std::make_pair(namStr, valStr));
}

void
TaxShoppingRequestHandlerOld::onStartCAL(const IAttributes& attrs)
{
  CAL* cal = nullptr;
  _taxTrx->dataHandle().get(cal);
  int32_t id = -1;
  StringBuf strBuf;

  getAttr(attrs, _TRQ_Q1A, strBuf);
  id = std::atoi(strBuf.data());

  getAttr(attrs, _TRQ_B50, cal->_farebasisCode);
  getAttr(attrs, _TRQ_S53, cal->_fareType);

  getAttr(attrs, _TRQ_P04, strBuf);

  if (strBuf.front() == '1')
  {
    cal->_owrt = ONE_WAY_MAY_BE_DOUBLED;
  }
  else if (strBuf.front() == '2')
  {
    cal->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
  }
  else if (strBuf.front() == '3')
  {
    cal->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
  }

  getAttr(attrs, _TRQ_S70, strBuf);

  if (strBuf.front() == 'F')
  {
    cal->_direction = FROM;
  }
  else if (strBuf.front() == 'T')
  {
    cal->_direction = TO;
  }
  else if (strBuf.front() == 'B')
  {
    cal->_direction = BOTH;
  }

  if (!fallback::fixed::taxShoppingPfcInfant())
  {
    if (attrs.has(_TRQ_C70))
    {
      getAttr(attrs, _TRQ_C70, strBuf);
      cal->_paxType = getPaxType(strBuf.data(), 1);
    }

    if (attrs.has(_TRQ_C2C) && attrs.has(_TRQ_C2P))
    {
      getAttr(attrs, _TRQ_C2C, strBuf);
      cal->_discountCategory = std::atoi(strBuf.data());
      getAttr(attrs, _TRQ_C2P, strBuf);
      cal->_discountPercent = std::atof(strBuf.data());
    }
  }

  getAttr(attrs, _TRQ_C5F, strBuf);
  cal->_equivalentAmount = std::atof(strBuf.data());

  if (!fallback::markupAnyFareOptimization(_trx))
  {
    if (attrs.has(_TRQ_C5M))
    {
      getAttr(attrs, _TRQ_C5M, strBuf);
      const MoneyAmount fareWithMarkup = std::atof(strBuf.data());
      cal->_markupAmount = fareWithMarkup - cal->_equivalentAmount;
    }
  }



  _fareComponentsMap[id] = cal;
}

void
TaxShoppingRequestHandlerOld::onStartFLI(const IAttributes& attrs)
{
  FLI* fli = nullptr;
  _taxTrx->dataHandle().get(fli);
  _currentFli = fli;
  int32_t id = -1;
  std::string tempString;

  getAttr(attrs, _TRQ_Q1C, tempString);
  id = std::atoi(tempString.c_str());

  getAttr(attrs, _TRQ_A01, fli->_departureAirport);
  getAttr(attrs, _TRQ_A02, fli->_arrivalAirport);
  getAttr(attrs, _TRQ_B00, fli->_marketingCarrier);
  getAttr(attrs, _TRQ_B01, fli->_operatingCarrier);

  getAttr(attrs, _TRQ_Q0B, tempString);
  fli->_flightNumber = std::atoi(tempString.c_str());

  getAttr(attrs, _TRQ_B40, fli->_equipmentType);
  getAttr(attrs, _TRQ_D31, tempString);
  fli->_departureTime = boost::posix_time::duration_from_string(tempString);
  getAttr(attrs, _TRQ_D32, tempString);
  fli->_arrivalTime = boost::posix_time::duration_from_string(tempString);

  fli->_origin =
      _taxTrx->dataHandle().getLoc(fli->_departureAirport, _taxTrx->getRequest()->ticketingDT());
  fli->_destination =
      _taxTrx->dataHandle().getLoc(fli->_arrivalAirport, _taxTrx->getRequest()->ticketingDT());

  setGeoTraveltype(*fli);

  const std::vector<tse::MultiTransport*>& boardMACList =
      _trx->dataHandle().getMultiTransportCity(fli->_departureAirport,
                                               fli->_marketingCarrier,
                                               fli->_geoTravelType,
                                               _taxTrx->getRequest()->ticketingDT());

  fli->_boardMultiCity =
      boardMACList.empty() ? fli->_departureAirport : boardMACList.front()->multitranscity();

  const std::vector<tse::MultiTransport*>& offMACList =
      _trx->dataHandle().getMultiTransportCity(fli->_arrivalAirport,
                                               fli->_marketingCarrier,
                                               fli->_geoTravelType,
                                               _taxTrx->getRequest()->ticketingDT());

  fli->_offMultiCity =
      offMACList.empty() ? fli->_arrivalAirport : offMACList.front()->multitranscity();

  _flightsMap[id] = fli;
}

void
TaxShoppingRequestHandlerOld::onEndFLI()
{
  _currentFli = nullptr;
}

void
TaxShoppingRequestHandlerOld::onStartDAT(const IAttributes& attrs)
{
  std::string tempString;

  getAttr(attrs, _TRQ_Q1E, tempString);
  int32_t id = std::atoi(tempString.c_str());

  getAttr(attrs, _TRQ_D01, tempString);

  boost::gregorian::date tvlDate = boost::gregorian::from_simple_string(tempString);
  _datesMap[id] = tvlDate;
  if (_firstTvlDate.is_not_a_date() || _firstTvlDate > tvlDate)
    _firstTvlDate = tvlDate;
  if (_lastTvlDate.is_not_a_date() || _lastTvlDate < tvlDate)
    _lastTvlDate = tvlDate;
}

void
TaxShoppingRequestHandlerOld::onStartHSP(const IAttributes& attrs)
{
  if (!_currentFli)
  {
    return;
  }

  LocCode hsp;
  getAttr(attrs, _TRQ_A00, hsp);
  const Loc* location = _taxTrx->dataHandle().getLoc(hsp, _taxTrx->getRequest()->ticketingDT());
  if (location)
    _currentFli->_hiddenStops.push_back(location);
}

void
TaxShoppingRequestHandlerOld::onStartCOM(const IAttributes& attrs)
{
  Itin* itin = nullptr;
  _taxTrx->dataHandle().get(itin);
  _currentItin = itin;

  StringBuf strBuf;

  getAttr(attrs, _TRQ_Q1D, strBuf);
  itin->sequenceNumber() = std::atol(strBuf.data());

  getAttr(attrs, _TRQ_B05, itin->ticketingCarrier());
  itin->validatingCarrier() = itin->ticketingCarrier();

  itin->originationCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  itin->calculationCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  itin->agentPCCOverride() = _taxTrx->getRequest()->ticketingAgent();
}

void
TaxShoppingRequestHandlerOld::onStartTAX(const IAttributes& attrs)
{
  if (!_currentItin || !_currentFarePath)
    return;

  StringBuf strBuf;
  getAttr(attrs, _TRQ_C6B, strBuf);
  MoneyAmount taxAmount(std::atof(strBuf.data()));
  if (taxAmount > EPSILON)
  {
    uint16_t segStartId(0), segEndId(0);
    std::string segRange("");
    if (!fallback::fixed::fallbackSplitYQYR_TaxShop())
    {
      if (attrs.has(_TRQ_C1S))
      {
        getAttr(attrs, _TRQ_C1S, strBuf);
        segRange = std::string(strBuf.data());
        segStartId = std::atoi(strBuf.data());
      }

      if (attrs.has(_TRQ_C1E))
      {
        getAttr(attrs, _TRQ_C1E, strBuf);
        segRange += std::string("-") + strBuf.data();
        segEndId = std::atoi(strBuf.data());
      }
    }

    getAttr(attrs, _TRQ_BC0, strBuf);
    if (TaxItem* item = getFakeYQItem(_taxTrx, taxAmount, strBuf.data(), segStartId, segEndId))
    {
      _currentFarePath->getMutableExternalTaxes().push_back(item);

      if (_taxTrx->isGroupItins() && !_applyUSCAgrouping)
      {
        _taxMap[segRange + strBuf.data()] += taxAmount;
      }
    }
  }
}

void
TaxShoppingRequestHandlerOld::generateUSCAKey(std::string& key)
{
  // Generate key for grouping
  std::string lastArrival(_currentItin->travelSeg().front()->origAirport().c_str());
  key.reserve(256);
  key = _currentItin->travelSeg().front()->origAirport();
  for (std::vector<TravelSeg*>::const_iterator iSeg = _currentItin->travelSeg().begin();
       iSeg != _currentItin->travelSeg().end();
       ++iSeg)
  {
    AirSeg* airSeg = static_cast<AirSeg*>(*iSeg);
    if (lastArrival != airSeg->origAirport())
    {
      key.append("-//-");
      key.append(airSeg->origAirport().c_str());
    }
    if (!airSeg->hiddenStops().empty())
    {
      for (std::vector<const Loc*>::const_iterator hs = airSeg->hiddenStops().begin();
           hs != airSeg->hiddenStops().end();
           ++hs)
      {
        key.append("-");
        key.append(airSeg->marketingCarrierCode().c_str());
        key.append("-h");
        key.append((*hs)->loc());
      }
    }
    key.append("-");
    key.append(airSeg->marketingCarrierCode().c_str());
    key.append("-");
    key.append(airSeg->destAirport().c_str());
    lastArrival = airSeg->destAirport();
  }
  // stopover vs connection
  key.append("|");
  for (std::vector<TravelSeg*>::const_iterator iSeg = _currentItin->travelSeg().begin();
       iSeg != _currentItin->travelSeg().end() - 1;
       ++iSeg)
  {
    TravelSeg* curSeg = *iSeg;
    TravelSeg* nextSeg = *(iSeg + 1);
    DateTime refDateTime = curSeg->arrivalDT();
    refDateTime += Hours(4); // A regular domestic stopover time limit
    if (nextSeg->departureDT() <= refDateTime)
    {
      key.append("X");
    }
    else
    {
      refDateTime +=
          Hours(8); // So now it equals to 12 hours, which is a stopover time limit for some taxes
      if (nextSeg->departureDT() <= refDateTime)
        key.append("s");
      else
        key.append("S");
    }
  }
  for (std::vector<TravelSeg*>::const_iterator iSeg = _currentItin->travelSeg().begin();
       iSeg != _currentItin->travelSeg().end();
       ++iSeg)
  {
    AirSeg* airSeg = static_cast<AirSeg*>(*iSeg);
    char AYkey[32];
    ::snprintf(AYkey,
               32,
               "%s%s%s%s%d",
               _currentItin->validatingCarrier().c_str(),
               airSeg->origAirport().c_str(),
               airSeg->marketingCarrierCode().c_str(),
               airSeg->destAirport().c_str(),
               airSeg->flightNumber());
    AYApplicationCache::const_iterator itAY = _ayCache.find(AYkey);
    bool AYapplies =
        (itAY != _ayCache.end())
            ? itAY->second
            : _ayCache.emplace(AYkey, ShoppingTaxUtil::doesAYApply(*iSeg, _currentItin, *_taxTrx))
                  .first->second;
    key.append(AYapplies ? "1" : "0");
  }
  key.append(_currentFPKey);
}

void
TaxShoppingRequestHandlerOld::setGeoTraveltype(FLI& fli)
{
  if (LocUtil::isDomestic(*fli._origin, *fli._destination))
    fli._geoTravelType = GeoTravelType::Domestic;
  else if (LocUtil::isInternational(*fli._origin, *fli._destination))
    fli._geoTravelType = GeoTravelType::International;
  else if (LocUtil::isTransBorder(*fli._origin, *fli._destination))
    fli._geoTravelType = GeoTravelType::Transborder;
  else if (LocUtil::isForeignDomestic(*fli._origin, *fli._destination))
    fli._geoTravelType = GeoTravelType::ForeignDomestic;
}

void
TaxShoppingRequestHandlerOld::generateIntlKey(std::string& key)
{

  std::set<NationCode> dateDepTaxNationsFound;
  std::set<NationCode> fltNoDepTaxNationsFound;

  // Generate key for grouping
  std::string lastArrival(_currentItin->travelSeg().front()->origAirport().c_str());
  key.reserve(256);
  key = _currentItin->travelSeg().front()->origAirport();

  const NationCodesVec& tvlDateDepTaxNations = _taxShoppingCfg.getTvlDateDepTaxNations();
  const NationCodesVec& fltNoDepTaxNations = _taxShoppingCfg.getFltNoDepTaxNations();

  for (TravelSeg* seg : _currentItin->travelSeg())
  {
    AirSeg* airSeg = static_cast<AirSeg*>(seg);

    if (std::binary_search(
            tvlDateDepTaxNations.begin(), tvlDateDepTaxNations.end(), airSeg->origin()->nation()))
      dateDepTaxNationsFound.insert(airSeg->origin()->nation());
    if (std::binary_search(tvlDateDepTaxNations.begin(),
                           tvlDateDepTaxNations.end(),
                           airSeg->destination()->nation()))
      dateDepTaxNationsFound.insert(airSeg->destination()->nation());

    if (_useFlightRanges)
    {
      if (std::binary_search(
              fltNoDepTaxNations.begin(), fltNoDepTaxNations.end(), airSeg->origin()->nation()))
        fltNoDepTaxNationsFound.insert(airSeg->origin()->nation());
      if (std::binary_search(fltNoDepTaxNations.begin(),
                             fltNoDepTaxNations.end(),
                             airSeg->destination()->nation()))
        fltNoDepTaxNationsFound.insert(airSeg->destination()->nation());
    }
    if (lastArrival != airSeg->origAirport())
    {
      key.append("-//-");
      key.append(airSeg->origAirport().c_str());
    }
    if (!airSeg->hiddenStops().empty())
    {
      for (std::vector<const Loc*>::const_iterator hs = airSeg->hiddenStops().begin();
           hs != airSeg->hiddenStops().end();
           ++hs)
      {
        key.append("-");
        key.append(airSeg->marketingCarrierCode().c_str());
        key.append("-h");
        key.append((*hs)->loc());
      }
    }
    key.append("-");
    key.append(airSeg->marketingCarrierCode().c_str());
    if (!_useFlightRanges)
    {
      char fltNoBuf[8];
      ::snprintf(fltNoBuf, 8, "%d", airSeg->flightNumber());
      key.append(fltNoBuf);
    }
    key.append("-");
    key.append(airSeg->destAirport().c_str());
    lastArrival = airSeg->destAirport();
  }

  if (!dateDepTaxNationsFound.empty())
  {
    for (const auto nation : dateDepTaxNationsFound)
    {
      std::map<NationCode, ShoppingTaxUtil::DateSegmentation*>::iterator dsIt =
          _dateSegmenters.find(nation);
      ShoppingTaxUtil::DateSegmentation* ds = nullptr;
      if (dsIt == _dateSegmenters.end())
      {
        ds = new ShoppingTaxUtil::DateSegmentation(nation, _taxTrx->getRequest()->ticketingDT());
        _trx->dataHandle().deleteList().adopt(ds);
        _dateSegmenters.insert(std::make_pair(nation, ds));
      }
      else
        ds = dsIt->second;

      ds->buildDateSegmentKey(*_currentItin, key);
    }
  }

  if (!fltNoDepTaxNationsFound.empty())
  {
    for (const auto nation : fltNoDepTaxNationsFound)
    {
      std::map<NationCode, ShoppingTaxUtil::FlightRanges*>::iterator fsIt =
          _fltNoSegmenters.find(nation);
      ShoppingTaxUtil::FlightRanges* fs = nullptr;
      if (fsIt == _fltNoSegmenters.end())
      {
        fs = new ShoppingTaxUtil::FlightRanges(nation, _taxTrx->getRequest()->ticketingDT());
        _trx->dataHandle().deleteList().adopt(fs);
        fs->initFlightRanges();
        _fltNoSegmenters.insert(std::make_pair(nation, fs));
      }
      else
        fs = fsIt->second;

      fs->buildFltRangeKey(*_currentItin, key);
    }
  }

  // stopover vs connection
  key.append("|");
  for (std::vector<TravelSeg*>::const_iterator iSeg = _currentItin->travelSeg().begin();
       iSeg != _currentItin->travelSeg().end() - 1;
       ++iSeg)
  {
    TravelSeg* curSeg = *iSeg;
    TravelSeg* nextSeg = *(iSeg + 1);

    const char h = getTransitHoursChar(curSeg->destination()->nation(),
                                       curSeg->arrivalDT(),
                                       nextSeg->departureDT(),
                                       _taxTrx->getRequest()->ticketingDT());
    key += h;
  }

  if (!fallback::fixed::fallbackTaxShoppingStopoverGrouping())
  {
    //nextstopover
    key.append("|");
    key += isNextStopoverRestr(_currentItin->travelSeg()) && is24HStopover(_currentItin->travelSeg()) ? 'Y' : 'N';
  }

  if (!fallback::fallbackGBTaxEquipmentException(_trx))
  {
    key.append("|");
    key.append(hasEquipmentException(_currentItin->travelSeg()) ? "Y" : "N");
  }

  // same day rule
  key.append("|");
  for (std::vector<TravelSeg*>::const_iterator iSeg = _currentItin->travelSeg().begin();
       iSeg != _currentItin->travelSeg().end() - 1;
       ++iSeg)
  {
    TravelSeg* curSeg = *iSeg;
    TravelSeg* nextSeg = *(iSeg + 1);

    key += getSameDayRuleChar(curSeg->arrivalDT(), nextSeg->departureDT(), _taxShoppingCfg.getSameDayDepTaxNations(), curSeg->destination()->nation());
  }

  if (!fallback::fixed::fallbackTaxShoppingFirstTvlDateGrouping())
  {
    //firsttvldate
    key.append("|");
    std::set<NationCode> nationsFirstTktDate;
    nationsFirstTktDate.emplace(_currentItin->travelSeg().front()->origin()->nation());
    key += getFirstTvlDateApplicableTaxes(_currentItin->travelSeg().front()->origin()->nation(), _currentItin->travelSeg(), _taxTrx->getRequest()->ticketingDT());
    for (const TravelSeg* ts : _currentItin->travelSeg())
    {
      if (nationsFirstTktDate.emplace(ts->destination()->nation()).second)
        key += getFirstTvlDateApplicableTaxes(ts->destination()->nation(), _currentItin->travelSeg(), _taxTrx->getRequest()->ticketingDT());
    }
  }

  if (!fallback::fixed::firstTvlDateRages())
  {
    key.append("|");
    key.append(getFirstTvlDateRangeString(_currentItin->travelSeg(), _taxTrx->getRequest()->ticketingDT()));
  }

  key.append(_currentFPKey);
}

std::string
TaxShoppingRequestHandlerOld::getFirstTvlDateApplicableTaxes(const NationCode& nation,
                                                             const std::vector<TravelSeg*>& segments,
                                                             const DateTime& tktDT)
{
  const TaxFirstTravelDates& taxFirstTravelDates = getTaxFirstTravelDates(nation, tktDT);
  size_t c = 0;
  for (const boost::gregorian::date firstTvlDate : taxFirstTravelDates)
  {
    for (const TravelSeg* ts : segments)
    {
      if (ts->departureDT().date() >= firstTvlDate )
      {
        ++c;
        break;
      }
    }
  }
  return std::to_string(c);
}

char
TaxShoppingRequestHandlerOld::getSameDayRuleChar(const DateTime& d1,
                                                 const DateTime& d2,
                                                 const NationCodesVec& nations,
                                                 const NationCode& nation) const
{
  const bool sameDay = d1.day() == d2.day();
  const bool sameDayRuleApplicable = !sameDay && std::binary_search(nations.begin(), nations.end(), nation);

  return sameDayRuleApplicable ? '1' : '0';
}

char
TaxShoppingRequestHandlerOld::getTransitHoursChar(const NationCode& nation,
                                                  const DateTime& arrivalDT,
                                                  const DateTime& departureDT,
                                                  const DateTime& tktDT)
{
  const NationTransitTimes& transitTime = getNationTransitTimes(nation, tktDT);

  char h = 'a';
  if (fallback::fixed::fallbackTaxShoppingTransitTimes())
  {
    const std::set<Hours>& transitHours = transitTime._transitHours;
    std::set<Hours>::const_iterator hoursIt = transitHours.begin();
    for (; hoursIt != transitHours.end(); ++hoursIt, ++h)
    {
      if (departureDT <= arrivalDT + *hoursIt)
        break;
    }
  }
  else
  {
    const std::set<Minutes>& transitTotalMinutes = transitTime._transitTotalMinutes;
    std::set<Minutes>::const_iterator minutesIt = transitTotalMinutes.begin();
    for (; minutesIt != transitTotalMinutes.end(); ++minutesIt, ++h)
    {
      if (departureDT <= arrivalDT + *minutesIt)
        break;
    }
  }
  return h;
}

bool
TaxShoppingRequestHandlerOld::isNextStopoverRestr(const std::vector<TravelSeg*>& segments)
{
  if (getRestrWithNextStopover(segments.front()->origin()->nation(), _taxTrx->getRequest()->ticketingDT()))
    return true;

  for (const auto& seg : segments)
  {
    if (getRestrWithNextStopover(seg->destination()->nation(), _taxTrx->getRequest()->ticketingDT()))
      return true;
  }

  return false;
}

bool
TaxShoppingRequestHandlerOld::getRestrWithNextStopover(const NationCode& nation, const DateTime& tktDT)
{
  NationWithNextStopoverRestrMap::const_iterator it = _nationWithNextStopoverRestr.find(nation);
  if (it == _nationWithNextStopoverRestr.end())
  {
    ShoppingTaxUtil stu;
    const bool restrictionExists = stu.getRestrWithNextStopover(nation, tktDT);
    _nationWithNextStopoverRestr.emplace(nation, restrictionExists);
    return restrictionExists;
  }

  return it->second;
}

const TaxShoppingRequestHandlerOld::NationTransitTimes&
TaxShoppingRequestHandlerOld::getNationTransitTimes(const NationCode& nation, const DateTime& tktDT)
{
  NationTransitTimesMap::const_iterator it = _nationTransitTimes.find(nation);
  if (it == _nationTransitTimes.end())
  {
    NationTransitTimesMap::iterator newIt =
        _nationTransitTimes.insert(NationTransitTimesMap::value_type(nation, NationTransitTimes{}))
            .first;

    collectNationTransitTimes(newIt->second, nation, tktDT);
    return newIt->second;
  }

  return it->second;
}

const TaxShoppingRequestHandlerOld::TaxFirstTravelDates&
TaxShoppingRequestHandlerOld::getTaxFirstTravelDates(const NationCode& nation,
                                                     const DateTime& tktDate)
{
  auto it = _nationTaxFirstTravelDatesMap.find(nation);
  if (it == _nationTaxFirstTravelDatesMap.end())
  {
    auto newIt = _nationTaxFirstTravelDatesMap.emplace(nation, TaxFirstTravelDates()).first;

    collectTaxFirstTravelDates(newIt->second, nation, tktDate);
    return newIt->second;
  }

  return it->second;
}

void
TaxShoppingRequestHandlerOld::collectTaxFirstTravelDates(TaxFirstTravelDates& firstTravelDates,
                                                         const NationCode& nation,
                                                         const DateTime& tktDate) const
{
  ShoppingTaxUtil stu;
  stu.getTaxFirstTravelDates(firstTravelDates, _firstTvlDate, _lastTvlDate, nation, tktDate);
}

void
TaxShoppingRequestHandlerOld::collectNationTransitTimes(NationTransitTimes& transitTime,
                                                        const NationCode& nation,
                                                        const DateTime& tktDate) const
{
  ShoppingTaxUtil stu;
  stu.getNationTransitTimes(transitTime._transitHours, transitTime._transitTotalMinutes, nation,
                            _taxShoppingCfg.roundTransitMinutesTaxCodes(), tktDate);

  // StopOvers:
  if (nation == "US")       // special case for tax AY with domestic->domestic transfer
    transitTime.insertTime(Hours(4));

  transitTime.insertTime(Hours(12));
  transitTime.insertTime(Hours(24));
}

std::string
TaxShoppingRequestHandlerOld::getFirstTvlDateRangeString(const std::vector<TravelSeg*>& segments,
                                                         const DateTime& tktDT)
{
  TaxFirstTravelDates firstTvlDates;
  NationSet nations;

  for (const TravelSeg* const tvl : segments)
  {
    nations.insert(tvl->origin()->nation());
  }

  using MapValue = NationFirstTvlDatesMap::value_type;
  using MapIterator = NationFirstTvlDatesMap::iterator;
  std::pair<MapIterator, bool> result = _nationsFirstTvlDates.emplace(MapValue(nations, TaxFirstTravelDates()));

  if (result.second)
  {
    for (const NationCode& nation : result.first->first)
    {
      const TaxFirstTravelDates& taxFirstTvlDates = getTaxFirstTravelDates(nation, tktDT);
      result.first->second.insert(taxFirstTvlDates.begin(), taxFirstTvlDates.end());
    }
  }

  std::string key;
  key.reserve(segments.size());
  for (const TravelSeg* ts : segments)
  {
    char c = 'a';
    for (const boost::gregorian::date firstTvlDate : result.first->second)
    {
      if (ts->departureDT().date() < firstTvlDate )
      {
        key.push_back(c);
        break;
      }
      ++c;
    }
  }

  return key;
}

void
TaxShoppingRequestHandlerOld::onEndCOM()
{
  if (!_currentItin->travelSeg().empty())
  {
    _currentItin->setTravelDate(_currentItin->travelSeg().at(0)->departureDT());
  }

  addFlightsToFareComponents();

  if (_taxTrx->isGroupItins())
  {
    //   Generate key for grouping
    std::string key;
    if (_applyUSCAgrouping)
      generateUSCAKey(key);
    else
      generateIntlKey(key);
    std::pair<KeyToTaxClassMap::iterator, bool> insResult =
        _keyToTaxClassId.emplace(key, _taxClassId);
    if (insResult.second) // new tax class
    {
      _taxTrx->itin().push_back(_currentItin);
      ++_taxClassId;
    }
    _taxTrx->shoppingItinMapping()[_currentItinId] = insResult.first->second;
    _taxTrx->allItins().push_back(_currentItin);
    ++_currentItinId;

    //    std::cout << _currentItin->sequenceNumber() <<  ": "  <<
    //        key << " : " << _keyToTaxClassId[key] << std::endl;
  }
  else
  {
    _taxTrx->itin().push_back(_currentItin);
  }

  _currentItin = nullptr;
  _currentFarePath = nullptr;
}

void
TaxShoppingRequestHandlerOld::onStartPXI(const IAttributes& attrs)
{
  if (!_currentItin)
  {
    return;
  }

  _taxMap.clear();
  FarePath* farePath = nullptr;
  _taxTrx->dataHandle().get(farePath);
  _currentFarePath = farePath;

  farePath->itin() = _currentItin;
  farePath->calculationCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  farePath->baseFareCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();

  StringBuf strBuf;
  PaxType* paxType = nullptr;

  if (fallback::fixed::taxShoppingPfcInfant())
  {
    _taxTrx->dataHandle().get(paxType);

    PaxTypeInfo* paxTypeInfo = nullptr;
    _taxTrx->dataHandle().get(paxTypeInfo);
    paxType->paxTypeInfo() = paxTypeInfo;

    getAttr(attrs, _TRQ_B70, paxType->paxType());
    if (!fallback::Taxes_BulkCharger_PassengerWithAge(_taxTrx))
    {
      PaxTypeUtil::parsePassengerWithAge(*paxType);
    }

    getAttr(attrs, _TRQ_Q0W, strBuf);
    paxType->number() = std::atoi(strBuf.data());
  }
  else
  {
    PaxType pt;
    getAttr(attrs, _TRQ_B70, pt.paxType());
    if (!fallback::Taxes_BulkCharger_PassengerWithAge(_taxTrx))
    {
      PaxTypeUtil::parsePassengerWithAge(pt);
    }

    getAttr(attrs, _TRQ_Q0W, strBuf);
    pt.number() = std::atoi(strBuf.data());
    paxType = getPaxType(pt.paxType(), pt.number(), pt.age());
  }

  if (_taxTrx->isGroupItins())
  {
    _currentFPKey = "|";
    _currentFPKey.append(paxType->paxType());
    _currentFPKey.append(strBuf.data());
  }
  getAttr(attrs, _TRQ_C5F, strBuf);
  farePath->setTotalNUCAmount(std::atof(strBuf.data()));

  if (!fallback::fixed::fallbackTaxShoppingAmountGrouping())
  {
    if (_taxTrx->isGroupItins() && !_applyUSCAgrouping)
    {
      _currentFPKey.append("|");
      _currentFPKey.append(strBuf.data());
    }
  }

  if (_taxTrx->paxType().empty())
  {
    _taxTrx->paxType().push_back(paxType);
  }

  _currentItin->paxGroup().push_back(paxType);
  farePath->paxType() = paxType;

  _currentItin->farePath().push_back(farePath);
}

void
TaxShoppingRequestHandlerOld::onEndPXI()
{
  if (_taxTrx->isGroupItins() && !_applyUSCAgrouping
      && !_taxMap.empty())
  {
    std::string key("");
    for (TaxMap::value_type val : _taxMap)
    {
      key += val.first + boost::lexical_cast<std::string>(val.second);
    }
    _currentFPKey.append("|");
    _currentFPKey.append(key);
    _taxMap.clear();
  }
}

void
TaxShoppingRequestHandlerOld::onStartCID(const IAttributes& attrs)
{
  if (!_currentItin)
  {
    return;
  }

  if (!_currentFarePath)
  {
    return;
  }

  StringBuf strBuf;

  getAttr(attrs, _TRQ_Q1A, strBuf);
  int id = std::atoi(strBuf.data());

  CAL* cal = _fareComponentsMap[id];

  FareInfo* fareInfo;
  _taxTrx->dataHandle().get(fareInfo);
  fareInfo->directionality() = cal->_direction;
  fareInfo->owrt() = cal->_owrt;
  fareInfo->fareClass() = cal->_farebasisCode;

  getAttr(attrs, _TRQ_A01, fareInfo->market1());
  getAttr(attrs, _TRQ_A02, fareInfo->market2());

  TariffCrossRefInfo* tariffCrossRefInfo;
  _taxTrx->dataHandle().get(tariffCrossRefInfo);

  Fare* fare = nullptr;
  _taxTrx->dataHandle().get(fare);
  fare->nucFareAmount() = cal->_equivalentAmount;
  fare->initialize(Fare::FS_International, fareInfo, _fareMarket, tariffCrossRefInfo);
  if (!fallback::fixed::repricingForTaxShopping())
  {
    if (fareInfo->market1() > fareInfo->market2() &&
        fareInfo->directionality() == TO)
    {
      fare->status().set(Fare::FS_ReversedDirection);
    }
  }

  FareClassAppInfo* fareClassAppInfo = nullptr;
  _taxTrx->dataHandle().get(fareClassAppInfo);
  fareClassAppInfo->_fareType = cal->_fareType;

  PaxTypeFare* paxTypeFare;
  _taxTrx->dataHandle().get(paxTypeFare);
  paxTypeFare->fareClassAppInfo() = fareClassAppInfo;
  paxTypeFare->paxType() = _currentItin->paxGroup().at(0);

  if (!fallback::fixed::taxShoppingPfcInfant())
  {
    if (cal->_paxType)
      paxTypeFare->paxType() = cal->_paxType;

    setDiscountData(_taxTrx, paxTypeFare, cal->_discountCategory, cal->_discountPercent);
  }

  paxTypeFare->setFare(fare);

  FareUsage* fareUsage = nullptr;
  _taxTrx->dataHandle().get(fareUsage);
  fareUsage->paxTypeFare() = paxTypeFare;

  PricingUnit* pricingUnit = nullptr;
  _taxTrx->dataHandle().get(pricingUnit);
  pricingUnit->fareUsage().push_back(fareUsage);

  _currentFarePath->pricingUnit().push_back(pricingUnit);

  if (_taxTrx->isGroupItins())
  {
    _currentFPKey.append("|");
    _currentFPKey.append(strBuf.data());
    _currentFPKey.append(fareInfo->market1());
    _currentFPKey.append(fareInfo->market2());
  }
}

void
TaxShoppingRequestHandlerOld::onStartFID(const IAttributes& attrs)
{
  if (!_currentItin)
  {
    return;
  }

  StringBuf strBuf;

  getAttr(attrs, _TRQ_Q1C, strBuf);
  int32_t id = std::atoi(strBuf.data());

  getAttr(attrs, _TRQ_D01, strBuf);
  int32_t depDateId = std::atoi(strBuf.data());

  int32_t arrDateId = depDateId;
  if (attrs.has(_TRQ_D02))
  {
    getAttr(attrs, _TRQ_D02, strBuf);
    arrDateId = std::atoi(strBuf.data());
  }

  BookingCode bookingCode;
  getAttr(attrs, _TRQ_B30, bookingCode);

  FLI* fli = _flightsMap[id];

  AirSeg* airSeg = nullptr;
  _taxTrx->dataHandle().get(airSeg);

  airSeg->origAirport() = fli->_departureAirport;
  airSeg->destAirport() = fli->_arrivalAirport;
  airSeg->equipmentType() = fli->_equipmentType;
  airSeg->setMarketingCarrierCode(fli->_marketingCarrier);
  airSeg->setOperatingCarrierCode(fli->_operatingCarrier);
  airSeg->marketingFlightNumber() = fli->_flightNumber;

  const boost::gregorian::date& depDate = _datesMap[depDateId];
  const boost::posix_time::time_duration& depTime = fli->_departureTime;
  airSeg->departureDT() = DateTime(depDate, depTime);

  const boost::gregorian::date& arrDate = _datesMap[arrDateId];
  const boost::posix_time::time_duration& arrTime = fli->_arrivalTime;
  airSeg->arrivalDT() = DateTime(arrDate, arrTime);

  ClassOfService* classOfService = nullptr;
  _taxTrx->dataHandle().get(classOfService);
  classOfService->bookingCode() = bookingCode;
  airSeg->setBookingCode(bookingCode);
  airSeg->classOfService().push_back(classOfService);

  airSeg->origin() = fli->_origin;
  airSeg->destination() = fli->_destination;
  airSeg->boardMultiCity() = fli->_boardMultiCity;
  airSeg->offMultiCity() = fli->_offMultiCity;
  airSeg->geoTravelType() = fli->_geoTravelType;
  airSeg->resStatus() = CONFIRM_RES_STATUS;
  airSeg->pnrSegment() = _currentItin->travelSeg().size() + 1;

  std::copy(fli->_hiddenStops.begin(),
            fli->_hiddenStops.end(),
            std::back_inserter(airSeg->hiddenStops()));

  _currentItin->travelSeg().push_back(airSeg);
}

void
TaxShoppingRequestHandlerOld::onStartDynamicConfig(const IAttributes& attrs)
{
  DynamicConfigHandler handler(*_trx);
  if (!handler.check())
    return;

  DynamicConfigInput input;

  if (attrs.has(_TRQ_Name))
    input.setName(attrs.get<std::string>(_TRQ_Name));
  if (attrs.has(_TRQ_Value))
    input.setValue(attrs.get<std::string>(_TRQ_Value));
  if (attrs.has(_TRQ_Substitute))
    input.setSubstitute(attrs.get<std::string>(_TRQ_Substitute));
  if (attrs.has(_TRQ_Optional))
    input.setOptional(attrs.get<std::string>(_TRQ_Optional));

  handler.process(input);
}

void
TaxShoppingRequestHandlerOld::onStartTRQ(const IAttributes& attrs)
{
  int timeout = 0;
  if (attrs.has(_TRQ_D70))
    attrs.get(_TRQ_D70, timeout);

  if (!fallback::fallbackTaxTrxTimeout(_taxTrx))
  if (timeout > 0)
  {
    TrxAborter* const aborter = &_taxTrx->dataHandle().safe_create<TrxAborter>(_taxTrx);
    aborter->setTimeout(timeout);
    _taxTrx->aborter() = aborter;
  }
}

void
TaxShoppingRequestHandlerOld::onEndTRQ()
{
  if (_taxTrx && !_taxTrx->itin().empty() && !_taxTrx->itin().at(0)->travelSeg().empty())
  {
    _taxTrx->setTravelDate(TseUtil::getTravelDate(_taxTrx->itin().at(0)->travelSeg()));
  }
}

void
TaxShoppingRequestHandlerOld::addFlightsToFareComponents()
{
  if (!_currentItin)
  {
    return;
  }

  if (!_currentFarePath)
  {
    return;
  }

  std::vector<PricingUnit*>::iterator pricingUnitsIter = _currentFarePath->pricingUnit().begin();
  std::vector<PricingUnit*>::iterator pricingUnitsIterE = _currentFarePath->pricingUnit().end();

  std::vector<TravelSeg*>::iterator travelSegIter = _currentItin->travelSeg().begin();
  std::vector<TravelSeg*>::iterator travelSegIterE = _currentItin->travelSeg().end();

  for (; pricingUnitsIter != pricingUnitsIterE; ++pricingUnitsIter)
  {
    PricingUnit* pricingUnit = *pricingUnitsIter;
    FareUsage* fareUsage = pricingUnit->fareUsage().at(0);
    PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();

    bool startFound = false;

    for (; travelSegIter != travelSegIterE; ++travelSegIter)
    {
      TravelSeg* travelSeg = *travelSegIter;

      if (travelSeg->origAirport() == paxTypeFare->market1())
      {
        startFound = true;
      }

      if (startFound)
      {
        travelSeg->fareBasisCode() = paxTypeFare->fareClass().c_str();
        fareUsage->travelSeg().push_back(travelSeg);
      }

      if (travelSeg->destAirport() == paxTypeFare->market2())
      {
        break;
      }
    }
  }
}

PaxType* TaxShoppingRequestHandlerOld::getPaxType(const PaxTypeCode& paxTypeCode, uint16_t paxNumber, uint16_t age)
{
  if (_paxTypes.count(paxTypeCode))
  {
    _paxTypes[paxTypeCode]->number() = std::max(_paxTypes[paxTypeCode]->number(), paxNumber);
    return _paxTypes[paxTypeCode];
  }

  PaxType* paxType = nullptr;
  _taxTrx->dataHandle().get(paxType);

  PaxTypeInfo* paxTypeInfo = nullptr;
  _taxTrx->dataHandle().get(paxTypeInfo);
  paxType->paxTypeInfo() = paxTypeInfo;

  paxType->paxType() = paxTypeCode;
  paxType->number() = paxNumber;
  paxType->age() = age;

  PaxTypeUtil::initialize(*_taxTrx, *paxType,
                          paxType->paxType(), paxType->number(), paxType->age(),
                          paxType->stateCode(), uint16_t(_taxTrx->paxType().size() + 1));

  _paxTypes[paxTypeCode] = paxType;

  return paxType;

}

}
