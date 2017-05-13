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
 *@class RequestedPaxTypeValidator.
 *
 */

class FareDisplayTrx;
class PaxTypeFare;

class RequestedPaxTypeValidator : public PaxTypeValidator
{
public:
  bool validate(const PaxTypeFare& paxType) override;
  bool initialize(const FareDisplayTrx& trx) override;
  Restriction restriction(const PaxTypeFare& fare) const override;

private:
  FareDisplayRequest* _request = nullptr;

protected:
  virtual bool isMatchQualifier(const PaxTypeInfo& fare) const override;
  virtual Validator::ValidatorType name() const override { return Validator::RQST_PAX_TYPE_VALIDATOR; }
};

} // namespace tse

