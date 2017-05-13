//----------------------------------------------------------------------------
//
//  File:           TravelDateQualifier.h
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

#include "Common/TseStringTypes.h"
#include "FareDisplay/Qualifier.h"

namespace tse
{
class PaxTypeFare;
class FareDisplayTrx;

class TravelDateQualifier : public Qualifier
{
public:
  TravelDateQualifier();
  virtual ~TravelDateQualifier();

  const PaxTypeFare::FareDisplayState qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const override;
  bool setup(FareDisplayTrx& trx) override;

private:
  bool checkEffDisc(const DateTime& x, const TSEDateInterval& range) const;
  bool checkDateRange(const DateTime& x, const DateTime& lo, const DateTime& hi) const;
  bool qualifyBothTravelDates(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const;
  bool qualifyAnyTravelDate(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const;
};
} // namespace tse

