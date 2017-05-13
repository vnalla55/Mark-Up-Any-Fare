//-------------------------------------------------------------------
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "FreeBagService/BaggageAllowanceValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/BaggageAncillarySecurityValidator.h"
#include "FreeBagService/BaggageAncillarySecurityValidatorAB240.h"
#include "FreeBagService/BaggageSecurityValidator.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/RegularBtaSubvalidator.h"
#include "FreeBagService/RegularNonBtaFareSubvalidator.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleUtil.h"
#include "ServiceFees/OCFees.h"
#include "TicketingFee/SvcFeesTktDesigValidator.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>

namespace tse
{
FALLBACK_DECL(ab240FixSsdmps171);
FALLBACK_DECL(fallbackGoverningCrxForT171);

struct IsRBD
{
  IsRBD(tse::AirSeg& seg, tse::BookingCode& bookingCode, tse::Diag877Collector* diag)
    : _seg(seg), _bookingCode(bookingCode), _diag(diag)
  {
  }

  bool operator()(tse::SvcFeesResBkgDesigInfo* info) const
  {
    StatusT198 status = PASS_T198;
    const CarrierCode& operatingCarrier =
        _seg.segmentType() == Open ? _seg.marketingCarrierCode() : _seg.operatingCarrierCode();

    if (_seg.marketingCarrierCode() != operatingCarrier &&
        info->mkgOperInd() == tse::OptionalServicesValidator::T198_MKT_OPER_IND_OPER)
      status = tse::FAIL_RBD_NOT_PROCESSED;
    else if (_seg.marketingCarrierCode() != info->carrier())
      status = tse::FAIL_ON_RBD_CXR;
    else
    {
      if (!(info->bookingCode1() == _bookingCode || info->bookingCode2() == _bookingCode ||
            info->bookingCode3() == _bookingCode || info->bookingCode4() == _bookingCode ||
            info->bookingCode5() == _bookingCode))
        status = tse::FAIL_ON_RBD_CODE;
    }
    if (UNLIKELY(_diag))
      _diag->printRBDTable198Info(
          _bookingCode, _seg.origAirport(), _seg.destAirport(), info, status);

    return (status == tse::PASS_T198);
  }

private:
  tse::AirSeg& _seg;
  tse::BookingCode& _bookingCode;
  tse::Diag877Collector* _diag;
};

struct IsOpenSegment
{
  bool operator()(const TravelSeg* travelSeg) const { return travelSeg->segmentType() == Open; }
};

BaggageAllowanceValidator::T183FieldDiagBuffer::T183FieldDiagBuffer(PricingTrx& trx)
  : _bufferForT183Table(nullptr)
{
  DiagManager diagMgr(trx, Diagnostic852);
  if (UNLIKELY(diagMgr.isActive()))
  {

    Diag852Collector* diag852 = &dynamic_cast<Diag852Collector&>(diagMgr.collector());
    if (diag852->shouldCollectInRequestedContext(Diag877Collector::PROCESSING_T183))
      _bufferForT183Table = static_cast<SvcFeesDiagCollector*>(diag852);
  }
}

bool
BaggageAllowanceValidator::T183FieldDiagBuffer::bufferingT183TableDiagRequired() const
{
  return _bufferForT183Table != nullptr;
}

SvcFeesDiagCollector*
BaggageAllowanceValidator::T183FieldDiagBuffer::getT183DiagBuffer() const
{
  return _bufferForT183Table;
}

void
BaggageAllowanceValidator::T183FieldDiagBuffer::flush(DiagCollector* bufferedDataTarget) const
{
  if (_bufferForT183Table)
  {
    if (bufferedDataTarget)
      (*bufferedDataTarget) << _bufferForT183Table->str();

    _bufferForT183Table->str("");
  }
}

BaggageAllowanceValidator::BaggageAllowanceValidator(const BagValidationOpt& opt)
  : CommonSoftMatchValidator(
        OcValidationContext(*opt._bt._trx, *opt._bt.itin(), *opt._bt.paxType(), opt._bt.farePath()),
        opt._bt.getTravelSegBegin(),
        opt._bt.getTravelSegEnd(),
        opt._bt.itin()->travelSeg().end(),
        opt._ts2ss,
        opt._isInternational,
        true,
        true,
        opt._diag),
    _baggageTravel(opt._bt),
    _furthestCheckedPoint(opt._fcp),
    _deferTargetCxr(opt._deferTargetCxr),
    _isUsDot(_baggageTravel.itin()->getBaggageTripType().isUsDot()),
    _t183TableBuffer(*opt._bt._trx)
{
  _nonBtaFareSubvalidator.reset(
      new RegularNonBtaFareSubvalidator(_baggageTravel, opt._ts2ss, _farePath));
  _btaSubvalidator.reset(new RegularBtaSubvalidator(
      _baggageTravel, *this, opt._ts2ss, static_cast<Diag852Collector*>(opt._diag)));

  const AncRequest* ancRq = dynamic_cast<const AncRequest*>(_trx.getRequest());
  if (UNLIKELY(ancRq && ancRq->majorSchemaVersion() >= 2))
  {
    _allowSoftMatch = !ancRq->hardMatchIndicator();
    _isAncillaryBaggage = true;
  }
}

void
BaggageAllowanceValidator::setBtaSubvalidator(IBtaSubvalidator* v)
{
  _btaSubvalidator.reset(v);
}

void
BaggageAllowanceValidator::setNonBtaFareSubvalidator(INonBtaFareSubvalidator* v)
{
  _nonBtaFareSubvalidator.reset(v);
}

const std::vector<OptionalServicesInfo*>&
BaggageAllowanceValidator::getOptionalServicesInfo(const SubCodeInfo& subCode) const
{
  const std::vector<OptionalServicesInfo*>& s7s =
      _trx.dataHandle().getOptionalServicesInfo(subCode.vendor(),
                                                subCode.carrier(),
                                                subCode.serviceTypeCode(),
                                                subCode.serviceSubTypeCode(),
                                                subCode.fltTktMerchInd(),
                                                _trx.ticketingDate());

  return s7s;
}

uint16_t
BaggageAllowanceValidator::getTvlDepartureDOW() const
{
  uint16_t depDOW = (*_segI)->departureDT().dayOfWeek();

  if (depDOW == 0)
    depDOW = 7;

  return depDOW;
}

void
BaggageAllowanceValidator::collectAllowance(const SubCodeInfo& s5,
                                            uint32_t startSeqNo,
                                            IAllowanceCollector& out)
{
  const std::vector<OptionalServicesInfo*>& optSrvInfos = getOptionalServicesInfo(s5);
  OCFees* ocFees = buildFees(s5);

  for (OptionalServicesInfo* s7 : optSrvInfos)
  {
    if (isCancelled(*s7) || s7->seqNo() < startSeqNo)
      continue;

    const StatusS7Validation rc = validateS7Data(*s7, *ocFees);
    const bool result = (rc == PASS_S7 || rc == PASS_S7_DEFER);

    if (!result)
    {
      ocFees->cleanBaggageResults();
      continue;
    }

    setSrvInfo(*s7, *ocFees);
    supplementFees(s5, ocFees);

    if (out.collect(*ocFees))
      break;

    ocFees = buildFees(s5);
  }
}

OCFees&
BaggageAllowanceValidator::validate(const SubCodeInfo& subCodeInfo, uint32_t startSeqNo)
{
  const std::vector<OptionalServicesInfo*>& optSrvInfos = getOptionalServicesInfo(subCodeInfo);

  if (_diag && !isDDInfo())
    _diag->printS7CommonHeader();

  if (optSrvInfos.empty())
    printDiagS7NotFound(subCodeInfo);

  OCFees* ocFees = buildFees(subCodeInfo);

  for (OptionalServicesInfo* optSrv : optSrvInfos)
  {
    if (isCancelled(*optSrv) || optSrv->seqNo() < startSeqNo)
      continue;

    checkDiagS7ForDetail(optSrv);
    StatusS7Validation rc = validateS7Data(*optSrv, *ocFees);

    if (UNLIKELY(ocFees->isAnyS7SoftPass()))
      rc = SOFT_PASS_S7;

    const bool result = (rc == PASS_S7 || rc == PASS_S7_DEFER);

    if (result)
      setSrvInfo(*optSrv, *ocFees);

    printDiagS7Info(optSrv, *ocFees, rc);

    if (result)
      break;

    ocFees->cleanOutCurrentSeg();
  }
  supplementFees(subCodeInfo, ocFees);
  return *ocFees;
}

bool
BaggageAllowanceValidator::isInLoc(const VendorCode& vendor,
                                   const LocKey& locKey,
                                   const Loc& loc,
                                   CarrierCode carrier) const
{
  return LocUtil::isInLoc(loc,
                          locKey.locType(),
                          locKey.loc(),
                          vendor,
                          RESERVED,
                          LocUtil::SERVICE_FEE,
                          _isIntrnl ? GeoTravelType::International : GeoTravelType::Domestic,
                          carrier,
                          _trx.ticketingDate());
}

StatusS7Validation
BaggageAllowanceValidator::validateS7Data_DEPRECATED(OptionalServicesInfo& optSrvInfo,
                                                     OCFees& ocFees)
{
  if (_diag && !isDiagSequenceMatch(optSrvInfo))
    return FAIL_S7_SEQUENCE;

  if (!checkTravelDate(optSrvInfo))
    return FAIL_S7_TVL_DATE;

  if (!checkInputPtc(optSrvInfo))
    return FAIL_S7_INPUT_PSG_TYPE;

  if (!checkFrequentFlyerStatus(optSrvInfo, ocFees))
    return FAIL_S7_FREQ_FLYER_STATUS;

  if (!checkAccountCodes(optSrvInfo.serviceFeesAccountCodeTblItemNo()))
    return FAIL_S7_ACCOUNT_CODE;

  if (!checkInputTicketDesignator(optSrvInfo.serviceFeesTktDesigTblItemNo()))
    return FAIL_S7_INPUT_TKT_DESIGNATOR;

  if (!checkSecurity(optSrvInfo))
    return FAIL_S7_SECUR_T183;

  {
    ScopeSwitcher switcher(this, optSrvInfo);

    if (!checkGeoFtwInd(optSrvInfo))
      return FAIL_S7_FROM_TO_WITHIN;

    std::vector<TravelSeg*> passedLoc3Dest;

    if (!checkIntermediatePoint(optSrvInfo, passedLoc3Dest))
      return FAIL_S7_INTERMEDIATE_POINT;

    if (!checkStopCnxDestInd(optSrvInfo, passedLoc3Dest))
      return FAIL_S7_STOP_CNX_DEST;
  }

  if (!checkCabin(optSrvInfo.cabin(), optSrvInfo.carrier(), ocFees))
    return FAIL_S7_CABIN;

  if (!checkRBD(optSrvInfo.vendor(), optSrvInfo.serviceFeesResBkgDesigTblItemNo(), ocFees))
    return FAIL_S7_RBD_T198;

  if (!checkResultingFareClass(
          optSrvInfo.vendor(), optSrvInfo.serviceFeesCxrResultingFclTblItemNo(), ocFees))
    return FAIL_S7_RESULT_FC_T171;

  if (!checkOutputTicketDesignator(optSrvInfo))
    return FAIL_S7_OUTPUT_TKT_DESIGNATOR;

  if (!checkRuleTariffInd(optSrvInfo.ruleTariffInd()))
    return FAIL_S7_RULE_TARIFF_IND;

  if (_allowSoftMatch
          ? !checkRuleTariff(optSrvInfo.ruleTariff(), ocFees)
          : !OptionalServicesValidator::checkRuleTariff(optSrvInfo.ruleTariff(), ocFees))
    return FAIL_S7_RULE_TARIFF;

  if (_allowSoftMatch ? !checkRule(optSrvInfo.rule(), ocFees)
                      : !OptionalServicesValidator::checkRule(optSrvInfo.rule(), ocFees))
    return FAIL_S7_RULE;

  if (!checkFareInd(optSrvInfo.fareInd()))
    return FAIL_S7_FARE_IND;

  if (!checkTourCode(optSrvInfo))
    return FAIL_S7_TOURCODE;

  bool skipDOWCheck = true;

  if (!checkStartStopTime(optSrvInfo, skipDOWCheck))
    return FAIL_S7_START_STOP_TIME;

  if (!skipDOWCheck && !checkDOW(optSrvInfo))
    return FAIL_S7_DOW;

  if (!checkCarrierFlightApplT186(optSrvInfo, optSrvInfo.carrierFltTblItemNo(), ocFees))
    return FAIL_S7_CXR_FLT_T186;

  if (!checkEquipmentType(optSrvInfo, ocFees))
    return FAIL_S7_EQUIPMENT;

  return checkServiceNotAvailNoCharge(optSrvInfo, ocFees);
}

StatusS7Validation
BaggageAllowanceValidator::validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
{
  if (UNLIKELY(!TrxUtil::isBaggageBTAActivated(_trx)))
    return validateS7Data_DEPRECATED(optSrvInfo, ocFees);

  if (UNLIKELY(_diag && !isDiagSequenceMatch(optSrvInfo)))
    return FAIL_S7_SEQUENCE;

  if (!checkTravelDate(optSrvInfo))
    return FAIL_S7_TVL_DATE;

  if (!checkInputPtc(optSrvInfo))
    return FAIL_S7_INPUT_PSG_TYPE;

  if (!checkFrequentFlyerStatus(optSrvInfo, ocFees))
    return FAIL_S7_FREQ_FLYER_STATUS;

  if (!checkAccountCodes(optSrvInfo.serviceFeesAccountCodeTblItemNo()))
    return FAIL_S7_ACCOUNT_CODE;

  if (!checkInputTicketDesignator(optSrvInfo.serviceFeesTktDesigTblItemNo()))
    return FAIL_S7_INPUT_TKT_DESIGNATOR;

  if (!checkSecurity(optSrvInfo))
    return FAIL_S7_SECUR_T183;

  {
    ScopeSwitcher switcher(this, optSrvInfo);

    if (!checkGeoFtwInd(optSrvInfo))
      return FAIL_S7_FROM_TO_WITHIN;

    std::vector<TravelSeg*> passedLoc3Dest;

    if (!checkIntermediatePoint(optSrvInfo, passedLoc3Dest))
      return FAIL_S7_INTERMEDIATE_POINT;

    if (UNLIKELY(!checkStopCnxDestInd(optSrvInfo, passedLoc3Dest)))
      return FAIL_S7_STOP_CNX_DEST;
  }

  StatusS7Validation status = checkBTA(optSrvInfo, ocFees);
  if (status != PASS_S7)
    return status;

  status = _nonBtaFareSubvalidator->validate(optSrvInfo, ocFees);
  if (status != PASS_S7)
    return status;

  bool skipDOWCheck = true;

  if (UNLIKELY(!checkStartStopTime(optSrvInfo, skipDOWCheck)))
    return FAIL_S7_START_STOP_TIME;

  if (UNLIKELY(!skipDOWCheck && !checkDOW(optSrvInfo)))
    return FAIL_S7_DOW;

  if (!checkEquipmentType(optSrvInfo, ocFees))
    return FAIL_S7_EQUIPMENT;

  return checkServiceNotAvailNoCharge(optSrvInfo, ocFees);
}

bool
BaggageAllowanceValidator::checkTravelDate(const OptionalServicesInfo& optSrvInfo) const
{
  const DateTime& firstTvlDate = (*_segI)->departureDT();

  try
  {
    if (ServiceFeeUtil::isStartDateSpecified(optSrvInfo) ||
        ServiceFeeUtil::isStopDateSpecified(optSrvInfo))
    {
      uint32_t tvlStartYear, tvlStartMonth, tvlStartDay, tvlStopYear, tvlStopMonth, tvlStopDay;

      if (ServiceFeeUtil::isStartDateSpecified(optSrvInfo))
      {
        tvlStartYear = optSrvInfo.tvlStartYear();
        tvlStartMonth = optSrvInfo.tvlStartMonth();
        tvlStartDay = optSrvInfo.tvlStartDay();
      }
      else
      {
        tvlStartYear = optSrvInfo.ticketEffDate().year();
        tvlStartMonth = optSrvInfo.ticketEffDate().month();
        tvlStartDay = optSrvInfo.ticketEffDate().day();
      }

      if (ServiceFeeUtil::isStopDateSpecified(optSrvInfo))
      {
        tvlStopYear = optSrvInfo.tvlStopYear();
        tvlStopMonth = optSrvInfo.tvlStopMonth();
        tvlStopDay = optSrvInfo.tvlStopDay();
      }
      else
      {
        tvlStopYear = optSrvInfo.ticketDiscDate().year();
        tvlStopMonth = optSrvInfo.ticketDiscDate().month();
        tvlStopDay = optSrvInfo.ticketDiscDate().day();
      }

      return tvlStartMonth && tvlStopMonth && // Months has to be valid number
             firstTvlDate.isBetween(
                 tvlStartYear, tvlStartMonth, tvlStartDay, tvlStopYear, tvlStopMonth, tvlStopDay);
    }
  }
  catch (std::exception&) { return false; }
  return (firstTvlDate.date() <= optSrvInfo.ticketDiscDate().date() &&
          firstTvlDate.date() >= optSrvInfo.ticketEffDate().date());
}

bool
BaggageAllowanceValidator::checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo,
                                                    OCFees& ocFees) const
{
  if (optSrvInfo.frequentFlyerStatus() == 0)
    return true;

  if (!_trx.getOptions()->isWPwithOutAE())
    return checkFrequentFlyerStatusImpl(optSrvInfo, ocFees);

  return false;
}

bool
BaggageAllowanceValidator::checkFrequentFlyerStatusImpl(const OptionalServicesInfo& optSrvInfo,
                                                        OCFees& ocFees) const
{
  if (_allowSoftMatch && _paxType.freqFlyerTierWithCarrier().empty())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_FREQFLYER_SOFT);
    return true;
  }

  for (const PaxType::FreqFlyerTierWithCarrier* ffData : _paxType.freqFlyerTierWithCarrier())
  {
    if (ffData->cxr() != optSrvInfo.carrier())
      continue;

    if (ffData->freqFlyerTierLevel() > optSrvInfo.frequentFlyerStatus())
      continue;

    return true;
  }
  return false;
}

StatusS7Validation
BaggageAllowanceValidator::checkServiceNotAvailNoCharge(const OptionalServicesInfo& info,
                                                        OCFees& ocFees) const
{
  if (info.notAvailNoChargeInd() == SERVICE_FREE_NO_EMD_ISSUED)
    return PASS_S7;

  if (!AllowanceUtil::isDefer(info) ||
      info.baggageTravelApplication() != BTA_APPLICATION_TYPE_EMPTY)
    return FAIL_S7_NOT_AVAIL_NO_CHANGE;

  if (_deferTargetCxr.empty() || _deferTargetCxr == info.carrier())
    return FAIL_S7_DEFER_BAGGAGE_RULE;

  if (info.notAvailNoChargeInd() == DEFER_BAGGAGE_RULES_FOR_MC && !_isUsDot &&
      TrxUtil::isIataReso302MandateActivated(_trx))
    return FAIL_S7_DEFER_BAGGAGE_RULE;

  if (info.notAvailNoChargeInd() == DEFER_BAGGAGE_RULES_FOR_OPC &&
      (_isUsDot || !TrxUtil::isIataReso302MandateActivated(_trx)))
    return FAIL_S7_DEFER_BAGGAGE_RULE;

  return PASS_S7_DEFER;
}

bool
BaggageAllowanceValidator::validateVia(const OptionalServicesInfo& optSrvInfo,
                                       std::vector<TravelSeg*>& passedLoc3Dest) const
{
  for (std::vector<TravelSeg*>::const_iterator i = _segI; i < _segIE - 1; ++i)
  {
    if (optSrvInfo.sectorPortionInd() == SEC_POR_IND_JOURNEY
        && shouldModifySegmentRange(_baggageTravel)
        && i == _furthestCheckedPoint.first)
      continue;

    if (optSrvInfo.viaLoc().isNull() || validateLocation(optSrvInfo.vendor(), // match via
                                                         optSrvInfo.viaLoc(),
                                                         *(*i)->destination(),
                                                         optSrvInfo.viaLocZoneTblItemNo(),
                                                         false,
                                                         optSrvInfo.carrier(),
                                                         nullptr))
    {
      passedLoc3Dest.push_back(*i);
    }
  }
  return (optSrvInfo.viaLoc().isNull() || !passedLoc3Dest.empty());
}

bool
BaggageAllowanceValidator::validateCnxOrStopover(const OptionalServicesInfo& optSrvInfo,
                                                 const std::vector<TravelSeg*>& passedLoc3Dest,
                                                 bool validateCnx) const
{
  if (_segI == _segIE - 1)
    return true;

  if (!optSrvInfo.stopoverTime().empty())
  {
    if (std::find_if(_segI, _segIE, IsOpenSegment()) != _segIE)
      return true;
  }

  uint16_t matched = 0;

  for (std::vector<TravelSeg*>::const_iterator i = _segI; i < _segIE - 1; ++i)
  {
    if (!findSegmentInVector(passedLoc3Dest, *i))
      continue; // we process only segments that passed Loc3 validation

    if ((*i)->segmentType() == Open)
      return true;

    bool stopOver = isStopover(optSrvInfo, *i, *(i + 1));

    if (validateCnx != stopOver)
      ++matched;
  }
  if (optSrvInfo.viaLoc().isNull()) // match all
    return (matched == passedLoc3Dest.size());

  return matched > 0;
}

bool
BaggageAllowanceValidator::validateStopoverWithGeo(const OptionalServicesInfo& optSrvInfo) const
{
  std::vector<TravelSeg*>::const_iterator end;

  if (optSrvInfo.sectorPortionInd() == SEC_POR_IND_JOURNEY && shouldModifySegmentRange(_baggageTravel))
    end = _furthestCheckedPoint.first;
  else
    end = _segIE - 1;

  std::vector<TravelSeg*>::const_iterator firstStopover = findFirstStopover(optSrvInfo, _segI, end);

  return firstStopover == end;
}

bool
BaggageAllowanceValidator::validateStartAndStopTime(const OptionalServicesInfo& info) const
{
  if (checkDOW(info))
  {
    if ((*_segI)->pssDepartureTime().empty() || (info.startTime() == 0 && info.stopTime() == 0))
      return true;

    uint16_t depTime = convertMinutesSinceMidnightToActualTime((*_segI)->pssDepartureTime());

    return (depTime >= info.startTime() && depTime <= info.stopTime());
  }
  return false;
}

bool
BaggageAllowanceValidator::validateStartAndStopTimeRange(const OptionalServicesInfo& info) const
{
  if (checkDOW(info))
  {
    if ((*_segI)->isOpenWithoutDate())
      return true;

    char depDOW;

    try { depDOW = (boost::lexical_cast<std::string>(getTvlDepartureDOW()))[0]; }
    catch (boost::bad_lexical_cast&) { return false; }
    std::string s7dow = info.dayOfWeek();
    char startDOW = *s7dow.begin();
    char endDOW = *(s7dow.end() - 1);

    uint16_t departTime = convertMinutesSinceMidnightToActualTime((*_segI)->pssDepartureTime());

    if (depDOW == startDOW)
    {
      if (info.startTime() == 0 || departTime >= info.startTime())
        return true;
    }
    else if (depDOW == endDOW)
    {
      if (info.stopTime() == 0 || departTime <= info.stopTime())
        return true;
    }
    else
      return true;
  }
  return false;
}

bool
BaggageAllowanceValidator::checkCarrierFlightApplT186(const OptionalServicesInfo& info,
                                                      uint32_t itemNo,
                                                      OCFees& ocFees) const
{
  if (itemNo == 0)
    return true;

  const TaxCarrierFlightInfo* cxrFltT186 =
      checkCarrierFlightApplT186Preconditions(itemNo, info.vendor());

  if (!cxrFltT186)
    return false;

  if (AllowanceUtil::isDefer(info))
  {
    const TravelSeg* specialSegment = determineSpecialSegmentForT186(_baggageTravel);
    if (!specialSegment)
      return false;

    return checkT186(dynamic_cast<const AirSeg*>(specialSegment), cxrFltT186);
  }

  bool ret = false;
  for (std::vector<TravelSeg*>::const_iterator i = _segI; i < _segIE; i++)
  {
    const AirSeg* seg = dynamic_cast<const AirSeg*>(*i);
    if (!seg)
      continue;
    ret = checkT186(seg, cxrFltT186);
    if (!ret)
      return false;
  }
  return ret;
}

const TaxCarrierFlightInfo*
BaggageAllowanceValidator::checkCarrierFlightApplT186Preconditions(uint32_t itemNo,
                                                                   const VendorCode& vendor) const
{
  if (_diag)
    _diag->printCarrierFlightT186Header(itemNo);

  const TaxCarrierFlightInfo* cxrFltT186 = getTaxCarrierFlight(vendor, itemNo);

  if (!cxrFltT186 || (cxrFltT186->segCnt() == 0) || cxrFltT186->segs().empty())
  {
    if (_diag)
      _diag->printNoCxrFligtT186Data();
    return nullptr;
  }
  return cxrFltT186;
}

bool
BaggageAllowanceValidator::checkT186(const AirSeg* seg, const TaxCarrierFlightInfo* cxrFltT186)
    const
{
  bool ret = false;
  std::vector<CarrierFlightSeg*>::const_iterator carrierFlightI = cxrFltT186->segs().begin();
  std::vector<CarrierFlightSeg*>::const_iterator carrierFlightEndI = cxrFltT186->segs().end();

  for (int i = 0; i < cxrFltT186->segCnt() && carrierFlightI != carrierFlightEndI;
       i++, carrierFlightI++)
  {
    CarrierFlightSeg* t186 = *carrierFlightI;
    StatusT186 rc186 = isValidCarrierFlight(*seg, *t186);

    if (UNLIKELY(_diag))
      _diag->printCarrierFlightApplT186Info(*t186, rc186);

    if (rc186 == PASS_T186)
    {
      ret = true;
      break;
    }
  }
  return ret;
}

bool
BaggageAllowanceValidator::checkGeoFtwInd(const OptionalServicesInfo& optSrvInfo) const
{
  const Loc* orig;
  const Loc* dest;

  switch (optSrvInfo.sectorPortionInd())
  {
  case SEC_POR_IND_PORTION:
    dest = (*(_segIE - 1))->destination();
    break;

  case SEC_POR_IND_JOURNEY:
    if(!shouldModifySegmentRange(_baggageTravel))
    {
      dest = (*(_segIE - 1))->destination();
      break;
    }

    if (optSrvInfo.fromToWithinInd() == FTW_TO)
      return false;

    dest = _furthestCheckedPoint.second == CP_AT_ORIGIN
               ? (*(_furthestCheckedPoint.first))->origin()
               : (*(_furthestCheckedPoint.first))->destination();
    break;

  case CHAR_BLANK:
  case CHAR_BLANK2:
    return true;

  default:
    return false;
  }

  orig = (*_segI)->origin();

  switch (optSrvInfo.fromToWithinInd())
  {
  case FTW_FROM:
    return validateFrom(optSrvInfo, *orig, *dest);
  case FTW_TO:
    return validateFrom(optSrvInfo, *dest, *orig);
  case CHAR_BLANK:
  case CHAR_BLANK2:
    return validateBetween(optSrvInfo, *orig, *dest);
  case FTW_WITHIN:
    return validateWithin(optSrvInfo);
  default:
    break;
  }
  return false;
}

bool
BaggageAllowanceValidator::checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                                                  std::vector<TravelSeg*>& passedLoc3Dest) const
{
  if (_segI + 1 == _segIE)
    return optSrvInfo.viaLoc().isNull();

  return validateVia(optSrvInfo, passedLoc3Dest);
}

bool
BaggageAllowanceValidator::checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                                               const std::vector<TravelSeg*>& passedLoc3Dest) const
{
  switch (optSrvInfo.stopCnxDestInd())
  {
  case SCD_CNX_POINT:
    return validateCnxOrStopover(optSrvInfo, passedLoc3Dest, true);
  case SCD_STOPOVER:
    return validateCnxOrStopover(optSrvInfo, passedLoc3Dest, false);
  case SCD_STOPOVER_WITH_GEO:
    return validateStopoverWithGeo(optSrvInfo);
  default:
    break; // do nothing
  }

  return true;
}

bool
BaggageAllowanceValidator::checkSecurity(const OptionalServicesInfo& optSrvInfo) const
{
  if (optSrvInfo.serviceFeesSecurityTblItemNo() <= 0)
    return optSrvInfo.publicPrivateInd() != T183_SCURITY_PRIVATE;

  bool view = false;
  if (UNLIKELY(_isAncillaryBaggage))
  {
    if (fallback::ab240FixSsdmps171(&_trx))
    {
      BaggageAncillarySecurityValidator securityValidator(
          static_cast<AncillaryPricingTrx&>(_trx),
          _segI,
          _segIE,
          _t183TableBuffer.bufferingT183TableDiagRequired(),
          false);
      return securityValidator.validate(optSrvInfo.seqNo(),
                                        optSrvInfo.serviceFeesSecurityTblItemNo(),
                                        view,
                                        optSrvInfo.vendor(),
                                        _t183TableBuffer.getT183DiagBuffer());
    }
    else
    {
      if(_trx.activationFlags().isAB240())
      {
        BaggageAncillarySecurityValidatorAB240 securityValidator(
            static_cast<AncillaryPricingTrx&>(_trx),
            _segI,
            _segIE,
            _t183TableBuffer.bufferingT183TableDiagRequired(),
            optSrvInfo.fltTktMerchInd(),
            _baggageTravel.itin());
        return securityValidator.validate(optSrvInfo.seqNo(),
                                          optSrvInfo.serviceFeesSecurityTblItemNo(),
                                          view,
                                          optSrvInfo.vendor(),
                                          _t183TableBuffer.getT183DiagBuffer());
      }
      else
      {
        BaggageAncillarySecurityValidator securityValidator(
            static_cast<AncillaryPricingTrx&>(_trx),
            _segI,
            _segIE,
            _t183TableBuffer.bufferingT183TableDiagRequired(),
            false);
        return securityValidator.validate(optSrvInfo.seqNo(),
                                          optSrvInfo.serviceFeesSecurityTblItemNo(),
                                          view,
                                          optSrvInfo.vendor(),
                                          _t183TableBuffer.getT183DiagBuffer());
      }
    }
  }
  else
  {
    BaggageSecurityValidator securityValidator(
        _trx, _segI, _segIE, _t183TableBuffer.bufferingT183TableDiagRequired());
    return securityValidator.validate(optSrvInfo.seqNo(),
                                      optSrvInfo.serviceFeesSecurityTblItemNo(),
                                      view,
                                      optSrvInfo.vendor(),
                                      _t183TableBuffer.getT183DiagBuffer());
  }
}

bool
BaggageAllowanceValidator::checkEquipmentType(const OptionalServicesInfo& info, OCFees& ocFees)
    const
{
  if (_trx.noPNRPricing() || info.equipmentCode().empty() || info.equipmentCode() == BLANK_CODE)
    return true;

  BOOST_FOREACH(const TravelSeg * tvlSeg, make_pair(_segI, _segIE))
  {
    if (tvlSeg->segmentType() == Open)
    {
      return true;
    }
    else
    {
      const bool hiddenStops = !tvlSeg->hiddenStops().empty();

      if (_allowSoftMatch &&
          (hiddenStops ? tvlSeg->equipmentTypes().empty() : tvlSeg->equipmentType().empty()))
      {
        ocFees.softMatchS7Status().set(OCFees::S7_EQUIPMENT_SOFT);
        return true;
      }
      else if (hiddenStops ? (std::find(tvlSeg->equipmentTypes().begin(),
                                        tvlSeg->equipmentTypes().end(),
                                        info.equipmentCode()) != tvlSeg->equipmentTypes().end())
                           : tvlSeg->equipmentType() == info.equipmentCode())
      {
        return true;
      }
    }
  }
  return false;
}

bool
BaggageAllowanceValidator::checkCabin(const Indicator cabin,
                                      const CarrierCode& carrier,
                                      OCFees& ocFees) const
{
  return checkCabinInSegment((*_baggageTravel._MSS), cabin);
}

bool
BaggageAllowanceValidator::checkCabinData(AirSeg& seg, const CabinType& cabin) const
{
  const auto tsI = _ts2ss.find(&seg);
  const PaxTypeFare::SegmentStatus* stat = (tsI != _ts2ss.end()) ? tsI->second.first : nullptr;

  const CabinType cabinType = (stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
                               stat->_reBookCabin.isValidCabin())
                                  ? stat->_reBookCabin
                                  : seg.bookedCabin();

  return cabin == cabinType;
}

bool
BaggageAllowanceValidator::checkTourCode(const OptionalServicesInfo& info) const
{
  if (info.tourCode().empty())
    return true; // When blank match S7
    
  // check if it is a cat 35 fare and if there is any tour code
  // if so validate it against S7 - tour code
  const CollectedNegFareData* negFareData = _farePath->collectedNegFareData();
  
  if (negFareData && negFareData->indicatorCat35())
  {
    const std::string& cat35TourCode = negFareData->tourCode();
    
    if (!cat35TourCode.empty())
      return (info.tourCode() == cat35TourCode);
  }

  // check if cat 27 tour code is equal to S7 - tour code
  const std::string& cat27TourCode = _farePath->cat27TourCode();
  
  if (!cat27TourCode.empty())
    return (info.tourCode() == cat27TourCode);
      
  const std::string& inputTourCode = _trx.getRequest()->getTourCode();

  if (!inputTourCode.empty())
    return (info.tourCode() == inputTourCode);

  return false;
}

bool
BaggageAllowanceValidator::checkStartStopTime(const OptionalServicesInfo& info, bool& skipDOWCheck)
    const
{
  switch (info.timeApplication())
  {
  case CHAR_BLANK:
  case CHAR_BLANK2:
    skipDOWCheck = false;
    return true;

  case TIME_DAILY:
    return validateStartAndStopTime(info);
    break;

  case TIME_RANGE:
    return validateStartAndStopTimeRange(info);
    break;

  default:
    return false;
  }
  return false;
}

bool
BaggageAllowanceValidator::checkDOW(const OptionalServicesInfo& info) const
{
  if (LIKELY(info.dayOfWeek().empty()))
    return true;

  char depDOW;

  try { depDOW = (boost::lexical_cast<std::string>(getTvlDepartureDOW()))[0]; }
  catch (boost::bad_lexical_cast&) { return false; }
  const std::string s7dow = info.dayOfWeek();

  return std::find(s7dow.begin(), s7dow.end(), depDOW) != s7dow.end();
}

StatusT186
BaggageAllowanceValidator::isValidCarrierFlight(const AirSeg& air, CarrierFlightSeg& t186) const
{
  if (air.marketingCarrierCode() != t186.marketingCarrier())
    return FAIL_ON_MARK_CXR;

  if (!t186.operatingCarrier().empty())
  {
    const CarrierCode& carrier =
        (air.segmentType() == Open ? air.marketingCarrierCode() : air.operatingCarrierCode());

    if (carrier != t186.operatingCarrier())
      return FAIL_ON_OPER_CXR;
  }

  if (t186.flt1() == -1) // DB returns '-1' for '****'
    return PASS_T186;

  return isValidCarrierFlightNumber(air, t186);
}

StatusT186
BaggageAllowanceValidator::isValidCarrierFlightNumber(const AirSeg& air, CarrierFlightSeg& t186)
    const
{
  if (air.segmentType() != Open)
  {
    if ((t186.flt2() == 0 && air.flightNumber() != t186.flt1()) ||
        (t186.flt2() != 0 &&
         (air.flightNumber() < t186.flt1() || air.flightNumber() > t186.flt2())))
      return FAIL_ON_FLIGHT;
  }
  return PASS_T186;
}

bool
BaggageAllowanceValidator::isStopover(const OptionalServicesInfo& optSrvInfo,
                                      const TravelSeg* seg,
                                      const TravelSeg* next) const
{
  if (optSrvInfo.stopoverUnit() != CHAR_BLANK && optSrvInfo.stopoverUnit() != CHAR_BLANK2)
  {
    char unit[2] = { optSrvInfo.stopoverUnit(), '\0' };

    PeriodOfStay maxStay(optSrvInfo.stopoverTime(), unit);
    DateTime startTime = seg->arrivalDT();
    DateTime endTime = next->departureDT();

    if (!seg->isAir())
      startTime = seg->departureDT();
    else if (!next->isAir())
      endTime = next->arrivalDT();

    DateTime calcStartTime;

    if (!optSrvInfo.stopoverTime().empty())
      calcStartTime = RuleUtil::addPeriodToDate(startTime, maxStay);
    else
      calcStartTime = startTime;

    if (optSrvInfo.stopoverUnit() == TIME_UNIT_DAY || optSrvInfo.stopoverUnit() == TIME_UNIT_MONTH)
    {
      if (optSrvInfo.stopoverTime().empty())
        return !(calcStartTime.day() == endTime.day());

      return (!endTime.isBetween(startTime.year(),
                                 startTime.month(),
                                 startTime.day(),
                                 calcStartTime.year(),
                                 calcStartTime.month(),
                                 calcStartTime.day()));
    }
    return endTime > calcStartTime;
  }

  if (seg->isForcedStopOver() || !seg->isAir())
    return true;

  if (seg->isForcedConx())
    return false;

  return next->isStopOverWithOutForceCnx(seg, _baggageTravel._stopOverLength);
}

bool
BaggageAllowanceValidator::isRBDValid(AirSeg* seg,
                                      const std::vector<SvcFeesResBkgDesigInfo*>& rbdInfos,
                                      OCFees& ocFees) const
{
  const auto tsI = _ts2ss.find(seg);
  const PaxTypeFare::SegmentStatus* stat = (tsI != _ts2ss.end()) ? tsI->second.first : nullptr;

  BookingCode bookingCode = (stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
                             !stat->_bkgCodeReBook.empty())
                                ? stat->_bkgCodeReBook
                                : seg->getBookingCode();

  Diag877Collector* const dc =
      (_diag && _diag->shouldCollectInRequestedContext(Diag877Collector::PROCESSING_RBD)) ? _diag
                                                                                          : nullptr;
  return std::any_of(rbdInfos.begin(), rbdInfos.end(), IsRBD(*seg, bookingCode, dc));
}

std::vector<TravelSeg*>::const_iterator
BaggageAllowanceValidator::findFirstStopover(const OptionalServicesInfo& optSrvInfo,
                                             const std::vector<TravelSeg*>::const_iterator& begin,
                                             const std::vector<TravelSeg*>::const_iterator& end)
    const
{
  for (std::vector<TravelSeg*>::const_iterator i = begin; i < end; ++i)
  {
    if (isStopover(optSrvInfo, *i, *(i + 1)))
      return i;
  }
  return end;
}

StatusT171
BaggageAllowanceValidator::isValidFareClassFareType(const PaxTypeFare* ptf,
                                                    SvcFeesCxrResultingFCLInfo& fclInfo,
                                                    OCFees& ocFees) const
{
  return _allowSoftMatch
             ? CommonSoftMatchValidator::isValidFareClassFareType(ptf, fclInfo, ocFees)
             : OptionalServicesValidator::isValidFareClassFareType(ptf, fclInfo, ocFees);
}

bool
BaggageAllowanceValidator::matchRuleTariff(const uint16_t& ruleTariff,
                                           const PaxTypeFare& ptf,
                                           OCFees& ocFees) const
{
  if (UNLIKELY(ptf.tcrRuleTariff() == -1))
    return true;

  return _allowSoftMatch ? CommonSoftMatchValidator::matchRuleTariff(ruleTariff, ptf, ocFees)
                         : OptionalServicesValidator::matchRuleTariff(ruleTariff, ptf, ocFees);
}

StatusT171
BaggageAllowanceValidator::isValidResultingFareClass(const PaxTypeFare* ptf,
                                                     SvcFeesCxrResultingFCLInfo& fclInfo,
                                                     OCFees& ocFees) const
{
  if (UNLIKELY(_isAncillaryBaggage))
  {
    if (!fallback::fallbackGoverningCrxForT171(&_trx))
    {
      if (_trx.activationFlags().isAB240())
      {
        if (!isValidFareClassCarrier(ptf, fclInfo))
          return FAIL_ON_CXR;
      }
      else
      {
        if (ptf->carrier() != fclInfo.carrier())
          return FAIL_ON_CXR;
      }
    }
    else
    {
      if (ptf->carrier() != fclInfo.carrier())
        return FAIL_ON_CXR;
    }

    return isValidFareClassFareType(ptf, fclInfo, ocFees);
  }
  else
  {
    return OptionalServicesValidator::isValidResultingFareClass(ptf, fclInfo, ocFees);
  }
}

void
BaggageAllowanceValidator::printT183TableBufferedData() const
{
  _t183TableBuffer.flush(_diag);
}

void
BaggageAllowanceValidator::printResultingFareClassInfo(const PaxTypeFare& ptf,
                                                       const SvcFeesCxrResultingFCLInfo& info,
                                                       StatusT171 status) const
{
  if (UNLIKELY(_isAncillaryBaggage && _diag))
  {
    if (!fallback::fallbackGoverningCrxForT171(&_trx))
    {
      if (_trx.activationFlags().isAB240())
        _diag->printResultingFareClassTable171Info(ptf, info, status, !ptf.carrier().empty());
      else
        _diag->printResultingFareClassTable171Info(ptf, info, status, true);
    }
    else
    {
      _diag->printResultingFareClassTable171Info(ptf, info, status, true);
    }
  }
  else
    OptionalServicesValidator::printResultingFareClassInfo(ptf, info, status);
}

OCFees*
BaggageAllowanceValidator::buildFees(const SubCodeInfo& subCodeInfo) const
{
  OCFees* ocFees = _trx.dataHandle().create<OCFees>();
  ocFees->subCodeInfo() = &subCodeInfo;
  return ocFees;
}

void
BaggageAllowanceValidator::supplementFees(const SubCodeInfo& subCodeInfo, OCFees* ocFees) const
{
  ocFees->subCodeInfo() = &subCodeInfo;
  ocFees->carrierCode() = subCodeInfo.carrier();
  ocFees->travelStart() = *(_segI);
  ocFees->travelEnd() = *(_segIE - 1);
  ocFees->farePath() = _farePath;
  ocFees->purchaseByDate() = time(nullptr);
  if (UNLIKELY(ocFees->optFee() && ocFees->optFee()->notAvailNoChargeInd() == 'X'))
    ocFees->isFeeGuaranteed() = false;
}

BaggageAllowanceValidator::ScopeSwitcher::ScopeSwitcher(BaggageAllowanceValidator* validator,
                                                        const OptionalServicesInfo& optSrvInfo)
  : _validator(validator), _restore(false)
{
  if (optSrvInfo.sectorPortionInd() == SEC_POR_IND_JOURNEY
      && validator->shouldModifySegmentRange(validator->_baggageTravel))
  {
    _segStartItToRestore = _validator->_segI;
    _segEndItToRestore = _validator->_segIE;
    _validator->_segI = _validator->_baggageTravel.itin()->travelSeg().begin();
    _validator->_segIE = _validator->_baggageTravel.itin()->travelSeg().end();
    _restore = true;
  }
}

BaggageAllowanceValidator::ScopeSwitcher::~ScopeSwitcher()
{
  if (_restore)
  {
    _validator->_segI = _segStartItToRestore;
    _validator->_segIE = _segEndItToRestore;
  }
}

const TravelSeg*
BaggageAllowanceValidator::determineSpecialSegmentForT186(const BaggageTravel& bt) const
{
  if (_deferTargetCxr.empty())
    return nullptr;
  return _isUsDot ? *bt._MSSJourney : bt._carrierTravelSeg;
}

bool
BaggageAllowanceValidator::checkCabinInSegment(TravelSeg* segment, const Indicator cabin) const
{
  if (cabin == BLANK)
    return true;

  if (LIKELY(segment && segment->isAir()))
    return checkCabinData(static_cast<AirSeg&>(*segment), mapS7CabinType(cabin));

  return true;
}

bool
BaggageAllowanceValidator::checkRBDInSegment(TravelSeg* segment,
                                             OCFees& ocFees,
                                             uint32_t serviceFeesResBkgDesigTblItemNo,
                                             const std::vector<SvcFeesResBkgDesigInfo*>& rbdInfos)
    const
{
  if (!serviceFeesResBkgDesigTblItemNo)
    return true;

  if (rbdInfos.empty())
    return false;

  if (_diag)
    _diag->printRBDTable198Header(serviceFeesResBkgDesigTblItemNo);

  if (segment && segment->isAir())
    return isRBDValid(static_cast<AirSeg*>(segment), rbdInfos, ocFees);

  return true;
}

bool
BaggageAllowanceValidator::checkResultingFareClassInSegment(
    const PaxTypeFare* paxTypeFare,
    uint32_t serviceFeesCxrResultingFclTblItemNo,
    OCFees& ocFees,
    const std::vector<SvcFeesCxrResultingFCLInfo*>& resFCLInfo) const
{
  if (!serviceFeesCxrResultingFclTblItemNo)
    return true;

  if (nullptr == paxTypeFare)
    return false;

  if (_diag)
    _diag->printResultingFareClassTable171Header(serviceFeesCxrResultingFclTblItemNo);

  bool res = false;
  for (SvcFeesCxrResultingFCLInfo* fcl : resFCLInfo)
  {
    StatusT171 resT171 = isValidResultingFareClass(paxTypeFare, *fcl, ocFees);
    printResultingFareClassInfo(*paxTypeFare, *fcl, resT171);

    if (resT171 == PASS_T171)
      return true;

    if (resT171 == SOFTPASS_FARE_CLASS || resT171 == SOFTPASS_FARE_TYPE)
      res = true;
  }
  return res;
}

bool
BaggageAllowanceValidator::checkOutputTicketDesignatorInSegment(TravelSeg* segment,
                                                                const PaxTypeFare* ptf,
                                                                const OptionalServicesInfo& s7)
    const
{
  using DC = Diag877Collector;

  if (s7.resultServiceFeesTktDesigTblItemNo() == 0) // No T173
    return true;

  DC* dc = (_diag && _diag->shouldCollectInRequestedContext(DC::PROCESSING_OUTPUT_TICKET_DESIG))
               ? _diag
               : nullptr;
  SvcFeesOutputTktDesigValidator validator(_trx, *ptf, dc);
  return svcFeesTktDesignatorValidate(validator, s7.resultServiceFeesTktDesigTblItemNo());
}

bool
BaggageAllowanceValidator::checkRuleInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                              const RuleNumber& rule,
                                              OCFees& ocFees) const
{
  if (rule.empty())
    return true;

  if (_allowSoftMatch &&
      (nullptr == paxTypeFareLinkedWithSegment || paxTypeFareLinkedWithSegment->ruleNumber().empty()))
  {
    ocFees.softMatchS7Status().set(OCFees::S7_RULE_SOFT);
    return true;
  }

  if (nullptr == paxTypeFareLinkedWithSegment)
    return true;

  return paxTypeFareLinkedWithSegment->ruleNumber() == rule;
}

bool
BaggageAllowanceValidator::checkRuleTariffInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                                    uint16_t ruleTariff,
                                                    OCFees& ocFees) const
{
  if (ruleTariff == (uint16_t) - 1)
    return true;

  if (_allowSoftMatch && nullptr == paxTypeFareLinkedWithSegment)
  {
    ocFees.softMatchS7Status().set(OCFees::S7_RULETARIFF_SOFT);
    return true;
  }

  if (nullptr == paxTypeFareLinkedWithSegment)
    return true;

  return matchRuleTariff(ruleTariff, *paxTypeFareLinkedWithSegment, ocFees);
}

bool
BaggageAllowanceValidator::checkFareIndInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                                 const Indicator& fareInd) const
{
  if (fareInd == CHAR_BLANK || fareInd == CHAR_BLANK2)
    return true;

  if (nullptr == paxTypeFareLinkedWithSegment)
    return true;

  return checkFareInd(fareInd, *paxTypeFareLinkedWithSegment);
}

StatusS7Validation
BaggageAllowanceValidator::checkBTA(OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
{
  return _btaSubvalidator->validate(optSrvInfo, ocFees);
}

bool
BaggageAllowanceValidator::checkCarrierFlightApplT186InSegment(const TravelSeg* segment,
                                                               const VendorCode& vendor,
                                                               uint32_t itemNo) const
{
  if (itemNo == 0)
    return true;

  if (nullptr == segment || !segment->isAir())
    return false;

  const TaxCarrierFlightInfo* cxrFltT186 = checkCarrierFlightApplT186Preconditions(itemNo, vendor);

  if (nullptr == cxrFltT186)
    return false;

  return checkT186(static_cast<const AirSeg*>(segment), cxrFltT186);
}

bool
BaggageAllowanceValidator::isCancelled(const OptionalServicesInfo& optSrvInfo) const
{
  return (optSrvInfo.effDate() == optSrvInfo.discDate() &&
          optSrvInfo.effDate() == _trx.ticketingDate());
}

bool
BaggageAllowanceValidator::shouldModifySegmentRange(const BaggageTravel& baggageTravel) const
{
  const bool isShortCheck =
    std::any_of(baggageTravel.getTravelSegBegin(), baggageTravel.getTravelSegEnd(),
                [](const TravelSeg* seg) { return seg->checkedPortionOfTravelInd() != 'T'; });

  const bool isUsDot = baggageTravel.itin()->getBaggageTripType().isUsDot();
  const bool isAB240 = baggageTravel._trx->activationFlags().isAB240();

  return (isUsDot || !isAB240 || !isShortCheck);
}

} // tse
