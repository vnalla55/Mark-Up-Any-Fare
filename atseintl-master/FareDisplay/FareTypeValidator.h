//----------------------------------------------------------------------------
//   File : FareTypeValidator.cpp
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
#include "FareDisplay/Validator.h"


#include <set>

namespace tse
{
/**
 *@class FareTypeValidator.
 *
 */

class FareDisplayTrx;
class PaxTypeFare;

class FareTypeValidator : public Validator
{
public:
  bool validate(const PaxTypeFare& paxType) override;
  bool initialize(const FareDisplayTrx& trx) override;
  void failValidator(bool& invalidDT, bool& invalidFT, bool& invalidPT) override;

private:
  /**
  * Read-only map for looking up supported fare types
  **/
  bool populateFareTypes(const FareDisplayTrx& trx);
  void setFlags();

  bool validateFareType(const FareType& fareType, bool exceptFareData) const;

protected:
  bool _exceptFareData;
  std::set<FareType> _fareTypes;
  virtual const FareType& getBaseFareFareType(const PaxTypeFare& fare) const;
  virtual Validator::ValidatorType name() const override { return Validator::FARE_TYPE_VALIDATOR; }
};

} // namespace tse

