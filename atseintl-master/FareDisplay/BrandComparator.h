#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class Brand;

class BrandComparator : public Comparator
{
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

  const std::map<BrandCode, uint64_t>& priorityMap() const { return _priorityMap; }

private:
  std::map<BrandCode, uint64_t> _priorityMap;

  /**
   * Compares to Brand Code sole based on Alphabetical Priority
   * when can't find brands/seqno
   */
  Comparator::Result alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r);

  /**
   * returns numerical priority for a given brand.
   */
  uint64_t brandPriority(BrandCode bc);

  /**
   * reads the user preference data and populates the priority map.
   */
  void populatePriorityList(const std::vector<Brand*>& brands);
};

} // namespace tse

