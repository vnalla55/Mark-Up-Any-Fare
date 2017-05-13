// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#ifndef MILEAGES_COMPARATOR_H
#define MILEAGES_COMPARATOR_H

#include "Common/Assert.h"
#include "DBAccess/Mileage.h"

namespace tse
{

class MileagesComparator
{
public:
  bool operator()(Mileage* mileage1, Mileage* mileage2) const
  {
    TSE_ASSERT(mileage1 != 0 && mileage2 != 0);
    return (mileage1->mileage() < mileage2->mileage());
  }
};

} // namespace tax

#endif
