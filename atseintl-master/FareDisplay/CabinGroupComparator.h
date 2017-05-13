#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class Group;

class CabinGroupComparator : public Comparator
{
  friend class CabinGroupComparatorTest;

public:
  /**
   * compares the brand for two fares using either the priority list
   * or the default priority, when user doesnt specify priority.
   */
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

  /**
   * prepares the priority list.
   */
  void prepare(const FareDisplayTrx& trx) override;

  const std::map<uint8_t, std::pair<uint8_t, std::string> >& priorityMap() const { return _priorityMap; }

private:
  // the 1st uint8_t is the order in which inclusion code comes in the request
  // the 2nd uint8_t is the inclusion code enum (number)
  // the std::string is the inclusion code itself
  std::map<uint8_t, std::pair<uint8_t, std::string> > _priorityMap;
  Comparator::Result alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r);

  /**
   * returns numerical priority for a given brand.
   */
  uint8_t inclusionNumPassed(const PaxTypeFare& ptf);
};

} // namespace tse
