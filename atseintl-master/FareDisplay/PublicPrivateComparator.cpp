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

#include "FareDisplay/PublicPrivateComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
PublicPrivateComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (_group->sortType() == Group::PUBLIC_OVER_PRIVATE)
  {
    if (l.tcrTariffCat() != PRIVATE_TARIFF && r.tcrTariffCat() != PRIVATE_TARIFF)
      return Comparator::EQUAL;
    if (l.tcrTariffCat() != PRIVATE_TARIFF)
      return Comparator::TRUE;
    if (r.tcrTariffCat() != PRIVATE_TARIFF)
      return Comparator::FALSE;
  }
  else if (_group->sortType() == Group::PRIVATE_OVER_PUBLIC)
  {
    if (l.tcrTariffCat() == PRIVATE_TARIFF && r.tcrTariffCat() == PRIVATE_TARIFF)
      return Comparator::EQUAL;
    if (l.tcrTariffCat() == PRIVATE_TARIFF)
      return Comparator::TRUE;
    if (r.tcrTariffCat() == PRIVATE_TARIFF)
      return Comparator::FALSE;
  }
  return EQUAL;
}

void
PublicPrivateComparator::prepare(const FareDisplayTrx& trx)
{
}
}
