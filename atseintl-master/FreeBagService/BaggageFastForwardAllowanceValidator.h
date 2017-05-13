//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#pragma once

#include "FreeBagService/BaggageAllowanceValidator.h"

namespace tse
{
class BaggageFastForwardAllowanceValidator : public BaggageAllowanceValidator
{
public:
  using BaggageAllowanceValidator::BaggageAllowanceValidator;

private:
  virtual StatusS7Validation
  checkServiceNotAvailNoCharge(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;

  friend class BaggageFastForwardAllowanceValidatorTest;
};
} // tse
