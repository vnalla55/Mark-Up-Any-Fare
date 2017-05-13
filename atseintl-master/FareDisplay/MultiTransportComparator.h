
#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;

class MultiTransportComparator : public Comparator
{
  friend class MultiTransportComparatorTest;

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

private:
  /**
   * returns numerical priority for a given global.
   */

  /**
    * initializes the priority map using the alphabetical order.
    * this is the default order of global direction grouping.
    */
  void initialize();

  uint16_t getPriority(const LocCode& origin, const LocCode& destination);
  std::map<std::pair<LocCode, LocCode>, uint16_t> _priorityMap;

  uint16_t _priority = 1;
};

} // namespace tse

