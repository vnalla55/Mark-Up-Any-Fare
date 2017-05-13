// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/PQ/SOPCombination.h"

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"

namespace tse
{
namespace shpq
{
SOPCombination::SOPCombination(const ShoppingTrx& trx,
                               const SoloPQItem& pqItem,
                               const std::vector<SOPInfo>& combination)
  : oSopVec(combination.size()), spId(pqItem.getSolPattern()->getSPId())
{
  bool isRoundTrip(combination.size() == 2);

  oSopVec[0] = combination[0]._sopIndex;
  status = combination[0].getStatus();
  if (isRoundTrip)
    oSopVec[1] = combination[1]._sopIndex;

  if (trx.isAltDates())
    updateForAltDates(trx, combination);

  if (isRoundTrip)
  {
    char secondLegStatus = combination[1].getStatus(duration);
    status = ((status == 'N' || secondLegStatus == 'N') ? 'N' : std::max(status, secondLegStatus));
  }
}

void
SOPCombination::updateForAltDates(const ShoppingTrx& trx, const std::vector<SOPInfo>& combination)
{
  DatePair datePair = DiversityUtil::getDatePairSops(trx, oSopVec);
  ShoppingTrx::AltDatePairs::const_iterator it = trx.altDatePairs().find(datePair);

  // invalid dates pair
  if (it == trx.altDatePairs().end())
  {
    status = 'N';
    return;
  }

  this->datePair = &it->first;
  this->duration = datePair.second.get64BitRepDateOnly() - datePair.first.get64BitRepDateOnly();
  // set status for dates duration
  status = combination[0].getStatus(duration);
}
}
} // ns tse::shpq
