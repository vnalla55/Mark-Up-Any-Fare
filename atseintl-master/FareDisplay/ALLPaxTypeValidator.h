//----------------------------------------------------------------------------
//   File : ALLPaxTypeValidator.h
// Copyright Sabre 2008
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
#include "FareDisplay/RequestedPaxTypeValidator.h"


#include <set>

namespace tse
{
/**
 *@class PaxTypeValidator.
 *
 */

class PaxTypeFare;

class ALLPaxTypeValidator : public RequestedPaxTypeValidator
{
public:
  virtual bool validate(const PaxTypeFare& paxType) override;
  Restriction restriction(const PaxTypeFare& fare) const override;

private:
  virtual Validator::ValidatorType name() const override { return Validator::ALL_PAX_TYPE_VALIDATOR; }
};

} // namespace tse

