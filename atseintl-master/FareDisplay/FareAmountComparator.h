#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class FareDisplayOptions;

class FareAmountComparator : public Comparator
{
public:
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

private:
  /**
   * checks if the user allows you to double one way fare or not.
   * if no user preference is found, it is always doubled.
   */
  bool isDoubleOWFare();
  /**
   * gets the actual money amount that is used for comparison.
   */
  MoneyAmount fareAmount(const PaxTypeFare& p);
  Comparator::Result compareSchedCnt(const PaxTypeFare& l, const PaxTypeFare& r);
  Comparator* _scheduleCountComparator = nullptr;
  void prepare(const FareDisplayTrx& trx) override;
  /**
   * checks if the both fares are OW or RT fares.
   */
  bool isSameDirectionality(const PaxTypeFare& l, const PaxTypeFare& r);
  /**
   * overrides the sort option when user uses SA/SD qualifier.
   */

  /**
   * sets the sort option for the user override.
   */
  //    void setSortOption(const FareDisplayOptions& options);
  void setSortOption(const FareDisplayTrx& trx);

  /**
   * multiplying factor to conver OW fare to RT fare.
   */
  static const uint16_t FACTOR = 2;
};

} // namespace tse

