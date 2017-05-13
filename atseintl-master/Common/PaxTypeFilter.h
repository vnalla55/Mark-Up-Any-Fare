//----------------------------------------------------------------------------
//
//  File:        PaxTypeFilter.h
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

#include "Common/TseCodeTypes.h"

#include <set>

namespace tse
{
class PricingTrx;

namespace PaxTypeFilter
{
enum PaxType
{ ADULT = 0,
  CHILD,
  INFANT,
  ALL };

bool
getAllAdultPaxType(PricingTrx& trx, std::set<PaxTypeCode>&);

bool
getAllChildPaxType(PricingTrx& trx, std::set<PaxTypeCode>&);

bool
getAllInfantPaxType(PricingTrx& trx, std::set<PaxTypeCode>&);

bool
getAllPaxType(PricingTrx& trx, std::set<PaxTypeCode>&);
}
} // end tse namespace
