#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class FDSGlobalDir;

class GlobalDirectionComparator : public Comparator
{
public:
  /**
   * compares the global direction for two fares using either the priority list
   * or the default priority, when user doesnt specify priority.
   */
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

  /**
   * prepares the priority list. If user specifies an order, it will use that
   * else do nothing.
   */
  void prepare(const FareDisplayTrx& trx) override;
  const std::map<GlobalDirection, uint16_t>& priorityMap() const { return _priorityMap; }

private:
  std::map<GlobalDirection, uint16_t> _priorityMap;

  /**
   * returns numerical priority for a given global.
   */
  uint16_t globalPriority(GlobalDirection gd);

  /**
   * reads the user preference data and populates the priority map.
   */
  void populatePriorityList(const std::vector<FDSGlobalDir*>& gds);
  /**
   * initializes the priority map using the alphabetical order.
   * this is the default order of global direction grouping.
   */
  void initialize();
};

} // namespace tse

