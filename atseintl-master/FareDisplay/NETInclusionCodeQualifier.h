//----------------------------------------------------------------------------
//
//  File:           NETInclusionCodeQualifier.h
//
//  Description:    Qualifies PaxTypeFare.
//
//  Updates:
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

#include "FareDisplay/Qualifier.h"

namespace tse
{
class PaxTypeFare;
class FareDisplayTrx;

class NETInclusionCodeQualifier : public Qualifier
{
  friend class NETInclusionCodeQualifierTest;

public:
  NETInclusionCodeQualifier();
  virtual ~NETInclusionCodeQualifier();

  const PaxTypeFare::FareDisplayState qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const override;
  bool setup(FareDisplayTrx& trx) override;

private:
  bool isNetFareDisabled(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const;
  static constexpr Indicator NET_LEVEL_NOT_ALLOWED = 'P';
};
} // namespace tse

