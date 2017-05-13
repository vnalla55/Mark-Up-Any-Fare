//-------------------------------------------------------------------
//
//  File:        NetFarePath.cpp
//  Created:     June 5, 2012
//  Authors:
//
//  Description: Net Fare Path is clone from original fare path and replaces
//  Cat35 fares with Net amounts for CWT Ticketing.
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

#include "DataModel/NetFarePath.h"

#include "Common/CurrencyUtil.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/NegFareRest.h"

namespace tse
{

class CalcNUCAmountNet
{
public:
  CalcNUCAmountNet(NetFarePath& fp) : _fp(fp) {}
  void operator()(PricingUnit* pu);

protected:
  NetFarePath& _fp;
};

class CloneAndAddPricingUnitNet
{
public:
  CloneAndAddPricingUnitNet(PricingTrx* trx,
                            DataHandle* dataHandle,
                            NetFarePath& fp,
                            std::vector<PricingUnit*>& pricingUnits)
    : _trx(trx),
      _dataHandle(dataHandle),
      _fp(fp),
      _pricingUnits(pricingUnits),
      _itin(*(fp.itin()))
  {
  }
  void operator()(PricingUnit* pu);

protected:
  PricingTrx* _trx;
  DataHandle* _dataHandle;
  NetFarePath& _fp;
  std::vector<PricingUnit*>& _pricingUnits;
  Itin& _itin;
};

class CloneAndAddFareUsageNet
{
public:
  CloneAndAddFareUsageNet(PricingTrx* trx,
                          DataHandle* dataHandle,
                          NetFarePath& fp,
                          PricingUnit& pu)
    : _trx(trx),
      _dataHandle(dataHandle),
      _fp(fp),
      _pu(pu),
      _itin(*(fp.itin()))
  {
  }
  void operator()(FareUsage* pu);

protected:
  PaxTypeFare* createFareForNetAmount(PaxTypeFare* ptf);

  PricingTrx* _trx;
  DataHandle* _dataHandle;
  NetFarePath& _fp;
  PricingUnit& _pu;
  Itin& _itin;
};

void
NetFarePath::initialize(PricingTrx* trx, FarePath* origFp)
{
  if (trx == nullptr || origFp == nullptr)
    return;
  _trx = trx;
  _dataHandle = &trx->dataHandle();
  _origFp = origFp;

  // Only copy information needed for Tax Processing and FareCalc
  // This fare path does not go through Pricing again.
  _itin = origFp->itin();
  _paxType = origFp->paxType();
  _intlSaleIndicator = origFp->intlSaleIndicator();
  this->baseFareCurrency() = origFp->baseFareCurrency();
  this->calculationCurrency() = origFp->calculationCurrency();
  this->processed() = true;

  CloneAndAddPricingUnitNet cloneAddPu(_trx, _dataHandle, *this, _pricingUnit);
  std::for_each(origFp->pricingUnit().begin(), origFp->pricingUnit().end(), cloneAddPu);

  // Now calculate new totalNUCAmount from new fare path
  CalcNUCAmountNet calcAmt(*this);
  std::for_each(_pricingUnit.begin(), _pricingUnit.end(), calcAmt);
}

void
CalcNUCAmountNet::operator()(PricingUnit* pu)
{
  // Add up total NUC amount for FP
  _fp.increaseTotalNUCAmount(pu->getTotalPuNucAmount());

  CalcNUCAmountNet calcAmt(_fp);
  std::for_each(pu->sideTripPUs().begin(), pu->sideTripPUs().end(), calcAmt);
}

void
CloneAndAddPricingUnitNet::operator()(PricingUnit* pu)
{
  if (_dataHandle == nullptr)
    return;

  PricingUnit* newPu = nullptr;
  _dataHandle->get(newPu);
  if (newPu == nullptr)
    return;

  *newPu = *pu;
  newPu->fareUsage().clear();
  newPu->setTotalPuNucAmount(0.00);
  newPu->minFarePlusUp().clear();
  newPu->hrtojNetPlusUp() = nullptr;
  newPu->hrtcNetPlusUp() = nullptr;
  if (pu->hrtojNetPlusUp())
    newPu->minFarePlusUp().addItem(OJM, pu->hrtojNetPlusUp());
  if (pu->hrtcNetPlusUp())
    newPu->minFarePlusUp().addItem(HRT, pu->hrtcNetPlusUp());

  CloneAndAddFareUsageNet cloneAddFu(_trx, _dataHandle, _fp, *newPu);
  std::for_each(pu->fareUsage().begin(), pu->fareUsage().end(), cloneAddFu);

  if (!pu->sideTripPUs().empty())
  {
    CloneAndAddPricingUnitNet cloneAddPu(
        _trx, _dataHandle, _fp, newPu->sideTripPUs());
    std::for_each(pu->sideTripPUs().begin(), pu->sideTripPUs().end(), cloneAddPu);
  }
  _pricingUnits.push_back(newPu);
}

void
CloneAndAddFareUsageNet::operator()(FareUsage* fu)
{
  if (_dataHandle == nullptr)
    return;

  FareUsage* newFu = nullptr;
  _dataHandle->get(newFu);
  if (newFu == nullptr)
    return;
  *newFu = *fu;

  PaxTypeFare* newPtf = createFareForNetAmount(fu->paxTypeFare());
  newFu->paxTypeFare() = newPtf;

  newFu->surchargeData() = fu->surchargeData();
  newFu->transferSurcharges() = fu->transferSurcharges();
  newFu->stopoverSurcharges() = fu->stopoverSurcharges();
  newFu->minFarePlusUp().clear();
  newFu->minFarePlusUpAmt() = 0;

  newFu->netCat35NucAmount() = newPtf->nucFareAmount();
  newFu->netCat35NucUsed();
  if (!fu->sideTripPUs().empty())
  {
    CloneAndAddPricingUnitNet cloneAddPu(
        _trx, _dataHandle, _fp, newFu->sideTripPUs());
    std::for_each(fu->sideTripPUs().begin(), fu->sideTripPUs().end(), cloneAddPu);
  }
  _pu.fareUsage().push_back(newFu);
  _pu.setTotalPuNucAmount(_pu.getTotalPuNucAmount() + newFu->totalFareAmount());

  _fp.netFareMap().insert(std::pair<FareUsage*, FareUsage*>(fu, newFu));
}

PaxTypeFare*
CloneAndAddFareUsageNet::createFareForNetAmount(PaxTypeFare* ptf)
{
  PaxTypeFare* newPtf = ptf->clone(*_dataHandle);
  Fare* newFare = ptf->fare()->clone(*_dataHandle);
  newPtf->setFare(newFare);

  if (ptf->isNegotiated())
  {
    const NegPaxTypeFareRuleData* ruleData = ptf->getNegRuleData();
    if (ruleData)
    {
      if (ptf->fcaDisplayCatType() != RuleConst::SELLING_FARE)
      {
        newPtf->nucFareAmount() = ruleData->nucNetAmount();
      }
      else
      {
        const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(ruleData->ruleItemInfo());
        if (negFareRest && negFareRest->negFareCalcTblItemNo() != 0)
        {
          newPtf->nucFareAmount() = ruleData->nucNetAmount();
        }
      }
    }
  }

  if (newPtf->mileageSurchargeAmt() > 0 && newPtf->mileageSurchargePctg() > 0)
  {
    newPtf->mileageSurchargeAmt() =
        (newPtf->nucFareAmount() * newPtf->mileageSurchargePctg()) / 100.00;
    CurrencyUtil::truncateNUCAmount(newPtf->mileageSurchargeAmt());
  }
  return newPtf;
}

NetFarePath*
NetFarePath::clone(DataHandle& dataHandle) const
{
  NetFarePath* res = dataHandle.create<NetFarePath>();
  *res = *this;

  return res;
}

FareUsage*
NetFarePath::getNetFareUsage(FareUsage* origFu)
{
  std::map<FareUsage*, FareUsage*>::const_iterator i = _netFareMap.find(origFu);
  if (i != _netFareMap.end())
    return i->second;
  return nullptr;
}

} // tse
