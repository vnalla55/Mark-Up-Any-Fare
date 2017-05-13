#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class OneProgramOneBrand;

struct OneProgramOneBrandComparator
    : std::binary_function<const OneProgramOneBrand*, const OneProgramOneBrand*, bool>
{
  bool operator()(const OneProgramOneBrand* spb1, const OneProgramOneBrand* spb2) const;
};

class S8BrandComparator : public Comparator
{
  friend class S8BrandComparatorTest;

public:
  using ProgramBrand = std::pair<ProgramCode, BrandCode>;
  /**
   * compares the brand for two fares using either the priority list
   * or the default priority, when user doesnt specify priority.
   */
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

  /**
   * prepares the priority list.
   */
  void prepare(const FareDisplayTrx& trx) override;

  const std::map<ProgramBrand, TierNumber>& priorityMap() const { return _priorityMap; }

private:
  std::map<ProgramBrand, TierNumber> _priorityMap;

  /**
   * Compares to Brand Code sole based on Alphabetical Priority
   * when can't find brands/seqno
   */
  Comparator::Result alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r);

  /**
   * returns numerical priority for a given brand.
   */
  TierNumber brandPriority(ProgramBrand pb);
};

} // namespace tse

