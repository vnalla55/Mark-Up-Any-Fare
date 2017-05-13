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

#include "ServiceFees/OptionalServicesValidator.h"

namespace tse
{
class CommonSoftMatchValidator : public OptionalServicesValidator
{
  friend class CommonSoftMatchValidatorTest;

public:
  using OptionalServicesValidator::OptionalServicesValidator;

protected:
  bool matchRuleTariff(const uint16_t& ruleTariff,
                       const PaxTypeFare& ptf,
                       OCFees& ocFees) const override;
  bool checkRuleTariff(const uint16_t& ruleTariff, OCFees& ocFees) const override;
  bool checkRule(const RuleNumber& rule, OCFees& ocFees) const override;
  StatusT171 isValidFareClassFareType(const PaxTypeFare* ptf,
                                      SvcFeesCxrResultingFCLInfo& fclInfo,
                                      OCFees& ocFees) const override;
};
} // tse namespace

