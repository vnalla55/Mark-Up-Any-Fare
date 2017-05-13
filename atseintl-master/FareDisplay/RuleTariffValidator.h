//----------------------------------------------------------------------------
//   File : RuleTariffValidator.cpp
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "DBAccess/FareDispInclRuleTrf.h"
#include "FareDisplay/Validator.h"


#include <vector>

namespace tse
{
/**
 *@class RuleTariffValidator.
 *
 */

class FareDisplayTrx;
class PaxTypeFare;

class RuleTariffValidator : public Validator
{
public:
  bool validate(const PaxTypeFare& paxType) override;
  bool initialize(const FareDisplayTrx& trx) override;

private:
  /**
  * Read-only map for looking up supported fare types
  **/
  std::vector<FareDispInclRuleTrf*> _ruleTariffRecs;
  bool getData(const FareDisplayTrx& trx);
  virtual Validator::ValidatorType name() const override { return Validator::RULE_TARIFF_VALIDATOR; }
};
} // namespace tse
