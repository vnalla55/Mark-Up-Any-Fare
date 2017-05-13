#pragma once
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupingAlgorithm.h"

#include <algorithm>
#include <list>
#include <vector>

namespace tse
{

/* Classes that represent grouping and sorting strategy for Fares. */
class GroupingStrategy
{
public:
  using PaxTypeFares = std::vector<PaxTypeFare*>;

  virtual ~GroupingStrategy() = default;
  virtual bool apply() = 0;

  bool _isInternationalFare = false;
  bool _isShopperRequest = false;

  std::vector<Group*>& groups() { return _groups; }
  const std::vector<Group*>& groups() const { return _groups; }

  FareDisplayTrx* _trx = nullptr;

  std::list<PaxTypeFare*>& fares() { return _fares; }
  const std::list<PaxTypeFare*>& fares() const { return _fares; }

  std::list<PaxTypeFare*>& yy_fares() { return _yy_fares; }
  const std::list<PaxTypeFare*>& yy_fares() const { return _yy_fares; }

protected:
  std::vector<Group*> _groups;
  std::list<PaxTypeFare*> _fares;
  std::list<PaxTypeFare*> _yy_fares;

  void createGroup(Group::GroupType grpType, Indicator sortType = Group::ASCENDING);
  void groupAndSort();
  void sortFares(std::list<PaxTypeFare*>& fareGroup);
  void copyFares(std::list<PaxTypeFare*>& fareGroup);
};

} // namespace tse

