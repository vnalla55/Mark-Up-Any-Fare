//-------------------------------------------------------------------
//  Created:Jul 1, 2005
//  Author:Abu
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/NormalSpecialComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
NormalSpecialComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (_group->sortType() == Group::NORMAL_OVER_SPECIAL)
  {
    if (l.isNormal() && r.isNormal())
      return Comparator::EQUAL;
    if (l.isNormal())
      return TRUE;
    if (r.isNormal())
      return FALSE;
  }
  else if (_group->sortType() == Group::SPECIAL_OVER_NORMAL)
  {
    if (l.isSpecial() && r.isSpecial())
      return Comparator::EQUAL;
    if (l.isSpecial())
      return TRUE;
    if (r.isSpecial())
      return FALSE;
  }
  return EQUAL;
}

void
NormalSpecialComparator::prepare(const FareDisplayTrx& trx)
{
}
}
