//----------------------------------------------------------------------------
//   File : QualifierPaxTypeValidator.cpp
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
#include "FareDisplay/PaxTypeValidator.h"


#include <set>

namespace tse
{
/**
 *@class QualifierPaxTypeValidator.
 *
 */

class FareDisplayTrx;
class PaxTypeFare;

class QualifierPaxTypeValidator : public PaxTypeValidator
{

  friend class PaxTypeFareValidatorTest;

public:
  bool validate(const PaxTypeFare& paxType) override;
  bool initialize(const FareDisplayTrx& trx) override;
  Restriction restriction(const PaxTypeFare& fare) const override;

private:

  /**
  * Read-only set for looking up supported published child/infant fare
  */
protected:
  bool isMatchQualifier(const PaxTypeInfo& fare) const override;
  virtual Validator::ValidatorType name() const override { return Validator::QUALIFIER_PAX_TYPE_VALIDATOR; }
};

} // namespace tse

