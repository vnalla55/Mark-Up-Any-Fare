#include "Xform/BaggageRequestHandler.h"

#include "Common/MCPCarrierUtil.h"
#include "Common/RtwUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/TravelSeg.h"
#include "Xform/AncillarySchemaNames.h"
#include "Xform/CustomXMLParser/IAttributes.h"


#include <limits>

namespace tse
{

using namespace ancillary;

void
BaggageRequestHandler::parse(DataHandle& dataHandle, const std::string& content)
{
  CommonRequestHandler::parse(dataHandle, content, *this);

  if (!_ffd.empty())
  {
    typedef std::pair<const Itin*, std::vector<PaxType*> > PairType;

    for (const PairType& pair : _request->paxTypesPerItin())
    {
      for (PaxType* paxType : pair.second)
      {
        paxType->freqFlyerTierWithCarrier().insert(
            paxType->freqFlyerTierWithCarrier().begin(), _ffd.begin(), _ffd.end());
      }
    }
  }
}

bool
BaggageRequestHandler::startElement(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case _AGI:
    onStartAGI(attrs);
    break;
  case _BIL:
    onStartBIL(attrs);
    break;
  case _PRO:
    onStartPRO(attrs);
    break;
  case _FFY:
    onStartFFY(attrs);
    break;
  case _DIG:
    onStartDIG(attrs);
    break;
  case _ITN:
    onStartITN(attrs);
    break;
  case _IRO:
    onStartIRO(attrs);
    break;
  case _SGI:
    onStartSGI(attrs);
    break;
  case _FBI:
    onStartFBI(attrs);
    break;
  case _PXI:
    onStartPXI(attrs);
    break;
  case _ACI:
    onStartACI(attrs);
    break;
  case _CII:
    onStartCII(attrs);
    break;
  case _FLI:
    onStartFLI(attrs);
    break;
  case _FBA:
    onStartFBA(attrs);
    break;
  case _HSP:
    onStartHSP(attrs);
    break;
  case _EQP:
    onStartEQP(attrs);
    break;
  case _DynamicConfig:
    onStartDynamicConfig(attrs);
    break;
  case _BaggageRequest:
    onStartBaggageRequest(attrs);
    break;
  }
  return true;
}

bool
BaggageRequestHandler::endElement(int idx)
{
  switch (idx)
  {
  case _AGI:
    onEndAGI();
    break;
  case _BIL:
    onEndBIL();
    break;
  case _FFY:
    onEndFFY();
    break;
  case _DIG:
    onEndDIG();
    break;
  case _ITN:
    onEndITN();
    break;
  case _IRO:
    onEndIRO();
    break;
  case _SGI:
    onEndSGI();
    break;
  case _FBI:
    onEndFBI();
    break;
  case _PXI:
    onEndPXI();
    break;
  case _ACI:
    onEndACI();
    break;
  case _CII:
    onEndCII();
    break;
  case _FLI:
    onEndFLI();
    break;
  case _FBA:
    onEndFBA();
    break;
  case _BaggageRequest:
    onEndBaggageRequest();
    break;
  }

  _value.clear();
  return true;
}

void
BaggageRequestHandler::createTransaction(DataHandle& dataHandle, const std::string& content)
{
  _trx = _pricingTrx = dataHandle.create<BaggageTrx>();
  _request = dataHandle.create<AncRequest>();
  _pricingTrx->setRequest(_request);
  _request->ticketingAgent() = dataHandle.create<Agent>();
  _pricingTrx->setOptions(dataHandle.create<PricingOptions>());
  _pricingTrx->billing() = dataHandle.create<Billing>();

  setDate(DateTime::localTime());
}

bool
BaggageRequestHandler::serviceSpecyficRtwProperty() const
{
  return RtwUtil::isRtwAncillaryRequest(*_request);
}

void
BaggageRequestHandler::onStartPRO(const IAttributes& attrs)
{
  // AF0 - Ticketing point override
  getAttr(attrs, _AF0, _request->ticketPointOverride());

  // AG0 - Sale point override
  getAttr(attrs, _AG0, _request->salePointOverride());

  // PS1 - hard match indicator
  attrs.get(_PS1, _request->hardMatchIndicator(), false);

  // D07
  if (attrs.has(_D07))
  {
    std::string ticketingDate;

    getAttr(attrs, _D07, ticketingDate);
    setDate(convertDate(ticketingDate));
  }

  // C45 - Equiv amount currency code
  std::string currencyOverride;

  getAttr(attrs, _C45, currencyOverride);
  _pricingTrx->getOptions()->currencyOverride() = currencyOverride;
  checkCurrency(_pricingTrx->getOptions()->currencyOverride());

  if (!_pricingTrx->getOptions()->currencyOverride().empty())
    _pricingTrx->getOptions()->mOverride() = 'T';

  // TKI - Ticketing Indicator
  attrs.get(_TKI, _pricingTrx->getOptions()->isTicketingInd(), false);

  // C10 - Diagnostic number
  const int diagnosticNumber = attrs.get<int>(_C10, 0);

  _request->diagnosticNumber() = diagnosticNumber;
  _pricingTrx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diagnosticNumber);
  _pricingTrx->diagnostic().activate();

  std::string eprKeys;
  getAttr(attrs, _S15, eprKeys);

  boost::tokenizer<> tok(eprKeys);

  for (boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); it++)
    _pricingTrx->getOptions()->eprKeywords().insert(*it);

  _request->setSpecificAgencyText(attrs.get<bool>(_SAT, false));
}

void
BaggageRequestHandler::onStartFLI(const IAttributes& attrs)
{
  // Q0B - Flight Number
  const int flightNumber = attrs.get<int>(_Q0B, 0);
  _currentTvlSeg->marketingFlightNumber() = flightNumber;

  if (!flightNumber)
    _currentTvlSeg->segmentType() = Open;

  // A01 - Departure Airport Code
  getAttr(attrs, _A01, _currentTvlSeg->origAirport());

  // A02 - Arrival Airport Code
  getAttr(attrs, _A02, _currentTvlSeg->destAirport());

  if (_currentTvlSeg->origAirport().empty() || _currentTvlSeg->destAirport().empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - MISSING CITY");

  const std::string depTime = attrs.get<std::string>(_D31, "");
  const std::string arrTime = attrs.get<std::string>(_D32, "");

  _currentTvlSeg->pssDepartureTime() = pssTime(depTime);
  _currentTvlSeg->pssArrivalTime() = pssTime(arrTime);

  // D01 - Departure Date
  const std::string departureDT = attrs.get<std::string>(_D01, "");

  // D02 - Arrival Date
  const std::string arrivalDT = attrs.get<std::string>(_D02, departureDT);

  setSegmentDates(departureDT, depTime, arrivalDT, arrTime);

  // B00 - Marketing carrier code
  const CarrierCode marketingCxr = attrs.get<std::string>(_B00, "");

  if (marketingCxr.empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MISSING CARRIER");

  _currentTvlSeg->setMarketingCarrierCode(MCPCarrierUtil::swapToActual(_trx, marketingCxr));

  // B01 - Operating carrier code
  // if no operating carrier, will use marketing carrier
  const CarrierCode operatingCxr = attrs.get<std::string>(_B01, marketingCxr);

  _currentTvlSeg->setOperatingCarrierCode(MCPCarrierUtil::swapToActual(_trx, operatingCxr));

  // B30 - Class of Service
  const BookingCode bookingCode = attrs.get<std::string>(_B30, "");

  if (!bookingCode.empty())
    _currentTvlSeg->setBookingCode(bookingCode);

  // S95
  const EquipmentType equipType = attrs.get<std::string>(_S95, "");

  if (!equipType.empty())
  {
    _currentTvlSeg->equipmentType() = equipType;
    _currentTvlSeg->equipmentTypes().push_back(equipType);
  }

  // set origin/destionation
  _currentTvlSeg->origin() =
            _pricingTrx->dataHandle().getLoc(_currentTvlSeg->origAirport(), _pricingTrx->ticketingDate());
  _currentTvlSeg->destination() =
            _pricingTrx->dataHandle().getLoc(_currentTvlSeg->destAirport(), _pricingTrx->ticketingDate());
  if (_currentTvlSeg->origin() == nullptr || _currentTvlSeg->destination() == nullptr)
   throw ErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE);

  // N0E - Cabin
  setCabin(attrs.get<Indicator>(_N0E, ' '));

  // BB0 - Reservation status
  getAttr(attrs, _BB0, _currentTvlSeg->resStatus());

  // C7A - Ticket coupon number
  attrs.get(_C7A, _currentTvlSeg->ticketCouponNumber(), uint16_t(0));

  // P72 - Forced connection
  attrs.get(_P72, _currentTvlSeg->forcedConx(), 'F');

  // P73 - Forced stopover
  attrs.get(_P73, _currentTvlSeg->forcedStopOver(), 'F');

  // C7B - Select travel segment for baggage disclosure
  _currentTvlSeg->setCheckedPortionOfTravelInd(attrs.get(_C7B, 'T'));
}

void
BaggageRequestHandler::onStartFFY(const IAttributes& attrs)
{
  // B00 - Carrier code
  CarrierCode carrier;
  getAttr(attrs, _B00, carrier);

  uint16_t ffStatus = 0;

  // B70 - Pax Type Code
  PaxTypeCode ptc;
  getAttr(attrs, _B70, ptc);

  // Q7D - Status
  ffStatus = attrs.get<uint16_t>(_Q7D, 0);

  if (ffStatus != 0)
  {
    PaxType::FreqFlyerTierWithCarrier* ffd =
        _pricingTrx->dataHandle().create<PaxType::FreqFlyerTierWithCarrier>();
    ffd->setFreqFlyerTierLevel(ffStatus);
    ffd->setCxr(carrier);
    ffd->setIsFromPnr(false);
    _ffd.push_back(ffd);
  }
  // N1O - VIP type
  const int vipType = attrs.get<int>(_N1O, 0);

  if (vipType && carrier == SPECIAL_CARRIER_AA)
  {
    if (vipType & 0x40)
      ffStatus = 1;
    else if (vipType & 0x20)
      ffStatus = 2;
    else if (vipType & 0x02)
      ffStatus = 3;

    if (ffStatus != 0)
    {
      PaxType::FreqFlyerTierWithCarrier* ffd =
          _pricingTrx->dataHandle().create<PaxType::FreqFlyerTierWithCarrier>();
      ffd->setFreqFlyerTierLevel(ffStatus);
      ffd->setCxr(carrier);
      _ffd.push_back(ffd);
    }
  }

  const Indicator pcc = attrs.get<Indicator>(_PCC, 'F');
  if (pcc == 'T')
  {
    PaxType::FreqFlyerTierWithCarrier* ffd =
        _pricingTrx->dataHandle().create<PaxType::FreqFlyerTierWithCarrier>();
    ffd->setFreqFlyerTierLevel(5);
    ffd->setCxr(carrier);
    _ffd.push_back(ffd);
  }
}

void
BaggageRequestHandler::onStartHSP(const IAttributes& attrs)
{
  if (!_currentTvlSeg)
    return;

  LocCode hiddenCity;
  getAttr(attrs, _A03, hiddenCity);
  const Loc* loc = _pricingTrx->dataHandle().getLoc(hiddenCity, _pricingTrx->ticketingDate());

  if (loc)
    _currentTvlSeg->hiddenStops().push_back(loc);
}

void
BaggageRequestHandler::onStartEQP(const IAttributes& attrs)
{
  if (!_currentTvlSeg)
    return;

  const EquipmentType equipType = attrs.get<std::string>(_S95, "");

  if (!equipType.empty())
    _currentTvlSeg->equipmentTypes().push_back(equipType);
}

void
BaggageRequestHandler::onStartBaggageRequest(const IAttributes& attrs)
{
  std::string fullVersion;
  getAttr(attrs, _Version, fullVersion);

  uint16_t majorSchemaVersion = 1;
  uint16_t minorSchemaVersion = 0;
  uint16_t revisionSchemeVersion = 0;

  // minimum size 3 i.e. "1.1"
  if (fullVersion.size() > 2)
  {
    try
    {
      majorSchemaVersion = boost::lexical_cast<uint16_t>(fullVersion[0]);
      minorSchemaVersion = boost::lexical_cast<uint16_t>(fullVersion[2]);
      if (fullVersion.size() > 4)
      {
        revisionSchemeVersion = boost::lexical_cast<uint16_t>(fullVersion[4]);
      }
    }
    catch (boost::bad_lexical_cast&) {}
  }

  _request->majorSchemaVersion() = majorSchemaVersion;
  _request->minorSchemaVersion() = minorSchemaVersion;
  _request->revisionSchemaVersion() = revisionSchemeVersion;
}

void
BaggageRequestHandler::onEndBaggageRequest()
{
  detectRtw();
}

void
BaggageRequestHandler::onEndBIL()
{
  _pricingTrx->billing()->updateTransactionIds(_pricingTrx->transactionId());
  _pricingTrx->billing()->updateServiceNames(Billing::SVC_TAX);

  _request->ancRequestType() = AncRequest::BaggageRequest;
  _reqType = M70;
}

void
BaggageRequestHandler::onEndITN()
{
  _prevTvlSeg = nullptr;
  _datesInRequest = FIRST_MATCH;
  _timesInRequest = FIRST_MATCH;

  parseOriginalPricingCommandWPAE();

  if (_request->masterItin() == nullptr)
    _request->masterItin() = _itin;
  else
  {
    _pricingTrx->itin().push_back(_itin);

    for (Itin* it : _pricingTrx->itin())
    {
      if (_request->itinAttrMap()[_itin].getHash(1) == _request->itinAttrMap()[it].getHash(1))
      {
        _request->pricingItins()[it].push_back(_itin);
        return;
      }
    }
  }

  _pricingTrx->getOptions()->isProcessAllGroups() =
      _pricingTrx->getOptions()->serviceGroupsVec().empty();

  checkFlight(*_itin, true, true);
  checkSideTrip(_itin);

  _itin = nullptr;
}
}
