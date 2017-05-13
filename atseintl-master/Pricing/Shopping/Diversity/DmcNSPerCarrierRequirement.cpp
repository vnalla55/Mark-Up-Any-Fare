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

#include "Pricing/Shopping/Diversity/DmcNSPerCarrierRequirement.h"

#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <iomanip>

namespace tse
{
DmcNSPerCarrierRequirement::DmcNSPerCarrierRequirement(DmcRequirementsSharedContext& sharedCtx)
  : _isEnabled(sharedCtx._diversity.getNonStopOptionsPerCarrierEnabled()), _sharedCtx(sharedCtx)
{
}

DmcNSPerCarrierRequirement::Value
DmcNSPerCarrierRequirement::getStatus() const
{
  if (UNLIKELY(!_isEnabled))
    return 0;

  if (isOptNeededForNSCarrier())
    return NEED_NONSTOPS_CARRIERS;
  else
    return 0;
}

DmcNSPerCarrierRequirement::Value
DmcNSPerCarrierRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  if (UNLIKELY(!_isEnabled))
    return 0;

  const bool couldDetectByCxr = (pqItem->getLevel() == shpq::SoloPQItem::CRC_LEVEL ||
                                 pqItem->getLevel() == shpq::SoloPQItem::FPF_LEVEL);
  if (!couldDetectByCxr)
    return (isOptNeededForNSCarrier() ? NEED_NONSTOPS_CARRIERS : 0);

  for (const CarrierCode& cxr : pqItem->getApplicableCxrs())
  {
    if (cxr != Diversity::INTERLINE_CARRIER && isOptNeededForNSCarrier(cxr))
      return NEED_NONSTOPS_CARRIERS;
  }
  return 0;
}

DmcNSPerCarrierRequirement::Value
DmcNSPerCarrierRequirement::getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price)
    const
{
  if (UNLIKELY(!_isEnabled))
    return 0;

  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_sharedCtx._trx, comb, &outbound, &inbound);

  const CarrierCode cxr = SopCombinationUtil::detectCarrier(outbound, inbound);
  bool isInterline = cxr.empty();
  if (isInterline)
    return 0;

  if (isOptNeededForNSCarrier(cxr))
    return NEED_NONSTOPS_CARRIERS;
  else
    return 0;
}

void
DmcNSPerCarrierRequirement::print() const
{
  if (!isEffective())
    return;

  DiagCollector& dc = *_sharedCtx._dc;
  const Diversity& diversity = _sharedCtx._diversity;
  const ItinStatistic& stats = _sharedCtx._stats;

  dc << "\t" << std::setw(12) << "NS Carriers:";
  for (CarrierCode cxr : diversity.getDirectOptionsCarriers())
  {
    const size_t count = stats.getNumOfNonStopItinsForCarrier(cxr);
    const size_t required =
        diversity.isHighDensityMarket() ? diversity.getMaxNonStopCountFor(cxr) : 1;

    dc << " " << cxr << "[" << count << "/" << required << "]";
  }
  dc << "\n";
}

bool
DmcNSPerCarrierRequirement::isOptNeededForNSCarrier() const
{
  typedef std::map<CarrierCode, size_t>::value_type CxrStat;
  for (const CxrStat& cxrStat : _sharedCtx._stats.getNumOfNonStopItinsPerCarrier())
  {
    bool couldHaveMore = (cxrStat.second == 0);
    if (couldHaveMore)
      return true;
  }

  return false;
}

bool
DmcNSPerCarrierRequirement::isOptNeededForNSCarrier(CarrierCode cxr) const
{
  int curNumOpt = _sharedCtx._stats.getNumOfNonStopItinsForCarrier(cxr);

  bool isNS = !(curNumOpt < 0);
  if (!isNS)
    return false;

  bool couldHaveMore = (curNumOpt == 0);
  return couldHaveMore;
}

} // ns tse
