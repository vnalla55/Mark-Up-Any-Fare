// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         17-10-2011
//! \file         ConxRouteCxrPQItem.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#include "Pricing/Shopping/PQ/ConxRouteCxrPQItem.h"

#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/ShpqTypes.h"
#include "DataModel/Diversity.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "Pricing/Shopping/PQ/FarePathFactoryPQItem.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/OwrtFmPatternSummaryInitializer.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloPUFactoryWrapper.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include <boost/bind.hpp>

namespace /* anon */
{

struct NullDeleter
{
  void operator()(void*) {}
}; // struct NullDeleter

} // namespace /*anon*/

#define ALL_ITEMS                                                                                  \
  {                                                                                                \
    &_outboundFMLeg1, &_outboundFMLeg2, &_inboundFMLeg1, &_inboundFMLeg2                           \
  }

namespace tse
{
namespace shpq
{

Logger
ConxRouteCxrPQItem::_logger("atseintl.ShoppingPQ.ConxRouteCxrPQItem");

ConxRouteCxrPQItem::FMLegInfo::FMLegInfo(const OwrtFareMarketPtr& fmPtr,
                                         const OwrtFareMarketPtr& defaultfmPtr)
  : Base(fmPtr, fmPtr.get() ? fmPtr->begin() : defaultfmPtr->begin())
{
}

ConxRouteCxrPQItem::FMLegInfo::FMLegInfo(const FMLegInfo& other, const bool getNext)
  : Base(other, getNext)
{
}

ConxRouteCxrPQItem::ConxRouteCxrPQItem(const ConxRoutePQItem& crPQItem, SoloTrxData& soloTrxData)
  : CommonSoloPQItem(*crPQItem.getSolPattern()),
    _outboundFMLeg1(crPQItem.getOutboundCxrLeg1(), crPQItem.getOutboundCxrLeg1()),
    _outboundFMLeg2(crPQItem.getOutboundCxrLeg2(), crPQItem.getOutboundCxrLeg1()),
    _inboundFMLeg1(crPQItem.getInboundCxrLeg1(), crPQItem.getOutboundCxrLeg1()),
    _inboundFMLeg2(crPQItem.getInboundCxrLeg2(), crPQItem.getOutboundCxrLeg1()),
    _score(calculateScore(UNKNOWN_MONEY_AMOUNT))
{
  detectApplicableCxrs(soloTrxData);
}

// virtual
void
ConxRouteCxrPQItem::expand(SoloTrxData& soloTrxData, SoloPQ& pq)
{
  // build FMPath
  SoloPUFactoryWrapper& puFactory = soloTrxData.getPUFactoryWrapper();
  SoloPUFactoryWrapper::PUStruct puStruct = puFactory.getPUStruct(this, pq.diagCollector());

  if (puStruct._puPath)
  {
    // The reason for doing this check here is that rec2Cat10 is acquired from DB in
    // puFactory.getPUStruct() method and we don't want to collect this data unnecessarily.
    const FMLegInfo* const items[] = ALL_ITEMS;
    OwrtFmPatternSummaryInitializer initializer(soloTrxData.getShoppingTrx());
    OwrtFmPatternSummary::Pattern pattern = getSolPattern()->isEoePattern()
                                                ? OwrtFmPatternSummary::EOE
                                                : OwrtFmPatternSummary::NOT_EOE;

    for (const FMLegInfo* const item : items)
    {
      if (!item->hasCurrent())
        continue;

      OwrtFareMarketPtr fm = item->getFM();
      if (!fm->getSolPatternSummary().isInitialized())
        initializer.initSummary(*fm.get());
      if (fm->getSolPatternSummary().isInvalidForPattern(pattern))
      {
        pq.incrementFailedFPs();
        pq.diagCollector().displayValidationMsg(
            "CRC->FPF expansion skipped: item invalid due to Cat10", this);
        return;
      }
    }

    SoloPQItemManager& pqItemMgr = soloTrxData.getPQItemManager();
    FarePathFactoryPQItemPtr fpfPQItem(pqItemMgr.constructFPFPQItem(*this, soloTrxData, puStruct));

    if (fpfPQItem->isFailed())
    {
      pq.diagCollector().onCrcToFpfExpandFail(fpfPQItem, "FAIL FAREPATH");
      pq.incrementFailedFPs();
    }
    else
      pq.enqueue(fpfPQItem, this);
  }
}

template <class P>
void
ConxRouteCxrPQItem::foreachFareMarket(P pred) const
{
  const FMLegInfo* const items[] = ALL_ITEMS;
  for (const FMLegInfo* const item : items)
  {
    if (item->hasCurrent() && (*item)->fare())
      pred(item->getFM());
  }
}

void
ConxRouteCxrPQItem::detectApplicableCxrs(SoloTrxData& soloTrxData)
{
  const ShoppingTrx& trx = soloTrxData.getShoppingTrx();
  const OwrtFMVector& owrtFareMarkets = getFMVector();
  CxrKeysPerLeg cxrKeysPerLeg;

  for (const OwrtFareMarketPtr& owrtFm : owrtFareMarkets)
  {
    const FareMarket& fm = *owrtFm->getFareMarket();
    ShoppingUtil::mergeCxrKeys(fm, cxrKeysPerLeg[fm.legIndex()]);
  }

  if (UNLIKELY(cxrKeysPerLeg.size() != trx.legs().size()))
    return;

  CxrKeys onlineCxrs;
  bool isInterlineAppl = false;
  ShoppingUtil::collectOnlineCxrKeys(cxrKeysPerLeg, onlineCxrs, isInterlineAppl);

  if (isInterlineAppl)
    _applCxrs.push_back(Diversity::INTERLINE_CARRIER);

  for (uint32_t cxrKey : onlineCxrs)
  {
    _applCxrs.push_back(trx.diversity().detectCarrier(cxrKey));
  }
}

ConxRouteCxrPQItem::OwrtFMVector
ConxRouteCxrPQItem::getFMVector() const
{
  using namespace boost;

  OwrtFMVector fmVector;
  foreachFareMarket(bind(
    static_cast<void (OwrtFMVector::*)(const OwrtFareMarketPtr&)> (&OwrtFMVector::push_back),
    &fmVector, _1));
  return fmVector;
}

// virtual
void
ConxRouteCxrPQItem::visitFareMarkets(FareMarketVisitor& visitor) const
{
  using namespace boost;

  foreachFareMarket(bind(&ConxRouteCxrPQItem::apply, _1, ref(visitor)));
}

// virtual
std::string
ConxRouteCxrPQItem::str(const StrVerbosityLevel strVerbosity /* = SVL_BARE */) const
{
  const char* const FIELD_SEPARTOR(" : ");

  std::ostringstream stream;
  stream.setf(std::ios::fixed, std::ios::floatfield);
  printBasicStr(stream, "CRC");

  if (strVerbosity >= SVL_BARE)
  {
    const FMLegInfo* const items[] = ALL_ITEMS;
    LegPosition currentLegPos(LEG_1ST);

    for (const FMLegInfo* const item : items)
    {
      stream << FIELD_SEPARTOR;

      if (item->hasCurrent() && (*item)->fareMarket() && (*item)->fare())
      {
        const FareMarket& fM(*((*item)->fareMarket()));
        stream << " " << fM.origin()->loc() << "-" << fM.governingCarrier() << "-"
               << fM.destination()->loc();

        if (strVerbosity >= SVL_NORMAL)
        {
          const Fare& fare(*((*item)->fare()));
          stream << " [" << fare.fareClass();
          printPlaceHolderStr(stream, currentLegPos, item->hasNext());
          stream << "]";
        }
      }
      else
      {
        stream << " -";
      }
      currentLegPos = CommonSoloPQItem::getNextLegPosition(currentLegPos);
    }
  }
  return stream.str();
}

MoneyAmount
ConxRouteCxrPQItem::calculateScore(const MoneyAmount defaultAmount) const
{
  if (UNLIKELY(!_outboundFMLeg1.hasCurrent()))
  {
    return defaultAmount;
  }

  const FMLegInfo* const items[] = ALL_ITEMS;

  MoneyAmount score = 0.0;

  for (const FMLegInfo* const item : items)
  {
    if (!item->hasCurrent())
      continue;
    if (UNLIKELY(!(*item)->fare()))
      continue;

    MoneyAmount segmentScore = (*item)->fare()->nucFareAmount();

    const OwrtFmPatternSummary& summary = item->getFM()->getSolPatternSummary();

    if (summary.isInitialized())
    {
      OwrtFmPatternSummary::Pattern pattern = getSolPattern()->isEoePattern()
                                                  ? OwrtFmPatternSummary::EOE
                                                  : OwrtFmPatternSummary::NOT_EOE;
      segmentScore = std::max(segmentScore, summary.lowerBoundForPattern(pattern));
    }

    score += segmentScore;
  }

  return score;
}

bool
ConxRouteCxrPQItem::shouldSkip(SoloTrxData& soloTrxData, const ItinStatistic& _stats, SoloPQ& pq)
{
  // If we haven't generated Q0S of options, ie, it hasn't
  // reach the min price yet, we should not skip
  if (_score < _stats.getMinPrice())
    return false;

  const ShoppingTrx& shopTrx = soloTrxData.getShoppingTrx();
  const Diversity& diversity = shopTrx.diversity();

  if (!diversity.hasDCL())
    return false;

  std::vector<const FMLegInfo*> leg1Items = { &_outboundFMLeg1, &_inboundFMLeg1 };
  std::vector<const FMLegInfo*> leg2Items = { &_outboundFMLeg2, &_inboundFMLeg2 };

  if (true==shouldSkipCheckLeg(soloTrxData, _stats, leg1Items, pq))
    return true;

  return shouldSkipCheckLeg(soloTrxData, _stats, leg2Items, pq);
}

bool
ConxRouteCxrPQItem::shouldSkipCheckLeg(SoloTrxData& soloTrxData,
                                       const ItinStatistic& _stats,
                                       std::vector<const FMLegInfo*>& legItems,
                                       SoloPQ& pq)
{
  bool isLocalFM = true;

  for (const FMLegInfo* item : legItems)
  {
    if (item->hasCurrent() && (*item)->fareMarket())
      continue;

    isLocalFM = false;
    break;
  }

  if (isLocalFM)
  // Both Fare Markets are valid so it's local
  {
    bool rc = shouldSkipCheckLocalFM(soloTrxData, _stats, legItems, pq);
    return rc;
  }
  else // at least one FM invalid so it's thru
  {
    bool rc = shouldSkipCheckThruFM(soloTrxData, _stats, legItems, pq);
    return rc;
  }

}

bool
ConxRouteCxrPQItem::shouldSkipCheckThruFM(SoloTrxData& soloTrxData,
                                          const ItinStatistic& _stats,
                                          std::vector<const FMLegInfo*>& legItems,
                                          SoloPQ& pq)
{
  uint16_t unmatchedGovCxr = 0;

  for (const FMLegInfo* item : legItems)
  {
    if (!item->hasCurrent())
      continue;

    if (!(*item)->fareMarket())
      continue;

    const FareMarket& fM(*((*item)->fareMarket()));
    const CarrierCode& govCxr = fM.governingCarrier();
    const ShoppingTrx& shopTrx = soloTrxData.getShoppingTrx();
    const Diversity& diversity = shopTrx.diversity();

    if (!diversity.hasDCL())
      return false;

    const std::map<CarrierCode, size_t>& DCLMap(diversity.getDCLMap());
    std::map<CarrierCode, size_t>::const_iterator cxrItr = DCLMap.find(govCxr);
    std::map<CarrierCode, size_t>::const_iterator cxrItrEnd = DCLMap.end();

    if (cxrItr == cxrItrEnd)
    {
      unmatchedGovCxr++;
      continue;
    }
    else
    {
      size_t optionsCollected = _stats.getNumOfItinsForCarrier(govCxr);
      size_t optionsRequired = (size_t)(cxrItr->second);
      if (optionsCollected < optionsRequired)
      {
        std::string str = "CRC EXP CXR kept: FM CXR="+govCxr+
          " options collected="+std::to_string(optionsCollected)+
          ", optionsRequired="+std::to_string(optionsRequired);
        pq.diagCollector().displayValidationMsg(str, this);
        return false;
      }
      else
      {
        std::string str = "CRC EXP CXR skipped: FM CXR="+govCxr+
          " options collected="+std::to_string(optionsCollected)+
          ", optionsRequired="+std::to_string(optionsRequired);
        pq.diagCollector().displayValidationMsg(str, this);
        return true;
      }
    }
  }

  if (unmatchedGovCxr>=2)
  {
    const ShoppingTrx& shopTrx = soloTrxData.getShoppingTrx();

    std::string strCRC=std::to_string(_score);
    std::string strCut=std::to_string(shopTrx.diversity().getFareCutoffAmount());

    std::string str = "CRC EXP CXR skipped: FM CXR not found in DCL list. CRC="+strCRC
      +", CUTOFF="+strCut;
    pq.diagCollector().displayValidationMsg(str, this);
    return true;
  }

  return false;
}

bool
ConxRouteCxrPQItem::shouldSkipCheckLocalFM(SoloTrxData& soloTrxData,
                                           const ItinStatistic& _stats,
                                           std::vector<const FMLegInfo*>& legItems,
                                           SoloPQ& pq)
{
  std::vector<std::vector<int> > sopIdsVecs;
  sopIdsVecs.resize(2);
  uint16_t currFM = 0;
  uint16_t unmatchedGovCxr = 0;

  for (const FMLegInfo* item : legItems)
  {
    if (!(*item)->fareMarket())
    {
      return false;
    }

    const FareMarket& fM(*((*item)->fareMarket()));
    const CarrierCode& govCxr = fM.governingCarrier();
    const ShoppingTrx& shopTrx = soloTrxData.getShoppingTrx();
    const Diversity& diversity = shopTrx.diversity();

    if (!diversity.hasDCL()) // no DCL, don't skip
      return false;

    const std::map<CarrierCode, size_t>& DCLMap(diversity.getDCLMap());
    std::map<CarrierCode, size_t>::const_iterator cxrItr = DCLMap.find(govCxr);
    std::map<CarrierCode, size_t>::const_iterator cxrItrEnd = DCLMap.end();

    if (cxrItr == cxrItrEnd)
    {
      unmatchedGovCxr++;
      continue;
    }
    else
    {
      ItinIndex::Key govCxrKey;
      ShoppingUtil::createCxrKey(govCxr, govCxrKey);

      if (!fM.getApplicableSOPs())
        return false;

      bool keepThisCRC = false;
      for (const ApplicableSOP::value_type& cxrAndSopUsages : *fM.getApplicableSOPs())
      {
        ItinIndex::Key currKey = cxrAndSopUsages.first;
        if (currKey == govCxrKey)
        {
          keepThisCRC = true;

          if ((*fM.getApplicableSOPs()).size()==1)
          {
            std::string str = "CRC EXP CXR kept: FM CXR="+govCxr+
              " found in DCL list, applicable SOPs size = 1";
            pq.diagCollector().displayValidationMsg(str, this);
            return false;
          }

          for (const SOPUsage& sopUsage : cxrAndSopUsages.second)
          {
            if (!sopUsage.applicable_)
              continue;
            sopIdsVecs[currFM].push_back(sopUsage.origSopId_);
          }
        }
      }
      if (keepThisCRC == false)
      {
        std::string str = "CRC EXP CXR skipped: FM CXR="+govCxr+
         " found in DCL list, but not in its applicable SOP";
        pq.diagCollector().displayValidationMsg(str, this);
        return true; // skip this CRC
      }
    }

    currFM++;
  }

  if (unmatchedGovCxr>=2)
  {
      const ShoppingTrx& shopTrx = soloTrxData.getShoppingTrx();

      std::string strCRC=std::to_string(_score);
      std::string strCut=std::to_string(shopTrx.diversity().getFareCutoffAmount());

      std::string str = "CRC EXP CXR skipped: FM CXR not found in DCL list. CRC="+strCRC
        +", CUTOFF="+strCut;
      pq.diagCollector().displayValidationMsg(str, this);
      return true;
  }

  std::vector<int> intersectedSops;

  std::sort(sopIdsVecs[0].begin(), sopIdsVecs[0].end());
  std::sort(sopIdsVecs[1].begin(), sopIdsVecs[1].end());

  std::set_intersection(sopIdsVecs[0].begin(),
                        sopIdsVecs[0].end(),
                        sopIdsVecs[1].begin(),
                        sopIdsVecs[1].end(),
                        back_inserter(intersectedSops));

  if (intersectedSops.empty())
  {
    std::string str = "CRC EXP CXR skipped: Local FM CXRs \
       found in DCL list, but FMs have no intersection";
    pq.diagCollector().displayValidationMsg(str, this);
    return true;
  }
  else
  {
    std::string str = "CRC EXP CXR kept: Local FM CXRs \
       found in DCL list and FMs have intersections";
    pq.diagCollector().displayValidationMsg(str, this);
    return false;
  }
}

} /* namespace shpq */
} /* namespace tse */
