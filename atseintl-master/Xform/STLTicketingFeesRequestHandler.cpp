#include "Xform/STLTicketingFeesRequestHandler.h"

#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Money.h"
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
#include "Xform/STLTicketingFeesSchemaNames.h"

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

using namespace boost::assign;

namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);

using namespace stlticketingfees;

namespace
{
Logger
_logger("atseintl.Xform.STLTicketingFeesRequestHandler");

ILookupMap _elemLookupMapTktFees, _attrLookupMapTktFees;

bool
init(IXMLUtils::initLookupMaps(_STLTicketingFeesElementNames,
                               _NumberOfElementNamesTktFees_,
                               _elemLookupMapTktFees,
                               _STLTicketingFeesAttributeNames,
                               _NumberOfAttributeNamesTktFees_,
                               _attrLookupMapTktFees));
}

STLTicketingFeesRequestHandler::STLTicketingFeesRequestHandler(Trx*& trx)
  : CommonRequestHandler(trx),
    _tktFeesRequest(nullptr),
    _currentTvlSegment(nullptr),
    _prevTvlSegment(nullptr),
    _pnrSegment(0),
    _indValueS(EMPTY_STRING()),
    _segType(EMPTY_STRING())
{
}

STLTicketingFeesRequestHandler::~STLTicketingFeesRequestHandler()
{
}

void
STLTicketingFeesRequestHandler::parse(DataHandle& dataHandle, const std::string& content)
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
STLTicketingFeesRequestHandler::startElement(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case _Agent:
    onStartAgent(attrs);
    break;
  case _BillingInformation:
    onStartBillingInformation(attrs);
    break;
  case _RequestOptions:
    onStartRequestOptions(attrs);
    break;
  case _RequestedDiagnostic:
    onStartRequestedDiagnostic(attrs);
    break;
  case _TravelSegment:
    onStartTravelSegment(attrs);
    break;
  case _FareBreakInformation:
    onStartFareBreakInformation(attrs);
    break;
  case _PricedSolution:
    onStartPricedSolution(attrs);
    break;
  case _AccountCode:
    onStartAccountCode(attrs);
    break;
  case _CorpID:
    onStartCorpID(attrs);
    break;
  case _Flight:
    onStartFlight(attrs);
    break;
  case _FareBreakAssociation:
    onStartFareBreakAssociation(attrs);
    break;
  case _FormOfPayment:
    onStartFormOfPayment(attrs);
    break;
  case _DifferentialHighClass:
    onStartDifferentialHighClass(attrs);
    break;
  case _OBTicketingFeeRQ:
    onStartOBTicketingFeeRQ(attrs);
    break;
  case _Arunk:
    onStartArunk(attrs);
    break;
  case _PassengerIdentity:
    onStartPassengerIdentity(attrs);
    break;
  case _PassengerPaymentInfo:
    onStartPassengerPaymentInfo(attrs);
    break;
  case _Diagnostic:
    onStartDiagnostic(attrs);
    break;
  case _Option:
    onStartOption(attrs);
    break;
  default:
    LOG4CXX_DEBUG(_logger, "unprocessed element ID");
    break;
  }
  return true;
}
bool
STLTicketingFeesRequestHandler::endElement(int idx)
{
  switch (idx)
  {
  case _Agent:
    onEndAgent();
    break;
  case _BillingInformation:
    onEndBillingInformation();
    break;
  case _RequestOptions:
    onEndRequestOptions();
    break;
  case _RequestedDiagnostic:
    onEndRequestedDiagnostic();
    break;
  case _TravelSegment:
    onEndTravelSegment();
    break;
  case _FareBreakInformation:
    onEndFareBreakInformation();
    break;
  case _PricedSolution:
    onEndPricedSolution();
    break;
  case _AccountCode:
    onEndAccountCode();
    break;
  case _CorpID:
    onEndCorpID();
    break;
  case _Flight:
    onEndFlight();
    break;
  case _FareBreakAssociation:
    onEndFareBreakAssociation();
    break;
  case _FormOfPayment:
    onEndFormOfPayment();
    break;
  case _DifferentialHighClass:
    onEndDifferentialHighClass();
    break;
  case _OBTicketingFeeRQ:
    onEndOBTicketingFeeRQ();
    break;
  case _Option:
    onEndOption();
    break;
  case _Arunk:
    onEndArunk();
    break;
  case _PassengerIdentity:
    onEndPassengerIdentity();
    break;
  case _PassengerPaymentInfo:
    onEndPassengerPaymentInfo();
    break;
  case _Diagnostic:
    onEndDiagnostic();
    break;
  default:
    LOG4CXX_DEBUG(_logger, "unprocessed element ID");
    break;
  }

  _value.clear(); // Need to clear to be avaialable for next tag
  return true;
}

void
STLTicketingFeesRequestHandler::createTransaction(DataHandle& dataHandle,
                                                  const std::string& content)
{
  _trx = _pricingTrx = dataHandle.create<TktFeesPricingTrx>();

  _pricingTrx->dataHandle().get(_tktFeesRequest);
  _pricingTrx->setRequest(_tktFeesRequest);
  _tktFeesRequest->collectOBFee() = 'T';
  _tktFeesRequest->setCollectRTypeOBFee(false);
  _tktFeesRequest->setCollectTTypeOBFee(false);
  _tktFeesRequest->formOfPaymentCard() = 'T';
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
STLTicketingFeesRequestHandler::onStartAgent(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Agent");
}

void
STLTicketingFeesRequestHandler::onStartBillingInformation(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter BillingInformation");

  // pseudoCityCode
  getAttr(attrs, _pseudoCityCode, _pricingTrx->billing()->userPseudoCityCode());
  // agentCity
  getAttr(attrs, _agentCity, _pricingTrx->billing()->aaaCity());
  // actionCode
  getAttr(attrs, _billingActionCode, _pricingTrx->billing()->actionCode());
  // agentSineIn
  getAttr(attrs, _agentSineIn, _pricingTrx->billing()->aaaSine());
  // userSetAddress
  getAttr(attrs, _userSetAddress, _pricingTrx->billing()->userSetAddress());
  // partitionID
  std::string tmpBuf = "";
  getAttr(attrs, _partitionID, tmpBuf);
  if (MCPCarrierUtil::isPseudoCarrier(tmpBuf))
    _pricingTrx->mcpCarrierSwap() = true;
  // LATAM MCP-S
  std::string realCxr;
  if (!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
      MCPCarrierUtil::isNeutralCarrier(tmpBuf))
  {
    realCxr = MCPCarrierUtil::swapFromNeutralToActual(tmpBuf);
    _pricingTrx->billing()->partitionID() = realCxr;
  }
  else
  {
    _pricingTrx->billing()->partitionID() = tmpBuf;
    realCxr = MCPCarrierUtil::swapToActual(_pricingTrx, _pricingTrx->billing()->partitionID());
  }
  if (MCPCarrierUtil::isIAPCarrierRestricted(realCxr))
    throw ErrorResponseException(
        ErrorResponseException::MCP_IAP_RESTRICTED,
        ("UNABLE TO PROCESS-ENTRY RESTRICTED IN " + realCxr + " PARTITION").c_str());
  // parentTransactionID
  std::string transactionID;
  getAttr(attrs, _parentTransactionID, transactionID);
  _pricingTrx->billing()->parentTransactionID() =
      Billing::string2transactionId(transactionID.c_str());
  // clientTransactionID
  transactionID = "";
  getAttr(attrs, _clientTransactionID, transactionID);
  _pricingTrx->billing()->clientTransactionID() =
      Billing::string2transactionId(transactionID.c_str());
  // parentServiceName
  getAttr(attrs, _parentServiceName, _pricingTrx->billing()->parentServiceName());
  // sourceOfRequest
  getAttr(attrs, _sourceOfRequest, _pricingTrx->billing()->requestPath());
  // userBranch
  getAttr(attrs, _userBranch, _pricingTrx->billing()->userBranch());
  // userStation
  getAttr(attrs, _userStation, _pricingTrx->billing()->userStation());

  // businessFunction
}

void
STLTicketingFeesRequestHandler::onStartRequestOptions(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter RequestOptions");

  // requestDate
  std::string ticketingDate;
  getAttr(attrs, _requestDate, ticketingDate);

  // requestTimeOfDay
  std::string requestTimeOfDay;
  getAttr(attrs, _requestTimeOfDay, requestTimeOfDay);
  setDate(DateTime(ticketingDate, atoi(requestTimeOfDay.c_str())));

  // validatingCarrier
  getAttr(attrs, _validatingCarrier, _tktFeesRequest->validatingCarrier());
}

void
STLTicketingFeesRequestHandler::onStartRequestedDiagnostic(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter RequestedDiagnostic");

  // number
  const int16_t diagnosticNumber = attrs.get<int16_t>(_number, 0);
  _tktFeesRequest->diagnosticNumber() = diagnosticNumber;
  _pricingTrx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diagnosticNumber);
  _pricingTrx->diagnostic().activate();
}

void
STLTicketingFeesRequestHandler::onStartFlight(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Flight");
  checkFlightData(attrs);

  AirSeg* airSeg = nullptr;
  _pricingTrx->dataHandle().get(airSeg);
  if (airSeg == nullptr)
    throw std::runtime_error("Null pointer to int data");
  _currentTvlSegment = airSeg;

  setSegmentType(_segType[0]);

  // originAirport - Departure Airport Code
  getAttr(attrs, _originAirport, _currentTvlSegment->origAirport());
  // destinationAirport - Arrival Airport Code
  getAttr(attrs, _destinationAirport, _currentTvlSegment->destAirport());

  if (_currentTvlSegment->origAirport().empty() || _currentTvlSegment->destAirport().empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - MISSING CITY");
  // set origin/destionation
  _currentTvlSegment->origin() = _pricingTrx->dataHandle().getLoc(_currentTvlSegment->origAirport(),
                                                                  _pricingTrx->ticketingDate());
  _currentTvlSegment->destination() = _pricingTrx->dataHandle().getLoc(
      _currentTvlSegment->destAirport(), _pricingTrx->ticketingDate());
  if (_currentTvlSegment->origin() == nullptr || _currentTvlSegment->destination() == nullptr)
    throw ErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE);

  // departure time
  const std::string depTime = attrs.get<std::string>(_departureTime, "");

  _currentTvlSegment->pssDepartureTime() = pssTime(depTime);

  // departureDate - Departure Date
  const std::string departureDT = attrs.get<std::string>(_departureDate, "");
  // if departure date is present -use it, oterwise calcualte date
  if (departureDT.length() > 0)
  {
    _currentTvlSegment->departureDT() = convertDate(departureDT);
    setTime(_currentTvlSegment->departureDT(), depTime);
  }

  // airline - Marketing carrier code
  CarrierCode marketingCxr;
  getAttr(attrs, _airline, marketingCxr);
  if (marketingCxr.empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MISSING CARRIER");
  airSeg->setMarketingCarrierCode(MCPCarrierUtil::swapToActual(_trx, marketingCxr));
  airSeg->setOperatingCarrierCode(airSeg->marketingCarrierCode());

  // set multicity
  bool isInternational = LocUtil::isInternational(*(airSeg->origin()), *(airSeg->destination()));
  airSeg->boardMultiCity() =
      FareMarketUtil::getMultiCity(airSeg->carrier(),
                                   airSeg->origAirport(),
                                   isInternational ? GeoTravelType::International : GeoTravelType::Domestic,
                                   airSeg->departureDT());
  airSeg->offMultiCity() = FareMarketUtil::getMultiCity(airSeg->carrier(),
                                                        airSeg->destAirport(),
                                                        isInternational ? GeoTravelType::International : GeoTravelType::Domestic,
                                                        airSeg->departureDT());

  addTravelSegData();
}

void
STLTicketingFeesRequestHandler::onStartTravelSegment(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter TravelSegment");
  resetTravelSegData();
  _pnrSegment = attrs.get<int16_t>(_segmentOrderNumber, 0);
  getAttr(attrs, _stopOverInd, _indValueS);
  getAttr(attrs, _segmentType, _segType);
}

void
STLTicketingFeesRequestHandler::addTravelSegData() const
{
  _currentTvlSegment->pnrSegment() = _pnrSegment;
  _currentTvlSegment->segmentOrder() = _segmentOrder;

  _currentTvlSegment->resStatus() = CONFIRM_RES_STATUS;
  // stopOverInd
  if (_indValueS == "true")
    _currentTvlSegment->forcedStopOver() = 'Y';
  else
    _currentTvlSegment->forcedStopOver() = 'F';
}

void
STLTicketingFeesRequestHandler::resetTravelSegData()
{
  _pnrSegment = 0;
  _indValueS.clear();
  _segType = EMPTY_STRING();
}

void
STLTicketingFeesRequestHandler::onStartOBTicketingFeeRQ(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter TicketingFeesRequest");

  std::string fullVersion;
  getAttr(attrs, _version, fullVersion);

  uint16_t majorSchemaVersion = 1;
  if (fullVersion.size() > 2)
  {
    try
    {
      majorSchemaVersion = boost::lexical_cast<uint16_t>(fullVersion[0]);
      _tktFeesRequest->majorSchemaVersion() = majorSchemaVersion;
      _tktFeesRequest->fullVersionRQ() = fullVersion;
    }
    catch (boost::bad_lexical_cast&)
    {
    }
  }
}

void
STLTicketingFeesRequestHandler::onStartPricedSolution(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PricedSolution");

  _pricingTrx->dataHandle().get(_itin);
  _pricingTrx->dataHandle().get(_paxType);
  if (_paxType == nullptr)
    throw std::runtime_error("Null pointer to int data");

  // passengerType - Passenger type
  getAttr(attrs, _passengerType, _paxType->paxType());
  if (_paxType->paxType().empty())
    _paxType->paxType() = ADULT;
  _paxType->requestedPaxType() = _paxType->paxType();

  PaxTypeUtil::parsePassengerWithAge(*_paxType);

  TktFeesRequest::PaxTypePayment* ptp(nullptr);
  _pricingTrx->dataHandle().get(ptp);
  if (ptp == nullptr)
    throw std::runtime_error("Null pointer to PTP data");

  ptp->paxType() = _paxType;
  // totalPriceAmount
  std::string totalAmount;
  getAttr(attrs, _totalPriceAmount, totalAmount);
  ptp->amount() = atof(totalAmount.c_str());
  // set Equivalent(payment) currency code
  std::string paymentCurrency;
  getAttr(attrs, _paymentCurrency, paymentCurrency);
  ptp->currency() = paymentCurrency;
  checkCurrency(paymentCurrency);
  Money money(paymentCurrency);
  ptp->noDec() = money.noDec(_pricingTrx->ticketingDate());

  LocCode tktOverride;
  getAttr(attrs, _ticketingLocationOverride, tktOverride);
  ptp->tktOverridePoint() = tktOverride;

  addTotalAmount(ptp);
}

class IsSameTSegment
{
public:
  IsSameTSegment(SequenceNumber tsOrderNumber) : _tsNumber(tsOrderNumber) {}

  bool operator()(TravelSeg* ts) { return ts->pnrSegment() == _tsNumber; }

private:
  SequenceNumber _tsNumber;
};

void
STLTicketingFeesRequestHandler::onStartFareBreakAssociation(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FareBreakAssociation");
  checkFareBreakAssociationData(attrs);

  TktFeesRequest::TktFeesFareBreakAssociation* fba(nullptr);
  _pricingTrx->dataHandle().get(fba);
  if (fba == nullptr)
    throw std::runtime_error("Null pointer to FareBreakAssociation data");

  // set segment ID
  SequenceNumber tsOrderNumber = 0;
  attrs.get(_travelSegmentOrderNumber, tsOrderNumber, 0);

  fba->segmentID() = tsOrderNumber;
  TktFeesRequest::TktFeesFareBreakInfo* tktFeesFbiCurrent =
      _tktFeesRequest->tktFeesFareBreakPerItin()[_itin].back();
  if (tktFeesFbiCurrent == nullptr)
    throw std::runtime_error("Null pointer to FareBreakInformation data");
  fba->fareComponentID() = tktFeesFbiCurrent->fareComponentID();
  SequenceNumber id = 0;
  // sideTripOrderNum - Side Trip ID
  attrs.get(_sideTripOrderNum, id, 0);
  fba->sideTripID() = id;

  std::string indValue;
  getAttr(attrs, _sideTripStartInd, indValue);
  if (indValue == "true")
    fba->sideTripStart() = 'Y';
  else
    fba->sideTripStart() = 'N';

  std::string indValueE;
  getAttr(attrs, _sideTripEndInd, indValueE);
  if (indValueE == "true")
    fba->sideTripEnd() = 'Y';
  else
    fba->sideTripEnd() = 'N';
  std::string tktDesgn;
  getAttr(attrs, _ticketDesignator, tktDesgn);
  fba->tktDesignator() = tktDesgn;

  _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin].push_back(fba);
}

void
STLTicketingFeesRequestHandler::onStartFareBreakInformation(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FareBreakInformation");
  checkFareBreakInformationData(attrs);

  TktFeesRequest::TktFeesFareBreakInfo* tktFbi(nullptr);
  _pricingTrx->dataHandle().get(tktFbi);
  if (tktFbi == nullptr)
    throw std::runtime_error("Null pointer to TKTFareBreakInformation data");

  // compOrderNum - Fare Component ID
  tktFbi->fareComponentID() = attrs.get<SequenceNumber>(_compOrderNum, 0);
  // governingCarrier - Governing Carrier
  getAttr(attrs, _governingCarrier, tktFbi->governingCarrier());
  // fareClass - Fare basis code with slashes and resulting ticket designator
  getAttr(attrs, _fareBasisCode, tktFbi->fareBasis());

  _tktFeesRequest->tktFeesFareBreakPerItin()[_itin].push_back(tktFbi);
}

void
STLTicketingFeesRequestHandler::onStartAccountCode(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter AccountCode");
}

void
STLTicketingFeesRequestHandler::onStartCorpID(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter CorpID");
}

void
STLTicketingFeesRequestHandler::onStartFormOfPayment(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FormOfPayment");
  std::string bin;
  getAttr(attrs, _identity, bin);
  std::string type;
  getAttr(attrs, _type, type);

  TktFeesRequest::FormOfPayment* fop(nullptr);
  _pricingTrx->dataHandle().get(fop);
  if (fop == nullptr)
    throw std::runtime_error("Null pointer to FOP data");
  fop->fopBinNumber() = bin;
  fop->type() = type;

  std::string chargeAmount;
  getAttr(attrs, _chargeAmount, chargeAmount);
  if (chargeAmount.empty())
    fop->chargeAmountInRequest() = false;
  else
    fop->chargeAmount() = atof(chargeAmount.c_str());

  addFopBin(fop);
}

void
STLTicketingFeesRequestHandler::onStartDifferentialHighClass(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter DifferentialHighClass");
  // DifferentialHighClass - Differential Data
  TktFeesRequest::TktFeesDifferentialData* tfdd(nullptr);
  _pricingTrx->dataHandle().get(tfdd);
  if (tfdd == nullptr)
    throw std::runtime_error("Null pointer to DifferentialHighClass data");

  // governingCarrier - Differential carrier
  getAttr(attrs, _governingCarrier, tfdd->diffCarrierCode());
  // fareClass
  getAttr(attrs, _fareBasisCode, tfdd->fareBasis());

  std::vector<TktFeesRequest::TktFeesDifferentialData*>& tfddVec =
      _tktFeesRequest->tktFeesFareBreakPerItin()[_itin].back()->tktFeesDiffData();

  tfddVec.push_back(tfdd);
}

void
STLTicketingFeesRequestHandler::onStartArunk(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Arunk");
  ArunkSeg* arunkSeg = nullptr;
  _pricingTrx->dataHandle().get(arunkSeg);
  if (arunkSeg == nullptr)
    throw std::runtime_error("Null pointer to int data");
  arunkSeg->segmentType() = Arunk;
  _currentTvlSegment = arunkSeg;

  getAttr(attrs, _originCity, arunkSeg->origAirport());
  getAttr(attrs, _destinationCity, arunkSeg->destAirport());
  // set origin/destionation
  arunkSeg->origin() =
      _pricingTrx->dataHandle().getLoc(arunkSeg->origAirport(), _pricingTrx->ticketingDate());
  arunkSeg->destination() =
      _pricingTrx->dataHandle().getLoc(arunkSeg->destAirport(), _pricingTrx->ticketingDate());

  if (arunkSeg->origin() == nullptr || arunkSeg->destination() == nullptr)
    throw ErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE);

  addTravelSegData();
  LOG4CXX_DEBUG(_logger, "Leave Arunk");
}

void
STLTicketingFeesRequestHandler::onStartPassengerIdentity(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PassengerIdentity");

  checkPassengerIdentityData(attrs);
  TktFeesRequest::PassengerIdentity* pi(nullptr);
  _pricingTrx->dataHandle().get(pi);
  if (pi == nullptr)
    throw std::runtime_error("Null pointer to PIdentity data");

  pi->firstNameNumber() = attrs.get<int16_t>(_firstNameNumber, 0);
  pi->surNameNumber() = attrs.get<int16_t>(_surNameNumber, 0);
  pi->pnrNameNumber() = attrs.get<int16_t>(_pnrNameNumber, 0);
  pi->objectId() = attrs.get<int16_t>(_objectID, 0);
  _tktFeesRequest->paxId().push_back(pi);
}

void
STLTicketingFeesRequestHandler::onStartPassengerPaymentInfo(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PassengerPaymentInfo");
  checkPassengerPaymentInfoData(attrs);

  TktFeesRequest::PassengerPaymentInfo* ppi(nullptr);
  _pricingTrx->dataHandle().get(ppi);
  if (ppi == nullptr)
    throw std::runtime_error("Null pointer to PassengerPaymentInfo data");
  ppi->paxRefObjectID() = attrs.get<int16_t>(_passengerRefObjectID, 0);

  TktFeesRequest::PaxTypePayment* ptp = _tktFeesRequest->paxTypePaymentPerItin()[_itin];
  if (ptp == nullptr)
  {
    throw std::runtime_error("Null pointer to PassengerPayment data");
  }
  ptp->ppiV().push_back(ppi);
}

void
STLTicketingFeesRequestHandler::onStartDiagnostic(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Diagnostic");
}

void
STLTicketingFeesRequestHandler::onStartOption(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter Option");
}

void
STLTicketingFeesRequestHandler::onEndAgent()
{
  Agent* agent = _pricingTrx->getRequest()->ticketingAgent();
  getValue(agent->tvlAgencyPCC());
  CommonRequestHandler::onEndAGI();

  LOG4CXX_DEBUG(_logger, "Leave Agent");
}

void
STLTicketingFeesRequestHandler::onEndBillingInformation()
{
  _pricingTrx->billing()->updateTransactionIds(_pricingTrx->transactionId());
  _pricingTrx->billing()->updateServiceNames(Billing::SVC_TAX);

  LOG4CXX_DEBUG(_logger, "Leave BillingInformation");
}

void
STLTicketingFeesRequestHandler::onEndRequestOptions()
{
  //  checkCurrencyAndSaleLoc();

  LOG4CXX_DEBUG(_logger, "Leave RequestOptions");
}

void
STLTicketingFeesRequestHandler::onEndFormOfPayment()
{
  LOG4CXX_DEBUG(_logger, "Leave FormOfPayment");
}

void
STLTicketingFeesRequestHandler::onEndDifferentialHighClass()
{
  LOG4CXX_DEBUG(_logger, "Leave DifferentialHighClass");
}

void
STLTicketingFeesRequestHandler::onEndOBTicketingFeeRQ()
{
  detectRtw();
  LOG4CXX_DEBUG(_logger, "Leave OBTicketingFeeRQ");
}

void
STLTicketingFeesRequestHandler::onEndTravelSegment()
{
  if (nullptr == _currentTvlSegment)
    throw std::runtime_error("TravelSegment is not found");
  _tSeg.push_back(_currentTvlSegment);

  LOG4CXX_DEBUG(_logger, "Leave TravelSegment");
}

void
STLTicketingFeesRequestHandler::onEndPricedSolution()
{
  populateCurrentItin();
  fillMissingSegDataInCurrentItin();
  addTktDesignator();
  checkRequiredDataFbaFbi();
  addPaxType();
  _pricingTrx->itin().push_back(_itin);

  // fix sidetrip_tkt  checkSideTrip(_itin);
  _itin->setTravelDate(TseUtil::getTravelDate(_itin->travelSeg()));

  LOG4CXX_DEBUG(_logger, "Leave PricedSolution");
}

void
STLTicketingFeesRequestHandler::fillMissingSegDataInCurrentItin()
{
  _prevTvlSegment = nullptr;
  _currentTvlSegment = nullptr;
  for (TravelSeg* ts : _itin->travelSeg())
  {
    _currentTvlSegment = ts;
    if (_currentTvlSegment != nullptr && _currentTvlSegment->segmentType() == Arunk)
    {
      if (_prevTvlSegment != nullptr && _prevTvlSegment->segmentType() != Arunk)
      {
        _currentTvlSegment->departureDT() = _prevTvlSegment->arrivalDT();
        _currentTvlSegment->boardMultiCity() = _prevTvlSegment->offMultiCity();
      }
      else
        LOG4CXX_DEBUG(_logger, "Error in Arunk");
    }
    if (_prevTvlSegment != nullptr && _prevTvlSegment->segmentType() == Arunk)
    {
      _prevTvlSegment->arrivalDT() = _currentTvlSegment->departureDT();
      _prevTvlSegment->offMultiCity() = _currentTvlSegment->boardMultiCity();
    }

    if (_currentTvlSegment->departureDT() == DateTime::emptyDate())
    {
      if (_prevTvlSegment != nullptr)
      {
        if (_prevTvlSegment->departureDT() != DateTime::emptyDate())
        {
          DateTime dt = _prevTvlSegment->departureDT().addDays(1);
          _currentTvlSegment->departureDT() = DateTime(dt.year(), dt.month(), dt.day());
        }
        else
          _currentTvlSegment->departureDT() =
              DateTime(_curDate.year(), _curDate.month(), _curDate.day());
      }
      else
        _currentTvlSegment->departureDT() =
            DateTime(_curDate.year(), _curDate.month(), _curDate.day());
    }
    _prevTvlSegment = _currentTvlSegment;
  }
  return;
}

void
STLTicketingFeesRequestHandler::populateCurrentItin()
{
  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>& tktFeesFbaVec =
      _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin];
  if (tktFeesFbaVec.empty())
    throw std::runtime_error("Empty FareBreakAssociationPerItin");

  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>::iterator it = tktFeesFbaVec.begin();
  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>::iterator ie = tktFeesFbaVec.end();
  int16_t segmentOrder = 0;
  for (; it != ie; it++)
  {
    TktFeesRequest::TktFeesFareBreakAssociation* fba = (*it);
    std::vector<TravelSeg*>::iterator iter =
        std::find_if(_tSeg.begin(), _tSeg.end(), IsSameTSegment(fba->segmentID()));
    if (iter == _tSeg.end())
      throw std::runtime_error("TravelSegment is not found");
    TravelSeg* ts = (*iter);
    ts->segmentOrder() = segmentOrder;

    _itin->travelSeg().push_back(ts);
    _pricingTrx->travelSeg().push_back(ts);
    segmentOrder++;
  }
  LOG4CXX_DEBUG(_logger, "Leave populateCurrentItin");
}

void
STLTicketingFeesRequestHandler::onEndOption()
{
  // Diagnostic Options
  std::string diagArgData;
  getValue(diagArgData);
  if (diagArgData.length() >= 2)
  {
    _pricingTrx->diagnostic().diagParamMap().insert(std::map<std::string, std::string>::value_type(
        diagArgData.substr(0, 2), diagArgData.substr(2, diagArgData.length() - 2)));
  }
  LOG4CXX_DEBUG(_logger, "Leave Option");
}

void
STLTicketingFeesRequestHandler::onEndAccountCode()
{
  // accountCode - Account Code
  std::string accountCode;
  getValue(accountCode);
  if (!accountCode.empty())
    addAccountCode(accountCode);
  LOG4CXX_DEBUG(_logger, "Leave AccountCode");
}

void
STLTicketingFeesRequestHandler::onEndCorpID()
{
  // corpID - CorporateID
  std::string corpId;
  getValue(corpId);
  if (!corpId.empty())
    addCorpId(corpId);
  LOG4CXX_DEBUG(_logger, "Leave CorpID");
}

void
STLTicketingFeesRequestHandler::onEndFareBreakInformation()
{
  LOG4CXX_DEBUG(_logger, "Leave FareBreakInformation");
}

void
STLTicketingFeesRequestHandler::onEndFareBreakAssociation()
{
  LOG4CXX_DEBUG(_logger, "Leave FareBreakAssociation");
}

void
STLTicketingFeesRequestHandler::onEndFlight()
{
  LOG4CXX_DEBUG(_logger, "Leave Flight");
}

void
STLTicketingFeesRequestHandler::onEndRequestedDiagnostic()
{
  LOG4CXX_DEBUG(_logger, "Leave RequestedDiagnostic");
}

void
STLTicketingFeesRequestHandler::onEndPassengerIdentity()
{
  LOG4CXX_DEBUG(_logger, "Leave PassengerIdentity");
}

void
STLTicketingFeesRequestHandler::onEndPassengerPaymentInfo()
{
  LOG4CXX_DEBUG(_logger, "Leave PassengerPaymentInfo");
}

void
STLTicketingFeesRequestHandler::onEndDiagnostic()
{
  LOG4CXX_DEBUG(_logger, "Leave Diagnostic");
}

void
STLTicketingFeesRequestHandler::onEndArunk()
{
  LOG4CXX_DEBUG(_logger, "Leave Arunk")
}

void
STLTicketingFeesRequestHandler::checkFareBreakAssociationData(const IAttributes& attrs) const
{
  std::vector<int> reqFareBreakAssociationAttrs;
  reqFareBreakAssociationAttrs += _travelSegmentOrderNumber;
  checkData(attrs,
            reqFareBreakAssociationAttrs,
            *_STLTicketingFeesAttributeNames,
            "ATTRIBUTE IN FareBreakAssociation SECTION");
}

void
STLTicketingFeesRequestHandler::checkPassengerIdentityData(const IAttributes& attrs) const
{
  std::vector<int> reqPassengerIdentityAttrs;
  reqPassengerIdentityAttrs += _objectID;
  checkData(attrs,
            reqPassengerIdentityAttrs,
            *_STLTicketingFeesAttributeNames,
            "ATTRIBUTE IN PassengerIdentity SECTION");
}

void
STLTicketingFeesRequestHandler::checkPassengerPaymentInfoData(const IAttributes& attrs) const
{
  std::vector<int> reqPassengerPaymentInfoAttrs;
  reqPassengerPaymentInfoAttrs += _passengerRefObjectID;
  checkData(attrs,
            reqPassengerPaymentInfoAttrs,
            *_STLTicketingFeesAttributeNames,
            "ATTRIBUTE IN PassengerPaymentInfo SECTION");
}

void
STLTicketingFeesRequestHandler::checkFareBreakInformationData(const IAttributes& attrs) const
{
  std::vector<int> reqFareBreakInformationAttrs;
  reqFareBreakInformationAttrs += _compOrderNum, _governingCarrier, _fareBasisCode;
  checkData(attrs,
            reqFareBreakInformationAttrs,
            *_STLTicketingFeesAttributeNames,
            "ATTRIBUTE IN FareBreakInformation SECTION");
}

void
STLTicketingFeesRequestHandler::checkFlightData(const IAttributes& attrs) const
{
  std::vector<int> reqFlightAttrs;
  reqFlightAttrs += _originAirport, _destinationAirport, _airline;
  checkData(attrs, reqFlightAttrs, *_STLTicketingFeesAttributeNames, "ATTRIBUTE IN Flight SECTION");
}

namespace
{
struct TravelSegSegmentOrderComparator
{
  bool operator()(TravelSeg* s1, TravelSeg* s2) { return s1->segmentOrder() < s2->segmentOrder(); }
};
}

void
STLTicketingFeesRequestHandler::checkSideTrip(Itin* itin) const
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
STLTicketingFeesRequestHandler::checkRequiredDataFbaFbi() const
{
  typedef std::map<const SequenceNumber, bool> mapType;
  std::map<SequenceNumber, bool> segWithFareBreakAssociation;
  std::map<SequenceNumber, bool> FareBreakAssociationWithFareBreakInformation;

  for (TravelSeg* ts : _itin->travelSeg())
    segWithFareBreakAssociation[ts->pnrSegment()] = false;

  // mark travel segment if have coresponding FareBreakAssociation  section
  for (TktFeesRequest::TktFeesFareBreakAssociation* fba :
       _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin])
  {
    VALIDATE_OR_THROW(segWithFareBreakAssociation.find(fba->segmentID()) !=
                          segWithFareBreakAssociation.end(),
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - FareBreakAssociation SECTION WITH ORDER "
                          << fba->segmentID() << " NOT MATCHED TO Flight");
    segWithFareBreakAssociation[fba->segmentID()] = true;
    FareBreakAssociationWithFareBreakInformation[fba->fareComponentID()] = false;
  }
  // check if all sectors have matching FareBreakAssociation section
  for (mapType::value_type& pair : segWithFareBreakAssociation)
  {
    VALIDATE_OR_THROW(pair.second,
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - NO MATCHING FareBreakAssociation SECTION FOR SEGMENT "
                          << pair.first);
  }

  // mark FareBreakAssociation if have coresponding FareBreakInformation section
  for (TktFeesRequest::TktFeesFareBreakInfo* fbi :
       _tktFeesRequest->tktFeesFareBreakPerItin()[_itin])
  {
    VALIDATE_OR_THROW(FareBreakAssociationWithFareBreakInformation.find(fbi->fareComponentID()) !=
                          FareBreakAssociationWithFareBreakInformation.end(),
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - FareBreakInformation SECTION WITH FARE COMPONENT ID "
                          << fbi->fareComponentID() << " NOT MATCHED TO FareBreakAssociation");
    FareBreakAssociationWithFareBreakInformation[fbi->fareComponentID()] = true;
  }
  // check if all fare break association have fare component info
  for (const mapType::value_type& pair : FareBreakAssociationWithFareBreakInformation)
  {
    VALIDATE_OR_THROW(pair.second,
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - NO MATCHING FareBreakInformation SECTION FOR FareBreakAssociation "
                      "WITH FARE COMPONENT ID "
                          << pair.first);
  }
}

void
STLTicketingFeesRequestHandler::addAccountCode(const std::string& accountCode)
{
  std::vector<std::string>& accCodeVec = _tktFeesRequest->accountCodeIdPerItin()[_itin];
  if (std::find(accCodeVec.begin(), accCodeVec.end(), accountCode) == accCodeVec.end())
    accCodeVec.push_back(accountCode);
}

void
STLTicketingFeesRequestHandler::addCorpId(const std::string& corpId)
{
  bool isACCValid = _pricingTrx->dataHandle().corpIdExists(corpId, _pricingTrx->ticketingDate());
  std::vector<std::string>& corpIdVec = isACCValid ? _tktFeesRequest->corpIdPerItin()[_itin]
                                                   : _tktFeesRequest->invalidCorpIdPerItin()[_itin];

  if (std::find(corpIdVec.begin(), corpIdVec.end(), corpId) == corpIdVec.end())
    corpIdVec.push_back(corpId);
}

void
STLTicketingFeesRequestHandler::addFopBin(TktFeesRequest::FormOfPayment* fop)
{
  TktFeesRequest::PaxTypePayment* ptp = _tktFeesRequest->paxTypePaymentPerItin()[_itin];
  if (ptp == nullptr)
    throw std::runtime_error("Null pointer to PTP data");
  std::vector<TktFeesRequest::PassengerPaymentInfo*>& ppiV = ptp->ppiV();
  if (ppiV.empty())
    throw std::runtime_error("Empty PassengerPaymentInfoVector");
  ppiV.back()->fopVector().push_back(fop);
}

void
STLTicketingFeesRequestHandler::addTotalAmount(TktFeesRequest::PaxTypePayment* ptp)
{
  _tktFeesRequest->paxTypePaymentPerItin()[_itin] = ptp;
}

void
STLTicketingFeesRequestHandler::addTktDesignator()
{
  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>& tktFeesFbaVec =
      _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin];
  if (tktFeesFbaVec.empty())
    throw std::runtime_error("Empty FareBreakAssociationPerItin");

  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>::iterator it = tktFeesFbaVec.begin();
  std::vector<TktFeesRequest::TktFeesFareBreakAssociation*>::iterator ie = tktFeesFbaVec.end();
  for (; it != ie; it++)
  {
    TktFeesRequest::TktFeesFareBreakAssociation* fba = (*it);
    if (fba->tktDesignator().empty())
      continue;
    std::vector<TravelSeg*>::iterator iter = std::find_if(
        _itin->travelSeg().begin(), _itin->travelSeg().end(), IsSameTSegment(fba->segmentID()));
    if (iter == _itin->travelSeg().end())
      throw std::runtime_error("TravelSegment is not found");
    TravelSeg* ts = (*iter);
    _tktFeesRequest->tktDesignatorPerItin()[_itin].insert(
        std::pair<int16_t, TktDesignator>(ts->segmentOrder(), fba->tktDesignator()));
  }
  LOG4CXX_DEBUG(_logger, "addTktDesignator");
}

void
STLTicketingFeesRequestHandler::addPaxType()
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

bool
STLTicketingFeesRequestHandler::serviceSpecyficRtwProperty() const
{
  if (_tktFeesRequest->tktFeesFareBreakPerItin().empty())
    return false;

  typedef std::pair<const Itin*, std::vector<TktFeesRequest::TktFeesFareBreakInfo*>>
  ItinToFareBreaks;

  for (const ItinToFareBreaks& itfb : _tktFeesRequest->tktFeesFareBreakPerItin())
  {
    if (!itfb.second.empty())
    {
      const std::vector<TktFeesRequest::TktFeesFareBreakInfo*>& tffbiVec = itfb.second;

      return std::all_of(tffbiVec.cbegin(),
                         tffbiVec.cend(),
                         [](const TktFeesRequest::TktFeesFareBreakInfo* tffbi)
                         { return tffbi->fareComponentID() == 1; });
    }
  }

  return false;
}

void
STLTicketingFeesRequestHandler::setSegmentType(const char& segType)
{
  switch (segType)
  {
  case Air:
  {
    _currentTvlSegment->segmentType() = Air;
    break;
  }
  case Open:
  {
    _currentTvlSegment->segmentType() = Open;
    break;
  }
  default:
    _currentTvlSegment->segmentType() = UnknownTravelSegType;
  }
}

} // tse
