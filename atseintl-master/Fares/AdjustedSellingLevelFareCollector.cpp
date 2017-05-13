//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Common/CarrierUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareFocusCarrierInfo.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/FareFocusSecurityDetailInfo.h"
#include "DBAccess/FareFocusRuleCodeInfo.h"
#include "DBAccess/FareRetailerCalcInfo.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag868Collector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Fares/FareRetailerRuleValidator.h"
#include "Fares/AdjustedFareCalc.h"
#include "Rules/RuleUtil.h"

#include "Fares/AdjustedSellingLevelFareCollector.h"

namespace tse
{

Logger
AdjustedSellingLevelFareCollector::_logger("atseintl.Fares.AdjustedSellingLevelFareCollector");

AdjustedSellingLevelFareCollector::AdjustedSellingLevelFareCollector(PricingTrx& trx)
  : _trx(trx),
    _frrv(trx)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic868)
  {
    _diag868 = dynamic_cast<Diag868Collector*>(DCFactory::instance()->create(_trx));
    if (_diag868 != nullptr)
      _diag868->activate();
  }
}

void
AdjustedSellingLevelFareCollector::process()
{
  _frrv.getFRRLookupAllSources(_frrlV, 0, "RS", 'S', true);

  if (_frrlV.empty())
    return;

  for (Itin* itin :  _trx.itin())
    for (FareMarket* fm : itin->fareMarket())
      processFareMarket(fm, itin);
}

void
AdjustedSellingLevelFareCollector::processFareMarket(FareMarket* fm, Itin* itin)
{
  _fdTrx = dynamic_cast<FareDisplayTrx*>(&_trx);
  bool isFareDisplay = _fdTrx;

  if (isFareDisplay)
    _fareDisplayPtfVec.clear();

  for (PaxTypeFare* ptf : fm->allPaxTypeFare())
  {
    if (checkPtfValidity(ptf, isFareDisplay))
    {
      std::vector<FareRetailerRuleContext> frrcV;
      _frrv.validateFRR(*ptf, fm, 'S', _frrlV, frrcV);

      // Process frrcV on the ptf
      if (!frrcV.empty())
        processFareRetailerRules(fm, ptf, frrcV, itin, isFareDisplay);
    }
  }

  if (isFareDisplay)
    updateFareMarket(fm);
}

bool
AdjustedSellingLevelFareCollector::checkPtfValidity(const PaxTypeFare* ptf,
                                                    const bool& isFareDisplay)
{
  bool rc = false;

  if (!ptf || (ptf->isValid()==false && ptf->validForCmdPricing(false)==false))
    return rc;

  if (isFareDisplay)
  {
    Indicator c35type = ptf->fareDisplayCat35Type();
    Indicator dispCatType = ptf->fcaDisplayCatType();

    if ((!ptf->matchedFareFocus())
        && ((c35type==RuleConst::SELLING_CARRIER_FARE) ||
            (c35type==RuleConst::SELLING_MARKUP_FARE)  ||
            (!ptf->isNegotiated() &&
              !(dispCatType==RuleConst::SELLING_FARE ||
                dispCatType==RuleConst::NET_SUBMIT_FARE ||
                dispCatType==RuleConst::NET_SUBMIT_FARE_UPD))))
      rc =  true;

  }
  else // for pricing
  {
    if (!ptf->isMatchFareFocusRule())
      rc = true;
  }

  return rc;
}

void
AdjustedSellingLevelFareCollector::processFareRetailerRules(FareMarket* fm,
                            PaxTypeFare* ptf,
                           const std::vector<FareRetailerRuleContext>& frrcV,
                                                                  Itin* itin,
                                                          bool isFareDisplay)
{
  AdjustedFareCalc calcObj(_trx, *itin);
  MoneyAmount minAmt(0.0), minAmtNuc(0.0), currAmt(0.0), currAmtNuc(0.0);
  bool isNewMinAmt = false;
  AdjustedSellingCalcData* calcData = nullptr;

  _trx.dataHandle().get(calcData);

  for (const FareRetailerRuleContext& context : frrcV)
  {
    for (const FareRetailerCalcInfo* calcInfo : context._frciV)
    {
      for (const FareRetailerCalcDetailInfo* calcDetail : calcInfo->details())
      {
        if (isSuitableCalcDetail(calcDetail))
        {
          calcObj.load(*calcDetail);

          calcObj.getPrice(_trx, *ptf, currAmt, currAmtNuc);
          if (currAmt < 0 || currAmtNuc < 0)
          {
            currAmt = 0;
            currAmtNuc = 0;
          }

          if (needPrintDiagnostic(fm, *ptf))
          {
            _diag868->printCalculation(_trx, ptf, calcObj, calcDetail,
                                        currAmt, currAmtNuc, context);
          }

          if (keepFareRetailerMinAmt(ptf, minAmt, minAmtNuc, currAmt, currAmtNuc,
                                     calcObj, calcData, context, calcDetail))
          {
            isNewMinAmt = true;
          }

          if (isFareDisplay)
          {
            createFareDisplayFares(fm, ptf, calcData, currAmt, currAmtNuc);
            _trx.dataHandle().get(calcData);
          }
        }
      }
    }
  }

  if (isFareDisplay)
  {
    ptf->setAdjustedSellingBaseFare();
    if (_trx.getOptions() && !_trx.getOptions()->isPDOForFRRule()) // check requested ORD tag
      ptf->invalidateFare(PaxTypeFare::FD_OriginalAdjusted_Fare);
  }
  else if (isNewMinAmt)
  {
    ptf->setAdjustedSellingCalcData(calcData);
    if (needPrintDiagnostic(fm, *ptf))
    {
      _diag868->printMinimumAmount(&_trx, ptf, calcData);
    }
  }

  if (_diag868 != nullptr)
    _diag868->flushMsg();
}

bool
AdjustedSellingLevelFareCollector::isSuitableCalcDetail(const FareRetailerCalcDetailInfo* calcDetail)
{
  // Not sure if we want to use every calc detail, so
  // may be some condition check is needed
  return (calcDetail->calculationTypeCd()=="SL");
}

bool
AdjustedSellingLevelFareCollector::keepFareRetailerMinAmt(PaxTypeFare* ptf,
                                                          MoneyAmount& minAmt,
                                                          MoneyAmount& minAmtNuc,
                                                          MoneyAmount  currAmt,
                                                          MoneyAmount  currAmtNuc,
                                                          AdjustedFareCalc& calcObj,
                                                          AdjustedSellingCalcData* calcData,
                                                          const FareRetailerRuleContext& context,
                                                          const FareRetailerCalcDetailInfo* calcDetail
                                                         )
{
  bool rc = false;

  if (_fdTrx != nullptr || (currAmt<minAmt && currAmtNuc<minAmtNuc) || (minAmt == 0.0))
  {
    minAmt = currAmt;
    minAmtNuc = currAmtNuc;

    getSellingInfo(calcData, calcObj);
    calcData->setCalcInd(calcDetail->fareCalcInd());
    calcData->setCalculatedAmt(minAmt);
    calcData->setCalculatedNucAmt(minAmtNuc);
    calcData->setFareRetailerRuleInfo(context._frri);
    calcData->setFareRetailerRuleId((context._frri)->fareRetailerRuleId());
    calcData->setFareRetailerRuleSeqNo((context._frri)->ruleSeqNo());
    calcData->setSourcePcc(context._sourcePcc);
    calcData->setMarkupAdjAmt(currAmt - ptf->fareAmount());

    rc = true;
  }

  return rc;
}

void
AdjustedSellingLevelFareCollector::createFareDisplayFares(FareMarket* fm,
                                                          PaxTypeFare* ptf,
                                                          AdjustedSellingCalcData* calcData,
                                                          MoneyAmount fareAmount,
                                                          MoneyAmount fareAmountNuc)
{
  PaxTypeFare* newPTFare = ptf->clone(_trx.dataHandle());

  // newPTFare->status().set();
  newPTFare->fareClassAppInfo() = ptf->fareClassAppInfo();

  Fare* newFare = ptf->fare()->clone(_trx.dataHandle());
  FareInfo* fareInfo = ptf->fare()->fareInfo()->clone(_trx.dataHandle());

  newFare->setFareInfo(fareInfo);
  newPTFare->setFare(newFare);

  newPTFare->setAdjustedSellingCalcData(calcData);

  // set up the amount
  fareInfo->originalFareAmount() = (ptf->isRoundTrip()) ? fareAmount * 2.0 : fareAmount;
  fareInfo->fareAmount() = fareAmount;
  newPTFare->nucFareAmount() = fareAmountNuc;

  if (_trx.isExchangeTrx() && _trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const RexBaseTrx& rexTrx = static_cast<const RexBaseTrx&>(_trx);

    if (!rexTrx.newItinSecondROEConversionDate().isEmptyDate())
    {
      newPTFare->rexSecondNucFareAmount() =
        CurrencyUtil::getMoneyAmountInNUC(fareAmount,
                                          ptf->currency(),
                                          rexTrx.newItinSecondROEConversionDate(),
                                          _trx);
    }
  }

  if (fabs(ptf->nucFareAmount() - fareAmountNuc) > EPSILON) // the amount is different
  {
    newPTFare->nucOriginalFareAmount() =
        (ptf->isRoundTrip()) ? fareAmountNuc * 2.0 : fareAmountNuc;
  }

  if (FareDisplayUtil::initFareDisplayInfo(_fdTrx, *newPTFare))
  {
    (ptf->fareDisplayInfo())->clone(newPTFare->fareDisplayInfo(), newPTFare);
  }

  newPTFare->setAdjustedSellingCalcData(calcData);
  _fareDisplayPtfVec.push_back(newPTFare);

  if (needPrintDiagnostic(fm, *ptf))
  {
    _diag868->printFareCreation(&_trx, newPTFare);
  }

}

void
AdjustedSellingLevelFareCollector::getSellingInfo(AdjustedSellingCalcData* calcData,
                                                  AdjustedFareCalc& calcObj)
{
  Percent percent = 0.0;
  MoneyAmount amount = 0.0;
  int noDecAmt = 0;
  int noDecPercent = 0;

  calcObj.getSellingInfo(percent, amount, noDecAmt, noDecPercent);
  calcData->setPercent(percent);
  calcData->setRuleAmount(amount);
  calcData->setNoDecPercent(noDecPercent);
  calcData->setNoDecAmt(noDecAmt);
  CurrencyCode& curr = calcObj.getCurrencyCode();
  calcData->setCalculatedASLCurrency(curr);
}

void
AdjustedSellingLevelFareCollector::updateFareMarket(FareMarket* fm)
{
  std::vector<PaxTypeFare*>& allFares = fm->allPaxTypeFare();
  std::vector<PaxTypeBucket>& ptCorteges = fm->paxTypeCortege();
  bool isCmdPriced = (_trx.billing() && _trx.billing()->partitionID() == "WN") ?
              (!fm->fareBasisCode().empty()) : (_trx.getOptions()->fbcSelected());

  if (!_fareDisplayPtfVec.empty())
  {
    auto newCapacityNeeded = allFares.size() + _fareDisplayPtfVec.size();
    if (newCapacityNeeded >= allFares.capacity())
      allFares.reserve(newCapacityNeeded);

    for (PaxTypeFare* ptFare : _fareDisplayPtfVec)
    {
      allFares.push_back(ptFare);

      // Add the PTFs to the cortege
      for (PaxTypeBucket& ptCortege : ptCorteges)
      {
        if (!checkAge(*ptFare, *ptCortege.requestedPaxType()))
          continue;

        if (find_if(ptCortege.actualPaxType().begin(),
                    ptCortege.actualPaxType().end(),
                    PaxTypeFinder(*ptFare, isCmdPriced)) != ptCortege.actualPaxType().end())
        {
          if (ptFare->actualPaxType()->paxType() != ptCortege.actualPaxType().front()->paxType())
          {
            continue;
          }

          ptCortege.paxTypeFare().push_back(ptFare);
        }
      }
    }
  }
}

bool
AdjustedSellingLevelFareCollector::checkAge(const PaxTypeFare& ptFare, const PaxType& paxType) const
{
  const uint16_t requestPaxAge = paxType.age();

  if (requestPaxAge == 0)
    return true;

  const uint16_t minAge = ptFare.fcasMinAge();
  if (minAge > 0)
  {
    if (UNLIKELY(minAge > requestPaxAge))
      return false;
  }
  const uint16_t maxAge = ptFare.fcasMaxAge();
  if (maxAge > 0)
  {
    if (maxAge < requestPaxAge)
      return false;
  }

  return true;
}
} //tse
