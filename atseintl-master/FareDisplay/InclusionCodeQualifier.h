//----------------------------------------------------------------------------
//
//  File:           InclusionCodeQualifier.h
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"
#include "FareDisplay/Qualifier.h"
#include "FareDisplay/Validator.h"

namespace tse
{
class PaxTypeFare;
class FareDisplayTrx;

class InclusionCodeQualifier : public Qualifier
{
public:
  InclusionCodeQualifier();
  virtual ~InclusionCodeQualifier();

  const PaxTypeFare::FareDisplayState qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const override;
  virtual bool setup(FareDisplayTrx& trx) override;

private:
  bool qualifyInclusionCode(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const;
  std::vector<Validator*> _validators;
  std::map<uint8_t, std::vector<Validator*> > _inclusionCodeValidators;
  bool qualifyFare(const FareDisplayTrx& trx, const PaxTypeFare& ptFare) const;
  bool qualifyFareForMultiInclCodes(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const;
  void setStatusIntoMap(const PaxTypeFare& ptFare, const uint8_t&, bool) const;
  bool failedAll(const uint16_t& noOfValidators, const uint16_t count) const;
  bool validatorPassed(const std::vector<Validator*>& validators,
                       const PaxTypeFare& ptFare,
                       uint16_t& failedCount, bool& invalidDT,
                       bool& invalidFT, bool& invalidPT) const;
  void build(FareDisplayTrx& trx);
};
} // namespace tse

