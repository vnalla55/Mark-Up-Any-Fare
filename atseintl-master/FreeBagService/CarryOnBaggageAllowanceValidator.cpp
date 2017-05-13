/*
 * CarryOnBaggageAllowanceValidator.cpp
 *
 *  Created on: Jan 17, 2013
 *      Author: SG0216830
 */

#include "FreeBagService/CarryOnBaggageAllowanceValidator.h"

#include "DBAccess/OptionalServicesInfo.h"


namespace tse
{

StatusS7Validation
CarryOnBaggageAllowanceValidator::validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
{
  StatusS7Validation status = BaggageAllowanceValidator::validateS7Data(optSrvInfo, ocFees);
  if (status == PASS_S7)
  {
    if (!checkSectorPortionInd(optSrvInfo))
      return FAIL_S7_SECTOR_PORTION;
    if (!checkFeeApplication(optSrvInfo))
      return FAIL_S7_FEE_APPL;
  }
  return status;
}

bool
CarryOnBaggageAllowanceValidator::checkSectorPortionInd(const OptionalServicesInfo& optSrvInfo)
    const
{
  return (optSrvInfo.sectorPortionInd() == SEC_POR_IND_SECTOR) ||
         ((optSrvInfo.sectorPortionInd() == CHAR_BLANK ||
           optSrvInfo.sectorPortionInd() == CHAR_BLANK2) &&
          (optSrvInfo.loc1().loc().empty()) && (optSrvInfo.loc2().loc().empty()) &&
          (optSrvInfo.viaLoc().loc().empty()));
}

bool
CarryOnBaggageAllowanceValidator::checkGeoFtwInd(const OptionalServicesInfo& optSrvInfo) const
{
  const Loc* orig = (*_baggageTravel.getTravelSegBegin())->origin();
  const Loc* dest = (*_baggageTravel.getTravelSegBegin())->destination();

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

StatusS7Validation
CarryOnBaggageAllowanceValidator::checkServiceNotAvailNoCharge(const OptionalServicesInfo& info,
                                                               OCFees& ocFees) const
{
  if (info.notAvailNoChargeInd() == SERVICE_FREE_NO_EMD_ISSUED)
    return PASS_S7;
  else
    return FAIL_S7_NOT_AVAIL_NO_CHANGE;
}

bool
CarryOnBaggageAllowanceValidator::checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                                                         std::vector<TravelSeg*>& passedLoc3Dest)
    const
{
  if (optSrvInfo.viaLoc().isNull())
    return true;

  for (const Loc* hiddenPointLoc : (*_baggageTravel.getTravelSegBegin())->hiddenStops())
  {
    if (validateLocation(optSrvInfo.vendor(),
                         optSrvInfo.viaLoc(),
                         *hiddenPointLoc,
                         optSrvInfo.viaLocZoneTblItemNo(),
                         false,
                         optSrvInfo.carrier(),
                         nullptr))
      return true;
  }

  return false;
}

bool
CarryOnBaggageAllowanceValidator::checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                                                      const std::vector<TravelSeg*>& passedLoc3Dest)
    const
{
  return optSrvInfo.stopCnxDestInd() == ' ';
}

bool
CarryOnBaggageAllowanceValidator::validateCnxOrStopover(
    const OptionalServicesInfo& optSrvInfo,
    const std::vector<TravelSeg*>& passedLoc3Dest,
    bool validateCnx) const
{
  return optSrvInfo.stopoverTime().empty();
}

bool
CarryOnBaggageAllowanceValidator::checkOccurrenceBlankBlank(const OptionalServicesInfo& optSrvInfo)
    const
{
  return (optSrvInfo.baggageOccurrenceFirstPc() <= 0) &&
         (optSrvInfo.baggageOccurrenceLastPc() <= 0);
}

bool
CarryOnBaggageAllowanceValidator::checkFeeApplication(const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.frequentFlyerMileageAppl() == ' ';
}
}
