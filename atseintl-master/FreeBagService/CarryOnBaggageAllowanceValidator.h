/*
 * CarryOnBaggageAllowanceValidator.h
 *
 *  Created on: Jan 17, 2013
 *      Author: SG0216830
 */

#pragma once

#include "FreeBagService/BaggageAllowanceValidator.h"

namespace tse
{
class CarryOnBaggageAllowanceValidator : public BaggageAllowanceValidator
{
  friend class CarryOnBaggageAllowanceValidatorTest;

public:
  using BaggageAllowanceValidator::BaggageAllowanceValidator;

private:
  StatusS7Validation validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees) override;

  bool checkSectorPortionInd(const OptionalServicesInfo& optSrvInfo) const;

  bool checkGeoFtwInd(const OptionalServicesInfo& optSrvInfo) const override;

  StatusS7Validation
  checkServiceNotAvailNoCharge(const OptionalServicesInfo& info, OCFees& ocFees) const override;

  virtual bool checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                                   const std::vector<TravelSeg*>& passedLoc3Dest) const override;

  virtual bool validateCnxOrStopover(const OptionalServicesInfo& optSrvInfo,
                                     const std::vector<TravelSeg*>& passedLoc3Dest,
                                     bool validateCnx) const override;

  bool checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                              std::vector<TravelSeg*>& passedLoc3Dest) const override;

  bool checkOccurrenceBlankBlank(const OptionalServicesInfo& optSrvInfo) const;

  bool checkFeeApplication(const OptionalServicesInfo& optSrvInfo) const;
};
}
