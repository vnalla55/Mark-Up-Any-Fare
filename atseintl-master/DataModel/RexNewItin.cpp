//-------------------------------------------------------------------
//
//  File:        RexNewItin.cpp
//  Created:     March 16, 2009
//  Authors:     Grzegorz Cholewiak
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

#include "DataModel/RexNewItin.h"

#include "DataModel/RexPricingTrx.h"

namespace tse
{
const DateTime&
RexNewItin::travelDate() const
{
  return _rexTrx->travelDate();
}
} // tse namespace
