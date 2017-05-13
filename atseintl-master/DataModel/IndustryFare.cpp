//-------------------------------------------------------------------
//
//  File:        IndustryFare.cpp
//  Created:     July 7, 2004
//  Authors:     Mark Kasprowicz
//
//  Description: A class to represent the Industry Fare specific part
//               of a Fare.
//
//  Updates:
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
//-------------------------------------------------------------------

#include "DataModel/IndustryFare.h"

#include "DBAccess/DataHandle.h"
#include "DBAccess/IndustryFareAppl.h"

namespace tse
{
const Indicator IndustryFare::MULTILATERAL = 'M';

bool
IndustryFare::initialize(const Fare& fare,
                         const IndustryFareAppl& fareAppl,
                         const IndustryFareAppl::ExceptAppl* exceptAppl,
                         DataHandle& dataHandle,
                         bool validForPricing)
{ // lint !e578

  fare.clone(dataHandle, *this);

  _status.set(FS_IndustryFare);

  if (UNLIKELY(fareAppl.selectionType() == MULTILATERAL))
    _carrier = fareAppl.carrier();
  else
    _carrier = INDUSTRY_CARRIER;

  _fareClass = fare.fareClass();

  _fareAppl = &fareAppl;
  _exceptAppl = exceptAppl;

  _validForPricing = validForPricing;

  return isValid();
}

Fare*
IndustryFare::clone(DataHandle& dataHandle, bool resetStatus) const
{
  IndustryFare* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  clone(dataHandle, *cloneObj, resetStatus); // lint !e413

  return ((Fare*)cloneObj);
}

void
IndustryFare::clone(DataHandle& dataHandle, IndustryFare& cloneObj, bool resetStatus) const
{
  Fare::clone(dataHandle, cloneObj, resetStatus);

  cloneObj._carrier = _carrier;
  cloneObj._fareClass = _fareClass;
  cloneObj._fareAppl = _fareAppl;
  cloneObj._exceptAppl = _exceptAppl;
  cloneObj._changeFareClass = _changeFareClass;
  cloneObj._validForPricing = _validForPricing;
  cloneObj._matchFareTypeOfTag2CxrFare = _matchFareTypeOfTag2CxrFare;
}
}
