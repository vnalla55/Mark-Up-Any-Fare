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

#include "Pricing/Shopping/Diversity/DmcLimitLongConxRequirement.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/Diversity.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

namespace tse
{

bool
DmcLimitLongConxRequirement::getThrowAwayCombination(shpq::SopIdxVecArg comb)
{
  if (_sharedCtx._diversity.getMaxLongConnectionSolutions() > 0)
  {
    if (_sharedCtx._stats.getLongConnectionCount() >=
        static_cast<int>(_sharedCtx._diversity.getMaxLongConnectionSolutions()))
    {
      if (ShoppingUtil::isLongConnection(_sharedCtx._trx, comb))
      {
        if (!_sharedCtx._trx.isNoMoreLngConOptionsNeeded())  // if the indicator for 'no more long connect fare required' is not set
        {
          ShoppingTrx* shTrx = const_cast<ShoppingTrx*>(&_sharedCtx._trx);
          // signal for no more long connect options needed
          shTrx->setNoMoreLngConOptionsNeeded(true);
          if (_dc)
          {
            _dc->addCombinationResult(comb, Diag941Collector::LONG_CONNECTION, true);
          }
        }
        return true;
      }
    }
  }
  return false;
}

bool
DmcLimitLongConxRequirement::getThrowAwaySop(std::size_t legId, int sopId) const
{
  int16_t numLongConx = _sharedCtx._stats.getLongConnectionCount();
  int16_t maxLongConx = _sharedCtx._diversity.getMaxLongConnectionSolutions();

  if (maxLongConx > 0 && numLongConx >= maxLongConx)
  {
    if (SopCombinationUtil::getSop(_sharedCtx._trx, legId, sopId).isLngCnxSop())
    {
      if (UNLIKELY(!_sharedCtx._trx.isNoMoreLngConOptionsNeeded()))  // if the indicator for 'no more long connect fare required' is not set
      {
        ShoppingTrx* shTrx = const_cast<ShoppingTrx*>(&_sharedCtx._trx);
        // signal for no more long connect options needed
        shTrx->setNoMoreLngConOptionsNeeded(true);

        if (_dc)
          _dc->addSopResult(legId, sopId, Diag941Collector::LONG_CONNECTION, true);
      }
      return true;
    }
  }

  return false;
}

} // ns tse
