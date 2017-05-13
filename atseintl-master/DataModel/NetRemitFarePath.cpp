//-------------------------------------------------------------------
//
//  File:        NetRemitFarePath.cpp
//  Created:     Feb 15, 2006
//  Authors:
//
//  Description: Net Remit Fare Path is clone from original fare path and replaces
//  Cat35 fares with published fares for Ticketing.
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

#include "DataModel/NetRemitFarePath.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/OpenJawRule.h"
#include "DBAccess/RoundTripRuleItem.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"

namespace tse
{
void
NetRemitFarePath::initialize(
    PricingTrx* trx,
    FarePath* origFp,
    std::map<const PaxTypeFare*, const CombinabilityRuleInfo*>* cat10ForNetFaresMap)
{
  if (trx == nullptr || origFp == nullptr)
    return;

  _trx = trx;
  _dataHandle = &trx->dataHandle();
  _origFp = origFp;

  copyItin();

  // Only copy information needed for Minimum Fares and Tax Processing
  // This fare path does not go through Pricing again.
  _paxType = _origFp->paxType();

  _commissionAmount = _origFp->commissionAmount();
  _commissionPercent = _origFp->commissionPercent();

  _intlSaleIndicator = _origFp->intlSaleIndicator();

  this->baseFareCurrency() = _origFp->baseFareCurrency();
  this->calculationCurrency() = _origFp->calculationCurrency();

  if (_origFp->applyNonIATARounding(*_trx) && _origFp->pricingUnit()
                                                  .front()
                                                  ->fareUsage()
                                                  .front()
                                                  ->paxTypeFare()
                                                  ->isNetRemitTktIndicatorAorB())
    _applyNonIATARounding = NO;

  CloneAndAddPricingUnit cloneAddPu(
      _trx, _dataHandle, *this, _pricingUnit, cat10ForNetFaresMap);
  std::for_each(_origFp->pricingUnit().begin(), _origFp->pricingUnit().end(), cloneAddPu);

  // Now calculate new totalNUCAmount from new fare path
  CalcNUCAmount calcAmt(*this);
  std::for_each(_pricingUnit.begin(), _pricingUnit.end(), calcAmt);
}

void
NetRemitFarePath::copyItin()
{
  Itin* itin = _origFp->itin();

  _dataHandle->get(_itin);

  _itin->geoTravelType() = itin->geoTravelType();
  _itin->ticketingCarrier() = itin->ticketingCarrier();
  _itin->validatingCarrier() = itin->validatingCarrier();
  _itin->ticketingCarrierPref() = itin->ticketingCarrierPref();

  _itin->intlSalesIndicator() = itin->intlSalesIndicator();

  _itin->calculationCurrency() = itin->calculationCurrency();
  _itin->originationCurrency() = itin->originationCurrency();

  _itin->setTravelDate(itin->travelDate());
  _itin->bookingDate() = itin->bookingDate();

  _itin->useInternationalRounding() = itin->useInternationalRounding();
  _itin->fareMarket() = itin->fareMarket();
}

void
NetRemitFarePath::getNetRemitFareUsage(FareUsage* origFu,
                                       FareUsage*& netRemitFu1,
                                       FareUsage*& netRemitFu2)
{
  if (origFu == nullptr)
    return;

  std::multimap<FareUsage*, FareUsage*>::iterator firstPos = _fareUsageMap.lower_bound(origFu);
  std::multimap<FareUsage*, FareUsage*>::iterator lastPos = _fareUsageMap.upper_bound(origFu);

  if (firstPos != lastPos)
    netRemitFu1 = firstPos->second;

  ++firstPos;

  if (firstPos != lastPos)
    netRemitFu2 = firstPos->second;
}

const FareUsage*
NetRemitFarePath::getOriginalFareUsage(const FareUsage* netRemitFu) const
{
  if (netRemitFu == nullptr)
    return nullptr;

  std::multimap<FareUsage*, FareUsage*>::const_iterator firstPos = _fareUsageMap.begin();
  for (; firstPos != _fareUsageMap.end(); ++firstPos)
  {
    if (firstPos->second == netRemitFu)
    {
      return firstPos->first;
    }
  }

  return nullptr;
}

bool
NetRemitFarePath::isTFDPSCFareAmtFare(const FareUsage* fu)
{
  if (fu->netRemitPscResults().empty())
    return false;
  const NegPaxTypeFareRuleData* ruleData = fu->paxTypeFare()->getNegRuleData();
  TSE_ASSERT(ruleData);
  const NegFareRestExt* negFareRestExt = ruleData->negFareRestExt();

  return negFareRestExt && (negFareRestExt->fareBasisAmtInd() == RuleConst::NR_VALUE_A ||
                            negFareRestExt->fareBasisAmtInd() == RuleConst::NR_VALUE_B);
}

void
CalcNUCAmount::
operator()(PricingUnit* pu)
{
  // Add up total NUC amount for FP
  _fp.increaseTotalNUCAmount(pu->getTotalPuNucAmount());

  CalcNUCAmount calcAmt(_fp);
  std::for_each(pu->sideTripPUs().begin(), pu->sideTripPUs().end(), calcAmt);
}

void
CloneAndAddPricingUnit::
operator()(PricingUnit* pu)
{
  if (_dataHandle == nullptr)
    return;

  PricingUnit* newPu = nullptr;
  _dataHandle->get(newPu);
  if (newPu == nullptr)
    return;

  newPu->paxType() = pu->paxType();
  newPu->puType() = pu->puType();
  newPu->puSubType() = pu->puSubType();
  newPu->sameNationOJ() = pu->sameNationOJ();
  newPu->geoTravelType() = pu->geoTravelType();
  newPu->turnAroundPoint() = pu->turnAroundPoint();
  newPu->isSideTripPU() = pu->isSideTripPU();
  newPu->hasSideTrip() = pu->hasSideTrip();

  // Determine new PU Fare Type based on new published fare
  newPu->puFareType() = PricingUnit::NL;

  CloneAndAddFareUsage cloneAddFu(
      _trx, _dataHandle, _fp, *newPu, _cat10ForNetFaresMap);
  std::for_each(pu->fareUsage().begin(), pu->fareUsage().end(), cloneAddFu);

  newPu->earliestTktDT() = pu->earliestTktDT();
  newPu->latestTktDT() = pu->latestTktDT();

  if (!pu->sideTripPUs().empty())
  {
    CloneAndAddPricingUnit cloneAddPu(
        _trx, _dataHandle, _fp, newPu->sideTripPUs(), _cat10ForNetFaresMap);
    std::for_each(pu->sideTripPUs().begin(), pu->sideTripPUs().end(), cloneAddPu);
  }

  // Note: Do not copy Minimum Fare Plus up since it will be recalculated.

  _pricingUnits.push_back(newPu);
}

void
CloneAndAddFareUsage::setupFareUsage(FareUsage*& newFu,
                                     FareUsage* fu,
                                     const std::vector<TravelSeg*>& travelSegs,
                                     bool isAsteriskInd)
{
  // Identify new PU type
  if (newFu->paxTypeFare()->isSpecial())
    _pu.puFareType() = PricingUnit::SP;

  // Add new travel segments
  newFu->travelSeg().insert(newFu->travelSeg().begin(), travelSegs.begin(), travelSegs.end());

  _pu.travelSeg().insert(
      _pu.travelSeg().end(), newFu->travelSeg().begin(), newFu->travelSeg().end());

  _itin.travelSeg().insert(
      _itin.travelSeg().end(), newFu->travelSeg().begin(), newFu->travelSeg().end());

  // Note: Do not copy Surcharges or Minimum Fare Plus up since it will be recalculated.

  newFu->isRoundTrip() = fu->isRoundTrip();
  newFu->hasSideTrip() = fu->hasSideTrip();

  if (!fu->sideTripPUs().empty())
  {
    CloneAndAddPricingUnit cloneAddPu(_trx, _dataHandle, _fp, newFu->sideTripPUs());
    std::for_each(fu->sideTripPUs().begin(), fu->sideTripPUs().end(), cloneAddPu);
  }

  newFu->inbound() = fu->inbound();
  overlayFareAmount(newFu, fu);
  _pu.fareUsage().push_back(newFu);

  if (isAsteriskInd || _fp.usedFareBox())
    _pu.setTotalPuNucAmount(_pu.getTotalPuNucAmount() + newFu->totalFareAmount());
  else
    _pu.setTotalPuNucAmount(_pu.getTotalPuNucAmount() + newFu->paxTypeFare()->totalFareAmount());
}

void
CloneAndAddFareUsage::cloneFareUsageForTFDPSC(FareUsage* fu)
{
  for (const FareUsage::TktNetRemitPscResult& pscResult : fu->netRemitPscResults())
  {
    if (pscResult._resultFare == nullptr)
      continue;

    FareUsage* newFu = nullptr;
    _dataHandle->get(newFu);
    if (newFu == nullptr)
      return;

    newFu->exemptMinFare() = true;
    _pu.exemptMinFare() = true;
    if (_pu.puType() == PricingUnit::Type::OPENJAW)
      _pu.exemptOpenJawRoundTripCheck() = true;

    newFu->highRT() = false;
    newFu->paxTypeFare() = const_cast<PaxTypeFare*>(pscResult._resultFare);
    setupFareUsage(newFu, fu, pscResult._resultFare->fareMarket()->travelSeg(), false);

    _fp.fareUsageMap().insert(std::pair<FareUsage*, FareUsage*>(fu, newFu));
  }
}

void
CloneAndAddFareUsage::
operator()(FareUsage* fu)
{
  if (_dataHandle == nullptr)
    return;

  if (_fp.isTFDPSCFareAmtFare(fu))
  {
    cloneFareUsageForTFDPSC(fu);
    return;
  }

  FareUsage* newFu = nullptr;
  _dataHandle->get(newFu);
  if (newFu == nullptr)
    return;

  // Change Net Remit Cat35 fare to new published fare
  if (fu->tktNetRemitFare())
  {
    newFu->highRT() = setHrtIndForNetRemitFare(fu->tktNetRemitFare(), fu->paxTypeFare());
    newFu->paxTypeFare() = const_cast<PaxTypeFare*>(fu->tktNetRemitFare());
  }
  else
  {
    newFu->paxTypeFare() = fu->paxTypeFare();
    newFu->highRT() = fu->highRT();
  }

  // Update NUC fare amount for Byte 101 value *
  bool isAsteriskInd = false;
  if (fu->tktNetRemitFare() == nullptr)
    updateNucFareAmount(newFu, fu, isAsteriskInd);

  setupFareUsage(newFu, fu, newFu->paxTypeFare()->fareMarket()->travelSeg(), isAsteriskInd);
  _fp.fareUsageMap().insert(std::pair<FareUsage*, FareUsage*>(fu, newFu));

  if (fu->tktNetRemitFare2() != nullptr) // Recurring segment
  {
    FareUsage* newFu2 = nullptr;
    _dataHandle->get(newFu2);
    if (newFu2 == nullptr)
      return;

    newFu2->highRT() = setHrtIndForNetRemitFare(fu->tktNetRemitFare2(), fu->paxTypeFare());

    newFu2->paxTypeFare() = const_cast<PaxTypeFare*>(fu->tktNetRemitFare2());

    setupFareUsage(newFu2, fu, newFu2->paxTypeFare()->fareMarket()->travelSeg(), false);

    _fp.fareUsageMap().insert(std::pair<FareUsage*, FareUsage*>(fu, newFu2));
  }
}

bool
CloneAndAddFareUsage::setHrtIndForNetRemitFare(const PaxTypeFare* tktNetRemitFare,
                                               const PaxTypeFare* ptf) const
{
  if (tktNetRemitFare->directionality() != ptf->directionality() ||
      tktNetRemitFare->fareMarket()->boardMultiCity() != ptf->fareMarket()->boardMultiCity() ||
      tktNetRemitFare->fareMarket()->offMultiCity() != ptf->fareMarket()->offMultiCity() ||
      tktNetRemitFare->fareMarket()->governingCarrier() != ptf->fareMarket()->governingCarrier())
  {
    return false;
  }

  const CombinabilityRuleInfo* pCat10 = tktNetRemitFare->rec2Cat10();
  if (!pCat10)
  {
    if (!_cat10ForNetFaresMap)
      return false;
    std::map<const PaxTypeFare*, const CombinabilityRuleInfo*>::const_iterator cIt =
        (*_cat10ForNetFaresMap).find(tktNetRemitFare);
    if (cIt == _cat10ForNetFaresMap->end())
      return false;
    pCat10 = (*cIt).second;
    if (!pCat10)
      return false;
  }

  const std::vector<CombinabilityRuleItemInfoSet*>& catSet = pCat10->categoryRuleItemInfoSet();
  std::vector<CombinabilityRuleItemInfoSet*>::const_iterator ruleSetIt = catSet.begin();
  for (; ruleSetIt != catSet.end(); ++ruleSetIt)
  {
    CombinabilityRuleItemInfoSet::const_iterator ruleItemIt =
        (*ruleSetIt)->begin();
    for (; ruleItemIt != (*ruleSetIt)->end(); ++ruleItemIt)
    {
      if (PricingUnit::Type::OPENJAW == _pu.puType() && ruleItemIt->itemcat() == 101)
      {
        const OpenJawRule* ojr =
            _dataHandle->getOpenJawRule(pCat10->vendorCode(), ruleItemIt->itemNo());
        if (ojr && ojr->highrtInd() == 'X')
          return true;
      }
      else if (PricingUnit::Type::ROUNDTRIP == _pu.puType() && ruleItemIt->itemcat() == 102)
      {
        const RoundTripRuleItem* rtr =
            _dataHandle->getRoundTripRuleItem(pCat10->vendorCode(), ruleItemIt->itemNo());
        if (rtr && rtr->highrtInd() == 'X')
          return true;
      }
    }
  }
  return false;
}

void
CloneAndAddFareUsage::updateNucFareAmount(FareUsage* newFu, FareUsage* fu, bool& isAsteriskInd)
    const
{
  const NegPaxTypeFareRuleData* ruleData = fu->paxTypeFare()->getNegRuleData();
  if (ruleData)
  {
    const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(ruleData->ruleItemInfo());

    if (negFareRest)
    {
      bool infiniUser = _trx->getRequest()->ticketingAgent()->infiniUser();

      if ((_trx->getRequest()->ticketingAgent()->abacusUser() || infiniUser) &&
          !negFareRest->cur11().empty() &&
          (negFareRest->netRemitMethod() == '2' || negFareRest->netRemitMethod() == '3' ||
           (infiniUser && negFareRest->netRemitMethod() == '1')))
      {
        PaxTypeFare* newPTFare = fu->paxTypeFare()->clone(*_dataHandle);
        Fare* newFare = fu->paxTypeFare()->fare()->clone(*_dataHandle);
        newPTFare->setFare(newFare);
        newFu->paxTypeFare() = newPTFare;
        fu->tktNetRemitFare() = newPTFare;
        return;
      }

      Indicator tktFareDataInd = negFareRest->tktFareDataInd1();
      if (tktFareDataInd == ' ' && ruleData->negFareRestExt())
        tktFareDataInd = ruleData->negFareRestExt()->fareBasisAmtInd();

      if (tktFareDataInd == RuleConst::NR_VALUE_N)
      {
        isAsteriskInd = true;
        PaxTypeFare* newPTFare = fu->paxTypeFare()->clone(*_dataHandle);
        Fare* newFare = fu->paxTypeFare()->fare()->clone(*_dataHandle);
        newPTFare->setFare(newFare);
        newFu->paxTypeFare() = newPTFare;

        newFu->netCat35NucUsed();
        newFu->netCat35NucAmount() = ruleData->nucNetAmount();
        newPTFare->nucFareAmount() = newFu->netCat35NucAmount();
        if (newPTFare->mileageSurchargeAmt() > 0 && newPTFare->mileageSurchargePctg() > 0)
        {
          newPTFare->mileageSurchargeAmt() =
              (newPTFare->nucFareAmount() * newPTFare->mileageSurchargePctg()) / 100.0;
          CurrencyUtil::truncateNUCAmount(newPTFare->mileageSurchargeAmt());
        }

        newFu->stopOverAmt() = fu->stopOverAmt();
        newFu->transferAmt() = fu->transferAmt();
        newFu->stopoverSurcharges() = fu->stopoverSurcharges();
        newFu->transferSurcharges() = fu->transferSurcharges();
        fu->tktNetRemitFare() = newPTFare;
      }
    }
  }
}

void
CloneAndAddFareUsage::overlayFareAmount(FareUsage* newFu, FareUsage* fu)
{
  const NegPaxTypeFareRuleData* ruleData = fu->paxTypeFare()->getNegRuleData();

  if (!fu->tktNetRemitFare() || !ruleData)
    return;

  const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(ruleData->ruleItemInfo());

  if (negFareRest && !negFareRest->cur11().empty() && useFareBox(*negFareRest))
  {
    PaxTypeFare* clonedPTF = fu->tktNetRemitFare()->clone(*_dataHandle, false);
    FareInfo* fareInfo = clonedPTF->fare()->fareInfo()->clone(*_dataHandle);
    Fare* fare = clonedPTF->fare()->clone(*_dataHandle, false);

    fareInfo->originalFareAmount() = negFareRest->fareAmt1();
    fareInfo->owrt() = fu->paxTypeFare()->fare()->owrt();
    fareInfo->fareAmount() = (fareInfo->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
                                 ? (fareInfo->originalFareAmount() / 2)
                                 : fareInfo->originalFareAmount();
    fareInfo->currency() = negFareRest->cur11();
    fareInfo->noDec() = negFareRest->noDec11();

    fare->setFareInfo(fareInfo);
    fare->mileageSurchargeAmt() = 0.0;
    fare->mileageSurchargePctg() = 0;
    clonedPTF->setFare(fare);
    newFu->paxTypeFare() = clonedPTF;
    fu->tktNetRemitFare() = clonedPTF;
    newFu->stopOverAmt() = fu->stopOverAmt();
    newFu->transferAmt() = fu->transferAmt();
    newFu->stopoverSurcharges() = fu->stopoverSurcharges();
    newFu->transferSurcharges() = fu->transferSurcharges();
    newFu->surchargeAmt() = fu->surchargeAmt();
    newFu->surchargeData() = fu->surchargeData();
    setNucAmounts(*clonedPTF);
    _fp.usedFareBox() = true;
  }
}

void
CloneAndAddFareUsage::setNucAmounts(PaxTypeFare& ptf)
{
  CurrencyConversionFacade ccFacade;
  Money nuc(NUC);
  Money originalFareAmount(ptf.originalFareAmount(), ptf.currency());
  MoneyAmount convertedAmount = 0.0;

  if (ccFacade.convert(nuc,
                       originalFareAmount,
                       *_trx,
                       _fp.itin()->calculationCurrency(),
                       convertedAmount,
                       _fp.itin()->useInternationalRounding()))
  {
    CurrencyUtil::truncateNUCAmount(convertedAmount);
    ptf.nucOriginalFareAmount() = convertedAmount;
  }

  ptf.fare()->nucFareAmount() = ptf.nucOriginalFareAmount();
  if (ptf.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    CurrencyUtil::halveNUCAmount(ptf.fare()->nucFareAmount());
  }
}

NetRemitFarePath*
NetRemitFarePath::clone(DataHandle& dataHandle) const
{
  NetRemitFarePath* res = dataHandle.create<NetRemitFarePath>();
  *res = *this;

  return res;
}

bool
CloneAndAddFareUsage::useFareBox(const NegFareRest& negFareRest) const
{
  bool abacus = _trx->getRequest()->ticketingAgent()->abacusUser();
  bool infini = _trx->getRequest()->ticketingAgent()->infiniUser();
  return ((negFareRest.netRemitMethod() == '2' && TrxUtil::isNetRemitEnabled(*_trx)) ||
          (negFareRest.netRemitMethod() == '3' && (abacus || infini)) ||
          (negFareRest.netRemitMethod() == '1' && infini));
}
}
