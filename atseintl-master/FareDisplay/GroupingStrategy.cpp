
#include "FareDisplay/GroupingStrategy.h"

#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupingAlgorithm.h"

namespace tse
{
void
GroupingStrategy::createGroup(Group::GroupType grpType, Indicator sortType)
{
  // lint --e{413}
  Group* grp = nullptr;
  _trx->dataHandle().get((grp));

  grp->groupType() = grpType;
  grp->sortType() = sortType;

  ComparatorFactory rVFactory(*_trx);
  grp->comparator() = rVFactory.getComparator(grp->groupType());
  grp->comparator()->group() = grp;
  if (grp->comparator() != nullptr)
  {
    grp->comparator()->prepare(*_trx);
  }
  _groups.insert(_groups.begin(), grp);
}

void
GroupingStrategy::groupAndSort()
{
  _trx->allPaxTypeFare().clear();

  if (!_fares.empty())
  {
    sortFares(_fares);
    copyFares(_fares);
  }

  if (!_yy_fares.empty())
  {
    sortFares(_yy_fares);
    copyFares(_yy_fares);
  }
}

void
GroupingStrategy::sortFares(std::list<PaxTypeFare*>& fareGroup)
{
  fareGroup.sort(GroupingAlgorithm(_groups));
}

void
GroupingStrategy::copyFares(std::list<PaxTypeFare*>& fareGroup)
{
  std::copy(fareGroup.begin(), fareGroup.end(), back_inserter(_trx->allPaxTypeFare()));
}
}
