//----------------------------------------------------------------------------
//  File:        TaxShoppingRequestHandler.cpp
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

#include "Xform/TaxShoppingRequestHandler.h"

#include "Common/Assert.h"
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

#include <limits>

namespace tse
{
FIXEDFALLBACK_DECL(tmpFixForRequestThrottling);
FIXEDFALLBACK_DECL(fallbackTaxShoppingTransitTimes);
FIXEDFALLBACK_DECL(fallbackTaxShoppingStopoverGrouping);
FIXEDFALLBACK_DECL(fallbackTaxShoppingFirstTvlDateGrouping);
FIXEDFALLBACK_DECL(taxShoppingPfcInfant);
FIXEDFALLBACK_DECL(firstTvlDateRages);
FIXEDFALLBACK_DECL(repricingForTaxShopping);
FALLBACK_DECL(fallbackGBTaxEquipmentException);
FALLBACK_DECL(fallbackFixForRTPricingInSplit);
FALLBACK_DECL(markupAnyFareOptimization);
FALLBACK_DECL(fallbackTaxTrxTimeout);
FALLBACK_DECL(Taxes_BulkCharger_PassengerWithAge);
}

namespace tse
{
using namespace taxshopping;

namespace
{
void setDiscountData(TaxTrx* trx, PaxTypeFare* ptFare,
                     uint8_t discountCategory,
                     MoneyAmount discountPercent)
{
  if (discountCategory)
  {
    DiscountInfo* discountInfo = trx->dataHandle().create<DiscountInfo>();
    discountInfo->category() = discountCategory;
    discountInfo->discPercent() = discountPercent;

    PaxTypeFareRuleData* ruleData = trx->dataHandle().create<PaxTypeFareRuleData>();
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

void addIntToString(std::string& s, int value)
{
  constexpr std::size_t BUFFER_SIZE = std::numeric_limits<int>::digits10 + 1;
  char buffer[BUFFER_SIZE];
  ::snprintf(buffer, BUFFER_SIZE, "%d", value);
  s += buffer;
}

void addDoubleToString(std::string& s, double value)
{
  constexpr std::size_t BUFFER_SIZE = std::numeric_limits<double>::digits10 + 1;
  char buffer[BUFFER_SIZE];
  ::snprintf(buffer, BUFFER_SIZE, "%g", value);
  s += buffer;
}

} // namespace

TaxShoppingRequestHandler::TaxShoppingRequestHandler(Trx*& trx, const TaxShoppingConfig& taxCfg, bool throttled)
  : CommonRequestHandler(trx),
    _taxTrx(nullptr),
    _firstTvlDate(boost::gregorian::not_a_date_time),
    _lastTvlDate(boost::gregorian::not_a_date_time),
    _currentItin(nullptr),
    _currentFli(nullptr),
    _taxClassId(0),
    _currentItinId(0),
    _applyUSCAgrouping(true),
    _useFlightRanges(false),
    _taxShoppingCfg(taxCfg),
    _throttled(throttled)
{
}

TaxShoppingRequestHandler::~TaxShoppingRequestHandler() {}

void
TaxShoppingRequestHandler::parse(DataHandle& dataHandle, const std::string& content)
{
  parse(dataHandle, content.c_str());
}

void
TaxShoppingRequestHandler::parse(DataHandle& dataHandle, const char*& content)
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
}

void
TaxShoppingRequestHandler::createTransaction(DataHandle& dataHandle, const std::string& content)
{
  _trx = _taxTrx = dataHandle.create<TaxTrx>();

  _taxTrx->requestType() = OTA_REQUEST;
  _taxTrx->otaXmlVersion() = "2.0.0";
  _taxTrx->setShoppingPath(true);
  _taxTrx->otaRequestRootElementType() = "TRQ";

  TaxRequest* request = _taxTrx->dataHandle().create<TaxRequest>();
  request->ticketingDT() = DateTime::localTime();
  _taxTrx->setRequest(request);

  PricingOptions* options = _taxTrx->dataHandle().create<PricingOptions>();
  _taxTrx->setOptions(options);

  Agent* agent = _taxTrx->dataHandle().create<Agent>();
  _taxTrx->getRequest()->ticketingAgent() = agent;

  Billing* billing = _taxTrx->dataHandle().create<Billing>();
  _taxTrx->billing() = billing;

  _taxTrx->billing()->parentServiceName() = _taxTrx->billing()->getServiceName(Billing::SVC_TAX);
  _taxTrx->billing()->updateTransactionIds(_taxTrx->transactionId());
  _taxTrx->billing()->updateServiceNames(Billing::SVC_TAX);
}

bool
TaxShoppingRequestHandler::startElement(int idx, const IAttributes& attrs)
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
TaxShoppingRequestHandler::characters(const char* value, size_t length)
{
}

bool
TaxShoppingRequestHandler::endElement(int idx)
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
TaxShoppingRequestHandler::startElement(const IKeyString& name, const IAttributes& attrs)
{
  return true;
}

bool
TaxShoppingRequestHandler::endElement(const IKeyString& name)
{
  return true;
}

void
TaxShoppingRequestHandler::onStartAGI(const IAttributes& attrs)
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
TaxShoppingRequestHandler::onStartBIL(const IAttributes& attrs)
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
TaxShoppingRequestHandler::onStartPRO(const IAttributes& attrs)
{

  _taxTrx->setGroupItins(TrxUtil::isHpsDomGroupingEnabled(*_taxTrx) ||
                         TrxUtil::isHpsIntlGroupingEnabled(*_taxTrx));

  _useFlightRanges = TrxUtil::useHPSFlightRanges(*_taxTrx);

  if (attrs.has(_TRQ_D07))
  {
    std::string date;
    getAttr(attrs, _TRQ_D07, date);

    int minutes = 1;
    if (attrs.has(_TRQ_D54))
      minutes = attrs.get<int>(_TRQ_D54);

    _taxTrx->getRequest()->ticketingDT() = DateTime(date, minutes);
  }

  if (attrs.has(_TRQ_Q0S))
  {
    const size_t noOfItineraries = attrs.get<size_t>(_TRQ_Q0S, 0);
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
TaxShoppingRequestHandler::onStartDIA(const IAttributes& attrs)
{
  int diag(-1);
  attrs.get(_TRQ_Q0A, diag);

  _taxTrx->getRequest()->diagnosticNumber() = static_cast<int>(diag);

  _taxTrx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diag);
  _taxTrx->diagnostic().activate();
}

void
TaxShoppingRequestHandler::onStartARG(const IAttributes& attrs)
{
  const IValueString& nam = attrs.get(_TRQ_NAM), &val = attrs.get(_TRQ_VAL);
  std::string namStr(nam.c_str(), nam.length()), valStr(val.c_str(), val.length());

  _taxTrx->diagnostic().diagParamMap().insert(std::make_pair(namStr, valStr));
}

void
TaxShoppingRequestHandler::onStartCAL(const IAttributes& attrs)
{
  CAL* cal = _taxTrx->dataHandle().create<CAL>();
  const int32_t id = attrs.get<int32_t>(_TRQ_Q1A);

  StringBuf strBuf;
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
      cal->_discountCategory = attrs.get<int>(_TRQ_C2C);
      cal->_discountPercent = attrs.get<double>(_TRQ_C2P);
    }
  }

  cal->_equivalentAmount = attrs.get<double>(_TRQ_C5F);

  if (!fallback::markupAnyFareOptimization(_trx))
  {
    if (attrs.has(_TRQ_C5M))
    {
      const MoneyAmount fareWithMarkup = attrs.get<double>(_TRQ_C5M);
      cal->_markupAmount = fareWithMarkup - cal->_equivalentAmount;
    }
  }

  _fareComponentsMap[id] = cal;
}

void
TaxShoppingRequestHandler::onStartFLI(const IAttributes& attrs)
{
  FLI* fli = _taxTrx->dataHandle().create<FLI>();
  _currentFli = fli;
  std::string tempString;

  const int32_t id = attrs.get<int32_t>(_TRQ_Q1C);

  getAttr(attrs, _TRQ_A01, fli->_departureAirport);
  getAttr(attrs, _TRQ_A02, fli->_arrivalAirport);
  getAttr(attrs, _TRQ_B00, fli->_marketingCarrier);
  getAttr(attrs, _TRQ_B01, fli->_operatingCarrier);

  fli->_flightNumber = attrs.get<int>(_TRQ_Q0B);

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
TaxShoppingRequestHandler::onEndFLI()
{
  _currentFli = nullptr;
}

void
TaxShoppingRequestHandler::onStartDAT(const IAttributes& attrs)
{
  const int32_t id = attrs.get<int32_t>(_TRQ_Q1E);

  std::string tempString;
  getAttr(attrs, _TRQ_D01, tempString);
  boost::gregorian::date tvlDate = boost::gregorian::from_simple_string(tempString);
  _datesMap[id] = tvlDate;
  if (_firstTvlDate.is_not_a_date() || _firstTvlDate > tvlDate)
    _firstTvlDate = tvlDate;
  if (_lastTvlDate.is_not_a_date() || _lastTvlDate < tvlDate)
    _lastTvlDate = tvlDate;
}

void
TaxShoppingRequestHandler::onStartHSP(const IAttributes& attrs)
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
TaxShoppingRequestHandler::onStartCOM(const IAttributes& attrs)
{
  Itin* itin = _taxTrx->dataHandle().create<Itin>();
  _currentItin = itin;

  itin->sequenceNumber() = attrs.get<uint32_t>(_TRQ_Q1D);

  getAttr(attrs, _TRQ_B05, itin->ticketingCarrier());
  itin->validatingCarrier() = itin->ticketingCarrier();

  itin->originationCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  itin->calculationCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  itin->agentPCCOverride() = _taxTrx->getRequest()->ticketingAgent();
}

void
TaxShoppingRequestHandler::onStartTAX(const IAttributes& attrs)
{
  TSE_ASSERT(_currentItin && !_currentFarePathInfos.empty());

  const MoneyAmount taxAmount = attrs.get<double>(_TRQ_C6B);
  if (taxAmount > EPSILON)
  {
    uint16_t segStartId = std::numeric_limits<uint16_t>::max();
    uint16_t segEndId = std::numeric_limits<uint16_t>::max();

    if (attrs.has(_TRQ_C1S))
      segStartId = attrs.get<uint16_t>(_TRQ_C1S);

    if (attrs.has(_TRQ_C1E))
      segEndId = attrs.get<uint16_t>(_TRQ_C1E);

    StringBuf strBuf;
    getAttr(attrs, _TRQ_BC0, strBuf);
    getCurrentFarePathInfo().addTaxItem(taxAmount, strBuf.data(), segStartId, segEndId);
  }
}

void
TaxShoppingRequestHandler::generateUSCAKey(std::string& key)
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

  for (const FarePathInfo& farePathInfo: _currentFarePathInfos)
    createFarePathKey(farePathInfo, key);
}

void
TaxShoppingRequestHandler::setGeoTraveltype(FLI& fli)
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
TaxShoppingRequestHandler::generateIntlKey(std::string& key)
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
      addIntToString(key, airSeg->flightNumber());

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
    addIntToString(key, getFirstTvlDateApplicableTaxes(_currentItin->travelSeg().front()->origin()->nation(),
                                                       _currentItin->travelSeg(), _taxTrx->getRequest()->ticketingDT()));
    for (const TravelSeg* ts : _currentItin->travelSeg())
    {
      if (nationsFirstTktDate.emplace(ts->destination()->nation()).second)
      {
        addIntToString(key, getFirstTvlDateApplicableTaxes(ts->destination()->nation(),
                                                           _currentItin->travelSeg(),
                                                          _taxTrx->getRequest()->ticketingDT()));
      }
    }
  }

  if (!fallback::fixed::firstTvlDateRages())
  {
    key.append("|");
    key.append(getFirstTvlDateRangeString(_currentItin->travelSeg(), _taxTrx->getRequest()->ticketingDT()));
  }

  for (const FarePathInfo& farePathInfo: _currentFarePathInfos)
    createFarePathKey(farePathInfo, key);
}

int
TaxShoppingRequestHandler::getFirstTvlDateApplicableTaxes(const NationCode& nation,
                                                          const std::vector<TravelSeg*>& segments,
                                                          const DateTime& tktDT)
{
  const TaxFirstTravelDates& taxFirstTravelDates = getTaxFirstTravelDates(nation, tktDT);
  int c = 0;
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
  return c;
}

char
TaxShoppingRequestHandler::getSameDayRuleChar(const DateTime& d1,
                                              const DateTime& d2,
                                              const NationCodesVec& nations,
                                              const NationCode& nation) const
{
  const bool sameDay = d1.day() == d2.day();
  const bool sameDayRuleApplicable = !sameDay && std::binary_search(nations.begin(), nations.end(), nation);

  return sameDayRuleApplicable ? '1' : '0';
}

char
TaxShoppingRequestHandler::getTransitHoursChar(const NationCode& nation,
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
TaxShoppingRequestHandler::isNextStopoverRestr(const std::vector<TravelSeg*>& segments)
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
TaxShoppingRequestHandler::getRestrWithNextStopover(const NationCode& nation, const DateTime& tktDT)
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

const TaxShoppingRequestHandler::NationTransitTimes&
TaxShoppingRequestHandler::getNationTransitTimes(const NationCode& nation, const DateTime& tktDT)
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

const TaxShoppingRequestHandler::TaxFirstTravelDates&
TaxShoppingRequestHandler::getTaxFirstTravelDates(const NationCode& nation,
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
TaxShoppingRequestHandler::collectTaxFirstTravelDates(TaxFirstTravelDates& firstTravelDates,
                                                      const NationCode& nation,
                                                      const DateTime& tktDate) const
{
  ShoppingTaxUtil stu;
  stu.getTaxFirstTravelDates(firstTravelDates, _firstTvlDate, _lastTvlDate, nation, tktDate);
}

void
TaxShoppingRequestHandler::collectNationTransitTimes(NationTransitTimes& transitTime,
                                                     const NationCode& nation,
                                                     const DateTime& tktDate) const
{
  ShoppingTaxUtil stu;
  stu.getNationTransitTimes(transitTime._transitHours, transitTime._transitTotalMinutes, nation,
                            _taxShoppingCfg.roundTransitMinutesTaxCodes(), tktDate);

  // StopOvers:
  if (nation.equalToConst("US"))       // special case for tax AY with domestic->domestic transfer
    transitTime.insertTime(Hours(4));

  transitTime.insertTime(Hours(12));
  transitTime.insertTime(Hours(24));
}

std::string
TaxShoppingRequestHandler::getFirstTvlDateRangeString(const std::vector<TravelSeg*>& segments,
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
TaxShoppingRequestHandler::onEndCOM()
{
  if (!_currentItin->travelSeg().empty())
  {
    _currentItin->setTravelDate(_currentItin->travelSeg().at(0)->departureDT());
  }

  if (_taxTrx->isGroupItins())
  {
    //   Generate key for grouping
    std::string key;
    if (_applyUSCAgrouping)
      generateUSCAKey(key);
    else
      generateIntlKey(key);
    std::pair<KeyToTaxClassMap::iterator, bool> insResult =
        _keyToTaxClassId.emplace(std::move(key), _taxClassId);
    if (insResult.second) // new tax class
    {
      createFarePaths();

      _taxTrx->itin().push_back(_currentItin);
      ++_taxClassId;
    }

    _taxTrx->shoppingItinMapping()[_currentItinId] = insResult.first->second;
    _taxTrx->allItins().push_back(_currentItin);
    ++_currentItinId;
  }
  else
  {
    createFarePaths();
    _taxTrx->itin().push_back(_currentItin);
  }

  _currentItin = nullptr;
  _currentFarePathInfos.clear();
}

void
TaxShoppingRequestHandler::onStartPXI(const IAttributes& attrs)
{
  TSE_ASSERT(_currentItin);

  const MoneyAmount totalNUCAmount = attrs.get<double>(_TRQ_C5F);

  PaxType* paxType = nullptr;
  if (fallback::fixed::taxShoppingPfcInfant())
  {
    paxType = _taxTrx->dataHandle().create<PaxType>();

    PaxTypeInfo* paxTypeInfo = _taxTrx->dataHandle().create<PaxTypeInfo>();
    paxType->paxTypeInfo() = paxTypeInfo;

    getAttr(attrs, _TRQ_B70, paxType->paxType());
    if (!fallback::Taxes_BulkCharger_PassengerWithAge(_taxTrx))
    {
      PaxTypeUtil::parsePassengerWithAge(*paxType);
    }

    const uint16_t paxNumber = attrs.get<uint16_t>(_TRQ_Q0W);
    paxType->number() = paxNumber;

    _currentFarePathInfos.emplace_back(FarePathInfo{*paxType, paxNumber, totalNUCAmount});
  }
  else
  {
    PaxType pt;
    getAttr(attrs, _TRQ_B70, pt.paxType());
    if (!fallback::Taxes_BulkCharger_PassengerWithAge(_taxTrx))
    {
      PaxTypeUtil::parsePassengerWithAge(pt);
    }

    pt.number() = attrs.get<uint16_t>(_TRQ_Q0W);
    paxType = getPaxType(pt.paxType(), pt.number(), pt.age());

    _currentFarePathInfos.emplace_back(FarePathInfo{*paxType, pt.number(), totalNUCAmount});
  }

  if (_taxTrx->paxType().empty())
    _taxTrx->paxType().push_back(paxType);

  _currentItin->paxGroup().push_back(paxType);
}

void
TaxShoppingRequestHandler::onEndPXI()
{
}

void
TaxShoppingRequestHandler::onStartCID(const IAttributes& attrs)
{
  TSE_ASSERT(_currentItin && !_currentFarePathInfos.empty());

  const int32_t calId = attrs.get<int32_t>(_TRQ_Q1A);

  LocCode market1;
  LocCode market2;
  getAttr(attrs, _TRQ_A01, market1);
  getAttr(attrs, _TRQ_A02, market2);

  getCurrentFarePathInfo().addPaxTypeFare(calId, market1, market2);
}

void
TaxShoppingRequestHandler::onStartFID(const IAttributes& attrs)
{
  TSE_ASSERT(_currentItin);

  const int32_t id = attrs.get<int32_t>(_TRQ_Q1C);
  const int32_t depDateId = attrs.get<int32_t>(_TRQ_D01);

  int32_t arrDateId = depDateId;
  if (attrs.has(_TRQ_D02))
    arrDateId = attrs.get<int32_t>(_TRQ_D02);

  BookingCode bookingCode;
  getAttr(attrs, _TRQ_B30, bookingCode);

  AirSeg& airSeg = getOrCreateAirSeg(id, bookingCode, depDateId, arrDateId,
                                     _currentItin->travelSeg().size() + 1);
  _currentItin->travelSeg().push_back(&airSeg);
}

void
TaxShoppingRequestHandler::onStartDynamicConfig(const IAttributes& attrs)
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
TaxShoppingRequestHandler::onStartTRQ(const IAttributes& attrs)
{
  int timeout = 0;
  if (attrs.has(_TRQ_D70))
    attrs.get(_TRQ_D70, timeout);

  if (!fallback::fallbackTaxTrxTimeout(_taxTrx))
  {
    if (timeout > 0)
    {
      TrxAborter* const aborter = &_taxTrx->dataHandle().safe_create<TrxAborter>(_taxTrx);
      aborter->setTimeout(timeout);
      _taxTrx->aborter() = aborter;
    }
  }
}

void
TaxShoppingRequestHandler::onEndTRQ()
{
  if (_taxTrx && !_taxTrx->itin().empty() && !_taxTrx->itin().at(0)->travelSeg().empty())
  {
    _taxTrx->setTravelDate(TseUtil::getTravelDate(_taxTrx->itin().at(0)->travelSeg()));
  }
}

void TaxShoppingRequestHandler::createFarePaths()
{
  for (FarePathInfo& farePathInfo: _currentFarePathInfos)
  {
    FarePath& farePath = createFarePath(farePathInfo);
    _currentItin->farePath().push_back(&farePath);
  }
}

FarePath& TaxShoppingRequestHandler::createFarePath(const FarePathInfo& farePathInfo)
{
  TSE_ASSERT(_currentItin);

  FarePath* farePath = _taxTrx->dataHandle().create<FarePath>();

  farePath->itin() = _currentItin;
  farePath->setTotalNUCAmount(farePathInfo.getTotalNUCAmount());
  farePath->calculationCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  farePath->baseFareCurrency() = _taxTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  farePath->paxType() = &farePathInfo.getPaxType();

  for (const auto& taxItemInfo: farePathInfo.getTaxItems())
  {
    TaxItem* item = _taxTrx->dataHandle().create<TaxItem>();
    item->taxAmount() = taxItemInfo._money;
    item->taxCode() = taxItemInfo._taxCode;
    item->serviceFee() = true;
    if (taxItemInfo._segStartId != std::numeric_limits<uint16_t>::max())
    {
      item->setTravelSegStartIndex(taxItemInfo._segStartId);
      if (!fallback::fallbackFixForRTPricingInSplit(_taxTrx))
      {
        TSE_ASSERT(taxItemInfo._segStartId < _currentItin->travelSeg().size());
        item->setTravelSegStart(_currentItin->travelSeg()[taxItemInfo._segStartId]);
      }
    }
    if (taxItemInfo._segEndId != std::numeric_limits<uint16_t>::max())
    {
      item->setTravelSegEndIndex(taxItemInfo._segEndId);
      if (!fallback::fallbackFixForRTPricingInSplit(_taxTrx))
      {
        TSE_ASSERT(taxItemInfo._segEndId < _currentItin->travelSeg().size());
        item->setTravelSegEnd(_currentItin->travelSeg()[taxItemInfo._segEndId]);
      }
    }
    farePath->getMutableExternalTaxes().emplace_back(item);
  }

  for (const PaxTypeFareInfo& paxTypeFareInfo: farePathInfo.getPaxTypeFares())
  {
    PaxTypeFare& paxTypeFare = getOrCreatePaxTypeFare(paxTypeFareInfo._calId,
                                                      paxTypeFareInfo._market1,
                                                      paxTypeFareInfo._market2,
                                                      farePathInfo.getPaxType());
    FareUsage* fareUsage = _taxTrx->dataHandle().create<FareUsage>();
    fareUsage->paxTypeFare() = &paxTypeFare;

    PricingUnit* pricingUnit = _taxTrx->dataHandle().create<PricingUnit>();
    pricingUnit->fareUsage().push_back(fareUsage);

    farePath->pricingUnit().push_back(pricingUnit);
  }

  addFlightsToFareComponents(*farePath);

  return *farePath;
}

void
TaxShoppingRequestHandler::addFlightsToFareComponents(FarePath& farePath)
{
  TSE_ASSERT(_currentItin);

  std::vector<TravelSeg*>::iterator travelSegIter = _currentItin->travelSeg().begin();
  std::vector<TravelSeg*>::iterator travelSegIterE = _currentItin->travelSeg().end();

  for (PricingUnit* pricingUnit: farePath.pricingUnit())
  {
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

PaxType* TaxShoppingRequestHandler::getPaxType(const PaxTypeCode& paxTypeCode, uint16_t paxNumber, uint16_t age)
{
  if (_paxTypes.count(paxTypeCode))
  {
    _paxTypes[paxTypeCode]->number() = std::max(_paxTypes[paxTypeCode]->number(), paxNumber);
    return _paxTypes[paxTypeCode];
  }

  PaxType* paxType = _taxTrx->dataHandle().create<PaxType>();

  PaxTypeInfo* paxTypeInfo = _taxTrx->dataHandle().create<PaxTypeInfo>();
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

AirSeg& TaxShoppingRequestHandler::getOrCreateAirSeg(const int32_t fliId,
                                                     const BookingCode bookingCode,
                                                     const int32_t departureDateId,
                                                     const int32_t arrivalDateId,
                                                     const int16_t pnrSegment)
{
  const AirSegCacheKey airSegKey(fliId, bookingCode, departureDateId, arrivalDateId, pnrSegment);
  AirSeg*& airSeg = _airSegCache[airSegKey];
  if (!airSeg)
    airSeg = &createAirSeg(fliId, bookingCode, departureDateId, arrivalDateId, pnrSegment);

  return *airSeg;
}

AirSeg& TaxShoppingRequestHandler::createAirSeg(const int32_t fliId,
                                                const BookingCode bookingCode,
                                                const int32_t departureDateId,
                                                const int32_t arrivalDateId,
                                                const int16_t pnrSegment)
{
  const FLI* fli = _flightsMap[fliId];
  TSE_ASSERT(fli);

  AirSeg* airSeg = _taxTrx->dataHandle().create<AirSeg>();

  airSeg->origAirport() = fli->_departureAirport;
  airSeg->destAirport() = fli->_arrivalAirport;
  airSeg->equipmentType() = fli->_equipmentType;
  airSeg->setMarketingCarrierCode(fli->_marketingCarrier);
  airSeg->setOperatingCarrierCode(fli->_operatingCarrier);
  airSeg->marketingFlightNumber() = fli->_flightNumber;

  airSeg->departureDT() = DateTime(_datesMap[departureDateId], fli->_departureTime);
  airSeg->arrivalDT() = DateTime(_datesMap[arrivalDateId], fli->_arrivalTime);

  ClassOfService* classOfService = _taxTrx->dataHandle().create<ClassOfService>();
  classOfService->bookingCode() = bookingCode;
  airSeg->setBookingCode(bookingCode);
  airSeg->classOfService().push_back(classOfService);

  airSeg->origin() = fli->_origin;
  airSeg->destination() = fli->_destination;
  airSeg->boardMultiCity() = fli->_boardMultiCity;
  airSeg->offMultiCity() = fli->_offMultiCity;
  airSeg->geoTravelType() = fli->_geoTravelType;
  airSeg->resStatus() = CONFIRM_RES_STATUS;
  airSeg->pnrSegment() = pnrSegment;

  std::copy(fli->_hiddenStops.begin(),
            fli->_hiddenStops.end(),
            std::back_inserter(airSeg->hiddenStops()));

  return *airSeg;
}

PaxTypeFare& TaxShoppingRequestHandler::getOrCreatePaxTypeFare(const int32_t calId,
                                                               const LocCode market1,
                                                               const LocCode market2,
                                                               PaxType& paxType)
{
  const PaxTypeCacheKey paxTypeKey(calId, market1, market2, &paxType);
  PaxTypeFare*& paxTypeFare = _paxTypeFareCache[paxTypeKey];
  if (!paxTypeFare)
    paxTypeFare = &createPaxTypeFare(calId, market1, market2, paxType);

  return *paxTypeFare;
}


PaxTypeFare& TaxShoppingRequestHandler::createPaxTypeFare(const int32_t calId,
                                                          const LocCode market1,
                                                          const LocCode market2,
                                                          PaxType& paxType)
{
  const CAL* cal = _fareComponentsMap[calId];
  TSE_ASSERT(cal);

  FareInfo* fareInfo = _taxTrx->dataHandle().create<FareInfo>();
  fareInfo->directionality() = cal->_direction;
  fareInfo->owrt() = cal->_owrt;
  fareInfo->fareClass() = cal->_farebasisCode;

  fareInfo->market1() = market1;
  fareInfo->market2() = market2;

  TariffCrossRefInfo* tariffCrossRefInfo = _taxTrx->dataHandle().create<TariffCrossRefInfo>();

  Fare* fare = _taxTrx->dataHandle().create<Fare>();
  fare->nucFareAmount() = cal->_equivalentAmount;
  fare->nucMarkupAmount() = cal->_markupAmount;
  fare->initialize(Fare::FS_International, fareInfo, _fareMarket, tariffCrossRefInfo);
  if (!fallback::fixed::repricingForTaxShopping())
  {
    if (fareInfo->market1() > fareInfo->market2() &&
        fareInfo->directionality() == TO)
    {
      fare->status().set(Fare::FS_ReversedDirection);
    }
  }

  FareClassAppInfo* fareClassAppInfo = _taxTrx->dataHandle().create<FareClassAppInfo>();
  fareClassAppInfo->_fareType = cal->_fareType;

  PaxTypeFare* paxTypeFare = _taxTrx->dataHandle().create<PaxTypeFare>();
  paxTypeFare->fareClassAppInfo() = fareClassAppInfo;
  paxTypeFare->paxType() = &paxType;

  if (!fallback::fixed::taxShoppingPfcInfant())
  {
    if (cal->_paxType)
      paxTypeFare->paxType() = cal->_paxType;

    setDiscountData(_taxTrx, paxTypeFare, cal->_discountCategory, cal->_discountPercent);
  }

  paxTypeFare->setFare(fare);

  return *paxTypeFare;
}

TaxShoppingRequestHandler::FarePathInfo& TaxShoppingRequestHandler::getCurrentFarePathInfo()
{
  TSE_ASSERT(!_currentFarePathInfos.empty());
  return _currentFarePathInfos.back();
}

void TaxShoppingRequestHandler::createFarePathKey(const FarePathInfo& farePathInfo, std::string& result) const
{
  result += "|";
  result += farePathInfo.getPaxType().paxType();
  addIntToString(result, farePathInfo.getPaxTypeNumber());
  if (!_applyUSCAgrouping)
  {
    result += "|";
    addDoubleToString(result, farePathInfo.getTotalNUCAmount());
  }

  for (const PaxTypeFareInfo& paxTypeFareInfo: farePathInfo.getPaxTypeFares())
  {
    result += "|";
    addIntToString(result, paxTypeFareInfo._calId);
    result += paxTypeFareInfo._market1;
    result += paxTypeFareInfo._market2;
  }

  if (!_applyUSCAgrouping && !farePathInfo.getTaxItems().empty())
  {
    result += "|";
    for (const auto& taxInfo: farePathInfo.getTaxItems())
    {
      if (taxInfo._segStartId != std::numeric_limits<uint16_t>::max())
        addIntToString(result, taxInfo._segStartId);

      if (taxInfo._segEndId != std::numeric_limits<uint16_t>::max())
      {
        result += "-";
        addIntToString(result, taxInfo._segEndId);
      }

      result += taxInfo._taxCode;
      addDoubleToString(result, taxInfo._money);
      result += ";";
    }
  }
}

} // namespace tse
