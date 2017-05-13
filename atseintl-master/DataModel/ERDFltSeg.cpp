//-------------------------------------------------------------------
//
//  File:           ERDFltSeg.h
//  Created:        29 September 2008
//  Authors:        Konrad Koch
//
//  Description:    Wrapper to SEG section data from WPRD request.
//
//  Copyright Sabre 2008
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

#include "DataModel/ERDFltSeg.h"

#include "DataModel/ERDFareComp.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
//----------------------------------------------------------------------------
// ERDFltSeg::select
//----------------------------------------------------------------------------
void
ERDFltSeg::select(FareDisplayTrx& trx)
{
  FareDisplayRequest* request = trx.getRequest();

  if (!_fareComponent || !request)
    return;

  _fareComponent->select(trx);
}

bool
ERDFltSeg::isOriginAndDestinationInRussia(FareDisplayTrx& trx)
{
  const Loc* origin = trx.dataHandle().getLoc(_departureAirport, DateTime::localTime());
  const Loc* destination = trx.dataHandle().getLoc(_arrivalAirport, DateTime::localTime());

  if (origin == nullptr || destination == nullptr)
  {
    return false;
  }

  return LocUtil::isRussianGroup(*origin) && LocUtil::isRussianGroup(*destination);
}
} // tse namespace
