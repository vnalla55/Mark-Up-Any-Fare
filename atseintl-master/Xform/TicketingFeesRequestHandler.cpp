#include "Xform/TicketingFeesRequestHandler.h"

#include "Common/FareMarketUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DBAccess/Customer.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/DataModelMap.h"
#include "Xform/TicketingFeesSchemaNames.h"

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

using namespace boost::assign;

namespace tse
{
using namespace ancillary;
using namespace ticketingfees;

namespace
{
Logger
_logger("atseintl.Xform.TicketingFeesRequestHandler");
}

namespace
{
ILookupMap _elemLookupMapTktFees, _attrLookupMapTktFees;

bool
init(IXMLUtils::initLookupMaps(_AncillaryElementNames,
                               _NumberOfElementNames_,
                               _TicketingFeesElementNames,
                               _NumberOfElementNamesTktFees_,
                               _elemLookupMapTktFees,
                               _AncillaryAttributeNames,
                               _NumberOfAttributeNames_,
                               _TicketingFeesAttributeNames,
                               _NumberOfAttributeNamesTktFees_,
                               _attrLookupMapTktFees));
}

TicketingFeesRequestHandler::TicketingFeesRequestHandler(Trx*& trx)
  : CommonRequestHandler(trx), _tktFbi(nullptr), _tktFeesRequest(nullptr)
{
}

TicketingFeesRequestHandler::~TicketingFeesRequestHandler()
{
}

void
TicketingFeesRequestHandler::parse(DataHandle& dataHandle, const std::string& content)
{
  createTransaction(dataHandle, content);

  IValueString attrValueArray[_NumberOfAttributeNamesTktFees_];
  int attrRefArray[_NumberOfAttributeNamesTktFees_];
  IXMLSchema schema(_elemLookupMapTktFees,
                    _attrLookupMapTktFees,
                    _NumberOfAttributeNamesTktFees_,
                    attrValueArray,
                    attrRefArray,
                    true);
  const char* pChar(content.c_str());
  size_t length(content.length());
  size_t pos(content.find_first_of('<'));
  if (pos != std::string::npos)
  {
    pChar += pos;
    length -= pos;
  }
  IParser parser(pChar, length, *this, schema);
  parser.parse();
}

bool
TicketingFeesRequestHandler::startElement(int idx, const IAttributes& attrs)
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
  case _BIN:
    onStartBIN(attrs);
    break;
  case _DIF:
    onStartDIF(attrs);
    break;
  case _TicketingFeesRequest:
    onStartTicketingFeesRequest(attrs);
    break;
  case _DynamicConfig:
    onStartDynamicConfig(attrs);
    break;
  }
  return true;
}
bool
TicketingFeesRequestHandler::endElement(int idx)
{
  switch (idx)
  {
  case _AGI:
    onEndAGI();
    break;
  case _BIL:
    onEndBIL();
    break;
  case _PRO:
    onEndPRO();
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
  case _BIN:
    onEndBIN();
    break;
  case _DIF:
    onEndDIF();
    break;
  case _TicketingFeesRequest:
    onEndTicketingFeesRequest();
    break;
  }

  _value.clear(); // Need to clear to be avaialable for next tag
  return true;
}

void
TicketingFeesRequestHandler::createTransaction(DataHandle& dataHandle, const std::string& content)
{
  _trx = _pricingTrx = dataHandle.create<TktFeesPricingTrx>();
  setTransactionParameters();
}

void
TicketingFeesRequestHandler::setTransactionParameters()
{
  _pricingTrx->dataHandle().get(_tktFeesRequest);
  _pricingTrx->setRequest(_tktFeesRequest);
  LOG4CXX_DEBUG(_logger, "Got _tktFeesRequest");

  Agent* checkInAgent(nullptr);
  _pricingTrx->dataHandle().get(checkInAgent);
  _tktFeesRequest->ticketingAgent() = checkInAgent;
  LOG4CXX_DEBUG(_logger, "Got check-in agent");

  PricingOptions* options(nullptr);
  _pricingTrx->dataHandle().get(options);
  _pricingTrx->setOptions(options);
  LOG4CXX_DEBUG(_logger, "Got options");

  Billing* billing(nullptr);
  _pricingTrx->dataHandle().get(billing);
  _pricingTrx->billing() = billing;
  LOG4CXX_DEBUG(_logger, "Got Billing");

  setDate(DateTime::localTime());
}

void
TicketingFeesRequestHandler::onStartPRO(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PRO");

  // AF0 - Ticketing point override
  getAttr(attrs, _AF0, _tktFeesRequest->ticketPointOverride());

  // AG0 - Sale point override
  getAttr(attrs, _AG0, _tktFeesRequest->salePointOverride());

  // C45 - Equiv amount currency code
  std::string currencyOverride;
  getAttr(attrs, _C45, currencyOverride);
  _pricingTrx->getOptions()->currencyOverride() = currencyOverride;
  checkCurrency(_pricingTrx->getOptions()->currencyOverride());

  if (!_pricingTrx->getOptions()->currencyOverride().empty())
    _pricingTrx->getOptions()->mOverride() = 'T';

  // D07
  if (attrs.has(_D07))
  {
    std::string ticketingDate;
    getAttr(attrs, _D07, ticketingDate);
    setDate(convertDate(ticketingDate));
  }
  // P63 - credit card FOP
  _pricingTrx->getRequest()->formOfPaymentCard() = 'T'; // default

  // SEZ - collect OB Fees (TJR)
  if (_pricingTrx->getRequest()->ticketingAgent() != nullptr &&
      _pricingTrx->getRequest()->ticketingAgent()->agentTJR() != nullptr &&
      _pricingTrx->getRequest()->ticketingAgent()->agentTJR()->doNotApplyObTktFees() != YES)
    _pricingTrx->getRequest()->collectOBFee() = 'T';

  // C10 - Diagnostic number
  const int diagnosticNumber = attrs.get<int>(_C10, 0);
  _tktFeesRequest->diagnosticNumber() = diagnosticNumber;
  _pricingTrx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diagnosticNumber);
  _pricingTrx->diagnostic().activate();
}

void
TicketingFeesRequestHandler::onStartFLI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FLI");
  checkFLIData(attrs);

  // A01 - Departure Airport Code
  getAttr(attrs, _A01, _currentTvlSeg->origAirport());
  // A02 - Arrival Airport Code
  getAttr(attrs, _A02, _currentTvlSeg->destAirport());

  if (_currentTvlSeg->origAirport().empty() || _currentTvlSeg->destAirport().empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - MISSING CITY");

  // departure arrive time
  const std::string depTime = attrs.get<std::string>(_D31, "");
  const std::string arrTime = attrs.get<std::string>(_D32, "");

  _currentTvlSeg->pssDepartureTime() = pssTime(depTime);
  _currentTvlSeg->pssArrivalTime() = pssTime(arrTime);

  // D01 - Departure Date
  const std::string departureDT = attrs.get<std::string>(_D01);

  // D02 - Arrival Date
  const std::string arrivalDT = attrs.get<std::string>(_D02, departureDT);

  // if departure date is present -use it, oterwise calcualte date
  if (departureDT.length() > 0)
    _currentTvlSeg->departureDT() = convertDate(departureDT);
  else
  {
    if (_prevTvlSeg != nullptr)
    {
      if (_prevTvlSeg->departureDT() != DateTime::openDate())
      {
        DateTime dt = _prevTvlSeg->departureDT().addDays(1);
        _currentTvlSeg->departureDT() = DateTime(dt.year(), dt.month(), dt.day());
      }
      else
        _currentTvlSeg->departureDT() = DateTime(_curDate.year(), _curDate.month(), _curDate.day());
    }
    else
      _currentTvlSeg->departureDT() = DateTime(_curDate.year(), _curDate.month(), _curDate.day());
  }

  // D02 - Arrival Date
  if (arrivalDT.length() > 0)
    _currentTvlSeg->arrivalDT() = convertDate(arrivalDT);
  else
  {
    // if departure date was calculated, then aarive date will be calculated also
    if (departureDT.empty())
    {
      _currentTvlSeg->arrivalDT() = _currentTvlSeg->departureDT();
      if (!depTime.empty() && !arrTime.empty() && (atoi(depTime.c_str()) > atoi(arrTime.c_str())))
      {
        _currentTvlSeg->arrivalDT() = _currentTvlSeg->arrivalDT().addDays(1);
      }
    }
    else
      _currentTvlSeg->arrivalDT() = _currentTvlSeg->departureDT();
  }
  setTime(_currentTvlSeg->departureDT(), depTime);
  setTime(_currentTvlSeg->arrivalDT(), arrTime);

  setCarrierAndSegmentStatusFLI(attrs);

  // B30 - Class of Service
  BookingCode bookingCode;
  getAttr(attrs, _B30, bookingCode);
  if (!bookingCode.empty())
  {
    _currentTvlSeg->setBookingCode(bookingCode);
  }
  // Q0B - Flight Number
  _currentTvlSeg->marketingFlightNumber() = attrs.get<int>(_Q0B, 0);
  // S95
  getAttr(attrs, _S95, _currentTvlSeg->equipmentType());
  // C7A - Ticket coupon number
  attrs.get(_C7A, _currentTvlSeg->ticketCouponNumber(), uint16_t(0));
  // P72 - Forced connection
  attrs.get(_P72, _currentTvlSeg->forcedConx(), 'F');
  // P73 - Forced stopover
  attrs.get(_P73, _currentTvlSeg->forcedStopOver(), 'F');
}

void
TicketingFeesRequestHandler::setCarrierAndSegmentStatusFLI(const IAttributes& attrs)
{
  // B00 - Marketing carrier code
  CarrierCode marketingCxr;
  getAttr(attrs, _B00, marketingCxr);
  if (marketingCxr.empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MISSING CARRIER");

  _currentTvlSeg->setMarketingCarrierCode(MCPCarrierUtil::swapToActual(_trx, marketingCxr));

  CarrierCode operatingCxr = marketingCxr;
  getAttr(attrs, _B01, operatingCxr);
  _currentTvlSeg->setOperatingCarrierCode(MCPCarrierUtil::swapToActual(_trx, operatingCxr));

  // BB0 - Reservation status
  getAttr(attrs, _BB0, _currentTvlSeg->resStatus());
}

void
TicketingFeesRequestHandler::onStartSGI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter SGI");
  _pricingTrx->dataHandle().get(_currentTvlSeg);
  if (_currentTvlSeg == nullptr)
    throw std::runtime_error("Null pointer to int data");

  // Q00 - Segment ID
  _currentTvlSeg->pnrSegment() = attrs.get<int16_t>(_Q00, 0);
  _currentTvlSeg->segmentOrder() = _segmentOrder;

  // BE0 - Ticket Desginator
  TktDesignator ticketDesignator;
  getAttr(attrs, _BE0, ticketDesignator);
  if (!ticketDesignator.empty())
  {
    _tktFeesRequest->tktDesignatorPerItin()[_itin].insert(
        std::pair<int16_t, TktDesignator>(_currentTvlSeg->segmentOrder(), ticketDesignator));
  }
}

void
TicketingFeesRequestHandler::onStartTicketingFeesRequest(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter TicketingFeesRequest");

  std::string fullVersion;
  getAttr(attrs, _Version, fullVersion);

  uint16_t majorSchemaVersion = 1;
  if (!fullVersion.empty())
  {
    try
    {
      majorSchemaVersion = boost::lexical_cast<uint16_t>(fullVersion[0]);
    }
    catch (boost::bad_lexical_cast&)
    {
    }
  }
  _tktFeesRequest->majorSchemaVersion() = majorSchemaVersion;
}

void
TicketingFeesRequestHandler::onStartIRO(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter IRO");
  checkIROData(attrs);

  // D07
  if (attrs.has(_D07))
  {
    std::string ticketingDate;
    getAttr(attrs, _D07, ticketingDate);
    _tktFeesRequest->ticketingDatesPerItin().insert(
        std::make_pair(_itin, convertDate(ticketingDate)));
  }

  // STA - Total Payment Amount
  TktFeesRequest::PaxTypePayment* ptp(nullptr);
  _pricingTrx->dataHandle().get(ptp);
  if (ptp == nullptr)
    throw std::runtime_error("Null pointer to PTP data");

  // STA -set total amount
  std::string totalAmount;
  getAttr(attrs, _STA, totalAmount);
  ptp->amount() = atof(totalAmount.c_str());

  // C45 - set Equivalent(payment) currency code
  std::string paymentCurrency;
  getAttr(attrs, _C45, paymentCurrency);
  ptp->currency() = paymentCurrency;
  checkCurrency(paymentCurrency);

  addTotalAmount(ptp);
}

void
TicketingFeesRequestHandler::onStartPXI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PXI");

  PaxTypeCode ptc = ADULT;
  getAttr(attrs, _B70, ptc);
  const uint16_t number = attrs.get<uint16_t>(_Q0U, 1);

  initPaxType(ptc, number);
}

void
TicketingFeesRequestHandler::onStartFBA(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FBA");
  checkFBAData(attrs);

  TktFeesRequest::TktFeesFareBreakAssociation* fba(nullptr);
  _pricingTrx->dataHandle().get(fba);
  if (fba == nullptr)
    throw std::runtime_error("Null pointer to FBA data");

  // set segment ID
  fba->segmentID() = _currentTvlSeg->segmentOrder();

  setSequenceNumberAndSideTripIdFBA(attrs, *fba);

  // S07 - Side Trip Start Indicator
  bool indicator;
  attrs.get(_S07, indicator, false);
  fba->sideTripStart() = indicator;

  // S08 - Side Trip End Indicator
  attrs.get(_S08, indicator, false);
  fba->sideTripEnd() = indicator;

  _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin].push_back(fba);
}

void
TicketingFeesRequestHandler::setSequenceNumberAndSideTripIdFBA(
    const IAttributes& attrs, TktFeesRequest::TktFeesFareBreakAssociation& fba)
{
  // Q6D - FareComponent ID
  SequenceNumber id = attrs.get<SequenceNumber>(_Q6D, 0);
  fba.fareComponentID() = id;

  // Q6E - Side Trip ID
  attrs.get(_Q6E, id, 0);
  fba.sideTripID() = id;
}

void
TicketingFeesRequestHandler::onStartFBI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FBI");
  checkFBIData(attrs);

  _pricingTrx->dataHandle().get(_tktFbi);
  if (_tktFbi == nullptr)
    throw std::runtime_error("Null pointer to TKTFBI data");

  // Q00 - Fare Component ID
  _tktFbi->fareComponentID() = attrs.get<SequenceNumber>(_Q00, 0);
  // B02 - Governing Carrier
  getAttr(attrs, _B02, _tktFbi->governingCarrier());
  // B50 - Fare basis code with slashes and resulting ticket designator
  getAttr(attrs, _B50, _tktFbi->fareBasis());
  // S37 - Vendor code
  getAttr(attrs, _S37, _tktFbi->vendorCode());
  _tktFeesRequest->tktFeesFareBreakPerItin()[_itin].push_back(_tktFbi);
}

void
TicketingFeesRequestHandler::onStartACI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter ACI");
  // ACC - Account Code
  std::string accountCode;
  getAttr(attrs, _ACC, accountCode);
  addAccountCode(accountCode);
}

void
TicketingFeesRequestHandler::onStartCII(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter CII");
  // CID - CorporateID
  std::string corpId;
  getAttr(attrs, _CID, corpId);
  addCorpId(corpId);
}

void
TicketingFeesRequestHandler::onStartBIN(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter BIN");
  // BIN Form Of Payment
  FopBinNumber bin;
  getAttr(attrs, _FOP, bin);
  addFopBin(bin);
}

void
TicketingFeesRequestHandler::onStartDIF(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter DIF");
  // DIF - Differential Data
  TktFeesRequest::TktFeesDifferentialData* tfdd(nullptr);
  _pricingTrx->dataHandle().get(tfdd);
  if (tfdd == nullptr)
    throw std::runtime_error("Null pointer to DIF data");

  // B02 - Differential carrier
  getAttr(attrs, _B02, tfdd->diffCarrierCode());
  // BJ0 - Differential Fare basis code with slashes and resulting ticket designator
  getAttr(attrs, _BJ0, tfdd->fareBasis());

  std::vector<TktFeesRequest::TktFeesDifferentialData*>& tfddVec =
      _tktFeesRequest->tktFeesFareBreakPerItin()[_itin].back()->tktFeesDiffData();

  tfddVec.push_back(tfdd);
}

void
TicketingFeesRequestHandler::onEndBIL()
{
  _pricingTrx->billing()->updateTransactionIds(_pricingTrx->transactionId());
  _pricingTrx->billing()->updateServiceNames(Billing::SVC_TAX);

  LOG4CXX_DEBUG(_logger, "Leave BIL");
}

void
TicketingFeesRequestHandler::onEndITN()
{
  // reset before next ITN
  checkRequiredDataFbaFbi();
  processOnEndITN();
  _itin = nullptr;

  LOG4CXX_DEBUG(_logger, "Leave ITN");
}

void
TicketingFeesRequestHandler::processOnEndITN()
{
  _prevTvlSeg = nullptr;
  _datesInRequest = FIRST_MATCH;
  _timesInRequest = FIRST_MATCH;

  // set date on open segments with empty dates
  int segs = _itin->travelSeg().size();
  for (int i = 0; i < segs; ++i)
  {
    if (_itin->travelSeg()[i]->departureDT() == DateTime::openDate())
    {
      // set departure date same as arrival date of prev segment
      if (i > 0)
        _itin->travelSeg()[i]->departureDT() = _itin->travelSeg()[i - 1]->arrivalDT();
      // iff arrival date is set, the use it as departure date
      else if (_itin->travelSeg()[i]->arrivalDT() != DateTime::openDate())
        _itin->travelSeg()[i]->departureDT() = _itin->travelSeg()[i]->arrivalDT();
      // if both dates not present, try to use deprature date of next
      // not open segment
      else
      {
        int j = 1;
        while (i + j < segs && _itin->travelSeg()[i + j]->departureDT() == DateTime::openDate())
          ++j;
        if (i + j < segs)
        {
          _itin->travelSeg()[i]->departureDT() = _itin->travelSeg()[i + j]->departureDT();
          _itin->travelSeg()[i]->arrivalDT() = _itin->travelSeg()[i + j]->departureDT();
        }
        // only one segment with no dates - use current date??
        else
        {
          _itin->travelSeg()[i]->departureDT() = _curDate;
          _itin->travelSeg()[i]->arrivalDT() = _curDate;
        }
      }
      _itin->travelSeg()[i]->hasEmptyDate() = true;
    }
    if (_itin->travelSeg()[i]->arrivalDT() == DateTime::openDate())
    {
      // try to set arrival date same as departure date of next not open segment
      int j = 1;
      while (i + j < segs && _itin->travelSeg()[i + j]->departureDT() == DateTime::openDate())
        ++j;
      if (i + j < segs)
        _itin->travelSeg()[i]->arrivalDT() = _itin->travelSeg()[i + j]->departureDT();
      // set same as departure date
      else
        _itin->travelSeg()[i]->arrivalDT() = _itin->travelSeg()[i]->departureDT();
      _itin->travelSeg()[i]->hasEmptyDate() = true;
    }
  }
  // check PaxType in request
  if (_tktFeesRequest->paxTypesPerItin()[_itin].empty())
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "PASSENGER TYPE IS   MISSING IN REQUEST");
  }
  checkSideTrip(_itin);
  _itin->setTravelDate(TseUtil::getTravelDate(_itin->travelSeg()));
  _pricingTrx->itin().push_back(_itin);
}

void
TicketingFeesRequestHandler::onEndPRO()
{
  //  checkCurrencyAndSaleLoc();

  LOG4CXX_DEBUG(_logger, "Leave PRO");
}

void
TicketingFeesRequestHandler::onEndBIN()
{
  LOG4CXX_DEBUG(_logger, "Leave BIN");
}
void
TicketingFeesRequestHandler::onEndDIF()
{
  LOG4CXX_DEBUG(_logger, "Leave DIF");
}

void
TicketingFeesRequestHandler::onEndTicketingFeesRequest()
{
  LOG4CXX_DEBUG(_logger, "Leave TicketingFeesRequest");
}

void
TicketingFeesRequestHandler::onEndSGI()
{
  // set origin/destionation
  _currentTvlSeg->origin() =
      _pricingTrx->dataHandle().getLoc(_currentTvlSeg->origAirport(), _pricingTrx->ticketingDate());
  _currentTvlSeg->destination() =
      _pricingTrx->dataHandle().getLoc(_currentTvlSeg->destAirport(), _pricingTrx->ticketingDate());

  if (_currentTvlSeg->origin() == nullptr || _currentTvlSeg->destination() == nullptr)
    throw ErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE);

  // set multicity
  bool isInternational =
      LocUtil::isInternational(*(_currentTvlSeg->origin()), *(_currentTvlSeg->destination()));
  _currentTvlSeg->boardMultiCity() =
      FareMarketUtil::getMultiCity(_currentTvlSeg->carrier(),
                                   _currentTvlSeg->origAirport(),
                                   isInternational ? GeoTravelType::International : GeoTravelType::Domestic,
                                   _currentTvlSeg->departureDT());
  _currentTvlSeg->offMultiCity() =
      FareMarketUtil::getMultiCity(_currentTvlSeg->carrier(),
                                   _currentTvlSeg->destAirport(),
                                   isInternational ? GeoTravelType::International : GeoTravelType::Domestic,
                                   _currentTvlSeg->departureDT());
  // vk?  _currentTvlSeg->resStatus() = "OK";  // set status to confirmed

  // if previos segment exist, and arrival difeerent than departure fo current, add arunk
  addArunk();

  // update prev segment
  _prevTvlSeg = _currentTvlSeg;
  _itin->travelSeg().push_back(_currentTvlSeg);
  _pricingTrx->travelSeg().push_back(_currentTvlSeg);
  _segmentOrder++;

  LOG4CXX_DEBUG(_logger, "Leave SGI");
}

void
TicketingFeesRequestHandler::addArunk()
{
  if ((_prevTvlSeg != nullptr) && (_prevTvlSeg->destAirport() != _currentTvlSeg->origAirport()))
  {
    fillArunkData();

    if (_tktFeesRequest && !_tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin].empty() &&
        (_tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin].back()->segmentID() ==
         _segmentOrder))
    {
      ++_tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin].back()->segmentID();
    }
  }
}

void
TicketingFeesRequestHandler::fillArunkData()
{
  ArunkSeg* arunkSeg = nullptr;
  _pricingTrx->dataHandle().get(arunkSeg);
  if (arunkSeg == nullptr)
    throw std::runtime_error("Null pointer to int data");
  arunkSeg->segmentType() = Arunk;
  arunkSeg->pnrSegment() = ARUNK_PNR_SEGMENT_ORDER;
  arunkSeg->segmentOrder() = _segmentOrder;
  arunkSeg->origAirport() = _prevTvlSeg->destAirport();
  arunkSeg->destAirport() = _currentTvlSeg->origAirport();
  arunkSeg->origin() = _prevTvlSeg->destination();
  arunkSeg->destination() = _currentTvlSeg->origin();
  arunkSeg->departureDT() = _prevTvlSeg->arrivalDT();
  arunkSeg->arrivalDT() = _currentTvlSeg->departureDT();
  arunkSeg->boardMultiCity() = _prevTvlSeg->offMultiCity();
  arunkSeg->offMultiCity() = _currentTvlSeg->boardMultiCity();
  _itin->travelSeg().push_back(arunkSeg);
  _pricingTrx->travelSeg().push_back(arunkSeg);
  _currentTvlSeg->segmentOrder() = ++_segmentOrder;
}

void
TicketingFeesRequestHandler::onEndPXI()
{
  addPaxType();
  LOG4CXX_DEBUG(_logger, "Leave PXI");
}

void
TicketingFeesRequestHandler::onEndIRO()
{
  LOG4CXX_DEBUG(_logger, "Leave IRO");
  // check for maximum number of account id & corp id
  if (_tktFeesRequest->accountCodeIdPerItin()[_itin].size() +
          _tktFeesRequest->corpIdPerItin()[_itin].size() +
          _tktFeesRequest->invalidCorpIdPerItin()[_itin].size() >
      4)
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MAXIMUM 4 ACCOUNT CODE AND/OR CORPORATE ID PERMITTED");
}

void
TicketingFeesRequestHandler::checkFBAData(const IAttributes& attrs) const
{
  std::vector<int> reqFBAAttrs;
  reqFBAAttrs += _Q6D;
  checkData(attrs, reqFBAAttrs, *_TicketingFeesAttributeNames, "ATTRIBUTE IN FBA SECTION");
}

void
TicketingFeesRequestHandler::checkFBIData(const IAttributes& attrs) const
{
  std::vector<int> reqFBIAttrs;
  reqFBIAttrs += _Q00, _B02, _B50;
  checkData(attrs, reqFBIAttrs, *_TicketingFeesAttributeNames, "ATTRIBUTE IN FBI SECTION");
}

void
TicketingFeesRequestHandler::checkFLIData(const IAttributes& attrs) const
{
  std::vector<int> reqFLIAttrs;
  reqFLIAttrs += _A01, _A02, _B00;
  checkData(attrs, reqFLIAttrs, *_TicketingFeesAttributeNames, "ATTRIBUTE IN FLI SECTION");
}

void
TicketingFeesRequestHandler::checkIROData(const IAttributes& attrs) const
{
  std::vector<int> reqIROAttrs;
  reqIROAttrs += _C45;
  checkData(attrs, reqIROAttrs, *_TicketingFeesAttributeNames, "ATTRIBUTE IN IRO SECTION");
}

namespace
{
struct TravelSegSegmentOrderComparator
{
  bool operator()(TravelSeg* s1, TravelSeg* s2) { return s1->segmentOrder() < s2->segmentOrder(); }
};
}

void
TicketingFeesRequestHandler::checkSideTrip(Itin* itin) const
{
  if (itin->travelSeg().empty())
    return;

  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>& tktFeesFbaVec =
      _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[itin];
  if (tktFeesFbaVec.empty())
    return;

  int segMin = (*std::min_element(itin->travelSeg().begin(),
                                  itin->travelSeg().end(),
                                  TravelSegSegmentOrderComparator()))->segmentOrder();
  int segMax = (*std::max_element(itin->travelSeg().begin(),
                                  itin->travelSeg().end(),
                                  TravelSegSegmentOrderComparator()))->segmentOrder();
  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>::iterator it = tktFeesFbaVec.begin();
  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>::iterator ie = tktFeesFbaVec.end();
  for (; it != ie; it++)
  {
    if ((*it)->sideTripID())
    {
      if ((*it)->segmentID() == segMin || (*it)->segmentID() == segMax)
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "INVALID - PLACE OF SIDE TRIP");
    }
  }
}

void
TicketingFeesRequestHandler::checkRequiredDataFbaFbi() const
{
  typedef std::map<const uint16_t, bool> mapType;
  std::map<uint16_t, bool> segWithFBA;
  std::map<uint16_t, bool> FBAWithFBI;

  for (TravelSeg* ts : _itin->travelSeg())
    if (ts->isAir())
      segWithFBA[ts->segmentOrder()] = false;

  // mark travel segment if have coresponding FBA  section
  for (TktFeesRequest::TktFeesFareBreakAssociation* fba :
       _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin])
  {
    VALIDATE_OR_THROW(segWithFBA.find(fba->segmentID()) != segWithFBA.end(),
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - FBA SECTION WITH ORDER " << fba->segmentID()
                                                          << " NOT MATCHED TO FLI");
    segWithFBA[fba->segmentID()] = true;
    FBAWithFBI[fba->fareComponentID()] = false;
  }
  // check if all sectors have matching FBA section
  for (mapType::value_type& pair : segWithFBA)
  {
    VALIDATE_OR_THROW(pair.second,
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - NO MATCHING FBA SECTION FOR SEGMENT " << pair.first);
  }
  // mark FBA if have coresponding FBI section
  for (TktFeesRequest::TktFeesFareBreakInfo* fbi :
       _tktFeesRequest->tktFeesFareBreakPerItin()[_itin])
  {
    VALIDATE_OR_THROW(FBAWithFBI.find(fbi->fareComponentID()) != FBAWithFBI.end(),
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - FBI SECTION WITH FARE COMPONENT ID " << fbi->fareComponentID()
                                                                      << " NOT MATCHED TO FBA");
    FBAWithFBI[fbi->fareComponentID()] = true;
  }
  // check if all fare break association have fare component info
  for (const mapType::value_type& pair : FBAWithFBI)
  {
    VALIDATE_OR_THROW(pair.second,
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - NO MATCHING FBI SECTION FOR FBA WITH FARE COMPONENT ID "
                          << pair.first);
  }
}

void
TicketingFeesRequestHandler::addAccountCode(const std::string& accountCode)
{
  std::vector<std::string>& accCodeVec = _tktFeesRequest->accountCodeIdPerItin()[_itin];
  if (std::find(accCodeVec.begin(), accCodeVec.end(), accountCode) == accCodeVec.end())
    accCodeVec.push_back(accountCode);
}

void
TicketingFeesRequestHandler::addCorpId(const std::string& corpId)
{
  bool isACCValid = _pricingTrx->dataHandle().corpIdExists(corpId, _pricingTrx->ticketingDate());
  std::vector<std::string>& corpIdVec = isACCValid ? _tktFeesRequest->corpIdPerItin()[_itin]
                                                   : _tktFeesRequest->invalidCorpIdPerItin()[_itin];

  if (std::find(corpIdVec.begin(), corpIdVec.end(), corpId) == corpIdVec.end())
    corpIdVec.push_back(corpId);
}

void
TicketingFeesRequestHandler::addFopBin(const FopBinNumber& bin)
{
  //  std::vector<FopBinNumber>& fopBin = _tktFeesRequest->binNumberPaymentPerItin()[_itin];
  //  if(std::find(fopBin.begin(), fopBin.end(), bin) == fopBin.end())
  //    fopBin.push_back(bin);
}

void
TicketingFeesRequestHandler::addTotalAmount(const TktFeesRequest::PaxTypePayment* ptp)
{
}

void
TicketingFeesRequestHandler::addTktDesignator(const std::string tktDesig,
                                              const std::vector<int>& segIds)
{
  if (segIds.empty())
  {
    for (TravelSeg* ts : _itin->travelSeg())
      _tktFeesRequest->tktDesignatorPerItin()[_itin].insert(
          std::pair<int16_t, TktDesignator>(ts->segmentOrder(), tktDesig.c_str()));
  }
  else
  {
    for (TravelSeg* ts : _itin->travelSeg())
      if (std::find(segIds.begin(), segIds.end(), ts->pnrSegment()) != segIds.end())
        _tktFeesRequest->tktDesignatorPerItin()[_itin].insert(
            std::pair<int16_t, TktDesignator>(ts->segmentOrder(), tktDesig.c_str()));
  }
}

void
TicketingFeesRequestHandler::initPaxType(const PaxTypeCode& ptc, uint16_t number)
{
  _pricingTrx->dataHandle().get(_paxType);
  _paxType->paxType() = ptc;
  _paxType->requestedPaxType() = ptc;
  _paxType->number() = number;

  if (PaxTypeUtil::extractAgeFromPaxType(_paxType->paxType(), _paxType->age()))
  {
    if (_paxType->age() <= 1)
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                   "INVALID PASSENGER TYPE");
  }
}

void
TicketingFeesRequestHandler::addPaxType()
{
  if (_paxType == nullptr)
    return;
  PaxTypeUtil::initialize(*_pricingTrx,
                          *_paxType,
                          _paxType->paxType(),
                          _paxType->number(),
                          _paxType->age(),
                          _paxType->stateCode(),
                          _paxInputOrder++);

  _tktFeesRequest->paxTypesPerItin()[_itin].push_back(_paxType);
  _pricingTrx->paxType().push_back(_paxType);
  _paxType = nullptr;
}

} // tse
