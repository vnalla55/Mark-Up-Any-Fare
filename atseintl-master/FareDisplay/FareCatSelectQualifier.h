//----------------------------------------------------------------------------
//
//  File:        FareCatSelectQualifier.h
//
//  Description:    Qualifies PaxTypeFare for Fare Cat Select criterion.
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

#include "Common/TseStringTypes.h"
#include "FareDisplay/Qualifier.h"

namespace tse
{
class PaxTypeFare;
class FareDisplayTrx;

class FareCatSelectQualifier : public Qualifier
{
public:
  FareCatSelectQualifier();
  virtual ~FareCatSelectQualifier();

  const PaxTypeFare::FareDisplayState qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const override;
  bool setup(FareDisplayTrx& trx) override;
};
} // namespace tse

