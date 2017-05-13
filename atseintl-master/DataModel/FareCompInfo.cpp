//-------------------------------------------------------------------
//
//  File:        FareCompInfo.cpp
//  Created:     13 June 2008
//  Authors:     Grzegorz Wanke
//
//  Description: Fare Component class
//
//  Updates:
//
//  Copyright Sabre 2008
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
#include "DataModel/FareCompInfo.h"

#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"

#include <boost/assign/list_of.hpp>
#include <boost/logic/tribool.hpp>

namespace tse
{
FALLBACK_DECL(fallbackCat31KeepWholeFareSetOnExcFM);

void
PartialFareBreakLimitationValidation::setUp()
{
  for (const ProcessTagInfo* pti : _processTags)
  {
    if (!pti->reissueSequence()->orig() || pti->reissueSequence()->samePointTblItemNo() ||
        pti->reissueSequence()->terminalPointInd() == 'Y')
    {
      _forceFareBreak = false;
      break;
    }
  }

  if (_processTags.front()->fareCompNumber() != 1)
  {
    _forceKeepFare = false;
    return;
  }

  for (const ProcessTagInfo* pti : _processTags)
  {
    if (!pti->reissueSequence()->orig() || pti->reissueSequence()->samePointTblItemNo() ||
        pti->reissueSequence()->firstBreakInd() == ' ')
    {
      _forceKeepFare = false;
      return;
    }
  }
}

namespace
{
class SameSegsByOrder : public std::binary_function<const TravelSeg*, int16_t, bool>
{
  const Itin& _itin;

public:
  SameSegsByOrder(const Itin& itin) : _itin(itin) {}

  bool operator()(const TravelSeg* newSeg, int16_t segOrd) const
  {
    return segOrd == _itin.segmentOrder(newSeg);
  }
};
}

void
PartialFareBreakLimitationValidation::setNewSegments(const std::vector<TravelSeg*>& tss,
                                                     const Itin& itin)
{
  if (_newSegments.empty())
  {
    for (const TravelSeg* ts : tss)
    {
      _newSegments.push_back(itin.segmentOrder(ts));
    }
  }
}

bool
PartialFareBreakLimitationValidation::doNotUseForPricing(const FareMarket& fm, const Itin& itin)
{
  return (_forceFareBreak || _forceKeepFare) && fm.travelSeg().size() != _newSegments.size() &&
         std::find_first_of(fm.travelSeg().begin(),
                            fm.travelSeg().end(),
                            _newSegments.begin(),
                            _newSegments.end(),
                            SameSegsByOrder(itin)) != fm.travelSeg().end();
}

FareCompInfo::OverridingIntlFcData*
FareCompInfo::findOverridingData(const VoluntaryChangesInfo* vcRec3)
{
  if (!vcRec3 || getMultiNewItinData().overridingFcs().empty())
    return nullptr;
  FareCompInfo::OverridingIntlFcCache::iterator i =
      getMultiNewItinData().overridingFcs().find(vcRec3);
  return i == getMultiNewItinData().overridingFcs().end() ? nullptr : &(*i).second;
}

PaxTypeBucket*
FareCompInfo::getPaxTypeBucket(const RexBaseTrx& trx, bool secondaryMarket) const
{
  const PaxType* paxType = trx.paxType().front();

  if (secondaryMarket && _secondaryFareMarket)
    return _secondaryFareMarket->paxTypeCortege(paxType);
  else
    return _fareMarket->paxTypeCortege(paxType);
}

class OneOfMatchedFares : public std::unary_function<const PaxTypeFare*, bool>
{
public:
  explicit OneOfMatchedFares(const FareCompInfo::MatchedFaresVec& mfv) : _mfv(mfv) {}

  bool operator()(const PaxTypeFare* ptf) const
  {
    return std::any_of(_mfv.cbegin(),
                       _mfv.cend(),
                       [ptf](const FareCompInfo::MatchedFare& mf)
                       { return mf.get() == ptf; });
  }

private:
  const FareCompInfo::MatchedFaresVec& _mfv;
};

void
FareCompInfo::updateFareMarket(const RexBaseTrx& trx)
{
  if (fallback::fallbackCat31KeepWholeFareSetOnExcFM(&trx))
  {
    std::vector<PaxTypeFare*> emptyVec;
    _fareMarket->allPaxTypeFare().swap(emptyVec);

    if (!_matchedFares.empty())
    {
      _fareMarket->allPaxTypeFare().reserve(_matchedFares.size());

      for (MatchedFare& fare : _matchedFares)
      {
        PaxTypeFare* ptf = fare.get();
        _fareMarket->allPaxTypeFare().push_back(ptf);
        ptf->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);
        ptf->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL, false);
        ptf->bookingCodeStatus().set(PaxTypeFare::BKS_PASS, true);
      }

      static const uint16_t FARE_BASIS_SIZE = 12;

      _fareMatchingPhase = trx.getCurrTktDateSeqStatus();
      _fareMarket->serviceStatus().set(FareMarket::RexFareSelector);
      _fareMarket->ruleApplicationDate() =
          trx.getRuleApplicationDate(_fareMarket->governingCarrier());

      if (_fareMarket->fareBasisCode().size() > FARE_BASIS_SIZE)
        _fareMarket->fareBasisCode() =
            (_matchedFares.front().get()->createFareBasis(const_cast<RexBaseTrx*>(&trx)));
    }

    PaxTypeBucket* ptc = _fareMarket->paxTypeCortege(trx.paxType().front());
    if (ptc)
    {
      std::vector<PaxTypeFare*> secAllPTFVec(_fareMarket->allPaxTypeFare());
      ptc->paxTypeFare().swap(secAllPTFVec);
    }
  }

  else
  {
    if (!_matchedFares.empty())
    {
      for (MatchedFare& fare : _matchedFares)
      {
        PaxTypeFare* ptf = fare.get();
        ptf->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);
        ptf->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL, false);
        ptf->bookingCodeStatus().set(PaxTypeFare::BKS_PASS, true);
      }

      static const uint16_t FARE_BASIS_SIZE = 12;

      _fareMatchingPhase = trx.getCurrTktDateSeqStatus();
      _fareMarket->serviceStatus().set(FareMarket::RexFareSelector);
      _fareMarket->ruleApplicationDate() =
          trx.getRuleApplicationDate(_fareMarket->governingCarrier());

      if (_fareMarket->fareBasisCode().size() > FARE_BASIS_SIZE)
        _fareMarket->fareBasisCode() =
            (_matchedFares.front().get()->createFareBasis(const_cast<RexBaseTrx*>(&trx)));

      if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      {
        std::remove_copy_if(_fareMarket->allPaxTypeFare().begin(),
                            _fareMarket->allPaxTypeFare().end(),
                            std::back_inserter(_otherFares),
                            OneOfMatchedFares(_matchedFares));
      }
    }

    _fareMarket->allPaxTypeFare().clear();

    for (MatchedFare& fare : _matchedFares)
    _fareMarket->allPaxTypeFare().push_back(fare.get());

    PaxTypeBucket* ptc = _fareMarket->paxTypeCortege(trx.paxType().front());
    if (ptc)
      ptc->paxTypeFare() = _fareMarket->allPaxTypeFare();
  }
}

void
FareCompInfo::loadOtherFares(RexPricingTrx& ntrx)
{
  ExchangeUtil::avoidValidationOfCategoriesInMinFares(ntrx, _otherFares);

  FareMarket* newFM;
  ntrx.dataHandle().get(newFM);
  _fareMarket->clone(*newFM);
  ntrx.fareMarket().push_back(newFM);
  newFM->setBreakIndicator(true);
  newFM->fareCompInfo() = nullptr;
  newFM->allPaxTypeFare() = _otherFares;
  ntrx.exchangeItin().front()->addSiblingMarket(_fareMarket, newFM);

  PaxTypeBucket* ptc = newFM->paxTypeCortege(ntrx.paxType().front());
  if (ptc)
    ptc->paxTypeFare() = _otherFares;

  for (PaxTypeFare* newPTFare : newFM->allPaxTypeFare())
    newPTFare->fareMarket() = newFM;
}

FareCompInfo::MultiNewItinData&
FareCompInfo::getMultiNewItinData()
{
  return _multiNewItinData[_itinIndex];
}

const FareCompInfo::MultiNewItinData&
FareCompInfo::getMultiNewItinData() const
{
  if (_multiNewItinData.count(_itinIndex) == 0)
    throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
  return _multiNewItinData.find(_itinIndex)->second;
}

FareCompInfo::MultiNewItinData&
FareCompInfo::getMultiNewItinData(uint16_t itinIndex)
{
  if (_multiNewItinData.count(itinIndex) == 0)
    throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
  return _multiNewItinData.find(itinIndex)->second;
}

const FareCompInfo::MultiNewItinData&
FareCompInfo::getMultiNewItinData(uint16_t itinIndex) const
{
  if (_multiNewItinData.count(itinIndex) == 0)
    throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
  return _multiNewItinData.find(itinIndex)->second;
}

void
FareCompInfo::updateFMMapping(Itin* itin)
{
  if (_fareMarket->travelSeg().front()->unflown())
    return;

  std::vector<FareMarket*>::const_iterator fmIter = itin->fareMarket().begin();
  std::vector<TravelSeg*>::const_iterator segIter;
  std::vector<TravelSeg*>::const_iterator excSegIter;

  for (; fmIter != itin->fareMarket().end(); ++fmIter)
  {
    segIter = (*fmIter)->travelSeg().begin();
    excSegIter = _fareMarket->travelSeg().begin();
    while (segIter != (*fmIter)->travelSeg().end() && excSegIter != _fareMarket->travelSeg().end())
    {
      if ((*segIter)->segmentOrder() == (*excSegIter)->segmentOrder())
      {
        _mappedFC.insert(*fmIter);
        break;
      }
      if ((*segIter)->segmentOrder() > (*excSegIter)->segmentOrder())
        ++excSegIter;
      else
        ++segIter;
    }
  }
}
}
