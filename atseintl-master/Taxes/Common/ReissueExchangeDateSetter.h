// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TsePrimitiveTypes.h"


namespace tse
{
class FarePath;
class PricingTrx;

class ReissueExchangeDateSetter
{
  friend class TaxItineraryTest;
  friend class ReissueExchangeDateSetterTest;

public:
  ReissueExchangeDateSetter(PricingTrx& pricingTrx, const FarePath& farePath);
  ~ReissueExchangeDateSetter();

private:
  bool useHistoricalDate();

  PricingTrx& _pricingTrx;
  const FarePath& _fp;
  bool _resetTrxSecondRoeIndicator;
  bool _savedTrxSecondRoeIndicator;
  DateTime _savedTicketingDate;
};
}
