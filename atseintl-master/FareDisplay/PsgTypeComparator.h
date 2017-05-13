#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class FareDisplayTrx;

class PsgTypeComparator : public Comparator
{
  friend class PaxTypeComparatorTest;

public:
  /**
   * compares two fares using passenger type criteria.
   */

  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

private:
  std::map<PaxTypeCode, uint16_t> _priorityMap;

  /**
   * Compares to PaxType sole based on Alphabetical Priority when User doesnt specify any
   * priority order.
   */
  Comparator::Result alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r);

  /**
   * returns numerical priority for a given passenger type.
   */
  uint16_t psgPriority(const PaxTypeCode& psg);

  /**
   * prepares the priority list. If user specifies an order, it will use that
   * else do nothing.
   */

  void prepare(const FareDisplayTrx& trx) override;

  /**
   * Translates Empty PaxType
   */
  std::string translate(const PaxTypeCode& paxCode);

  /**
   * Translates Empty PaxType
   */
  std::string paxType(const PaxTypeFare& fare);

  /**
   * reads the user preference data and populates the priority map.
   */

  void populatePriorityList(const std::vector<FDSPsgType*>& gds);
};

} // namespace tse

