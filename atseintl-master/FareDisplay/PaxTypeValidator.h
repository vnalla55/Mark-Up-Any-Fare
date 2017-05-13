//----------------------------------------------------------------------------
//   File : PaxTypeValidator.cpp
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
 *@class PaxTypeValidator.
 *
 */

class FareDisplayTrx;
class PaxTypeFare;

class PaxTypeValidator : public Validator
{
public:
  virtual bool validate(const PaxTypeFare& paxType) override;
  virtual bool initialize(const FareDisplayTrx& trx) override;
  void failValidator(bool& invalidDT, bool& invalidFT, bool& invalidPT) override;

protected:
  void setFlags();
  bool populatePaxType(const FareDisplayTrx& trx);
  /**
  * Validate passenger type.
  *
  * @param ptFare - a valid PaxTypeFare
  * @param exceptPsgData - true if passenger type data is exception data
  *
  * @return - true if OK, false if disqualified
  */
  bool validatePsgType(const PaxTypeCode& code,
                       const std::set<PaxTypeCode>&,
                       bool exceptPsgType = false) const;
  /**
  * Read-only map for looking up supported inclusion code pax types
  **/
  std::set<PaxTypeCode> _inclCdPaxSet;

  std::set<PaxTypeCode> _publishedChildInfant;

  bool _exceptPsgData = false;

  bool validateChildInfantFare(const PaxTypeFare& fare) const;
  bool validateAdultFare(const PaxTypeFare& fare) const;
  virtual const PaxTypeCode& getBaseFarePaxType(const PaxTypeFare& fare) const;
  virtual bool isMatchQualifier(const PaxTypeInfo& fare) const;

protected:
  virtual Validator::ValidatorType name() const override { return Validator::PAX_TYPE_VALIDATOR; }
};

} // namespace tse

