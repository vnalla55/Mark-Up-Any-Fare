//----------------------------------------------------------------------------
//
//  File:           FareClassQualifier.h
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "FareDisplay/Qualifier.h"

namespace tse
{
class PaxTypeFare;
class FareDisplayTrx;

class FareClassQualifier : public Qualifier
{
public:
  FareClassQualifier();
  virtual ~FareClassQualifier();

  const PaxTypeFare::FareDisplayState qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const override;
  bool setup(FareDisplayTrx& trx) override;

private:
  bool qualifyFareClass(FareDisplayTrx& trx,
                        const FareClassCode& requestedFareClassCode,
                        const PaxTypeFare& ptFare) const;
  bool removeDiscountedChildAndInfantFares(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const;
};
} // namespace tse

