//----------------------------------------------------------------------------
//
// Copyright Sabre 2013
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

#include "Common/DateTime.h"

namespace tse
{

class PricingTrx;
class BaseExchangeTrx;
class FarePath;
class Itin;

class BaggageTicketingDateScope
{
  friend class BaggageTicketingDateScopeTest;

public:
  BaggageTicketingDateScope(PricingTrx& trx, const FarePath* farePath);
  BaggageTicketingDateScope(PricingTrx& trx, const Itin* itin);
  ~BaggageTicketingDateScope();

private:
  bool isItReissue(const FarePath& farePath) const;
  bool isItReissue(const Itin& itin) const;
  void setTicketingDate(const bool useOriginalTktDate);

  BaseExchangeTrx* _rexTrx;
  DateTime _oldDataHandleTicketingDate;
  DateTime _oldTrxTicketingDate;
};

}

