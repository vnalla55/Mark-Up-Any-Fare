#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/RtwUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DBAccess/OptionalServicesActivationInfo.h"
#include "Xform/AncillaryPricingRequestHandler.h"
#include "Xform/AncillarySchemaNames.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/DataModelMap.h"

#include <boost/assign/std/vector.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>
#include <functional>
#include <regex>

using namespace boost::assign;

namespace tse
{

FALLBACK_DECL(fallbackAB240);
FALLBACK_DECL(ab240FallbackNoC7bValidation);
FALLBACK_DECL(fallbackAncillaryPricingMetrics);
FALLBACK_DECL(fallbackNewRBDforM70);

DynamicConfigurableFlagOn
monetaryDiscoutEnabledCfg("ACTIVATION_FLAG", "MONETARY_DISCOUNT", false);

using namespace ancillary;

const std::string AncillaryPricingRequestHandler::RCP_CHECKIN_PATH = "CKI";
const std::string AncillaryPricingRequestHandler::RCP_RESERVATION_PATH = "RES";


namespace
{
Logger
_logger("atseintl.Xform.AncillaryPricingRequestHandler");

struct CheckPortionOfTravelIndicator : std::unary_function<TravelSeg*, bool>
{
  CheckPortionOfTravelIndicator(char portionOfTravelIndicator)
    : _portionOfTravelIndicator(portionOfTravelIndicator)
  {
  }

  bool operator()(const TravelSeg* travelSeg) const
  {
    return travelSeg->checkedPortionOfTravelInd() == _portionOfTravelIndicator;
  }

private:
  char _portionOfTravelIndicator;
};

struct IsTravelSegFromRequest : std::unary_function<TravelSeg*, bool>
{
  bool operator()(const TravelSeg* travelSeg) const
  {
    return travelSeg->pnrSegment() != ARUNK_PNR_SEGMENT_ORDER;
  }
};

bool
checkBaggageRouteIndicator(const std::vector<Itin*>& itins,
                           const char portionOfTravelIndicator,
                           bool found)
{
  for (Itin* itin : itins)
  {
    if (itin->getItinOrderNum() == 1)
      continue;

    std::vector<TravelSeg*> travelSegFromRequest;
    remove_copy_if(itin->travelSeg().begin(),
                   itin->travelSeg().end(),
                   back_inserter(travelSegFromRequest),
                   std::not1(IsTravelSegFromRequest()));

    if (found == (std::find_if(travelSegFromRequest.begin(),
                               travelSegFromRequest.end(),
                               CheckPortionOfTravelIndicator(portionOfTravelIndicator)) !=
                  travelSegFromRequest.end()))
      return true;
  }
  return false;
}

struct GroupCodeAndReqDataPredicate
{
  GroupCodeAndReqDataPredicate(RequestedOcFeeGroup::RequestedInformation ri, const ServiceGroup& groupCode) :
                  _ri(ri), _groupCode(groupCode) {}

  bool operator()(const RequestedOcFeeGroup& group) const
  {
    return (group.groupCode() == _groupCode) &&
           (group.getRequestedInformation() == _ri);
  }

  RequestedOcFeeGroup::RequestedInformation _ri;
  const ServiceGroup& _groupCode;
};

}

AncillaryPricingRequestHandler::AncillaryPricingRequestHandler(Trx*& trx)
  : CommonRequestHandler(trx), _processAllGroupsForAncillary(false),
    _isRfgElementOpen(false),
    _isItnElementOpen(false),
    _isPnmFound(false),
    _isOCFeeGroupAdded(false)
{
}

AncillaryPricingRequestHandler::~AncillaryPricingRequestHandler() {}

void
AncillaryPricingRequestHandler::parse(DataHandle& dataHandle, const std::string& content)
{
  CommonRequestHandler::parse(dataHandle, content, *this);
}

bool
AncillaryPricingRequestHandler::startElement(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case _AGI:
    onStartAGI(attrs);
    break;
  case _TAG:
    onStartTAG(attrs);
    break;
  case _BIL:
    onStartBIL(attrs);
    break;
  case _PRO:
    onStartPRO(attrs);
    break;
  case _RFG:
    onStartRFG(attrs);
    break;
  case _AST:
    onStartAST(attrs);
    break;
  case _BTS:
    onStartBTS(attrs);
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
  case _OSC:
    onStartOSC(attrs);
    break;
  case _FBI:
    onStartFBI(attrs);
    break;
  case _PXI:
    onStartPXI(attrs);
    break;
  case _PNM:
    _isPnmFound = true;
    onStartPNM(attrs);
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
  case _DynamicConfig:
    onStartDynamicConfig(attrs);
    break;
  case _AncillaryPricingRequest:
    onStartAncillaryPricingRequest(attrs);
    break;
  }
  return true;
}
bool
AncillaryPricingRequestHandler::endElement(int idx)
{
  switch (idx)
  {
  case _AGI:
    onEndAGI();
    break;
  case _TAG:
    onEndTAG();
    break;
  case _BIL:
    onEndBIL();
    break;
  case _PRO:
    onEndPRO();
    break;
  case _RFG:
    onEndRFG();
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
  case _PNM:
    onEndPNM();
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
  case _GroupCode:
    onEndS01();
    break;
  case _AncillaryPricingRequest:
    onEndAncillaryPricingRequest();
    break;
  }

  _value.clear(); // Need to clear to be avaialable for next tag
  return true;
}

void
AncillaryPricingRequestHandler::createTransaction(DataHandle& dataHandle,
                                                  const std::string& content)
{
  _trx = _pricingTrx = dataHandle.create<AncillaryPricingTrx>();

  _pricingTrx->dataHandle().get(_request);
  _pricingTrx->setRequest(_request);
  LOG4CXX_DEBUG(_logger, "Got _request");

  Agent* checkInAgent(nullptr);
  _pricingTrx->dataHandle().get(checkInAgent);
  _request->ticketingAgent() = checkInAgent;
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
AncillaryPricingRequestHandler::createTicketingAgent(uint8_t ticketNumber)
{
  Agent* ticketingAgent(nullptr);
  _pricingTrx->dataHandle().get(ticketingAgent);
  _request->setBaggageTicketingAgent(ticketingAgent, ticketNumber);
  LOG4CXX_DEBUG(_logger, "Got ticketing agent");
}

void
AncillaryPricingRequestHandler::onStartRFG(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter RFG");
  _isRfgElementOpen = true;

  // S01 - Group code
  getAttr(attrs, _S01, _currentRequestedOcFeeGroup.groupCode());

  if (!fallback::fallbackAB240(_pricingTrx))
  {
    // GBA - Baggage and Ancillary informations
    std::string gba;
    getAttr(attrs, _GBA, gba);

    if (_request && _request->majorSchemaVersion() >= 3)
    {
      if ("DXT" == gba)
        _currentRequestedOcFeeGroup.setRequestedInformation(RequestedOcFeeGroup::DisclosureData);
      else if ("CAT" == gba)
        _currentRequestedOcFeeGroup.setRequestedInformation(RequestedOcFeeGroup::CatalogData);
      else if ("OCF" == gba)
        _currentRequestedOcFeeGroup.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);
      else
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - GBA VALUE");
      }
      _isOCFeeGroupAdded = true;
    }
  }
  // Q0A - Number of Groups
  _currentRequestedOcFeeGroup.numberOfItems() = attrs.get<int>(_Q0A, 0);
}

void
AncillaryPricingRequestHandler::onStartAST(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter AST");

  if (_currentRequestedOcFeeGroup.groupCode().equalToConst("BG") ||
      _currentRequestedOcFeeGroup.groupCode().equalToConst("PT"))
  {
    std::string ancillaryServiceType;
    getAttr(attrs, _ASN, ancillaryServiceType);

    if (ancillaryServiceType != EMPTY_STRING())
      _currentRequestedOcFeeGroup.addAncillaryServiceType(ancillaryServiceType[0]);
  }

  if (_currentRequestedOcFeeGroup.groupCode().equalToConst("BG"))
  {
    if (_currentRequestedOcFeeGroup.isAncillaryServiceType(BAGGAGE_ALLOWANCE))
      _request->wpbgDisplayAllowance() = true;
    if (_currentRequestedOcFeeGroup.isAncillaryServiceType(BAGGAGE_CHARGE))
      _request->wpbgDisplayCharges() = true;
  }
}

void
AncillaryPricingRequestHandler::onEndS01()
{
  if (_isRfgElementOpen)
  {
    ServiceGroup groupCode;
    getValue(groupCode);
    _groupCodes.push_back(groupCode);
  }
}

void
AncillaryPricingRequestHandler::onStartBIL(const IAttributes& attrs)
{
  CommonRequestHandler::onStartBIL(attrs);

  // B05 - Validating carrier
  getAttr(attrs, _B05, _pricingTrx->billing()->validatingCarrier());
}

void
AncillaryPricingRequestHandler::onStartPRO(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PRO");

  if (_reqType == M70)
  {
    // AF0 - Ticketing point override
    getAttr(attrs, _AF0, _request->ticketPointOverride());

    // AG0 - Sale point override
    getAttr(attrs, _AG0, _request->salePointOverride());

    // PS1 - hard match indicator
    attrs.get(_PS1, _request->hardMatchIndicator(), false);
  }
  else if (isWPAEReqType())
  {
    // S14 - original Pricing command
    getAttr(attrs, _S14, _originalPricingCommand);
    _ignoreBE0 = !_originalPricingCommand.empty();
  }
  else if (isPostTktReqType())
  {
    // Q5X - number of hours before departure
    attrs.get(_Q5X, _request->noHoursBeforeDeparture(), uint32_t(4));
  }
  else if (isWPBGRequest())
  {
    // PS1 - hard match indicator
    attrs.get(_PS1, _request->hardMatchIndicator(), false);
  }
  // C45 - Equiv amount currency code
  std::string currencyOverride;
  getAttr(attrs, _C45, currencyOverride);
  _pricingTrx->getOptions()->currencyOverride() = currencyOverride;
  checkCurrency(_pricingTrx->getOptions()->currencyOverride());

  if (_reqType == M70 || isPostTktReqType() || isWPBGRequest() )
  {
    if (!_pricingTrx->getOptions()->currencyOverride().empty())
      _pricingTrx->getOptions()->mOverride() = 'T';
  }

  if (!isPostTktReqType())
  {
    // TKI - Ticketing Indicator
    attrs.get(_TKI, _pricingTrx->getOptions()->isTicketingInd(), false);
  }

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

  attrs.get(_P53, _request->exemptSpecificTaxes(), _request->exemptSpecificTaxes());
  attrs.get(_P54, _request->exemptAllTaxes(), _request->exemptAllTaxes());

  std::string taxCodeExempted;
  attrs.get(_BH0, taxCodeExempted, "");

  if (!taxCodeExempted.empty())
    _request->taxIdExempted().push_back(taxCodeExempted);

  getAttr(attrs, _B01, _request->carrierOverriddenForBaggageAllowance());
  getAttr(attrs, _B02, _request->carrierOverriddenForBaggageCharges());

  // L01 - select first for occurrence
  attrs.get(_L01, _request->selectFirstChargeForOccurrence(), true);

  // for AB240 _acsBaggageRequest should be updated in RFG elements if necessary
  if(_pricingTrx->activationFlags().isAB240())
    _acsBaggageRequest = false;

  parseResOrCheckinPath(attrs);
}

void
AncillaryPricingRequestHandler::parseResOrCheckinPath(const IAttributes& attrs)
{
  std::string resOrCheckInPath;
  getAttr(attrs, _RCP, resOrCheckInPath);

  if (!resOrCheckInPath.empty())
  {
    if (resOrCheckInPath == RCP_CHECKIN_PATH)
      _pricingTrx->getOptions()->setAncRequestPath(AncRequestPath::AncCheckInPath);
    else if (resOrCheckInPath == RCP_RESERVATION_PATH)
      _pricingTrx->getOptions()->setAncRequestPath(AncRequestPath::AncReservationPath);
    else
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - RCP VALUE");
  }
}

void
AncillaryPricingRequestHandler::onStartFLI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FLI");

  if (_reqType != M70)
    checkFLIDataForWPAE(attrs);

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
  const std::string departureDT = attrs.get<std::string>(_D01, "");

  // D02 - Arrival Date
  const std::string arrivalDT = attrs.get<std::string>(_D02, departureDT);

  const bool baggageRequest = _acsBaggageRequest || _MISC6BaggageRequest || isWPBGRequest();

  if (baggageRequest)
  {
    // Q0B - Flight Number
    const int flightNumber = attrs.get<int>(_Q0B, 0);
    _currentTvlSeg->marketingFlightNumber() = flightNumber;

    if (!flightNumber)
      _currentTvlSeg->segmentType() = Open;

    setSegmentDates(departureDT, depTime, arrivalDT, arrTime);

    if (arrTime.empty() && (_MISC6BaggageRequest || _request->wpbgPostTicket()))
    {
      // Force system to get arrival date from DSS
      _currentTvlSeg->arrivalDT() = DateTime::emptyDate();
    }
  }
  else
  {
    if (_reqType == M70)
    {
      // check if dates and times are present
      checkForDateConsistency(departureDT);
      checkForTimeConsistency(depTime, arrTime);
    }

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
          _currentTvlSeg->departureDT() =
              DateTime(_curDate.year(), _curDate.month(), _curDate.day());
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
  }

  // B00 - Marketing carrier code
  CarrierCode marketingCxr;
  getAttr(attrs, _B00, marketingCxr);
  if (marketingCxr.empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MISSING CARRIER");

  _currentTvlSeg->setMarketingCarrierCode(MCPCarrierUtil::swapToActual(_trx, marketingCxr));

  // B01 - Operating carrier code
  // if no operating carrier, will use marketing carrier
  CarrierCode operatingCxr = marketingCxr;
  getAttr(attrs, _B01, operatingCxr);
  _currentTvlSeg->setOperatingCarrierCode(MCPCarrierUtil::swapToActual(_trx, operatingCxr));

  // B30 - Class of Service
  BookingCode bookingCode;
  getAttr(attrs, _B30, bookingCode);
  if (!bookingCode.empty())
  {
    _currentTvlSeg->setBookingCode(bookingCode);
  }
  else if (_request->majorSchemaVersion() >= 2)
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MISSING CLASS OF SERVICE");
  }

  // Q0B - Flight Number
  _currentTvlSeg->marketingFlightNumber() = attrs.get<int>(_Q0B, 0);

  // S95
  getAttr(attrs, _S95, _currentTvlSeg->equipmentType());

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

  // C7B - Checked Portion of Travel Indicator (Baggage)
  _currentTvlSeg->setCheckedPortionOfTravelInd(attrs.get<char>(_C7B, ' '));

  // P72 - Forced connection
  attrs.get(_P72, _currentTvlSeg->forcedConx(), 'F');

  // P73 - Forced stopover
  attrs.get(_P73, _currentTvlSeg->forcedStopOver(), 'F');

  if (_reqType != M70)
  {
    int fliHashIndex = 1;
    if (!baggageRequest &&
        (_currentTvlSeg->resStatus().empty() || _currentTvlSeg->resStatus() == "  "))
    {
      fliHashIndex = -1;
      // mark open seg with empty date - will be set later
      if (departureDT.empty())
        _currentTvlSeg->departureDT() = DateTime::openDate();
      if (arrivalDT.empty())
        _currentTvlSeg->arrivalDT() = DateTime::openDate();
    }

    AncRequest::AncAttrMapTree& map =
        _request->itinAttrMap()[_itin]["FLI"][_currentTvlSeg->pnrSegment()];
    addAttributeToHashMap(map, _A01, attrs, 1);
    addAttributeToHashMap(map, _A02, attrs, 1);
    addAttributeToHashMap(map, _D31, attrs, fliHashIndex);
    addAttributeToHashMap(map, _D32, attrs, fliHashIndex);
    addAttributeToHashMap(map, _D01, attrs, fliHashIndex);
    addAttributeToHashMap(map, _D02, attrs, fliHashIndex);
    addAttributeToHashMap(map, _B00, attrs, 1);
    addAttributeToHashMap(map, _B01, attrs, 1);
    addAttributeToHashMap(map, _B30, attrs, -1);
    addAttributeToHashMap(map, _Q0B, attrs, fliHashIndex);
    //    addAttributeToHashMap(map, _BB0, attrs, 1);
    if(_pricingTrx->activationFlags().isAB240())
      addAttributeToHashMap(map, _S95, attrs, -1);
    addAttributeToHashMap(map, _N0E, attrs, -1);
    addAttributeToHashMap(map, _C7A, attrs, -1);
    addAttributeToHashMap(map, _C7B, attrs, -1);

    Itin *itn = _request->masterItin();
    if (!itn)
      itn = _request->subItin(_itinGroupNum);

    if (itn)
    {
      if (isPostTktReqType())
      {
        AncRequest::AncAttrMapTree& masterMap =
            _request->itinAttrMap()[itn]["FLI"][_currentTvlSeg->pnrSegment()];
        if (map.getAttributeValue("A01") != masterMap.getAttributeValue("A01") ||
            map.getAttributeValue("A02") != masterMap.getAttributeValue("A02") ||
            map.getAttributeValue("B00") != masterMap.getAttributeValue("B00") ||
            (_currentTvlSeg->resStatus() != "" && _currentTvlSeg->resStatus() != "  " &&
             map.getAttributeValue("D01") != masterMap.getAttributeValue("D01")))
          throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                       "ITINERARY/TRAVEL DATES CHANGED");
      }

      _currentTvlSeg->equipmentType() =
          _request->itinAttrMap()[itn]["FLI"][_currentTvlSeg->pnrSegment()]
              .getAttributeValue("S95");

      // if this is first pricing Itin - check carriers and uppdate Mater Itin if needed
      if (_request->pricingItins()[itn].size() == 0)
      {
        AncRequest::AncAttrMapTree& masterMap =
            _request->itinAttrMap()[itn]["FLI"][_currentTvlSeg->pnrSegment()];
        for (TravelSeg* ts : itn->travelSeg())
        {
          // find segment in masterItin
          if (ts->pnrSegment() == _currentTvlSeg->pnrSegment() && ts->isAir())
          {
            if (masterMap.getAttributeValue("B00") != map.getAttributeValue("B00"))
            {
              masterMap.addAttribute("B00", map.getAttributeValue("B00"), 1);
              static_cast<AirSeg*>(ts)->setMarketingCarrierCode(_currentTvlSeg->marketingCarrierCode());
            }
            if (masterMap.getAttributeValue("B01") != map.getAttributeValue("B01"))
            {
              masterMap.addAttribute("B01", map.getAttributeValue("B01"), 1);
              static_cast<AirSeg*>(ts)->setOperatingCarrierCode(_currentTvlSeg->operatingCarrierCode());
            }
            break;
          }
        }
      }
    }
  }
}

void
AncillaryPricingRequestHandler::onStartBTS(const IAttributes& attrs)
{
  // Q00 - Baggage Travel Index
  const int btIndex = attrs.get<int>(_Q00, 0);
  _request->displayBaggageTravelIndices().insert(btIndex);
}

void
AncillaryPricingRequestHandler::onStartFFY(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FFY");

  if (_itin != nullptr && !_ffData.empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID - FFY");

  // B00 - Carrier code
  CarrierCode carrier;
  getAttr(attrs, _B00, carrier);

  // B70 - Pax Type Code
  PaxTypeCode ptc;
  getAttr(attrs, _B70, ptc);

  // Q7D - Status
  int ffStatus = 0;
  attrs.get(_Q7D, ffStatus, 0);

  PaxType::FreqFlyerTierWithCarrier* ffd =
      _pricingTrx->dataHandle().create<PaxType::FreqFlyerTierWithCarrier>();
  ffd->setFreqFlyerTierLevel(ffStatus);
  ffd->setCxr(carrier);
  ffd->setIsFromPnr(false);
  addFrequentFlyerStatus(ffd, ptc);

  if (isWPBGRequest())
  {
    // N1O - VIP type
    int vipType = attrs.get<int>(_N1O, 0);

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
        addFrequentFlyerStatus(ffd, ptc);
      }
    }

    const Indicator pcc = attrs.get<Indicator>(_PCC, 'F');
    if (pcc == 'T')
    {
      PaxType::FreqFlyerTierWithCarrier* ffd =
          _pricingTrx->dataHandle().create<PaxType::FreqFlyerTierWithCarrier>();
      ffd->setFreqFlyerTierLevel(5);
      ffd->setCxr(carrier);
      addFrequentFlyerStatus(ffd, ptc);
    }
  }
}

void
AncillaryPricingRequestHandler::onStartAncillaryPricingRequest(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter AncillaryPricingRequest");

  std::string fullVersion;
  getAttr(attrs, _Version, fullVersion);

  uint16_t majorSchemaVersion = 1;
  if (!fullVersion.empty())
  {
    try { majorSchemaVersion = boost::lexical_cast<uint16_t>(fullVersion[0]); }
    catch (boost::bad_lexical_cast&) {}
  }
  _request->majorSchemaVersion() = majorSchemaVersion;
}

void
AncillaryPricingRequestHandler::onStartTAG(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter TAG");
  unsigned ticketNumber = 0;
  attrs.get(_TKN, ticketNumber, ticketNumber);

  if (ticketNumber > 0xFF)
  {
    std::ostringstream excMsg;
    excMsg << "INVALID TICKET NUMBER (" << ticketNumber << ")";
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, excMsg.str().c_str());
  }

  if (_request->isTicketNumberValid(ticketNumber))
  {
    std::ostringstream excMsg;
    excMsg << "DUPLICATED TICKET NUMBER (" << ticketNumber << ")";
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, excMsg.str().c_str());
  }

  createTicketingAgent(ticketNumber);
  _request->setActiveAgent(AncRequest::TicketingAgent, ticketNumber);
  readAgentAttrs(attrs, _request->ticketingAgent());
}

void
AncillaryPricingRequestHandler::onStartIRO(const IAttributes& attrs)
{
  _itinTicketIssueDate.emptyDate();
  CommonRequestHandler::onStartIRO(attrs);

  // D07
  if (attrs.has(_D07))
  {
    std::string ticketingDate;
    getAttr(attrs, _D07, ticketingDate);
    _request->ticketingDatesPerItin().insert(std::make_pair(_itin, convertDate(ticketingDate)));
  // RBD By Cabin needs ticket issuanse date
  // The _pricingTrx->isBaggageRequest() is for AB
  if ( _acsBaggageRequest || _MISC6BaggageRequest || isWPBGRequest())
    _itinTicketIssueDate = convertDate(ticketingDate);
  }
}

boost::optional<AncillaryPriceModifier::Type> priceModificationTypeFromString(const IValueString& elementValue)
{
  if (elementValue == "D")
    return boost::optional<AncillaryPriceModifier::Type>(AncillaryPriceModifier::Type::DISCOUNT);
  else if (elementValue == "R")
    return boost::optional<AncillaryPriceModifier::Type>(AncillaryPriceModifier::Type::RISE);
  else
    return boost::optional<AncillaryPriceModifier::Type>();
}

void
AncillaryPricingRequestHandler::onStartOSC(const IAttributes& attrs)
{
  if (_isItnElementOpen && _pricingTrx->activationFlags().isMonetaryDiscount())
  {
    if (!attrs.has(_AID))
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Attribute ITN/OSC/@AID is missing");

    AncillaryIdentifier ancillaryId = getAidAttributeValueFromOsc(attrs);
    AncillaryPriceModifier priceModifier;

    if (attrs.has(_PMI))
      priceModifier._identifier = getPmiAttributeValueFromOsc(attrs);

    if (attrs.has(_QTY))
      priceModifier._quantity = getQtyAttributeValueFromOsc(attrs);

    if (attrs.has(_DRT))
    {
      priceModifier._type = priceModificationTypeFromString(attrs.get(_DRT));
    }

    if (attrs.has(_DRP))
    {
      if (attrs.has(_DRM))
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Attributes DRP and DRM cannot be both present in a single ITN/OSC element");

      priceModifier._percentage = getDrpAttributeValueFromOsc(attrs, priceModifier._type == AncillaryPriceModifier::Type::DISCOUNT);
    }

    if (attrs.has(_DRM))
    {
      if (!attrs.has(_DRC))
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Attributes DRM is present but attribute DRC is not."
                                     " Currency is required for a monetary discount.");
      CurrencyCode currency = attrs.get(_DRC).c_str();
      CurrencyUtil::validateCurrencyCode(*_pricingTrx, currency);
      priceModifier._money = Money(getDrmAttributeValueFromOsc(attrs), currency);
    }

    _itin->addAncillaryPriceModifier(ancillaryId, priceModifier);
  }
}

AncillaryIdentifier
AncillaryPricingRequestHandler::getAidAttributeValueFromOsc(const IAttributes& attrs)
{
  std::string ancillaryId;
  getAttr(attrs, _AID, ancillaryId);
  return AncillaryIdentifier{ancillaryId};
}

std::string
AncillaryPricingRequestHandler::getPmiAttributeValueFromOsc(const IAttributes& attrs)
{
  std::string pmi;
  getAttr(attrs, _PMI, pmi);

  auto identicalIdentifier = _priceModificationIdentifiers.find(pmi);
  if (identicalIdentifier != _priceModificationIdentifiers.end())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
        "Repeated ITN/OSC/@PMI attribute of value: " + pmi + " found in request\n");

  _priceModificationIdentifiers.insert(pmi);
  return pmi;
}

unsigned int
AncillaryPricingRequestHandler::getQtyAttributeValueFromOsc(const IAttributes& attrs)
{
  std::string quantity;
  getAttr(attrs, _QTY, quantity);
  const std::string quantityPath = "ITN/OSC";
  return convertAttributeValueToUInt(quantityPath, _AncillaryAttributeNames[_QTY], quantity);
}

unsigned int
AncillaryPricingRequestHandler::getDrpAttributeValueFromOsc(const IAttributes& attrs, bool clampTo100Percent)
{
  std::string drpValue;
  getAttr(attrs, _DRP, drpValue);
  const std::string percentageModificationPath = "ITN/OSC";

  unsigned int percentage = convertAttributeValueToUInt(percentageModificationPath, _AncillaryAttributeNames[_DRP], drpValue);

  if (clampTo100Percent && percentage > 100)
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
        "Invalid input. Attribute " + percentageModificationPath + "/@" + _AncillaryAttributeNames[_DRP] + " with value of: "
        + std::to_string(percentage) + " exceeds limit of 100");

  return percentage;
}

MoneyAmount
AncillaryPricingRequestHandler::getDrmAttributeValueFromOsc(const IAttributes& attrs)
{
  std::string drmValue;
  getAttr(attrs, _DRM, drmValue);
  const std::string moneyDiscountPath = "ITN/OSC";

  MoneyAmount discount = convertAttributeValueToMoneyAmount(moneyDiscountPath, _AncillaryAttributeNames[_DRM], drmValue);

  return discount;
}

void
AncillaryPricingRequestHandler::checkValueWithRegex(const std::string& path,
                                                    const std::string& name,
                                                    const std::string& value,
                                                    const std::string& regex)
{
  if (!std::regex_match(value, std::regex(regex)))
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
        "Parsing attribute " + path + "/@" + name + " with value of: " + value
        + " failed on matching regexp: " + regex);
}

unsigned int
AncillaryPricingRequestHandler::convertAttributeValueToUInt(const std::string& path,
                                                            const std::string& name,
                                                            const std::string& value)
{
  const std::string uintRegex("^[0-9]+$");

  checkValueWithRegex(path, name, value, uintRegex);

  unsigned int convertedValue;
  try
  {
    convertedValue = std::stoul(value);
  }
  catch (const std::logic_error &ex)
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
        "Parsing attribute " + path + "/@" + name + " with value of: " + value
        + " failed on converting to unsigned long");
  }
  return convertedValue;
}

MoneyAmount
AncillaryPricingRequestHandler::convertAttributeValueToMoneyAmount(const std::string& path,
                                                                   const std::string& name,
                                                                   const std::string& value)
{
  const std::string moneyAmountRegex("^[0-9.]+$");

  checkValueWithRegex(path, name, value, moneyAmountRegex);

  return convertValueToMoneyAmount(path, name, value);
}

MoneyAmount
AncillaryPricingRequestHandler::convertValueToMoneyAmount(const std::string& path,
                                                          const std::string& name,
                                                          const std::string& value)
{
  MoneyAmount convertedValue;
  try
  {
    convertedValue = std::stod(value);
  }
  catch (const std::logic_error &ex)
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
        "Parsing attribute " + path + "/@" + name + " with value of: " + value
        + " failed on converting to money amount");
  }
  return convertedValue;
}

void
AncillaryPricingRequestHandler::onEndBIL()
{
  _pricingTrx->billing()->updateTransactionIds(_pricingTrx->transactionId());
  _pricingTrx->billing()->updateServiceNames(Billing::SVC_TAX);

  _pricingTrx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(
      TrxUtil::isEmdValidationFlightRelatedServiceAndPrepaidBaggageActive(*_pricingTrx));

  if (_request->majorSchemaVersion() >= 3)
  {
    if (fallback::fallbackAB240(_pricingTrx))
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "FAILED - AB240 IS NOT ACTIVE");
    }
    else
    {
      if (_pricingTrx->billing()->requestPath() == ANCS_PO_ATSE_PATH)
      {
        if(_actionCode.substr(0, 1) == "A")
        {
          _pricingTrx->modifiableActivationFlags().setAB240(true);

          if (TrxUtil::isEmdValidationInterlineChargeChecksActivated(*_pricingTrx))
          {
            if (TrxUtil::isRequestFromAS(*_pricingTrx))
              _pricingTrx->modifiableActivationFlags().setEmdForCharges(true);
          }

          _pricingTrx->modifiableActivationFlags().setMonetaryDiscount(monetaryDiscoutEnabledCfg.isValid(_pricingTrx));
        }
        else
          throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID REQUEST");

        if(_pricingTrx->activationFlags().isAB240())
        {
          // default behavior for AB240
          _acsBaggageRequest = true;
        }
      }
      else
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID REQUEST");
    }
  }
  else if (_request->majorSchemaVersion() == 2)
  { // Schema Version == 2
    // ReqPath = PPSS Valid action codes: WPBG, WPBG*, WFM*B, MISC6
    // ReqPath = PACS Valid action codes: A
    if (_pricingTrx->billing()->requestPath() == PSS_PO_ATSE_PATH)
    {
      if (_actionCode.substr(0, 5) == "WP*BG" || _actionCode.substr(0, 5) == "WFM*B")
      {
        _request->ancRequestType() = AncRequest::WPBGRequest;
        _reqType = WPBG;
      }
      else if (_actionCode.substr(0, 5) == "WPBG*")
      {
        _request->ancRequestType() = AncRequest::WPBGRequest;
        _request->wpbgPostTicket() = true;
        _reqType = WPBG;
      }
      else if (_actionCode.substr(0, 5) == "MISC6")
        _MISC6BaggageRequest = true;
      else
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID REQUEST");
    }
    else if (_pricingTrx->billing()->requestPath() == ACS_PO_ATSE_PATH)
    {
      _acsBaggageRequest = true;

      if (_actionCode.substr(0, 1) != "A")
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID REQUEST");
    }
    else
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID REQUEST");
  }
  else
  {
    if (!_actionCode.empty())
    {
      if (_actionCode.substr(0, 1) == "I" || _actionCode.substr(0, 5) == "WP*AE")
      {
        _request->ancRequestType() = AncRequest::WPAERequest;
        _reqType = WPAE;
      }
      else if (_actionCode.substr(0, 1) == "T" || _actionCode.substr(0, 5) == "WPAE*")
      {
        _request->ancRequestType() = AncRequest::PostTktRequest;
        _reqType = PostTkt;
      }
      else if (_actionCode.substr(0, 1) != "A")
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID REQUEST");
    }
    setNewRBDforM70ActivationFlag();
  }

  LOG4CXX_DEBUG(_logger, "Leave BIL");
}

void
AncillaryPricingRequestHandler::onEndRFG()
{
  LOG4CXX_DEBUG(_logger, "Leave RFG");
  _isRfgElementOpen = false;

  if (_currentRequestedOcFeeGroup.getRequestedInformation() == RequestedOcFeeGroup::NoData ||
      fallback::fallbackAB240(_pricingTrx))
    _pricingTrx->getOptions()->serviceGroupsVec().push_back(_currentRequestedOcFeeGroup);
  else
  {
    defineOcFeeGroup();
    _groupCodes.clear();
  }
   _currentRequestedOcFeeGroup.groupCode() = "";
   _currentRequestedOcFeeGroup.emptyAncillaryServiceType();
   _currentRequestedOcFeeGroup.numberOfItems() = 0;
}

void
AncillaryPricingRequestHandler::defineOcFeeGroup()
{
  switch(_currentRequestedOcFeeGroup.getRequestedInformation())
  {
  case RequestedOcFeeGroup::DisclosureData:
    insertDisclosure();
    break;
  case RequestedOcFeeGroup::CatalogData:
    insertCatalog();
    break;
  case RequestedOcFeeGroup::AncillaryData:
    insertAncillary();
    break;
  default:
    break;
  }
}

void
AncillaryPricingRequestHandler::insertDisclosure()
{
  insertRequestedOcFeeGroup(RequestedOcFeeGroup::DisclosureData, "BG", BAGGAGE_ALLOWANCE);
  insertRequestedOcFeeGroup(RequestedOcFeeGroup::DisclosureData, "BG", BAGGAGE_CHARGE);
  insertRequestedOcFeeGroup(RequestedOcFeeGroup::DisclosureData, "BG", CARRY_ON_ALLOWANCE);
  insertRequestedOcFeeGroup(RequestedOcFeeGroup::DisclosureData, "BG", BAGGAGE_EMBARGO);

  _acsBaggageRequest = true;
}

void
AncillaryPricingRequestHandler::insertCatalog()
{
  if (_groupCodes.empty())
  {
    insertRequestedOcFeeGroup(RequestedOcFeeGroup::CatalogData, "BG", BAGGAGE_CHARGE);
    insertRequestedOcFeeGroup(RequestedOcFeeGroup::CatalogData, "PT", BAGGAGE_CHARGE);
  }
  else
  {
    // remove all groups except BG and PT
    _groupCodes.erase(std::remove_if(_groupCodes.begin(), _groupCodes.end(),
                                      !boost::lambda::bind(&ServiceFeeUtil::checkServiceGroupForAcs, boost::lambda::_1)),
                       _groupCodes.end());

    if (std::find(_groupCodes.begin(), _groupCodes.end(), "BG") != _groupCodes.end() || _groupCodes.empty())
      insertRequestedOcFeeGroup(RequestedOcFeeGroup::CatalogData, "BG", BAGGAGE_CHARGE);

    if (std::find(_groupCodes.begin(), _groupCodes.end(), "PT") != _groupCodes.end() || _groupCodes.empty())
      insertRequestedOcFeeGroup(RequestedOcFeeGroup::CatalogData, "PT", BAGGAGE_CHARGE);
  }
  _acsBaggageRequest = true;
}

void
AncillaryPricingRequestHandler::insertRequestedOcFeeGroups(
    RequestedOcFeeGroup::RequestedInformation ri,
    const std::vector<OptionalServicesActivationInfo*>& activeGroups,
    const Indicator* serviceTypes)
{
  if (!activeGroups.empty())
  {
    for (const auto activeGroup : activeGroups)
    {
      if (!_pricingTrx->ticketingDate().isBetween(activeGroup->effDate().date(),
                                                  activeGroup->discDate().date()))
        continue;

      insertRequestedOcFeeGroups(ri, activeGroup->groupCode(), serviceTypes);
    }
  }
}

void
AncillaryPricingRequestHandler::insertAncillary()
{
  if (_groupCodes.empty())
  {
    _processAllGroupsForAncillary = true;
    UserApplCode userCode = getUserApplCode();

    Indicator crs = _pricingTrx->activationFlags().isAB240() ? MULTIHOST_USER_APPL : CRS_USER_APPL;
    static const std::string appPricing = "PRICING";
    insertRequestedOcFeeGroups(RequestedOcFeeGroup::AncillaryData,
                               _pricingTrx->dataHandle().getOptServiceActivation(crs, userCode, appPricing),
                               "FP");

    static const std::string appAll = "ALL";
    insertRequestedOcFeeGroups(RequestedOcFeeGroup::AncillaryData,
                               _pricingTrx->dataHandle().getOptServiceActivation(crs, userCode, appAll),
                               "FP");
  }
  else
  {
    for (ServiceGroup serviceGroup : _groupCodes)
      insertRequestedOcFeeGroup(RequestedOcFeeGroup::AncillaryData, serviceGroup);
  }

}

void
AncillaryPricingRequestHandler::insertDefaultOCFeeGroups()
{
  insertAncillary();
  insertCatalog();
  insertDisclosure();
}

void
AncillaryPricingRequestHandler::insertRequestedOcFeeGroup(RequestedOcFeeGroup::RequestedInformation ri,
    const ServiceGroup& groupCode, const Indicator& serviceType)
{
  RequestedOcFeeGroup& requestedOcFeeGroup = findOrCreateRequestedOcFeeGroup(ri, groupCode);
  requestedOcFeeGroup.addAncillaryServiceType(serviceType);
}

void
AncillaryPricingRequestHandler::insertRequestedOcFeeGroups(RequestedOcFeeGroup::RequestedInformation ri,
    const ServiceGroup& groupCode, const Indicator* serviceTypes)
{
  RequestedOcFeeGroup& requestedOcFeeGroup = findOrCreateRequestedOcFeeGroup(ri, groupCode);
  while(const Indicator serviceType = *serviceTypes++)
    requestedOcFeeGroup.addAncillaryServiceType(serviceType);
}

void
AncillaryPricingRequestHandler::insertRequestedOcFeeGroup(RequestedOcFeeGroup::RequestedInformation ri,
    const ServiceGroup& groupCode)
{
  findOrCreateRequestedOcFeeGroup(ri, groupCode);
}

RequestedOcFeeGroup&
AncillaryPricingRequestHandler::findOrCreateRequestedOcFeeGroup(RequestedOcFeeGroup::RequestedInformation ri, const ServiceGroup& groupCode)
{
  std::vector<RequestedOcFeeGroup>& serviceGroups = _pricingTrx->getOptions()->serviceGroupsVec();
  std::vector<RequestedOcFeeGroup>::iterator it = std::find_if(serviceGroups.begin(), serviceGroups.end(),
                                                               GroupCodeAndReqDataPredicate(ri, groupCode));
  if (it != serviceGroups.end())
    return *it;
  else
  {
    RequestedOcFeeGroup reqGroup;
    reqGroup.setRequestedInformation(ri);
    reqGroup.groupCode() = groupCode;
    serviceGroups.push_back(reqGroup);
    return serviceGroups.back();
  }
}


void
AncillaryPricingRequestHandler::onEndITN()
{
  bool isNewSubItin = false;
  bool isNewMasterItin = false;

  // reset before next ITN
  _prevTvlSeg = nullptr;
  _datesInRequest = FIRST_MATCH;
  _timesInRequest = FIRST_MATCH;

  if (_reqType != M70)
  {
    if (isWPAEReqType())
      parseOriginalPricingCommandWPAE();
    else if (isPostTktReqType())
    {
      if (!_unFlownSegMatch)
      {
        // if no unflown segment in master itin, then there won't be any in ticket itins also
        if (!_request->masterItin())
          throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                       "AIR EXTRAS NOT APPLICABLE FOR USED TICKET");

        // must have at least one unflown segment
        _itin = nullptr;
        _originalPricingCommand = "";
        return;
      }
      parseOriginalPricingCommandPostTkt();
    }

    // new master itin, or new sub itin for multi-ticket
    if ((!_request->isMultiTicketRequest() && _request->masterItin() == nullptr) ||
        ( _request->isMultiTicketRequest() && _request->subItin(_itinGroupNum)==nullptr))
    {
      if (_request->isMultiTicketRequest())
      {
        _request->subItins().insert( std::pair<uint16_t, Itin*>(_itinGroupNum,_itin) );
        isNewSubItin = true;
      }
      else
      {
        _request->masterItin() = _itin;
        isNewMasterItin = true;
      }
    }

    if (_request->isMultiTicketRequest() || !isNewMasterItin)
    {
      if (!_request->wpbgPostTicket() && !_pricingTrx->activationFlags().isAB240())
        checkFBAandFBIdata();

      if (!_request->isWPBGRequest())
      {
        for (Itin* it : _pricingTrx->itin())
        {
          // if same FLI data
          if (_request->itinAttrMap()[_itin].getHash(1) == _request->itinAttrMap()[it].getHash(1))
          {
            // preliminary comparsion against all pricing itins
            for (Itin* prcIt : _request->pricingItins()[it])
            {
              // try to merge on any duplicates if same FBA or FBI
              if (_request->itinAttrMap()[_itin].getHash(2) ==
                      _request->itinAttrMap()[prcIt].getHash(2) &&
                  _request->itinAttrMap()[_itin].getHash(3) ==
                      _request->itinAttrMap()[prcIt].getHash(3))
              {
                // for post tkt request, we also need to check IRO, ACI, CII, SGI
                if ((isPostTktReqType()) && (_request->itinAttrMap()[_itin].getHash(5) !=
                                              _request->itinAttrMap()[prcIt].getHash(5)))
                  continue;

                // try to merge and accumulate
                mergeSameFBAAndFBI(prcIt);
                if (!isNewSubItin)
                  return;
              }
            }
            // same FLI, but diferent FBA and/or FBI -> add new pricing Itin
            _request->pricingItins()[it].push_back(_itin);
            if (!isNewSubItin && !_pricingTrx->activationFlags().isAB240())
              return;
          }
        }
      }
      // no match to master itin -> multiple itins ?
    }
    if (isPostTktReqType())
    {
      // for WPAE* we can have ITN with diferent FLI than master ITN
      // if so than we remove master itin from trx->itin, we will use ticketed itins
      if (_pricingTrx->itin().size() == 1 && _pricingTrx->itin().front() == _request->masterItin())
      {
        _pricingTrx->itin().clear();
        // if any pricing itins exist, then restore them
        Itin* tmp = _itin;
        for (Itin* it : _request->pricingItins()[_request->masterItin()])
        {
          _itin = it;
          onEndITN();
        }
        _request->pricingItins().erase(_request->masterItin());
        _itin = tmp;
      }
    }
  }
  else if (_request->majorSchemaVersion() < 2 &&
           _request->fareBreakAssociationPerItin()[_itin].size() > 0)
  {
    // if there is at least one FBA element in the itinerary
    // validation of FBA and FBI data is performed
    checkFBAandFBIdata();
  }

  if (_processAllGroupsForAncillary || _pricingTrx->getOptions()->serviceGroupsVec().empty())
    _pricingTrx->getOptions()->isProcessAllGroups() = true;

  if (_reqType != M70)
  {
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
  }

  // M70 if no flight in _itin then return
  if (_reqType == M70 &&
      _request->ancRequestType() != AncRequest::BaggageRequest && _itin->travelSeg().empty())
  {
    // must have at least one unflown segment
    _itin = nullptr;
    LOG4CXX_DEBUG(_logger, "Leave ITN");
    return;
  }

  // check flight continuity TODO!
  checkFlight(*_itin,
              !_MISC6BaggageRequest && !_request->wpbgPostTicket(),
              _reqType == M70 && !_acsBaggageRequest && !_MISC6BaggageRequest);

  // if no passenger in request - add ADULT
  if (_reqType == M70)
    if (_request->paxTypesPerItin()[_itin].empty())
    {
      _pricingTrx->dataHandle().get(_paxType);
      if (_paxType == nullptr)
        throw std::runtime_error("Null pointer to int data");
      _paxType->paxType() = ADULT;
      _paxType->requestedPaxType() = ADULT;
      _paxType->number() = 1;
      addPaxType();
    }

  checkSideTrip(_itin);

  setFrequentFlyerStatus();

  if (!(_reqType == WPBG
        || _MISC6BaggageRequest
        || _pricingTrx->itin().empty()
        || _request->isMultiTicketRequest()
        || _pricingTrx->activationFlags().isAB240())
     )
  {
    checkFlights(_pricingTrx->itin().front(), _itin);
  }
  // for post tkt we don't keep master Itin like in WP*AE
  // if(_reqType != PostTkt ||
  //   ancReq->masterItin() != _itin)
  _pricingTrx->itin().push_back(_itin);
  if (isNewSubItin)
    _request->pricingItins()[_itin].push_back(_itin);

  _ffDataForItin.clear();
  _itin = nullptr;

  _isItnElementOpen = false;
  LOG4CXX_DEBUG(_logger, "Leave ITN");
}

void
AncillaryPricingRequestHandler::onEndPRO()
{
  checkCurrencyAndSaleLoc();
  if (_pricingTrx->activationFlags().isAB240() && !_isOCFeeGroupAdded)
  {
    insertDefaultOCFeeGroups();
    _isOCFeeGroupAdded = true;
  }
  LOG4CXX_DEBUG(_logger, "Leave PRO");
}

void
AncillaryPricingRequestHandler::onEndAncillaryPricingRequest()
{
  if (_reqType == M70 &&
      _request->ancRequestType() != AncRequest::BaggageRequest && _pricingTrx->itin().empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - HISTORICAL DATES NOT ALLOWED");

  if (_request->isMultiTicketRequest() )
  {
    _pricingTrx->itin().erase(_pricingTrx->itin().begin());
  }

  if (_pricingTrx->activationFlags().isAB240() && !_isOCFeeGroupAdded)
  {
    insertDefaultOCFeeGroups();
  }

  std::vector<Itin*>::iterator itinsIt = _pricingTrx->itin().begin();
  bool isBaggageOrAcsBaggage = (isWPBGRequest() || _MISC6BaggageRequest ||
                               (_pricingTrx->billing()->requestPath() == ACS_PO_ATSE_PATH &&
                                _pricingTrx->billing()->actionCode().substr(0, 1) == "A"));

  if ((isBaggageOrAcsBaggage || _pricingTrx->activationFlags().isAB240())
      && *itinsIt && (*itinsIt)->getItinOrderNum() == 1)
  {
    if (_pricingTrx->itin().size() >= 2)
      _pricingTrx->itin().erase(itinsIt);
    else
      throw ErrorResponseException(ErrorResponseException::NO_ITIN_SEGS_FOUND, "INVALID REQUEST");
    // sending ths exception if there is only one itin or no itin's

    if (_isPnmFound)
    {
      if (_actionCode == PRETKT)
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "INVALID REQUEST - TICKET IN PRETICKETED REQUEST");
      }
    }
    else
    {
      if (_actionCode == POSTTKT)
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "INVALID REQUEST - NO TICKET IN POSTTICKETED REQUEST");
      }
    }
  }

  if (fallback::ab240FallbackNoC7bValidation(_trx))
  {
    if (_acsBaggageRequest)
    {
      if (checkBaggageRouteIndicator(_pricingTrx->itin(), ' ', true))
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "BAGGAGE ROUTE MISSING INDICATOR");
      else if (checkBaggageRouteIndicator(_pricingTrx->itin(), 'T', false))
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "INVALID BAGGAGE ROUTE INDICATOR");
    }
  }
  else
  {
    if (!_pricingTrx->activationFlags().isAB240() && _acsBaggageRequest)
    {
      if (checkBaggageRouteIndicator(_pricingTrx->itin(), ' ', true))
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "BAGGAGE ROUTE MISSING INDICATOR");
      else if (checkBaggageRouteIndicator(_pricingTrx->itin(), 'T', false))
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "INVALID BAGGAGE ROUTE INDICATOR");
    }
  }

  detectRtw();

  if (!fallback::fallbackAncillaryPricingMetrics(_pricingTrx))
  {
    // Metrics recording setup
    if (_pricingTrx->diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::TOPLINE_METRICS))
    {
      _pricingTrx->recordMetrics() = true;
      _pricingTrx->recordTopLevelMetricsOnly() = true;
    }

    if (_pricingTrx->diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::FULL_METRICS))
    {
      _pricingTrx->recordMetrics() = true;
      _pricingTrx->recordTopLevelMetricsOnly() = false;
    }
  }

  if (_pricingTrx->activationFlags().isAB240())
  {
    if (!TrxUtil::isEmdValidationOnReservationPathForAB240Active(*_pricingTrx)
        && _pricingTrx->getOptions()->getAncRequestPath() == AncRequestPath::AncReservationPath)
    {
      _pricingTrx->modifiableActivationFlags().setEmdForCharges(false);
      _pricingTrx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(false);
    }

    if (!TrxUtil::isEmdValidationFlightRelatedOnCheckingPathForAB240Active(*_pricingTrx)
        && _pricingTrx->getOptions()->getAncRequestPath() == AncRequestPath::AncCheckInPath)
    {
      _pricingTrx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(false);
    }

  }
  else
  {
    if (!TrxUtil::isEmdValidationForM70Active(*_pricingTrx) && _reqType == M70)
    {
      _pricingTrx->modifiableActivationFlags().setEmdForCharges(false);
      _pricingTrx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(false);
    }
  }

  LOG4CXX_DEBUG(_logger, "Leave AncillaryPricingRequest");
}

void
AncillaryPricingRequestHandler::onEndTAG()
{
  finalizeAgent(_pricingTrx->getRequest()->ticketingAgent());
  _request->setActiveAgent(AncRequest::CheckInAgent);
  LOG4CXX_DEBUG(_logger, "Leave TAG");
}

namespace
{

bool
sameFlights(const TravelSeg* firstTravelSeg, const TravelSeg* secondTravelSeg)
{
  if (firstTravelSeg->isAir() && secondTravelSeg->isAir())
  {
    const AirSeg* firstAirSeg = static_cast<const AirSeg*>(firstTravelSeg);
    const AirSeg* secondAirSeg = static_cast<const AirSeg*>(secondTravelSeg);
    return firstAirSeg->origAirport() == secondAirSeg->origAirport() &&
           firstAirSeg->destAirport() == secondAirSeg->destAirport() &&
           firstAirSeg->departureDT() == secondAirSeg->departureDT() &&
           firstAirSeg->arrivalDT() == secondAirSeg->arrivalDT() &&
           firstAirSeg->marketingCarrierCode() == secondAirSeg->marketingCarrierCode() &&
           firstAirSeg->operatingCarrierCode() == secondAirSeg->operatingCarrierCode() &&
           firstAirSeg->getBookingCode() == secondAirSeg->getBookingCode() &&
           firstAirSeg->marketingFlightNumber() == secondAirSeg->marketingFlightNumber() &&
           firstAirSeg->equipmentType() == secondAirSeg->equipmentType();
  }
  else if (!firstTravelSeg->isAir() && !secondTravelSeg->isAir())
    return true;
  else
    return false;
}
}

void
AncillaryPricingRequestHandler::checkCurrencyAndSaleLoc()
{
  short utcDiff = getTimeDiff();
  if (utcDiff != 0)
    setDate(_curDate.addSeconds(utcDiff * 60));

  (static_cast<AncillaryPricingTrx*>(_pricingTrx))->checkCurrencyAndSaleLoc();
}

void
AncillaryPricingRequestHandler::checkFBAandFBIdata() const
{
  typedef std::map<const uint16_t, bool> mapType;
  std::map<uint16_t, bool> segWithFBA;
  std::map<uint16_t, bool> FBAWithFBI;
  // populate vector map segment Order
  for (TravelSeg* ts : _itin->travelSeg())
  {
    if (ts->isAir())
      segWithFBA[ts->segmentOrder()] = false;
  }
  // mark traavel segment if have coresponding FBA  section
  // populate FBA to FBI map
  for (AncRequest::AncFareBreakAssociation* fba : _request->fareBreakAssociationPerItin()[_itin])
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
  for (AncRequest::AncFareBreakInfo* fbi : _request->fareBreakPerItin()[_itin])
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
AncillaryPricingRequestHandler::checkFLIDataForWPAE(const IAttributes& attrs) const
{
  std::vector<int> reqFLIAttrs;
  std::string atrBB0 = "  ";
  getAttr(attrs, _BB0, atrBB0);
  if (atrBB0 != "  ")
    reqFLIAttrs += _A01, _A02, _D31, _D32, _D01, _D02, _B00, _B01, _Q0B;
  else
    reqFLIAttrs += _A01, _A02, _B00;

  // if not master itin
  if ( (_request->masterItin() || _request->subItin(_itinGroupNum))
       && !_request->isWPBGRequest())
    reqFLIAttrs += _B30, _N0E;

  for (int attr : reqFLIAttrs)
  {
    VALIDATE_OR_THROW(attrs.has(attr),
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - MISSING " << _AncillaryAttributeNames[attr]
                                           << " ATTRIBUTE IN FLI SECTION");
  }
}

void
AncillaryPricingRequestHandler::checkFlights(const Itin* pnrItin, const Itin* itin) const
{
  if (pnrItin->travelSeg().size() != itin->travelSeg().size() ||
      !std::equal(pnrItin->travelSeg().begin(),
                  pnrItin->travelSeg().end(),
                  itin->travelSeg().begin(),
                  sameFlights))
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID DATA - CHECK ITN1");
}

void
AncillaryPricingRequestHandler::onStartITN(const IAttributes& attrs)
{
  _isItnElementOpen = true;
  CommonRequestHandler::onStartITN(attrs);

  if (attrs.has(_GRP))
  {
    if (isWPAEReqType())
    {
      if (_request->isMultiTicketRequest() == false)
      {
        _request->masterItin() = nullptr;
        _request->paxTypesPerItin().clear();
        _request->isMultiTicketRequest() = true;
      }

      attrs.get(_GRP, _itinGroupNum, _itinGroupNum);
      _itin->setItinOrderNum(_itinGroupNum);
    }
  }

  if ((_itin->getItinOrderNum() != 1) && attrs.has(_TKN)) // Not a Master Itin - Master Itins don't have Ticket Numbers
  {
    if (_pricingTrx->activationFlags().isAB240()) // TKN is only processed in AncillaryPricingRequest v3 (AB240 project), it is ignored otherwise
    {
      unsigned ticketNumber = 0;
      attrs.get(_TKN, ticketNumber, ticketNumber);
      if ((ticketNumber > 0xFF) || (!_request->isTicketNumberValid(ticketNumber)))
      {
        std::ostringstream excMsg;
        excMsg << "INVALID TICKETING AGENT (TKN" << static_cast<unsigned>(ticketNumber) << ") IN ITINERARY " << static_cast<unsigned>(_itin->getItinOrderNum());
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, excMsg.str().c_str());
      }
      _itin->setTicketNumber(ticketNumber);
    }
  }
}

bool
AncillaryPricingRequestHandler::serviceSpecyficRtwProperty() const
{
  // FBA/Q6D need for below is not sent properly for all kind of trx
  // waiting for fix in xml request creator
  if (_actionCode.empty() || (_actionCode.substr(0, 1) != "A" && !isWPAEReqType()))
    return false;

  return RtwUtil::isRtwAncillaryRequest(*_request);
}

void
AncillaryPricingRequestHandler::setNewRBDforM70ActivationFlag()
{
  bool isRBDActive = !fallback::fallbackNewRBDforM70(_pricingTrx) &&
      (_reqType == M70) &&
      !_pricingTrx->activationFlags().isAB240();

  _pricingTrx->modifiableActivationFlags().setNewRBDbyCabinForM70(isRBDActive);
}

UserApplCode
AncillaryPricingRequestHandler::getUserApplCode()
{
  UserApplCode userCode;
  if (_pricingTrx->activationFlags().isAB240())
    userCode = _pricingTrx->billing()->partitionID();
  else if (_pricingTrx->getRequest()->ticketingAgent()->sabre1SUser())
    userCode = SABRE_USER;
  else if (_pricingTrx->getRequest()->ticketingAgent()->abacusUser())
    userCode = ABACUS_USER;
  else if (_pricingTrx->getRequest()->ticketingAgent()->axessUser())
    userCode = AXESS_USER;
  else
    userCode = INFINI_USER;

  return userCode;
}


} // tse

