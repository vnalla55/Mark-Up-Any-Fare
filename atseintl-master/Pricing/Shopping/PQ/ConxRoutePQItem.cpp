// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         29-09-2011
//! \file         ConxRoutePQItem.cpp
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

#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "DataModel/OwrtFareMarket.h"
#include "Pricing/Shopping/PQ/ConxRouteCxrPQItem.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"


namespace /* anon */
{

struct NullDeleter
{
  void operator()(void*) {}
}; // struct NullDeleter

} // namespace /*anon*/

#define ALL_ITEMS                                                                                  \
  {                                                                                                \
    &_outboundCxrLeg1, &_outboundCxrLeg2, &_inboundCxrLeg1, &_inboundCxrLeg2                       \
  }

namespace tse
{

namespace shpq
{

Logger
ConxRoutePQItem::_logger("atseintl.ShoppingPQ.ConxRoutePQItem");

ConxRoutePQItem::CRLevelTuning::CRLevelTuning(const ConxRoutePQItem& crItem) : _crItem(crItem) {}

bool
ConxRoutePQItem::CRLevelTuning::isFirstGroup() const
{
  if (_crItem._solutionPattern.getSPId() == SolutionPattern::SP20)
    return true;
  return false;
}

bool
ConxRoutePQItem::CRLevelTuning::isSecondGroup() const
{
  SolutionPattern::SPEnumType solutionPatternId = _crItem._solutionPattern.getSPId();
  if (solutionPatternId == SolutionPattern::SP30 || solutionPatternId == SolutionPattern::SP31 ||
      solutionPatternId == SolutionPattern::SP32 || solutionPatternId == SolutionPattern::SP33 ||
      solutionPatternId == SolutionPattern::SP34 || solutionPatternId == SolutionPattern::SP35 ||
      solutionPatternId == SolutionPattern::SP40 || solutionPatternId == SolutionPattern::SP44 ||
      solutionPatternId == SolutionPattern::SP45 || solutionPatternId == SolutionPattern::SP46 ||
      solutionPatternId == SolutionPattern::SP47 || solutionPatternId == SolutionPattern::SP48 ||
      solutionPatternId == SolutionPattern::SP49)
    return true;
  return false;
}

bool
ConxRoutePQItem::CRLevelTuning::isThirdGroup() const
{
  SolutionPattern::SPEnumType solutionPatternId = _crItem._solutionPattern.getSPId();
  if (solutionPatternId == SolutionPattern::SP42 || solutionPatternId == SolutionPattern::SP43)
    return true;
  return false;
}

bool
ConxRoutePQItem::CRLevelTuning::isFirstGroupFailAllowed(LegPosition legPosition) const
{
  if (legPosition == LEG_NONE || legPosition == LEG_3RD)
    return true;
  return false;
}

bool
ConxRoutePQItem::CRLevelTuning::isSecondGroupFailAllowed(LegPosition legPosition) const
{
  if (legPosition == LEG_NONE)
    return true;

  SolutionPattern::SPEnumType solutionPatternId = _crItem._solutionPattern.getSPId();
  if (solutionPatternId == SolutionPattern::SP30 || solutionPatternId == SolutionPattern::SP31 ||
      solutionPatternId == SolutionPattern::SP32)
  {
    if (legPosition == LEG_3RD)
      return true;
  }
  if (legPosition == LEG_4TH)
    return true;
  return false;
}

bool
ConxRoutePQItem::CRLevelTuning::isThirdGroupFailAllowed(LegPosition legPosition) const
{
  if (legPosition == LEG_NONE || legPosition == LEG_4TH)
    return true;
  return false;
}

void
ConxRoutePQItem::CRLevelTuning::collectFareMarkets(std::vector<OwrtFareMarketPtr>& container) const
{
  const CxrLegInfo* const items[] = { _crItem.getCarrierLegInfo(LEG_1ST),
                                      _crItem.getCarrierLegInfo(LEG_2ND),
                                      _crItem.getCarrierLegInfo(LEG_3RD),
                                      _crItem.getCarrierLegInfo(LEG_4TH) };
  for (const CxrLegInfo* const itemPtr : items)
  {
    const CxrLegInfo& item(*itemPtr);
    if (!item.hasCurrent())
      continue;
    container.push_back(*item);
  }
}

bool
ConxRoutePQItem::CRLevelTuning::validate(const SoloTrxData& soloTrxData, LegPosition legPosition)
    const
{
  if (LIKELY(!soloTrxData.isCRTuning1Active() && !soloTrxData.isCRTuning2Active()))
    return true;

  if (isFirstGroup())
  {
    if (!soloTrxData.isCRTuning1Active())
      return true;
    return validateFirstGroup(legPosition);
  }
  else if (isSecondGroup())
  {
    if (!soloTrxData.isCRTuning2Active())
      return true;
    return validateSecondGroup(legPosition);
  }
  else if (isThirdGroup())
  {
    if (!soloTrxData.isCRTuning2Active())
      return true;
    return validateThirdGroup(legPosition);
  }
  // By default we pass combination
  return true;
}

bool
ConxRoutePQItem::CRLevelTuning::validateDomesticReturning(const SoloTrxData& soloTrxData,
                                                          LegPosition legPosition) const
{
  if (!soloTrxData.isCRTuningTag2Active() || !_crItem._solutionPattern.hasHRT())
    return true;

  if (isFirstGroup())
  {
    if (!isFirstGroupFailAllowed(legPosition))
      return true;
  }
  else if (isSecondGroup())
  {
    if (!isSecondGroupFailAllowed(legPosition))
      return true;
  }
  else if (isThirdGroup())
  {
    if (!isThirdGroupFailAllowed(legPosition))
      return true;
  }
  return validateTag2();
}

bool
ConxRoutePQItem::CRLevelTuning::validateFirstGroup(LegPosition legPosition) const
{
  if (!isFirstGroupFailAllowed(legPosition))
    return true;
  std::vector<OwrtFareMarketPtr> container;
  collectFareMarkets(container);

  TSE_ASSERT(container.size() == 2);
  return ((container[0]->hasNormal() && container[1]->hasSpecial()) ||
          (container[1]->hasNormal() && container[0]->hasSpecial())) &&
         ((container[0]->hasTag1() && container[1]->hasTag3()) ||
          (container[0]->hasTag3() && container[1]->hasTag1()));
}

bool
ConxRoutePQItem::CRLevelTuning::validateSecondGroup(LegPosition legPosition) const
{
  if (!isSecondGroupFailAllowed(legPosition))
    return true;
  std::vector<OwrtFareMarketPtr> container;
  collectFareMarkets(container);

  TSE_ASSERT(container.size() == 3 || container.size() == 4);
  for (const std::vector<OwrtFareMarketPtr>::value_type& item : container)
    if (item->hasSpecial())
      return true;
  return false;
}

bool
ConxRoutePQItem::CRLevelTuning::validateThirdGroup(LegPosition legPosition) const
{
  if (!isThirdGroupFailAllowed(legPosition))
    return true;
  std::vector<OwrtFareMarketPtr> container;
  collectFareMarkets(container);

  TSE_ASSERT(container.size() == 4);
  size_t idx0(1), idx1(3);

  if (_crItem._solutionPattern.getSPId() == SolutionPattern::SP43)
  {
    idx0 = 0;
    idx1 = 2;
  }

  return ((container[idx0]->hasNormal() && container[idx1]->hasSpecial()) ||
          (container[idx1]->hasNormal() && container[idx0]->hasSpecial()));
  return false;
}

bool
ConxRoutePQItem::CRLevelTuning::validateTag2() const
{
  std::vector<OwrtFareMarketPtr> container;
  collectFareMarkets(container);

  for (const OwrtFareMarketPtr fm : container)
    if (fm->hasTag2())
      return true;
  return false;
}

// static
CxrFareMarketsPtr
ConxRoutePQItem::CxrLegInfo::getFareMarketsPtr(const DirFMPathPtr& dfM,
                                               const int idx,
                                               const CxrFareMarketsPtr& defaultCfm)
{
  if (dfM.get() && (dfM->end() - dfM->begin() > idx))
    return *(dfM->begin() + idx);
  return defaultCfm;
}

ConxRoutePQItem::CxrLegInfo::CxrLegInfo(const DirFMPathPtr& dfM,
                                        const int idx,
                                        const DirFMPathPtr& defaultDfm)
  : Base(getFareMarketsPtr(dfM, idx, CxrFareMarketsPtr()),
         getFareMarketsPtr(dfM, idx, *(defaultDfm->begin()))->begin())
{
}

ConxRoutePQItem::CxrLegInfo::CxrLegInfo(const CxrLegInfo& other, const bool getNext)
  : Base(other, getNext)
{
}

ConxRoutePQItem::ConxRoutePQItem(const SolutionPattern& solutionPattern,
                                 const DirFMPathPtr& outboundDFm,
                                 const DirFMPathPtr& inboundDFm)
  : CommonSoloPQItem(solutionPattern),
    _outboundCxrLeg1(outboundDFm, 0, outboundDFm),
    _outboundCxrLeg2(outboundDFm, 1, outboundDFm),
    _inboundCxrLeg1(inboundDFm, 0, outboundDFm),
    _inboundCxrLeg2(inboundDFm, 1, outboundDFm),
    _score(calculateScore(UNKNOWN_MONEY_AMOUNT)),
    _validator(*this)
{
}

ConxRoutePQItem::ConxRoutePQItem(const ConxRoutePQItem& other, const LegPosition expandedLegPos)
  : CommonSoloPQItem(other, expandedLegPos),
    _outboundCxrLeg1(other._outboundCxrLeg1, expandedLegPos == LEG_1ST),
    _outboundCxrLeg2(other._outboundCxrLeg2, expandedLegPos == LEG_2ND),
    _inboundCxrLeg1(other._inboundCxrLeg1, expandedLegPos == LEG_3RD),
    _inboundCxrLeg2(other._inboundCxrLeg2, expandedLegPos == LEG_4TH),
    _score(calculateScore(other._score)),
    _validator(*this)
{
}

bool
ConxRoutePQItem::isOnlineSolution() const
{
  CarrierCode govCxr("");
  const CxrLegInfo* const items[] = ALL_ITEMS;
  for (const CxrLegInfo* const itemPtr : items)
  {
    if (itemPtr->hasCurrent())
    {
      const OwrtFareMarketPtr owrtFM = **itemPtr;

      if (govCxr.empty())
        govCxr = owrtFM->getGoverningCarrier();
      if (govCxr != owrtFM->getGoverningCarrier())
        return false;
    }
  }
  return true;
}

bool
ConxRoutePQItem::detectCarriers(const ShoppingTrx& trx) const
{
  return checkCarriers(_outboundCxrLeg1, _outboundCxrLeg2, trx) &&
         checkCarriers(_inboundCxrLeg1, _inboundCxrLeg2, trx);
}

bool
ConxRoutePQItem::checkCarriers(const CxrLegInfo& legInfo1,
                               const CxrLegInfo& legInfo2,
                               const ShoppingTrx& trx) const
{
  if (legInfo1.hasCurrent() && legInfo2.hasCurrent())
  {
    const FareMarket& fm1 = *(*legInfo1)->getFareMarket();
    const FareMarket& fm2 = *(*legInfo2)->getFareMarket();

    if (!fm1.getApplicableSOPs() || !fm2.getApplicableSOPs())
      return false;

    const ApplicableSOP& appSop1 = *fm1.getApplicableSOPs();
    const ApplicableSOP& appSop2 = *fm2.getApplicableSOPs();

    for (const ApplicableSOP::value_type& cxrSops1 : appSop1)
    {
      const ItinIndex::Key& cxr1 = cxrSops1.first;
      if (appSop2.count(cxr1))
        return true;
    }

    return false;
  }
  return true;
}

bool
ConxRoutePQItem::validateCandidate(const SoloTrxData& soloTrxData, LegPosition legPosition) const
{
  /*
   * Apply only if not NA Domestic or Foreign domestic
   */
  GeoTravelType tvlType =
      soloTrxData.getShoppingTrx().journeyItin()->travelSeg().front()->geoTravelType();
  if (UNLIKELY(tvlType == GeoTravelType::Domestic && soloTrxData.getShoppingTrx().legs().size() >= 2))
    return _validator.validateDomesticReturning(soloTrxData, legPosition);
  if (UNLIKELY(tvlType == GeoTravelType::Domestic || tvlType == GeoTravelType::ForeignDomestic))
    return true;
  return _validator.validate(soloTrxData, legPosition);
}

// virtual
void
ConxRoutePQItem::expand(SoloTrxData& soloTrxData, SoloPQ& pq)
{
  if (!_solutionPattern.isItemValid(this, pq.diagCollector()))
    return;

  const CxrLegInfo* const items[] = ALL_ITEMS;

  LegPosition expandedLegPos(LEG_1ST);
  size_t numberOfExpandedItems(0);

  SoloPQItemManager& pqItemMgr = soloTrxData.getPQItemManager();
  for (const CxrLegInfo* const itemPtr : items)
  {
    const CxrLegInfo& item(*itemPtr);
    if (isPlaceholderLeg(expandedLegPos) && item.hasNext())
    {
      ConxRoutePQItemPtr pqItem(pqItemMgr.constructCRPQItem(*this, expandedLegPos));
      ConxRoutePQItem* pqItemParent = this;
      do
      {
        if (LIKELY(pqItem->validateCandidate(soloTrxData, expandedLegPos)))
        {
          pq.enqueue(pqItem, this);
          ++numberOfExpandedItems;
          break;
        }
        pq.diagCollector().onSkipCRExpansion(pqItem, pqItemParent);
        pqItemParent = &(*pqItem);
        if ((*pqItem->getCarrierLegInfo(expandedLegPos)).hasNext())
          pqItem = pqItemMgr.constructCRPQItem(*pqItem, expandedLegPos);
        else
          break;
      } while (true);
    }
    expandedLegPos = CommonSoloPQItem::getNextLegPosition(expandedLegPos);
  }

  // next level expansion
  {
    const ShoppingTrx& trx = soloTrxData.getShoppingTrx();
    bool proceedExpansion = detectCarriers(trx);
    std::string validationMsg("");
    if (!proceedExpansion)
    {
      validationMsg = "Common carrier index not found";
    }
    else
    {
      if (trx.onlineSolutionsOnly() && !isOnlineSolution())
      {
        validationMsg = "CR->CRC expansion skipped: item invalid for online solution only";
        proceedExpansion = false;
      }
    }

    if (proceedExpansion)
      if (UNLIKELY(!(proceedExpansion = validateCandidate(soloTrxData))))
        validationMsg = "CR-CRC expansion skipped: item invalid due to Tag-1/2/3";

    if (proceedExpansion)
    {
      const ConxRouteCxrPQItemPtr cnxRtCxrPqem(pqItemMgr.constructCRCPQItem(*this, soloTrxData));

      if (cnxRtCxrPqem->isFailed())
      {
        validationMsg = "CR->CRC expansion skipped: item invalid due to Cat10";
        pq.diagCollector().displayValidationMsg(validationMsg, this);
      }
      else
      {
        const Diversity& div = trx.diversity();
        MoneyAmount cutOffAmt = div.getFareCutoffAmount();
        MoneyAmount crcScore = cnxRtCxrPqem->getScore();

        if (div.hasDCL() && (cutOffAmt>EPSILON) && (crcScore>cutOffAmt))
        {
          if (cnxRtCxrPqem->shouldSkip(soloTrxData, pq.getItinStats() ,pq)
              ==false)
          {
            pq.enqueue(cnxRtCxrPqem, this);
            ++numberOfExpandedItems;
          }
        }
        else
        {
          pq.enqueue(cnxRtCxrPqem, this);
          ++numberOfExpandedItems;
        }
      }
    }
    else
      pq.diagCollector().displayValidationMsg(validationMsg, this);
  }

  LOG4CXX_TRACE(_logger,
                "ConxRoutePQItem::expand(): " << this << " expanded into " << numberOfExpandedItems
                                              << " new items");
}

// virtual
std::string
ConxRoutePQItem::str(const StrVerbosityLevel strVerbosity /* = SVL_BARE */) const
{
  const char* const FIELD_SEPARTOR(" : ");

  std::ostringstream stream;
  printBasicStr(stream, "CR ");

  if (strVerbosity >= SVL_BARE)
  {
    const CxrLegInfo* const items[] = ALL_ITEMS;
    LegPosition currentLegPos(LEG_1ST);

    for (const CxrLegInfo* const item : items)
    {
      stream << FIELD_SEPARTOR;

      if (item->hasCurrent())
      {
        stream << " " << (*item)->str();
        if (strVerbosity >= SVL_NORMAL)
        {
          stream << " [" << (*item)->getGoverningCarrier();
          const Fare* fare((*item)->getLowerBoundFare());
          if (fare && strVerbosity >= SVL_DETAILS)
            stream << "-" << (fare->fareClass());
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

ConxRoutePQItem::CxrFMVector
ConxRoutePQItem::getFMVector() const
{
  CxrFMVector fmVector;
  const CxrLegInfo* const items[] = ALL_ITEMS;
  for (const CxrLegInfo* const item : items)
  {
    if (item->hasCurrent())
      fmVector.push_back(item->getFM());
  }
  return fmVector;
}

MoneyAmount
ConxRoutePQItem::calculateScore(const MoneyAmount defaultAmount) const
{
  if (UNLIKELY(!_outboundCxrLeg1.hasCurrent()))
  {
    return defaultAmount;
  }

  const CxrLegInfo* const items[] = ALL_ITEMS;

  MoneyAmount score = 0;
  for (const CxrLegInfo* const item : items)
  {
    if (item->hasCurrent())
      score += (*item)->lowerBound();
  }

  return score;
}

} /* namespace shpq */
} /* namespace tse */
