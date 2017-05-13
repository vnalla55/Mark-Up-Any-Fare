//-------------------------------------------------------------------
//
//  File:        ExchangeNewItin.cpp
//  Created:     Nov 20, 2009
//  Authors:
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

#include "DataModel/ExchangeNewItin.h"

#include "DataModel/BaseExchangeTrx.h"

namespace tse
{
const DateTime&
ExchangeNewItin::travelDate() const
{
  return _trx->travelDate();
}
}
