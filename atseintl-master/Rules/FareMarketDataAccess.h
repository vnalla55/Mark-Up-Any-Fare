//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Rules/RuleController.h"

namespace tse
{
class Itin;
class PaxTypeFare;
class PricingTrx;

struct FareMarketDataAccess : public RuleControllerDataAccess
{
  FareMarketDataAccess(PricingTrx& transaction, Itin* itinerary, PaxTypeFare& ptFare)
    : RuleControllerDataAccess(ptFare.vendor() == SITA_VENDOR_CODE),
      _trx(transaction), _itin(itinerary), _paxTypeFare(ptFare)
  {
  }

  Itin* itin() override { return _itin; }
  PaxTypeFare& paxTypeFare() const override { return getBaseOrPaxTypeFare(_paxTypeFare); }
  PricingTrx& trx() override { return _trx; }

private:
  PricingTrx& _trx;
  Itin* _itin;
  PaxTypeFare& _paxTypeFare;
};
}
