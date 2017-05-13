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

#include "ServiceFees/AncillaryPricingValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "Diagnostic/Diag877Collector.h"
#include "Rules/RuleConst.h"
#include "ServiceFees/AncillarySecurityValidator.h"
#include "ServiceFees/OCFees.h"

#include <boost/bind.hpp>

using namespace std;

namespace tse
{

static Logger
logger("atseintl.ServiceFees.AncillaryPricingValidator");

bool
AncillaryPricingValidator::validate(OCFees& ocFees, bool stopMatch) const
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryPricingValidator::validate()");

  const vector<OptionalServicesInfo*>& optSrvInfos = getOptionalServicesInfo(*ocFees.subCodeInfo());

  if (optSrvInfos.empty())
  {
    if (!isDDPass())
      printDiagS7NotFound(*ocFees.subCodeInfo());
    return false;
  }

  typedef vector<OptionalServicesInfo*>::const_iterator OptSrvIt;
  AncRequest* req = static_cast<AncRequest*>(_trx.getRequest());

  bool atLeastOneMatch = false;
  for (OptSrvIt optSrvIt = optSrvInfos.begin(); optSrvIt != optSrvInfos.end(); ++optSrvIt)
  {
    checkDiagS7ForDetail(*optSrvIt);
    StatusS7Validation rc = validateS7Data(**optSrvIt, ocFees);

    if (rc == PASS_S7 || rc == PASS_S7_FREE_SERVICE || rc == PASS_S7_NOT_AVAIL)
    {
      ocFees.farePath() = _farePath;
      if (!ocFees.isAnyS7SoftPass())
      {
        if ((*optSrvIt)->upgrdServiceFeesResBkgDesigTblItemNo() != 0)
        {
          addPadisCodesToOCFees(ocFees, **optSrvIt);
        }

        printDiagS7Info(*optSrvIt, ocFees, rc);
        atLeastOneMatch = true;

        if ((*optSrvIt)->upgrdServiceFeesResBkgDesigTblItemNo() == 0)
        {
          return true;
        }

        if (optSrvIt + 1 != optSrvInfos.end())
          ocFees.addSeg(_trx.dataHandle());
      }
      else if (req->hardMatchIndicator())
      {
        rc = getStatusCode(**optSrvIt, ocFees);
        ocFees.cleanOutCurrentSeg();
        printDiagS7Info(*optSrvIt, ocFees, rc);
      }
      else
      {
        if ((*optSrvIt)->upgrdServiceFeesResBkgDesigTblItemNo() != 0)
        {
          addPadisCodesToOCFees(ocFees, **optSrvIt);
        }

        rc = SOFT_PASS_S7;
        printDiagS7Info(*optSrvIt, ocFees, rc);
        atLeastOneMatch = true;

        if (optSrvIt + 1 != optSrvInfos.end())
          ocFees.addSeg(_trx.dataHandle());
      }
    }
    else
    {
      printDiagS7Info(*optSrvIt, ocFees, rc);
      ocFees.cleanOutCurrentSeg();
    }
  }

  ocFees.pointToFirstOCFee();
  if (ocFees.segCount() > 1 && !ocFees.segments().back()->_optFee)
    ocFees.segments().erase(ocFees.segments().end() - 1);
  return atLeastOneMatch;
}

StatusS7Validation
AncillaryPricingValidator::getStatusCode(const OptionalServicesInfo& info, OCFees& ocFees) const
{
  if (ocFees.softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT))
    return FAIL_S7_FREQ_FLYER_STATUS;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_TIME_SOFT))
  {
    if (info.stopoverUnit() != BLANK)
      return FAIL_S7_INTERMEDIATE_POINT;

    if (info.stopCnxDestInd() != BLANK)
      return FAIL_S7_STOP_CNX_DEST;
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_CABIN_SOFT))
    return FAIL_S7_CABIN;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RBD_SOFT) && !ocFees.softMatchRBDT198().empty())
    return FAIL_S7_RBD_T198;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RESULTING_FARE_SOFT) &&
      !ocFees.softMatchResultingFareClassT171().empty())
    return FAIL_S7_RESULT_FC_T171;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RULETARIFF_SOFT))
    return FAIL_S7_RULE_TARIFF;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RULE_SOFT))
    return FAIL_S7_RULE;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_TIME_SOFT) &&
      (info.startTime() != 0 || info.stopTime() != 0))
    return FAIL_S7_START_STOP_TIME;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_CARRIER_FLIGHT_SOFT) &&
      !ocFees.softMatchCarrierFlightT186().empty())
    return FAIL_S7_CXR_FLT_T186;

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT))
    return FAIL_S7_EQUIPMENT;

  return SOFT_PASS_S7;
}

bool
AncillaryPricingValidator::isStopover(const OptionalServicesInfo& optSrvInfo,
                                      const TravelSeg* seg,
                                      const TravelSeg* next) const
{
  if (isMissingDeptArvlTime())
    return true;
  return OptionalServicesValidator::isStopover(optSrvInfo, seg, next);
}

bool
AncillaryPricingValidator::isMissingDeptArvlTime() const
{
  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE && i + 1 < _endOfJourney; ++i)
  {
    if ((*i)->pssDepartureTime().empty() || (*i)->pssArrivalTime().empty())
      return true;
  }
  return false;
}

bool
AncillaryPricingValidator::checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                                                  std::vector<TravelSeg*>& passedLoc3Dest,
                                                  OCFees& ocFees) const
{
  if (optSrvInfo.stopoverUnit() != CHAR_BLANK && isMissingDeptArvlTime())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT);
    return true;
  }

  return OptionalServicesValidator::checkIntermediatePoint(optSrvInfo, passedLoc3Dest, ocFees);
}

bool
AncillaryPricingValidator::checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                                               const std::vector<TravelSeg*>& passedLoc3Dest,
                                               OCFees& ocFees) const
{
  if (optSrvInfo.stopCnxDestInd() != CHAR_BLANK && isMissingDeptArvlTime())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT);
    return true;
  }

  return OptionalServicesValidator::checkStopCnxDestInd(optSrvInfo, passedLoc3Dest, ocFees);
}

bool
AncillaryPricingValidator::isValidCabin(SvcFeesResBkgDesigInfo* rbdInfo,
                                        AirSeg& seg,
                                        OCFees& ocFees,
                                        const BookingCode& (SvcFeesResBkgDesigInfo::*mhod)(void)
                                        const) const
{
  const BookingCode& bookingCode = (rbdInfo->*mhod)();
  if (bookingCode.empty())
    return false;

  const Cabin* cabin = getCabin(rbdInfo->carrier(), seg, bookingCode);

  if (!cabin)
    return false;

  return OptionalServicesValidator::checkCabinData(seg, cabin->cabin(), rbdInfo->carrier(), ocFees);
}

const Cabin*
AncillaryPricingValidator::getCabin(const CarrierCode& carrier,
                                    AirSeg& seg,
                                    const BookingCode& bookingCode) const
{
  if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(_trx) &&
     _trx.activationFlags().isNewRBDbyCabinForM70())
  {
    RBDByCabinUtil rbdUtil(_trx, OPTIONAL_SVC);
    const Cabin* cabin = rbdUtil.getCabinByRBD(carrier, bookingCode, seg, true);
    return cabin;
  }
  else
  {
    const Cabin* cabin = _trx.dataHandle().getCabin(carrier, bookingCode, seg.departureDT());
    return cabin;
  }
  return nullptr;
}

bool
AncillaryPricingValidator::isRBDValid(AirSeg* seg,
                                      const vector<SvcFeesResBkgDesigInfo*>& rbdInfos,
                                      OCFees& ocFees) const
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryPricingValidator::isRBDValid()");
  if (!seg->getBookingCode().empty())
    return OptionalServicesValidator::isRBDValid(seg, rbdInfos, ocFees);

  if (!seg->bookedCabin().isValidCabin())
  {
    std::vector<SvcFeesResBkgDesigInfo*>::const_iterator rbdI = rbdInfos.begin();
    std::vector<SvcFeesResBkgDesigInfo*>::const_iterator rbdIE = rbdInfos.end();
    for (; rbdI != rbdIE; rbdI++)
    {
      if (!(*rbdI)->bookingCode1().empty() || !(*rbdI)->bookingCode2().empty() ||
          !(*rbdI)->bookingCode3().empty() || !(*rbdI)->bookingCode4().empty() ||
          !(*rbdI)->bookingCode5().empty())
      {
        ocFees.softMatchRBDT198().push_back((*rbdI));
      }
    }
    if (ocFees.softMatchRBDT198().empty())
      return false;
    ocFees.softMatchS7Status().set(OCFees::S7_RBD_SOFT);
    return true;
  }

  typedef const BookingCode& (SvcFeesResBkgDesigInfo::*MHOD)(void) const;

  return std::find_if(
             rbdInfos.begin(),
             rbdInfos.end(),
             boost::bind<bool>(&AncillaryPricingValidator::isValidCabin,
                               this,
                               _1,
                               boost::ref(*seg),
                               boost::ref(ocFees),
                               static_cast<MHOD>(&SvcFeesResBkgDesigInfo::bookingCode1)) ||
                 boost::bind<bool>(&AncillaryPricingValidator::isValidCabin,
                                   this,
                                   _1,
                                   boost::ref(*seg),
                                   boost::ref(ocFees),
                                   static_cast<MHOD>(&SvcFeesResBkgDesigInfo::bookingCode2)) ||
                 boost::bind<bool>(&AncillaryPricingValidator::isValidCabin,
                                   this,
                                   _1,
                                   boost::ref(*seg),
                                   boost::ref(ocFees),
                                   static_cast<MHOD>(&SvcFeesResBkgDesigInfo::bookingCode3)) ||
                 boost::bind<bool>(&AncillaryPricingValidator::isValidCabin,
                                   this,
                                   _1,
                                   boost::ref(*seg),
                                   boost::ref(ocFees),
                                   static_cast<MHOD>(&SvcFeesResBkgDesigInfo::bookingCode4)) ||
                 boost::bind<bool>(&AncillaryPricingValidator::isValidCabin,
                                   this,
                                   _1,
                                   boost::ref(*seg),
                                   boost::ref(ocFees),
                                   static_cast<MHOD>(&SvcFeesResBkgDesigInfo::bookingCode5))) !=
         rbdInfos.end();
}

bool
AncillaryPricingValidator::checkCabinData(AirSeg& seg,
                                          const CabinType& cabin,
                                          const CarrierCode& carrier,
                                          OCFees& ocFees) const
{
  if (seg.bookedCabin().isValidCabin())
    return OptionalServicesValidator::checkCabinData(seg, cabin, carrier, ocFees);

  ocFees.softMatchS7Status().set(OCFees::S7_CABIN_SOFT);

  return true;
}

bool
AncillaryPricingValidator::checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo,
                                                    OCFees& ocFees) const
{
  if (_paxType.freqFlyerTierWithCarrier().empty() && optSrvInfo.frequentFlyerStatus() != 0)
  {
    ocFees.softMatchS7Status().set(OCFees::S7_FREQFLYER_SOFT);
    return true; // If not specified in the request and S7 contains a FQTV Tier status
    // ignore the field and soft pass all fees filed with FQTV Tier Status.
  }
  return OptionalServicesValidator::checkFrequentFlyerStatus(optSrvInfo, ocFees);
}

bool
AncillaryPricingValidator::validateStartAndStopTime(const OptionalServicesInfo& info,
                                                    OCFees& ocFees) const
{
  if ((*_segI)->pssDepartureTime().empty())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT);
    return true;
  }

  return OptionalServicesValidator::validateStartAndStopTime(info, ocFees);
}

bool
AncillaryPricingValidator::validateStartAndStopTime(OCFees& ocFees) const
{
  if ((*_segI)->pssDepartureTime().empty())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_TIME_SOFT);
  }
  return true;
}

bool
AncillaryPricingValidator::checkEquipmentType(const OptionalServicesInfo& info, OCFees& ocFees)
    const
{
  if (info.equipmentCode().empty() || info.equipmentCode() == BLANK_CODE)
    return true;

  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE; i++)
  {
    if (!(*i)->equipmentType().empty())
      return OptionalServicesValidator::checkEquipmentType(info, ocFees);
  }

  // no eqipment - softpaas
  ocFees.softMatchS7Status().set(OCFees::S7_EQUIPMENT_SOFT);
  return true;
}

bool
AncillaryPricingValidator::isValidRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd,
                                                TariffCategory tariffCategory) const
{
  if (tariffCategory == RuleConst::PRIVATE_TARIFF)
    return true;
  return OptionalServicesValidator::isValidRuleTariffInd(ruleTariffInd, tariffCategory);
}

StatusT186
AncillaryPricingValidator::isValidCarrierFlightNumber(const AirSeg& air, CarrierFlightSeg& t186)
    const
{
  if (t186.flt1() == 0)
    return FAIL_ON_FLIGHT;
  if (air.flightNumber() == 0)
    return SOFTPASS_FLIGHT;

  return OptionalServicesValidator::isValidCarrierFlightNumber(air, t186);
}

bool
AncillaryPricingValidator::validateLocation(const VendorCode& vendor,
                                            const LocKey& locKey,
                                            const Loc& loc,
                                            const LocCode& zone,
                                            bool emptyRet,
                                            CarrierCode carrier,
                                            LocCode* matchLoc) const
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryPricingValidator::validateLocation()");
  // if airport, try to match on city
  if (locKey.locType() == LOCTYPE_AIRPORT && locKey.loc() != loc.loc())
  {
    LocCode city = LocUtil::getMultiTransportCity(
        locKey.loc(), carrier, _isIntrnl ? GeoTravelType::International : GeoTravelType::Domestic, _trx.ticketingDate());
    if (city == loc.loc())
    {
      if (matchLoc)
        *matchLoc = locKey.loc();
      return true;
    }
    return false;
  }
  return OptionalServicesValidator::validateLocation(
      vendor, locKey, loc, zone, emptyRet, carrier, matchLoc);
}

bool
AncillaryPricingValidator::checkSecurity(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
    const
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryPricingValidator::checkSecurity()");
  if (optSrvInfo.serviceFeesSecurityTblItemNo() <= 0)
  {
    return optSrvInfo.publicPrivateInd() != T183_SCURITY_PRIVATE;
  }
  bool view = false;
  AncillarySecurityValidator securityValidator(_trx, _segI, _segIE);

  return securityValidator.validate(optSrvInfo.seqNo(),
                                    optSrvInfo.serviceFeesSecurityTblItemNo(),
                                    view,
                                    optSrvInfo.vendor(),
                                    _diag);
}

bool
AncillaryPricingValidator::setPBDate(const OptionalServicesInfo& optSrvInfo,
                                     OCFees& ocFees,
                                     const DateTime& pbDate) const
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryPricingValidator::setPBDate()");
  if (isAdvPurUnitNHDM(optSrvInfo))
  {
    if (optSrvInfo.advPurchUnit() == ServicePurchaseUnit('D'))
    {
      uint32_t days = 0;
      istringstream stream(optSrvInfo.advPurchPeriod());
      stream >> days;
      if (days <= 0)
      {
        ocFees.purchaseByDate() = pbDate;
        return true;
      }
      ocFees.purchaseByDate() = (*_segI)->departureDT().subtractDays(days);
      return true;
    }
    else if ((optSrvInfo.advPurchUnit() == ServicePurchaseUnit('M')))
    {
      int32_t numberOfMonths = 0;
      istringstream stream(optSrvInfo.advPurchPeriod());
      stream >> numberOfMonths;
      if (numberOfMonths < 0)
        numberOfMonths = 0;
      ocFees.purchaseByDate() = (*_segI)->departureDT().subtractMonths(numberOfMonths);
      return true;
    }
    boost::posix_time::time_duration count = _trx.ticketingDate() - pbDate;
    if (count.is_negative())
      count = count.invert_sign();

    ocFees.purchaseByDate() = (*_segI)->departureDT() - count;
  }
  else
  {
    uint32_t calcDays = 0;
    int advPinDays = 0;
    getAdvPurPeriod(optSrvInfo, (*_segI)->departureDT(), calcDays, advPinDays);

    if (calcDays == 0 && advPinDays == 0)
      calcDays = 7;
    calcDays += advPinDays;
    ocFees.purchaseByDate() = (*_segI)->departureDT().subtractDays(calcDays);
  }
  return true;
}

// get times difference between agent city location and deprature city location
short
AncillaryPricingValidator::getTimeDiff(const DateTime& time) const
{
  short utcOffset = 0;
  // since titkceting time is of agent time -- we should send agent location as reference
  const Loc* pccLoc = _trx.getRequest()->ticketingAgent()->agentLocation();
  const Loc* departureOrigLoc = nullptr;
  if (!(*_segI)->origAirport().empty())
    departureOrigLoc = getLocation((*_segI)->origAirport(), time);
  else
    departureOrigLoc = getLocation((*_segI)->boardMultiCity(), time);

  if (pccLoc && departureOrigLoc)
  {
    if (getUTCOffsetDifference(*departureOrigLoc, *pccLoc, utcOffset, time))
      return utcOffset;
  }
  return 0;
}

bool
AncillaryPricingValidator::skipUpgradeCheck(const OptionalServicesInfo& optSrvInfo,
                                            const OCFees& ocFees) const
{
  if (skipUpgradeForUpGroupCode(ocFees.subCodeInfo()))
      return true;

  return _trx.billing()->requestPath() == ACS_PO_ATSE_PATH
             ? (optSrvInfo.upgradeCabin() == BLANK &&
                optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo() == 0)
             : skipUpgradeCheckCommon(optSrvInfo, ocFees);
}

}
