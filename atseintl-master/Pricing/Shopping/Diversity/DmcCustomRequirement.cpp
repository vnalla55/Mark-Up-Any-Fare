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

#include "Pricing/Shopping/Diversity/DmcCustomRequirement.h"

#include "Common/ShoppingUtil.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{

namespace
{
class CouldProduceCustomSolutionFMVisitor : public shpq::SoloPQItem::FareMarketVisitor
{
public:
  CouldProduceCustomSolutionFMVisitor(const ShoppingTrx& trx)
    : _trx(trx), _couldProduceCustomSolution(true)
  {
  }

  // virtual
  void visit(const FareMarket* fm) override
  {
    if (_couldProduceCustomSolution && _trx.legs()[fm->legIndex()].isCustomLeg() &&
        !_trx.isCustomSolutionFM(fm))
      _couldProduceCustomSolution = false;
  }

  bool getCouldProduceCustomSolution() const { return _couldProduceCustomSolution; }

private:
  const ShoppingTrx& _trx;
  bool _couldProduceCustomSolution;
};

} // anon ns

bool
DmcCustomRequirement::getThrowAwayCombination(shpq::SopIdxVecArg comb)
{
  if (_sharedCtx._trx.getNumOfCustomSolutions() > 0)
  {
    int16_t limit = _isFareCutoffReached ? _sharedCtx._diversity.getMinimalNumCustomSolutions()
                                         : _sharedCtx._trx.getNumOfCustomSolutions();
    if (_sharedCtx._stats.getCustomSolutionCount() >= limit)
    {
      if (ShoppingUtil::isCustomSolution(_sharedCtx._trx, comb))
      {
        if (_dc)
          _dc->addCombinationResult(comb, Diag941Collector::CUSTOM);

        return true;
      }
    }
  }
  return false;
}

DmcCustomRequirement::Value
DmcCustomRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  if (LIKELY(!_sharedCtx._trx.getNumOfCustomSolutions()))
    return 0;

  if (pqItem->getLevel() < shpq::SoloPQItem::CRC_LEVEL)
    return NEED_CUSTOM;

  CouldProduceCustomSolutionFMVisitor fmCheckVisitor(_sharedCtx._trx);
  pqItem->visitFareMarkets(fmCheckVisitor);

  return (fmCheckVisitor.getCouldProduceCustomSolution()) ? NEED_CUSTOM : 0;
}

DmcCustomRequirement::Value
DmcCustomRequirement::getCombinationCouldSatisfy(shpq::SopIdxVecArg comb) const
{
  if (LIKELY(!_sharedCtx._trx.getNumOfCustomSolutions()))
    return 0;

  bool isCustom = ShoppingUtil::isCustomSolution(_sharedCtx._trx, comb);
  return (isCustom ? NEED_CUSTOM : 0);
}

void
DmcCustomRequirement::print()
{
  if (!_sharedCtx._diversity.getNumOfCustomSolutions())
    return;

  *_sharedCtx._dc << "\n\tCUS: ";
  *_sharedCtx._dc << "[" << _sharedCtx._stats.getCustomSolutionCount() << "/"
                  << _sharedCtx._diversity.getMinimalNumCustomSolutions() << "]";
}

} // ns tse
