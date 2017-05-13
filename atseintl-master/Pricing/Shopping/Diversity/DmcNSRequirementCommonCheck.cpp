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

#include "Pricing/Shopping/Diversity/DmcNSRequirementCommonCheck.h"

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

namespace tse
{
namespace
{

class CouldProduceNonStopCheck_FMVisitor : public shpq::SoloPQItem::FareMarketVisitor
{
public:
  CouldProduceNonStopCheck_FMVisitor(const Diversity& diversity)
    : _couldProduceNonStop(true), _smd(diversity.getScheduleMarketDataStorage())
  {
  }

 
  void visit(const FareMarket* fm) override
  {
    if (!_couldProduceNonStop)
      return;

    Diversity::ScheduleMarketDataStorage::const_iterator smdIt = _smd.find(fm);
    TSE_ASSERT(smdIt != _smd.end());

    Diversity::ScheduleMarketDataStorage::mapped_type::const_iterator cxrSmdIt =
        smdIt->second.find(fm->governingCarrier());
    TSE_ASSERT(cxrSmdIt != smdIt->second.end());

    if (cxrSmdIt->second.nonStopSOPs == 0)
    {
      _couldProduceNonStop = false;
    }
  }

  int getResult() const { return _couldProduceNonStop; }

private:
  bool _couldProduceNonStop;
  const Diversity::ScheduleMarketDataStorage& _smd;
};

} // anon ns

bool
DmcNSRequirementCommonCheck::canPQProduceMoreNonStop() const
{
  const Diversity& diversity = _sharedCtx._diversity;
  const ItinStatistic& stats = _sharedCtx._stats;

  if (diversity.isOnlineNonStopCapable() &&
      (stats.getOnlineNonStopsCount() + stats.getAdditionalOnlineNonStopsCount() <
       diversity.getMaxOnlineNonStopCount()))
  {
    return true;
  }
  if ((diversity.shouldUseInterlineNonStops() || diversity.isAdditionalNonStopsEnabled()) &&
      diversity.isInterlineNonStopCapable() &&
      (stats.getInterlineNonStopsCount() + stats.getAdditionalInterlineNonStopsCount() <
       diversity.getMaxInterlineNonStopCount()))
  {
    return true;
  }
  return false;
}

bool
DmcNSRequirementCommonCheck::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  // Detect by SP
  if (!pqItem->getSolPattern()->isThruPattern())
    return false;

  const bool couldDetectByCxrOrFM = (pqItem->getLevel() == shpq::SoloPQItem::CRC_LEVEL ||
                                     pqItem->getLevel() == shpq::SoloPQItem::FPF_LEVEL);
  if (!couldDetectByCxrOrFM)
    return true;

  // Detect by carrier
  if (!couldHaveMore(pqItem))
    return false;

  // Detect by fare markets
  CouldProduceNonStopCheck_FMVisitor fmVisitor(_sharedCtx._diversity);
  pqItem->visitFareMarkets(fmVisitor);
  return fmVisitor.getResult();
}

bool
DmcNSRequirementCommonCheck::getCombinationCouldSatisfy(shpq::SopIdxVecArg comb,
                                                        DmcRequirement::Value status) const
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_sharedCtx._trx, comb, &outbound, &inbound);

  SopCombinationUtil::NonStopType nonStopType =
      SopCombinationUtil::detectNonStop(outbound, inbound);

  if (!(nonStopType & SopCombinationUtil::NON_STOP)) // this is not a non stop
    return false;

  return couldHaveMore(SopCombinationUtil::detectCarrier(outbound, inbound));
}

bool
DmcNSRequirementCommonCheck::getSopCouldSatisfy(
    unsigned legId,
    const SOPInfo& sopInfo,
    const DmcRequirement::SopInfosStatistics& /*statistics*/) const
{
  const ShoppingTrx::SchedulingOption& sop =
      SopCombinationUtil::getSop(_sharedCtx._trx, legId, sopInfo._sopIndex);
  return SopCombinationUtil::mayBeNonStop(sop);
}

bool
DmcNSRequirementCommonCheck::couldHaveMore(const shpq::SoloPQItem* pqItem) const
{
  for (const CarrierCode& cxr : pqItem->getApplicableCxrs())
  {
    if (couldHaveMore(cxr))
      return true;
  }
  return false;
}

bool
DmcNSRequirementCommonCheck::couldHaveMore(CarrierCode cxr) const
{
  int curNumOpt = _sharedCtx._stats.getNumOfNonStopItinsForCarrier(cxr);
  if (curNumOpt < 0)
    return false;

  return (curNumOpt < static_cast<int>(_sharedCtx._diversity.getMaxNonStopCountFor(cxr)));
}

} // ns tse
