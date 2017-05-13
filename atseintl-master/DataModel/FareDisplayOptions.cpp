//-------------------------------------------------------------------
//
//  File:        FareDisplayOptions.cpp
//  Created:     February 15, 2005
//  Authors:
//
//  Description: Process options
//
//  Updates:
//          02/15/05 - Mike Carroll - file created.
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

#include "DataModel/FareDisplayOptions.h"

#include "DBAccess/ConstructedFareInfo.h"

namespace tse
{
FareDisplayOptions::FareDisplayOptions()
{
  // FD doesn't hide and fares based on these
  _nationality = WILDCARD;
  _residency = WILDCARD;
  _employment = WILDCARD;
  _fareByRuleShipRegistry = WILDCARD;
} // lint !e1401

bool
FareDisplayOptions::isPublishedFare()
{
  bool rc = true;

  if ((_cat25Values.isNonPublishedFare()) || (_cat35Values.isNonPublishedFare()) ||
      (_discountedValues.isNonPublishedFare()) || (_constructedAttributes.isConstructedFare()))
  {
    rc = false;
  }
  return rc;
}
}
