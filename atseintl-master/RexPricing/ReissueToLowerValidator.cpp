//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "RexPricing/ReissueToLowerValidator.h"

#include "Common/DateTime.h"
#include "Common/FarePathCopier.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Fares/RoutingController.h"
#include "Pricing/Combinations.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/RuleUtil.h"

namespace tse
{
ReissueToLowerValidator::ReissueToLowerValidator(
    RexPricingTrx& trx,
    const FarePath& fp,
    Combinations& ccontroller,
    PricingUnitRuleController& rcontroller,
    std::map<FareMarket*, FareMarket*>& FM2PrevReissueFMCache)
  : _trx(trx),
    _farePath(fp),
    _combinationsController(ccontroller),
    _puRuleController(rcontroller),
    _FM2PrevReissueFMCache(FM2PrevReissueFMCache),
    _date(trx.lastTktReIssueDT() != DateTime::emptyDate() ? trx.lastTktReIssueDT()
                                                          : trx.originalTktIssueDT()),
    _flag(trx.lastTktReIssueDT() != DateTime::emptyDate() ? FareMarket::RetrievLastReissue
                                                          : FareMarket::RetrievHistorical)
{
  _combinationsController.trx() = &_trx;
  if (_trx.getTrxType() == PricingTrx::MIP_TRX)
    _itinIndex = _trx.getItinPos(_farePath.itin());
  else
    _itinIndex = 0;
}

bool
ReissueToLowerValidator::process(const ProcessTagPermutation& permutation)
{
  switch (permutation.getReissueToLowerByte())
  {
  case ProcessTagPermutation::REISSUE_TO_LOWER_BLANK:
    return true;
    break;
  case ProcessTagPermutation::REISSUE_TO_LOWER_F:
    return validationForValueF();
    break;
  case ProcessTagPermutation::REISSUE_TO_LOWER_R:
    return validationForValueR();
    break;
  }
  return false;
}

namespace
{

struct CompareFares : public std::binary_function<const PaxTypeFare*, const PaxTypeFare*, bool>
{
  CompareFares(const DateTime& date, RexPricingTrx& trx, const PaxTypeFare& ptf)
    : _date(date), _trx(trx), _ptf(ptf), _ptfFareBasis(ptf.createFareBasis(trx))
  {
  }

  bool operator()(const PaxTypeFare* curr) const
  {
    return curr->retrievalDate() == _date && curr->vendor() == _ptf.vendor() &&
           curr->carrier() == _ptf.carrier() && curr->fareTariff() == _ptf.fareTariff() &&
           curr->ruleNumber() == _ptf.ruleNumber() &&
           curr->fare()->currency() == _ptf.fare()->currency() &&
           curr->directionality() == _ptf.directionality() &&
           fabs(curr->fare()->originalFareAmount() - _ptf.fare()->originalFareAmount()) < EPSILON &&
           curr->createFareBasis(_trx) == _ptfFareBasis;
  }

private:
  const DateTime _date;
  RexPricingTrx& _trx;
  const PaxTypeFare& _ptf;
  const std::string _ptfFareBasis;
};

struct SameFareMarketWithDate
{
  SameFareMarketWithDate(FareMarket* fm, FareMarket::FareRetrievalFlags flag)
    : _compFM(fm), _flag(flag)
  {
  }

  bool operator()(FareMarket* fm) const
  {
    return _compFM->travelSeg() == fm->travelSeg() &&
           _compFM->governingCarrier() == fm->governingCarrier() && (_flag & fm->retrievalFlag());
  }

private:
  FareMarket* _compFM;
  FareMarket::FareRetrievalFlags _flag;
};
}

const FareMarket*
ReissueToLowerValidator::getFareMarket(FareMarket* fareMarket) const
{
  const FareMarket* fm = nullptr;
  const std::map<FareMarket*, FareMarket*>::const_iterator fm2fmIter =
      _FM2PrevReissueFMCache.find(fareMarket);
  if (fm2fmIter == _FM2PrevReissueFMCache.end())
  {
    std::vector<FareMarket*>::const_iterator fmIter =
        _trx.newItin()[_itinIndex]->fareMarket().begin();
    const std::vector<FareMarket*>::const_iterator fmIterEnd =
        _trx.newItin()[_itinIndex]->fareMarket().end();

    fmIter = std::find_if(fmIter, fmIterEnd, SameFareMarketWithDate(fareMarket, _flag));
    if (fmIter != fmIterEnd)
    {
      fm = *fmIter;
      _FM2PrevReissueFMCache.insert(std::make_pair(fareMarket, *fmIter));
    }
  }
  else
    fm = fm2fmIter->second;
  return fm;
}

bool
ReissueToLowerValidator::hasNonHistoricalFare(PaxTypeFareMap& faresMap) const
{
  std::vector<PricingUnit*>::const_iterator pu = _farePath.pricingUnit().begin();
  for (; pu != _farePath.pricingUnit().end(); ++pu)
  {
    std::vector<FareUsage*>::const_iterator fu = (*pu)->fareUsage().begin();
    for (; fu != (*pu)->fareUsage().end(); ++fu)
    {
      PaxTypeFare* ptf = (*fu)->paxTypeFare();

      const FareMarket* fm = getFareMarket(ptf->fareMarket());
      if (!fm)
        return true;

      const std::vector<PaxTypeFare*>& vec = fm->paxTypeCortege(_trx.paxType()[0])->paxTypeFare();
      std::vector<PaxTypeFare*>::const_iterator c =
          std::find_if(vec.begin(), vec.end(), CompareFares(_date, _trx, *ptf));
      if (c == vec.end())
        return true;
      else
        faresMap[ptf] = *c;
    }
  }
  return false;
}

bool
ReissueToLowerValidator::validationForValueF()
{
  PaxTypeFareMap faresMap;
  return hasNonHistoricalFare(faresMap);
}

namespace
{

struct RecalcPUTotalNUCAmount
{
  RecalcPUTotalNUCAmount(PricingUnit& pu) : _pu(pu) { _pu.setTotalPuNucAmount(0.0); }

  void operator()(FareUsage* fu)
  {
    fu->surchargeAmt() = fu->transferAmt() = fu->stopOverAmt() = 0.0;
    fu->stopovers().clear();
    fu->transfers().clear();
    fu->stopoverSurcharges().clear();
    fu->transferSurcharges().clear();
    fu->surchargeData().clear();

    _pu.setTotalPuNucAmount(_pu.getTotalPuNucAmount() + fu->paxTypeFare()->totalFareAmount());
  }

protected:
  PricingUnit& _pu;
};

struct RecalcFPTotalNUCAmount
{
  RecalcFPTotalNUCAmount(FarePath& fPath) : _fPath(fPath) {}

  void operator()(PricingUnit* pu)
  {
    std::for_each(pu->fareUsage().begin(), pu->fareUsage().end(), RecalcPUTotalNUCAmount(*pu));
    _fPath.increaseTotalNUCAmount(pu->getTotalPuNucAmount());

    std::for_each(
        pu->sideTripPUs().begin(), pu->sideTripPUs().end(), RecalcFPTotalNUCAmount(_fPath));
  }

protected:
  FarePath& _fPath;
};

class DateStorage
{
public:
  DateStorage(RexPricingTrx& trx, const DateTime& date)
    : _trx(trx), _faDT(_trx.fareApplicationDT()), _dhDT(_trx.dataHandle().ticketDate())
  {
    _trx.setFareApplicationDT(date);
    _trx.dataHandle().setTicketDate(date);
  }
  ~DateStorage()
  {
    _trx.setFareApplicationDT(_faDT);
    _trx.dataHandle().setTicketDate(_dhDT);
  }

private:
  RexPricingTrx& _trx;
  DateTime _faDT, _dhDT;
};
}

bool
ReissueToLowerValidator::validationForValueR()
{
  PaxTypeFareMap faresMap;
  if (hasNonHistoricalFare(faresMap))
    return true;

  if (checkFCLevelValidation(faresMap))
    return true;

  DateStorage ds(_trx, _date);

  FarePath* fPath = FarePathCopier(_trx.dataHandle()).getDuplicate(_farePath);

  exchangePaxTypeFares(*fPath, faresMap);
  recalculateTotalAmount(*fPath);

  if (checkCombinations(*fPath))
    return true;

  if (checkPULevelValidation(*fPath))
    return true;

  RuleUtil::getSurcharges(_trx, *fPath);

  if (fabs(fPath->getTotalNUCAmount() - _farePath.getTotalNUCAmount()) > EPSILON)
    return true;

  return false;
}

void
ReissueToLowerValidator::exchangePaxTypeFares(FarePath& fPath, PaxTypeFareMap& faresMap) const
{
  std::vector<PricingUnit*>::const_iterator pu = fPath.pricingUnit().begin();
  for (; pu != fPath.pricingUnit().end(); ++pu)
  {
    std::vector<FareUsage*>::const_iterator fu = (*pu)->fareUsage().begin();
    for (; fu != (*pu)->fareUsage().end(); ++fu)
    {
      PaxTypeFare* ptf = faresMap[(*fu)->paxTypeFare()];
      (*fu)->paxTypeFare() = ptf;
      bool locSwap = false;
      (*fu)->rec2Cat10() = ptf->rec2Cat10() =
          RuleUtil::getCombinabilityRuleInfo(_trx, *ptf, locSwap);
    }
  }
}

void
ReissueToLowerValidator::recalculateTotalAmount(FarePath& fPath) const
{
  fPath.setTotalNUCAmount(0);
  fPath.plusUpAmount() = 0.0;

  std::for_each(
      fPath.pricingUnit().begin(), fPath.pricingUnit().end(), RecalcFPTotalNUCAmount(fPath));
}

bool
ReissueToLowerValidator::checkFCLevelValidation(const PaxTypeFareMap& faresMap) const
{
  PaxTypeFareMap::const_iterator i = faresMap.begin();
  for (; i != faresMap.end(); ++i)
  {
    const PaxTypeFare& ptf = *i->second;
    if (!ptf.areAllCategoryValid() || !ptf.isRoutingValid())
      return true;
  }
  return false;
}

bool
ReissueToLowerValidator::checkPULevelValidation(FarePath& fPath) const
{
  _puRuleController.setPhase(PURuleValidation);
  std::vector<PricingUnit*>::iterator pu = fPath.pricingUnit().begin();
  for (; pu != fPath.pricingUnit().end(); ++pu)
    if (!_puRuleController.validate(_trx, fPath, **pu))
      return true;

  _puRuleController.setPhase(FPRuleValidation);
  pu = fPath.pricingUnit().begin();
  for (; pu != fPath.pricingUnit().end(); ++pu)
    if (!_puRuleController.validate(_trx, fPath, **pu))
      return true;

  return false;
}

bool
ReissueToLowerValidator::checkCombinations(FarePath& fPath) const
{
  FareUsage* fFU;
  FareUsage* fTargetFU;
  DiagCollector diag;
  CombinabilityScoreboard comboScoreboard;
  comboScoreboard.trx() = &_trx;

  _combinationsController.comboScoreboard() = &comboScoreboard;

  std::vector<PricingUnit*>::iterator pu = fPath.pricingUnit().begin();
  for (; pu != fPath.pricingUnit().end(); ++pu)
    if (_combinationsController.process(**pu, fFU, fTargetFU, diag, fPath.itin()) != CVR_PASSED)
      return true;

  if (_combinationsController.process(fPath, 0, fFU, fTargetFU, diag) != CVR_PASSED)
    return true;

  return false;
}

} // end of tse
