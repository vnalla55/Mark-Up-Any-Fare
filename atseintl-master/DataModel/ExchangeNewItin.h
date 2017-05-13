//-------------------------------------------------------------------
//
//  File:        ExchangeNewItin.h
//  Created:     March 16, 2009
//  Authors:     Simon Li
//
//  Description: The Itinerary class object represents one
//               itinerary from incoming shopping/pricing request.
//
//  Updates:

//
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/Itin.h"

namespace tse
{
class BaseExchangeTrx;
class DateTime;

class ExchangeNewItin : public Itin
{

public:
  const BaseExchangeTrx*& trx() { return _trx; }
  const BaseExchangeTrx* trx() const { return _trx; }
  const DateTime& travelDate() const override;

private:
  const BaseExchangeTrx* _trx = nullptr;
};
} // tse namespace
