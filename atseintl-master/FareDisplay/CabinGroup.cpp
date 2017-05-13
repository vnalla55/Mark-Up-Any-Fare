//-------------------------------------------------------------------
//
//  File:        CabinGroup.cpp
//  Created:     January 2016
//
//  Copyright Sabre 2016
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

#include "FareDisplay/CabinGroup.h"

#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupHeader.h"

namespace tse
{
CabinGroup::CabinGroup() {}
//------------------------------------------------------
// CabinGroup::initializeCabinGroupGroup()
//------------------------------------------------------
void
CabinGroup::initializeCabinGroup(FareDisplayTrx& trx, std::vector<Group*>& groups)
{
  // Create Cabin Group
  Group* grp = nullptr;
  trx.dataHandle().get(grp);

  grp->groupType() = Group::GROUP_BY_CABIN;
  grp->sortType() = Group::ASCENDING;

  ComparatorFactory rVFactory(trx);

  grp->comparator() = rVFactory.getComparator(grp->groupType());

  if (grp->comparator() != nullptr)
  {
    grp->comparator()->group() = grp;
    grp->comparator()->prepare(trx);
  }

  groups.push_back(grp);

  // Initialize Cabin header
  GroupHeader header(trx);
  header.setCabinHeader();
}
}
