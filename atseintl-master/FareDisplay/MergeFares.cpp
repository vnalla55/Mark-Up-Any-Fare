//-------------------------------------------------------------------
//
//  File:        MergeFares.h
//  Authors:     Jeff Hoffman
//
//  Description: creates final list of PaxTypeFares to be used by FareDisplayTrx
//
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "FareDisplay/MergeFares.h"

#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Fares/AdjustedSellingLevelFareCollector.h"
#include "Common/FallbackUtil.h"

#include <algorithm>
#include <ext/functional>

namespace tse
{

//-----------------------------------------------------------------
// tse::EqualBy unary predicate to find out the duplicate PaxTypeFare
//-----------------------------------------------------------------
bool
EqualBy::areEqual(const PaxTypeFare* lhs, const PaxTypeFare* rhs) const
{
  if (lhs == nullptr || rhs == nullptr)
    return false;

  if (lhs->nucFareAmount() != rhs->nucFareAmount())
    return false;
  if (!matchCreateDate(lhs, rhs))
    return false;
  if (lhs->vendor() != rhs->vendor())
    return false;
  if (lhs->carrier() != rhs->carrier())
    return false;
  if (lhs->fareTariff() != rhs->fareTariff())
    return false;
  if (lhs->ruleNumber() != rhs->ruleNumber())
    return false;
  if (!matchFareClass(lhs, rhs))
    return false;
  if (!matchRoutingNumber(lhs, rhs))
    return false;
  if (lhs->globalDirection() != rhs->globalDirection())
    return false;
  if (lhs->fareDisplayCat35Type() != rhs->fareDisplayCat35Type())
    return false;
  if (lhs->cat25BasePaxFare() != rhs->cat25BasePaxFare())
    return false;
  if (lhs->penaltyRestInd() != rhs->penaltyRestInd())
    return false;
  if (lhs->fareTypeApplication() != rhs->fareTypeApplication())
    return false;
  if (lhs->fareTypeDesignator() != rhs->fareTypeDesignator())
    return false;
  if (lhs->cabin() != rhs->cabin())
    return false;
  //  if (lhs->matchedCorpID()      != rhs->matchedCorpID())   return false;
  if (!matchCorpID(lhs, rhs))
    return false;
  if (lhs->bookingCode() != rhs->bookingCode())
    return false;

  if (lhs->getAdjustedSellingCalcData() && rhs->getAdjustedSellingCalcData())
  {
    if (lhs->getAdjustedSellingCalcData()->getSourcePcc() != rhs->getAdjustedSellingCalcData()->getSourcePcc())
      return false;
  }

  if (lhs->isAdjustedSellingBaseFare() !=  rhs->isAdjustedSellingBaseFare())
      return false;

  const NegPaxTypeFareRuleData* lhsNegData = lhs->getNegRuleData();
  if (!lhsNegData)
    return true;
  const NegPaxTypeFareRuleData* rhsNegData = rhs->getNegRuleData();
  if (!rhsNegData)
    return true;

  if (lhsNegData->sourcePseudoCity() != rhsNegData->sourcePseudoCity())
    return false;

  return true;
};

bool
EqualBy::
operator()(const PaxTypeFare* p) const
{
  return areEqual(_pFare, p);
}

const RoutingNumber&
EqualBy::validRtgNum(const PaxTypeFare* p) const
{
  if (p->routingNumber() == CAT25_EMPTY_ROUTING)
    return p->fareWithoutBase()->routingNumber();

  return p->routingNumber();
}

bool
EqualBy::matchRoutingNumber(const PaxTypeFare* pFare, const PaxTypeFare* p) const
{
  if (pFare->isConstructed() && p->isConstructed())
  {
    return (pFare->routingNumber() == p->routingNumber() &&
            pFare->origAddonRouting() == p->origAddonRouting() &&
            pFare->destAddonRouting() == p->destAddonRouting());
  }
  return (validRtgNum(pFare) == validRtgNum(p));
}

bool
EqualBy::matchFareClass(const PaxTypeFare* p, const PaxTypeFare* q) const
{
  return p->fareClass() == q->fareClass();
}

bool
EqualBy::matchCreateDate(const PaxTypeFare* p, const PaxTypeFare* q) const
{
  const PaxTypeFare* baseFareP = p->fareWithoutBase();
  const PaxTypeFare* baseFareQ = q->fareWithoutBase();
  if (baseFareP->createDate().isValid() && baseFareQ->createDate().isValid())
  {
    // Using full time of day to get as much precision on the seconds as possible
    return (baseFareP->createDate().get64BitRep() == baseFareQ->createDate().get64BitRep());
  }
  return true;
}

//-----------------------------------------------------------------
// Utility functors
//-----------------------------------------------------------------

bool
NotValid::
operator()(const PaxTypeFare* p) const
{
  return !(p && p->isValidForPricing());
}

// only test if invalidated by this process
bool
ValidFD::
operator()(const PaxTypeFare* p) const
{
  return p && p->fareDisplayStatus().isNull();
}

class CopyAllFares
{
public:
  CopyAllFares(std::vector<PaxTypeFare*>& fares) : _fares(fares) {}

  void operator()(tse::FareMarket* fm)
  {
    if (fm == nullptr)
      return;
    std::copy(fm->allPaxTypeFare().begin(), fm->allPaxTypeFare().end(), std::back_inserter(_fares));
  }

private:
  std::vector<PaxTypeFare*>& _fares;
};

//-----------------------------------------------------------------
// additional filtering when global dir = ZZ
// TODO: move to validator or selector?
//-----------------------------------------------------------------

void
MergeFares::markZZ(std::vector<FareMarket*>& fareMarkets, bool isDomestic)
{
  if (isDomestic)
    return;

  std::for_each(fareMarkets.begin(), fareMarkets.end(), MergeFares::markZZinMkt);
}

void
MergeFares::markZZinMkt(tse::FareMarket* fm)
{
  std::for_each(fm->allPaxTypeFare().begin(), fm->allPaxTypeFare().end(), MergeFares::markZZinPTF);
}
void
MergeFares::markZZinPTF(PaxTypeFare* p)
{
  if (p->globalDirection() == GlobalDirection::ZZ)
  {
    // careful - sequence in || matters!!!
    // must check for cat25 before specified
    if (!p->isFareByRule() || !p->isSpecifiedFare())
      p->invalidateFare(PaxTypeFare::FD_ZZ_Global_Dir);
  }
}

void
MergeFares::markDupeInPTF(PaxTypeFare* dummy, PaxTypeFare* p)
{
  p->invalidateFare(PaxTypeFare::FD_Dupe_Fare);
}

//-----------------------------------------------------------------
//  dupe and merge routines
//-----------------------------------------------------------------

void
MergeFares::markDupe()
{
  std::vector<PaxTypeBucket*>::iterator b = _buckets.begin();
  for (; b != _buckets.end(); b++)
  {
    if ((*b) == nullptr)
      return;
    forAllMatches((*b)->paxTypeFare(), (*b)->paxTypeFare(), MergeFares::markDupeInPTF);
  }
}

bool
MergeFares::isChildOrInfant(PaxType* pt)
{
  return (pt->paxTypeInfo()->isChild() || pt->paxTypeInfo()->isInfant());
}

bool
MergeFares::canPrice(PaxTypeFare* ptf)
{
  return (ptf->fareDisplayInfo() && !ptf->fareDisplayInfo()->unsupportedPaxType());
}

bool
MergeFares::canMerge(PaxTypeBucket* bucket1, PaxTypeBucket* bucket2)
{
  if (bucket1->paxTypeFare().size() == 0 || bucket2->paxTypeFare().size() == 0)
    return false;

  bool CHorIN1 = isChildOrInfant(bucket1->requestedPaxType());
  bool CHorIN2 = isChildOrInfant(bucket2->requestedPaxType());
  if (CHorIN1 != CHorIN2)
    return false;

  bool canPrice1 = canPrice(bucket1->paxTypeFare().front());
  bool canPrice2 = canPrice(bucket2->paxTypeFare().front());
  return (canPrice1 == canPrice2);
}

// put ptf2's data into ptf1 and mark ptf2
void
MergeFares::doMergeInPTF(PaxTypeFare* ptf1, PaxTypeFare* ptf2)
{
  PaxTypeCode paxType = ptf2->actualPaxType() ? ptf2->actualPaxType()->paxType() : "ADT";

  FareDisplayInfo* fdiP = const_cast<FareDisplayInfo*>(ptf1->fareDisplayInfo());
  if (fdiP)
  {
    fdiP->passengerTypes().insert(paxType);
  }
  ptf2->invalidateFare(PaxTypeFare::FD_Merged_Fare);
}

void
MergeFares::mergeAllBuckets()
{
  if (_buckets.size() < 2)
    return; // no pairs possible

  // get all pairs of different buckets
  std::vector<PaxTypeBucket*>::iterator iter1 = _buckets.begin();
  std::vector<PaxTypeBucket*>::iterator iter2End = _buckets.end();
  std::vector<PaxTypeBucket*>::iterator iter1End = iter2End - 1;
  for (; iter1 != iter1End; iter1++)
  {
    std::vector<PaxTypeBucket*>::iterator iter2 = iter1 + 1;
    for (; iter2 != iter2End; iter2++)
    {
      if (canMerge(*iter1, *iter2))
        forAllMatches((*iter1)->paxTypeFare(), (*iter2)->paxTypeFare(), MergeFares::doMergeInPTF);
    }
  }
}

// call funct() for each PTF in fare2 that matches a PTF in fare1
//
// for duplicates, the two PTF vectors will be the same; funct invalidates PTF
// for merging, the two PTF vector will be buckets of mergable psgTypes; funct does merge

void
MergeFares::forAllMatches(std::vector<PaxTypeFare*>& fares1,
                          std::vector<PaxTypeFare*>& fares2,
                          void (*funct)(PaxTypeFare* ptf1, PaxTypeFare* ptf2))
{
  bool sameVec = (&fares1 == &fares2);
  // need at least 2 fares to compare
  if (fares1.size() == 0 || fares2.size() == 0 || (fares1.size() == 1 && sameVec))
    return;

  std::vector<PaxTypeFare*>::iterator iter1 = fares1.begin();
  std::vector<PaxTypeFare*>::iterator iter1End = fares1.end();
  std::vector<PaxTypeFare*>::iterator iter2End = fares2.end();
  std::vector<PaxTypeFare*>::iterator iter2Found;

  ValidFD isValid;

  // if called with the same vector for both, don't compare twice
  if (sameVec)
    iter1End = iter2End - 1;

  for (; iter1 != iter1End; iter1++)
  {
    // just need to check for FD status set recently in MergeFare
    // [init() checks input status]

    if (!isValid(*iter1))
      continue;

    std::vector<PaxTypeFare*>::iterator iter2 = fares2.begin();
    if (sameVec)
      iter2 = iter1 + 1;

    while (iter2End !=
           (iter2Found =
                find_if(iter2,
                        iter2End,
                        __gnu_cxx::compose2(std::logical_and<bool>(), EqualBy(*iter1, &_trx), isValid))))
    {
      (*funct)(*iter1, *iter2Found);
      iter2 = ++iter2Found;
    }
  } // endfor - each PTF in fares1
}

void
MergeFares::copyResults(std::vector<PaxTypeFare*>& fares)
{
  std::vector<PaxTypeBucket*>::iterator iterBucket = _buckets.begin();
  std::vector<PaxTypeBucket*>::iterator iterBucketEnd = _buckets.end();
  for (; iterBucket != iterBucketEnd; iterBucket++)
  {
    remove_copy_if((*iterBucket)->paxTypeFare().begin(),
                   (*iterBucket)->paxTypeFare().end(),
                   std::back_inserter(fares),
                   not1(ValidFD()));
  }
}

//-----------------------------------------------------------------
//  workarea and initialzation
//-----------------------------------------------------------------

class simBucket : public std::unary_function<PaxTypeBucket*, bool>
{
public:
  simBucket(PaxTypeCode ptc) : _ptc(ptc) {};
  bool operator()(const PaxTypeBucket* b) { return _ptc == b->requestedPaxType()->paxType(); }
  PaxTypeCode _ptc;
};

void
MergeFares::validFaresToWorkarea(FareMarket* fm, bool isDomestic)
{
  std::vector<PaxTypeBucket>::iterator iterCortege;
  PaxTypeBucket* targetBucket = nullptr;

  for (iterCortege = fm->paxTypeCortege().begin(); iterCortege != fm->paxTypeCortege().end();
       iterCortege++)
  {
    _trx.dataHandle().get(targetBucket);
    targetBucket->requestedPaxType() = iterCortege->requestedPaxType();
    _buckets.push_back(targetBucket);

    remove_copy_if((*iterCortege).paxTypeFare().begin(),
                   (*iterCortege).paxTypeFare().end(),
                   std::back_inserter(targetBucket->paxTypeFare()),
                   NotValid());
  }
}

//-----------------------------------------------------------------
//  what the outside word calls
//  TODO: make new run() calls for different situations, perhaps remove bools
//        e.g. runCopyAll(), runRemoveDupes(), runMergeAndRemoveDupes()
//-----------------------------------------------------------------

void
MergeFares::run(std::vector<FareMarket*>& fareMarkets, bool copyAllFares, bool isDomestic)
{
  markZZ(fareMarkets, isDomestic);

  if (copyAllFares)
  {
    std::for_each(fareMarkets.begin(), fareMarkets.end(), CopyAllFares(_trx.allPaxTypeFare()));
    return;
  }

  std::vector<FareMarket*>::iterator iterMkt = fareMarkets.begin();
  for (; iterMkt != fareMarkets.end(); iterMkt++)
  {
    _buckets.clear();
    validFaresToWorkarea(*iterMkt, isDomestic);

    bool allowMerge = true;

    if (_trx.getRequest()->requestedInclusionCode() == ALL_FARES && !_trx.isShortRequest())
    {
      allowMerge = false;
    }
    if (_trx.getRequest()->displayPassengerTypes().size() > 1)
    {
      allowMerge = false;
    }

    // TODO: error ?!?!?!
    // should still mark/remove dupe when no merge?
    if (allowMerge)
      markDupe();

    if (allowMerge)
      mergeAllBuckets();

    copyResults(_trx.allPaxTypeFare());

  } // endfor - each market
}

bool
EqualBy::matchCorpID(const PaxTypeFare* pFare, const PaxTypeFare* p) const
{
  if (pFare->matchedCorpID() == p->matchedCorpID())
    return true;

  if (pFare->isFareByRule() && p->isFareByRule())
  {
    const FareByRuleItemInfo& pFareItemInfo = pFare->fareByRuleInfo();
    const FareByRuleItemInfo& pItemInfo = p->fareByRuleInfo();
    if (pFareItemInfo.vendor() == pItemInfo.vendor() &&
            pFareItemInfo.itemNo() == pItemInfo.itemNo() && pFare->ruleNumber() == p->ruleNumber(),
        pFare->fareTariff() == p->fareTariff())
      return true;
  }
  return false;
}
}
