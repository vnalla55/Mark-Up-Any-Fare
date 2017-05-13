#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class FDSFareBasisComb;

class CharCombinationsComparator : public Comparator
{
  friend class CharCombinationsComparatorTest;

public:
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

  void prepare(const FareDisplayTrx& trx) override;

  const std::map<FareCombinationCode, uint16_t>& priorityMap() const { return _priorityMap; }
  std::map<FareCombinationCode, uint16_t>& priorityMap() { return _priorityMap; }

private:
  void populatePriorityList(const std::vector<FDSFareBasisComb*>& charCombs);
  uint16_t combPriority(const FareCombinationCode& code);
  std::map<FareCombinationCode, uint16_t> _priorityMap;
  Comparator::Result compareChars(const std::string& l, const std::string& r);
};

} // namespace tse

