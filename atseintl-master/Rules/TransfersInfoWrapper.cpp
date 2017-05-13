//-------------------------------------------------------------------
//
//  File:        TransfersInfoWrapper1.cpp
//  Created:     May 24, 2006
//  Authors:     Andrew Ahmad
//
//  Description: Version 2 of Transfers processing code. This
//               second version is required for processing the
//               new Cat-09 record format as mandated by ATPCO.
//
//
//  Copyright Sabre 2004-2006
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

#include "Rules/TransfersInfoWrapper.h"

#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VCTR.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleItem.h"
#include "Rules/RuleItemCaller.h"
#include "Rules/RuleSetPreprocessor.h"
#include "Rules/Transfers1.h"
#include "Rules/TransfersTravelSegWrapper.h"
#include "Util/BranchPrediction.h"

#include <map>
#include <string>

namespace tse
{
FALLBACK_DECL(cat9FixMaxTransfers2)

namespace
{
uint16_t parseNoTransfers(const std::string& str)
{
  return (str.empty() || str == "XX") ? RuleConst::MAX_NUMBER_XX : std::stoi(str);
}
}

Logger
TransfersInfoWrapper::_logger("atseintl.Rules.TransfersInfoWrapper");

bool
TransfersInfoWrapper::transferFCscopeReal(const PricingTrx& trx) const
{
  // This function should return value indicating if fare component has fare component validation
  // or not, as determined by RuleSetPreprocesor or as taken from the maxTransfers field of
  // Cat9 R3 data.
  // Unfortunately transferFCscope() cannot be used for this purpose, as it is modified to `true'
  // based on some other conditions, e.g. when R3 contains FC-validated recurring segment.
  return transferFCscopeInitial() || (_trInfo && _trInfo->noTransfersMax().empty());
}

void
TransfersInfoWrapper::setCurrentTrInfo(const TransfersInfo1* trInfo)
{
  if (UNLIKELY(!trInfo))
    return;

  _trInfo = trInfo;
  copyFrom(*_trInfo);

  const std::string& charge1MaxStr = _trInfo->maxNoTransfersCharge1();
  const std::string& charge2MaxStr = _trInfo->maxNoTransfersCharge2();

  _charge1Unlimited =
      (charge1MaxStr == Transfers1::NUM_TRANSFERS_UNLIMITED) || (charge1MaxStr.empty());

  _charge2Unlimited =
      (charge2MaxStr == Transfers1::NUM_TRANSFERS_UNLIMITED) || (charge2MaxStr.empty());

  if (!_charge1Unlimited)
  {
    _charge1Max = atoi(charge1MaxStr.c_str());
  }
  else
  {
    _charge1Max = 0;
  }

  if (!_charge2Unlimited)
  {
    _charge2Max = atoi(charge2MaxStr.c_str());
  }
  else
  {
    _charge2Max = 0;
  }

  _charge1Count = 0;
  _charge2Count = 0;

  for (const Surcharge& surcharge : _surcharges)
  {
    if (surcharge.matchRuleItemNo != _trInfo->itemNo())
      continue;
    if (surcharge.isCharge1)
      ++_charge1Count;
    else
      ++_charge2Count;
  }

  _noTransfersMax = parseNoTransfers(trInfo->noTransfersMax());
}

void
TransfersInfoWrapper::setCurrentTrInfo(const TransfersInfo1* ti,
                                       const RuleItemCaller& ruleItemCaller,
                                       bool isLocSwapped)
{
  setCurrentTrInfo(ti);

  _isRelationAndExists = false;
  _isRelationOrExists = false;
  detectRelationIndicators(ruleItemCaller.r2Segments());

  if (_isRelationAndExists)
    calcNoTransfersForThenAnd(ruleItemCaller, isLocSwapped);
}

void
TransfersInfoWrapper::crInfo(const CategoryRuleInfo* crInfo)
{
  _crInfo = crInfo;

  if (LIKELY(crInfo))
  {
    _vctr.vendor() = crInfo->vendorCode();
    _vctr.carrier() = crInfo->carrierCode();
    _vctr.tariff() = crInfo->tariffNumber();
    _vctr.rule() = crInfo->ruleNumber();
    _vctr.sequenceNumber() = crInfo->sequenceNumber();
  }
  else
  {
    _vctr.clear();
  }
}

void
TransfersInfoWrapper::copyFrom(const RuleItemInfo& ruleItemInfo)
{
  vendor() = ruleItemInfo.vendor();
  itemNo() = ruleItemInfo.itemNo();
  textTblItemNo() = ruleItemInfo.textTblItemNo();
  overrideDateTblItemNo() = ruleItemInfo.overrideDateTblItemNo();
}

bool
TransfersInfoWrapper::checkAllPassed()
{
  if (ignoreNoMatch())
    return true;

  bool applyLeastRestrictive = applyLeastRestrictiveProvision();
  bool numTransfersUnlimited = leastRestrictiveTransfersUnlimited();
  int16_t numTransfersMax = leastRestrictiveTransfersPermitted();

  int16_t transferCount = 0;

  for (const auto& ttswI : _transfersTravelSegWrappers)
  {
    const TransfersTravelSegWrapper& ttsw = ttswI.second;

    if (UNLIKELY(!ttsw.isTransfer()))
      continue;

    if (!ttsw.passedValidation())
    {
      if (ttsw.fareUsage() != _fareUsage && !ttsw.ruleItemMatch())
        continue;

      if (LIKELY(!ttsw.noMatchValidation()))
        return false;
    }

    if (UNLIKELY(!applyLeastRestrictive && !ttsw.ruleItemMatch()))
    {
      return false;
    }
    if (applyLeastRestrictive)
    {
      if (UNLIKELY(!numTransfersUnlimited))
      {
        ++transferCount;
        if (transferCount > numTransfersMax)
        {
          return false;
        }
      }
    }
  }
  return true;
}

void
TransfersInfoWrapper::setIsTransfer(TravelSeg* ts, FareUsage* fu, bool entireRule) const
{
  if (UNLIKELY(!ts))
    return;

  TransfersTravelSegWrapper& ttsw = getOrCreateTsWrapper(ts, fu);
  ttsw.isTransfer() = true;
  ttsw.validateEntireRule() = entireRule;
  _needToProcessResults = true;
}

bool
TransfersInfoWrapper::checkIsTransfer(TravelSeg* ts) const
{
  if (UNLIKELY(!ts))
    return false;

  const auto ttswI = _transfersTravelSegWrappers.find(ts);

  if (ttswI == _transfersTravelSegWrappers.end())
    return false;

  const TransfersTravelSegWrapper& ttsw = ttswI->second;
  return ttsw.isTransfer();
}

void
TransfersInfoWrapper::setPassedValidation(TravelSeg* ts,
                                          FareUsage* fu,
                                          bool passedByLeastRestrictive,
                                          bool isRecurringFCScope) const
{
  if (UNLIKELY(!ts))
    return;

  TransfersTravelSegWrapper& ttsw = getOrCreateTsWrapper(ts, fu);
  ttsw.passedValidation() = true;
  ttsw.passedByLeastRestrictive() = passedByLeastRestrictive;
  ttsw.isRecurringFCScope() = isRecurringFCScope;
  _needToProcessResults = true;
}

bool
TransfersInfoWrapper::checkPassedValidation(TravelSeg* ts) const
{
  if (UNLIKELY(!ts))
    return false;

  const auto ttswI = _transfersTravelSegWrappers.find(ts);

  if (UNLIKELY(ttswI == _transfersTravelSegWrappers.end()))
    return false;

  TransfersTravelSegWrapper& ttsw = ttswI->second;
  return ttsw.passedValidation();
}

void
TransfersInfoWrapper::setRuleItemMatch(TravelSeg* ts,
                                       FareUsage* fu,
                                       const uint32_t itemNo,
                                       const bool isTentativeMatch) const
{
  if (UNLIKELY(!ts))
    return;

  TransfersTravelSegWrapper& ttsw = getOrCreateTsWrapper(ts, fu);

  if (UNLIKELY(ttsw.ruleItemMatch()))
  {
    if (ttsw.isTentativeMatch() && !isTentativeMatch)
    {
      ttsw.clearSurcharges();
      ttsw.ruleItemMatch() = itemNo;
      ttsw.ruleVCTRMatch() = getVCTR();
      ttsw.isFareRuleMatch() = _isFareRule;
      ttsw.isTentativeMatch() = isTentativeMatch;
      ttsw.passedValidation() = false;
    }
  }
  else
  {
    ttsw.ruleItemMatch() = itemNo;
    ttsw.ruleVCTRMatch() = getVCTR();
    ttsw.isFareRuleMatch() = _isFareRule;
    ttsw.isTentativeMatch() = isTentativeMatch;
    ttsw.passedValidation() = false;
  }

  _needToProcessResults = true;
}

uint32_t
TransfersInfoWrapper::getRuleItemMatch(const TravelSeg* ts) const
{
  bool notUsed = false;
  return getRuleItemMatch(ts, notUsed);
}

uint32_t
TransfersInfoWrapper::getRuleItemMatch(const TravelSeg* ts, bool& isTentativeMatch) const
{
  if (UNLIKELY(!ts))
    return 0;

  const auto ttswI = _transfersTravelSegWrappers.find(ts);

  if (ttswI == _transfersTravelSegWrappers.end())
    return 0;

  TransfersTravelSegWrapper& ttsw = ttswI->second;
  isTentativeMatch = ttsw.isTentativeMatch();
  return ttsw.ruleItemMatch();
}

bool
TransfersInfoWrapper::isSameVCTR(const FareUsage& fu) const
{
  if (_isFareRule)
    return (getVCTR() == fu.transfersMatchingVCTR());

  return (getVCTR() == fu.transfersMatchingGeneralRuleVCTR());
}

const VCTR&
TransfersInfoWrapper::getVCTR() const
{
  return _vctr;
}

bool
TransfersInfoWrapper::hasPricingUnitScope(const FareUsage* fu) const
{
  return _fareUsageWithPricingUnitScope.find(fu) != _fareUsageWithPricingUnitScope.end();
}

void
TransfersInfoWrapper::doRuleSetPreprocessing(PricingTrx& trx,
                                             const RuleSetPreprocessor& rsp,
                                             const PricingUnit& pu)
{
  applyLeastRestrictiveProvision() = rsp.applyLeastRestrictiveTransfers();

  leastRestrictiveTransfersUnlimited() = rsp.leastRestrictiveTransfersUnlimited();

  leastRestrictiveTransfersPermitted() = rsp.leastRestrictiveTransfersPermitted();

  transferFCscopeInitial() = rsp.transferFCscope();
  transferFailsPUInitial() = rsp.transferFailPU();
  transferFCscope() = rsp.transferFCscope();
  transferFailsPU() = rsp.transferFailPU();

  crInfo(rsp.categoryRuleInfo());

  detectRelationIndicators();

  for (FareUsage* fu : pu.fareUsage())
  {
    if (!rsp.hasPricingUnitScope(RuleConst::TRANSFER_RULE, fu))
      continue;

    _fareUsageWithPricingUnitScope.insert(fu);

    FareMarket* fm = nullptr;
    PaxTypeFare* ptf = fu->paxTypeFare();

    if (LIKELY(ptf))
      fm = ptf->fareMarket();

    if (fm && (fm->governingCarrier() == crInfo()->carrierCode()))
    {
      for (const auto& fuSurcharge : fu->transferSurcharges())
      {
        const FareUsage::TransferSurcharge* trs = fuSurcharge.second;
        _surcharges.push_back(Surcharge{trs->matchRuleItemNo(), trs->isCharge1()});
      }
    }
  }
}

void
TransfersInfoWrapper::detectRelationIndicators()
{
  if (UNLIKELY(!_crInfo))
    return;

  for (const auto& criiSetPtr: _crInfo->categoryRuleItemInfoSet())
    detectRelationIndicators(*criiSetPtr);
}

void
TransfersInfoWrapper::detectRelationIndicators(const CategoryRuleItemInfoSet& set)
{
  for (const CategoryRuleItemInfo& r2s : set)
  {
    const Indicator rel = r2s.relationalInd();

    if (rel == CategoryRuleItemInfo::IF)
      break;

    if (rel == CategoryRuleItemInfo::AND)
    {
      _isRelationAndExists = true;
      break;
    }

    if (rel == CategoryRuleItemInfo::OR)
    {
      _isRelationOrExists = true;
      break;
    }
  }
}

void
TransfersInfoWrapper::calcNoTransfersForThenAnd(const RuleItemCaller& ruleItemCaller,
                                                bool isLocSwapped)
{
  // Per ATPCO when THEN/AND subset is being processed a sum of MAX/MIN/OUT/IN values for each
  // valid table within the subset should be calculated and used during the validation.
  // I believe currently we don't apply this rule correctly so I'm going to start with fixing
  // MAX value (which is the most important) and proceed with others when we have more time
  // for testing.

  DataHandle& dh = ruleItemCaller.trx().dataHandle();
  uint16_t noTransfersMax = 0;

  for (const CategoryRuleItemInfo& r2s : ruleItemCaller.r2Segments())
  {
    if (r2s.relationalInd() == CategoryRuleItemInfo::IF)
      break;

    if (r2s.itemcat() != RuleConst::TRANSFER_RULE)
      continue;

    const TransfersInfo1* const r3 =
        dynamic_cast<const TransfersInfo1*>(dh.getRuleItemInfo(&ruleItemCaller.record2(), &r2s));

    if (UNLIKELY(!r3))
      continue;

    // Even in case of soft pass we increase MAX not to fail valid fares too early
    if (r3->overrideDateTblItemNo() &&
        ruleItemCaller.validateT994DateOverride(r3, RuleConst::TRANSFER_RULE) == SKIP)
      continue;

    // Even in case of soft pass we increase MAX not to fail valid fares too early
    const Record3ReturnTypes rc = ruleItemCaller.isDirectionPass(&r2s, isLocSwapped);
    if (rc != PASS && rc != SOFTPASS)
      continue;

    noTransfersMax += parseNoTransfers(r3->noTransfersMax());
  }

  setNoTransfersMax(noTransfersMax);
}

bool
TransfersInfoWrapper::addSurcharge(PricingTrx& trx,
                                   TravelSeg* ts,
                                   FareUsage* fu,
                                   const PaxTypeCode& paxTypeCode,
                                   const bool isSegmentSpecific,
                                   const bool forceCharge2) const
{
  if (UNLIKELY(!ts))
    return false;

  if (UNLIKELY(!fu))
    return false;

  TransfersTravelSegWrapper& ttsw = getOrCreateTsWrapper(ts, fu);

  if (UNLIKELY(!ttsw.fareUsage()))
    ttsw.fareUsage() = fu;

  PaxTypeFare* fare = fu->paxTypeFare();

  if (UNLIKELY(!fare))
  {
    LOG4CXX_ERROR(_logger, "Rules.TransfersInfoWrapper1::addSurcharge(): PaxTypeFare = 0");
    return false;
  }

  MoneyAmount charge1Amt, charge1AmtLc;
  CurrencyCode charge1Cur, charge1CurLc;
  int16_t charge1NoDec, charge1NoDecLc;
  MoneyAmount charge2Amt, charge2AmtLc;
  CurrencyCode charge2Cur, charge2CurLc;
  int16_t charge2NoDec, charge2NoDecLc;

  if (UNLIKELY(!selectApplCharge(trx,
                        charge1AmtLc,
                        charge1CurLc,
                        charge1NoDecLc,
                        charge2AmtLc,
                        charge2CurLc,
                        charge2NoDecLc,
                        *fare,
                        paxTypeCode)))
  {
    return false;
  }

  Indicator appl = _trInfo->transfersChargeAppl();

  charge1Amt = charge1AmtLc;
  charge1Cur = charge1CurLc;
  charge1NoDec = charge1NoDecLc;
  charge2Amt = charge2AmtLc;
  charge2Cur = charge2CurLc;
  charge2NoDec = charge2NoDecLc;
  double multiplier = 0.0;

  if (UNLIKELY(!applyPaxTypeDiscount(trx,
                            charge1AmtLc,
                            charge1CurLc,
                            charge1NoDecLc,
                            charge2AmtLc,
                            charge2CurLc,
                            charge2NoDecLc,
                            *fu,
                            paxTypeCode,
                            appl,
                            multiplier)))
  {
    return false;
  }

  if (UNLIKELY(multiplier != 0.0 && (charge1Amt != charge1AmtLc)))
  {
    if (!convertDiscountToNUCamounts(trx,
                                     charge1Amt,
                                     charge1Cur,
                                     charge1NoDec,
                                     charge2Amt,
                                     charge2Cur,
                                     charge2NoDec,
                                     *fare,
                                     multiplier))
    {
      return false;
    }
  }

  if ((forceCharge2) || ((!_charge1Unlimited) && (_charge1Count == _charge1Max)))
  {
    if (_charge2Unlimited || (_charge2Count < _charge2Max))
    {
      if (!charge2Cur.empty())
      {
        ++_charge2Count;

        TransfersTravelSegWrapper::Surcharge chrg(charge2Amt,
                                                  charge2Cur,
                                                  charge2NoDec,
                                                  isSegmentSpecific,
                                                  false,
                                                  false,
                                                  charge2AmtLc,
                                                  charge2CurLc,
                                                  charge2NoDecLc);

        ttsw.surcharges().push_back(chrg);
      }
    }
  }
  else
  {
    if (!charge1Cur.empty())
    {
      ++_charge1Count;

      TransfersTravelSegWrapper::Surcharge chrg(charge1Amt,
                                                charge1Cur,
                                                charge1NoDec,
                                                isSegmentSpecific,
                                                true,
                                                true,
                                                charge1AmtLc,
                                                charge1CurLc,
                                                charge1NoDecLc);

      ttsw.surcharges().push_back(chrg);
    }
  }

  _needToProcessResults = true;
  return true;
}

bool
TransfersInfoWrapper::convertChargeToNUC(PricingTrx& trx, bool isInternational, Charge& charge)
    const
{
  if (charge.first < EPSILON && charge.second < EPSILON)
  {
    charge.assign(0.0, 0.0, NUC, 2);
    return true;
  }

  const Money source1(charge.first, charge.currency);
  Money result1(NUC), result2(NUC);
  CurrencyConversionRequest req1(result1,
                                 source1,
                                 trx.getRequest()->ticketingDT(),
                                 *trx.getRequest(),
                                 trx.dataHandle(),
                                 isInternational);

  const Money source2(charge.second, charge.currency);
  CurrencyConversionRequest req2(result2,
                                 source2,
                                 trx.getRequest()->ticketingDT(),
                                 *trx.getRequest(),
                                 trx.dataHandle(),
                                 isInternational);

  NUCCurrencyConverter nucConv;
  if (nucConv.convert(req1, nullptr) && nucConv.convert(req2, nullptr))
  {
    charge.assign(result1.value(), result2.value(), result1.code(), result1.noDec());
    return true;
  }

  LOG4CXX_ERROR(_logger,
                "Rules.TransfersInfoWrapper::convertChargeToNUC(): "
                "Error converting currency to NUC");
  return false;
}

TransfersInfoWrapper::Charge
TransfersInfoWrapper::selectApplChargeImpl(PricingTrx& trx,
                                           const PaxTypeFare& fare,
                                           const PaxTypeCode& paxTypeCode) const
{
  if (!chargeForPaxType(trx, _trInfo->transfersChargeAppl(), fare, paxTypeCode))
    return Charge(true);

  Charge charge1(
      _trInfo->charge1Cur1Amt(), _trInfo->charge2Cur1Amt(), _trInfo->cur1(), _trInfo->noDec1()),
      charge2(
          _trInfo->charge1Cur2Amt(), _trInfo->charge2Cur2Amt(), _trInfo->cur2(), _trInfo->noDec2());

  if (charge1.currency.empty() && charge2.currency.empty())
    return Charge(true);

  if (LIKELY(!charge1.currency.empty() && charge2.currency.empty()))
    return charge1;

  if (charge1.currency.empty() && !charge2.currency.empty())
    return charge2;

  if (fare.currency() == charge1.currency)
    return charge1;

  if (fare.currency() == charge2.currency)
    return charge2;

  // Neither currency matches the fare, so convert both currencies
  //  to NUC and use the currency with the lowest surcharge amount.

  if (!convertChargeToNUC(trx, fare.isInternational(), charge1) ||
      !convertChargeToNUC(trx, fare.isInternational(), charge2))
    return Charge(false);

  if (charge1.first < charge2.first)
    return charge1;

  if (charge2.first < charge1.first)
    return charge2;

  if (charge1.second < charge2.second)
    return charge1;

  if (charge2.second < charge1.second)
    return charge2;

  return charge1;
}

bool
TransfersInfoWrapper::selectApplCharge(PricingTrx& trx,
                                       MoneyAmount& charge1Amt,
                                       CurrencyCode& charge1Cur,
                                       int16_t& charge1NoDec,
                                       MoneyAmount& charge2Amt,
                                       CurrencyCode& charge2Cur,
                                       int16_t& charge2NoDec,
                                       const PaxTypeFare& fare,
                                       const PaxTypeCode& paxTypeCode) const
{
  Charge charge = selectApplChargeImpl(trx, fare, paxTypeCode);
  charge1Amt = charge.first;
  charge2Amt = charge.second;
  charge1Cur = charge2Cur = charge.currency;
  charge1NoDec = charge2NoDec = charge.noDec;
  return charge.status;
}

bool
TransfersInfoWrapper::applyPaxTypeDiscount(PricingTrx& trx,
                                           MoneyAmount& charge1Amt,
                                           CurrencyCode& charge1Cur,
                                           int16_t& charge1NoDec,
                                           MoneyAmount& charge2Amt,
                                           CurrencyCode& charge2Cur,
                                           int16_t& charge2NoDec,
                                           const FareUsage& fareUsage,
                                           const PaxTypeCode& paxTypeCode,
                                           const Indicator appl,
                                           double& multiplier) const
{
  const PaxTypeFare* fare;
  if (fareUsage.paxTypeFare()->isDiscounted())
  {
    fare = fareUsage.paxTypeFare();
  }
  else if (fareUsage.cat25Fare() && // Access to Cat 25 fare or Cat 19/25 fare
           fareUsage.cat25Fare()->isDiscounted())
  {
    fare = fareUsage.cat25Fare();
  }
  else
    return true;

  if (UNLIKELY(PaxTypeUtil::isInfant(trx, paxTypeCode, fare->vendor()) &&
      (appl == RuleConst::CHARGE_PAX_ADULT_CHILD_INFANT_FREE ||
       appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE)))
  {
    charge1Amt = 0;
    charge2Amt = 0;
    return true;
  }

  if (fare->discountInfo().farecalcInd() != RuleConst::CALCULATED)
    return true;

  MoneyAmount discPercent = fare->discountInfo().discPercent();

  // discPercent is not the discount percentage, it is the percentage to charge
  /*double*/ multiplier = discPercent / 100.0;

  if (PaxTypeUtil::isChild(trx, paxTypeCode, fare->vendor()))
  {
    if (appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC ||
        appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC ||
        appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE)
    {
      charge1Amt = charge1Amt * multiplier;
      charge2Amt = charge2Amt * multiplier;
    }
  }
  else if (PaxTypeUtil::isInfant(trx, paxTypeCode, fare->vendor()))
  {
    if (appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC)
    {
      charge1Amt = charge1Amt * multiplier;
      charge2Amt = charge2Amt * multiplier;
    }
  }
  return true;
}

bool
TransfersInfoWrapper::chargeForPaxType(PricingTrx& trx,
                                       const Indicator& chargeAppl,
                                       const PaxTypeFare& fare,
                                       const PaxTypeCode& paxTypeCode) const
{
  if (fare.fareAmount() < EPSILON && PaxTypeUtil::isInfant(trx, paxTypeCode, fare.vendor()))
    return false;

  // assumption: PaxTypeUtil::isAdult --> isAdult == !isChild && !isInfant

  switch (chargeAppl)
  {
  case RuleConst::CHARGE_PAX_ANY:
    return true;

  case RuleConst::CHARGE_PAX_CHILD:
    return PaxTypeUtil::isChild(trx, paxTypeCode, fare.vendor());

  case RuleConst::CHARGE_PAX_ADULT_CHILD:
  case RuleConst::CHARGE_PAX_ADULT_CHILD_DISC:
    return !PaxTypeUtil::isInfant(trx, paxTypeCode, fare.vendor());

  case RuleConst::CHARGE_PAX_ADULT:
    return PaxTypeUtil::isAdult(trx, paxTypeCode, fare.vendor());

  case RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC:
    return true;

  case RuleConst::CHARGE_PAX_ADULT_CHILD_INFANT_FREE:
  case RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE:
    return !PaxTypeUtil::isInfant(trx, paxTypeCode, fare.vendor());

  case RuleConst::CHARGE_PAX_INFANT:
    return PaxTypeUtil::isInfant(trx, paxTypeCode, fare.vendor());

  default:
    ;
  }

  return true;
}

void
TransfersInfoWrapper::clearResults()
{
  _transfersTravelSegWrappers.clear();

  _charge1Count = 0;
  _charge2Count = 0;

  _needToProcessResults = false;
  _ignoreNoMatch = false;

  _noTransfersMax = RuleConst::MAX_NUMBER_XX;
}

Record3ReturnTypes
TransfersInfoWrapper::processResults(PricingTrx& trx, const PaxTypeFare& ptf)
{
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic309 &&
               diagPtr->parseFareMarket(trx, *(ptf.fareMarket()))))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic309);
    diagEnabled = true;
  }

  if (diagEnabled)
  {
    (*diagPtr) << "CATEGORY 09 - TRANSFERS DIAGNOSTICS" << std::endl;
    (*diagPtr) << "PHASE: FARE FINAL VALIDATION" << std::endl << " " << std::endl;

    (*diagPtr) << "FARECOMP-1 : " << std::setw(12) << std::left << ptf.fareClass() << std::right
               << "   ";

    const FareMarket* fm = ptf.fareMarket();

    (*diagPtr) << fm->getDirectionAsString();

    int16_t airSegCount = 0;

    std::vector<TravelSeg*>::const_iterator tsI = fm->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tsEnd = fm->travelSeg().end();

    for (; tsI != tsEnd; ++tsI)
    {
      if (dynamic_cast<const AirSeg*>(*tsI))
      {
        ++airSegCount;
      }
    }

    if (airSegCount > 2)
    {
      (*diagPtr) << std::setw(3) << airSegCount - 1 << " OFFPOINTS";
    }
    else if (airSegCount == 2)
    {
      (*diagPtr) << "  1 OFFPOINT ";
    }
    else
    {
      (*diagPtr) << " NO OFFPOINTS";
    }

    (*diagPtr) << " FROM " << fm->origin()->loc() << " TO " << fm->destination()->loc()
               << std::endl;

    int16_t segNumber = 1;

    for (tsI = fm->travelSeg().begin(); tsI != tsEnd; ++tsI, ++segNumber)
    {
      const TravelSeg* ts = *tsI;
      const AirSeg* as = dynamic_cast<const AirSeg*>(ts);

      if (as)
      {
        (*diagPtr) << std::setw(3) << std::right << segNumber << " " << std::setw(3) << std::left
                   << as->carrier() << std::setw(4) << std::right << as->flightNumber() << std::left
                   << " " << ts->departureDT().dateToString(DDMMM, "") << " " << ts->origin()->loc()
                   << ts->destination()->loc() << "  " << ts->departureDT().timeToString(HHMM, "")
                   << "  " << ts->arrivalDT().timeToString(HHMM, "") << "   ";

        if (needToProcessResults())
        {
          TransfersTravelSegWrapperMapCI ttswIter = _transfersTravelSegWrappers.find(ts);

          if (ttswIter != _transfersTravelSegWrappers.end())
          {
            const TransfersTravelSegWrapper& ttsw = (*ttswIter).second;

            if (ttsw.isTransfer())
            {
              (*diagPtr) << "TFR ";
            }
            else
            {
              (*diagPtr) << "    ";
            }

            if (ttsw.passedValidation())
            {
              (*diagPtr) << "PASS";
            }
            else if (!ignoreNoMatch())
            {
              (*diagPtr) << "FAIL";
            }
            else
            {
              (*diagPtr) << "----";
            }
            uint32_t ruleItem = getRuleItemMatch(ts);
            if (ruleItem)
            {
              (*diagPtr) << " " << ruleItem;
            }
            else if (ttsw.passedByLeastRestrictive())
            {
              (*diagPtr) << " LEAST REST";
            }
            else
            {
              (*diagPtr) << " NO MATCH";
            }
          }
          else
          {
            (*diagPtr) << "    PASS";
          }
        }
        (*diagPtr) << std::endl;
      }
      else
      {
        (*diagPtr) << "    ARNK" << std::endl;
      }
    }
    (*diagPtr) << " " << std::endl;
  }
  if (needToProcessResults() && (!checkAllPassed()))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "TRANSFERS : FAIL - SEE ABOVE FOR FAILED SEGMENTS" << std::endl;

      diagPtr->printLine();
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << " " << std::endl << "TRANSFERS : SOFTPASS - FINAL VALIDATION" << std::endl;

    diagPtr->printLine();
    diagPtr->flushMsg();
  }
  return SOFTPASS;
}

Record3ReturnTypes
TransfersInfoWrapper::processResults(PricingTrx& trx,
                                     FarePath& fp,
                                     const PricingUnit& pu,
                                     const FareUsage& fu,
                                     const bool processCharges)
{
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  Diagnostic& trxDiag = trx.diagnostic();

  bool diagEnabled = false;
  const FareUsage* lastFU = pu.fareUsage().back();

  noMatchValidation(pu, fu);
  bool isCmdPricing = false;
  if (UNLIKELY(((fu.paxTypeFare()->isFareByRule() || !fu.cat25Fare()) && fu.paxTypeFare()->isCmdPricing()) ||
      (fu.cat25Fare() && fu.cat25Fare()->isCmdPricing())))
  {
    isCmdPricing = true;
  }

  if (UNLIKELY(trxDiag.diagnosticType() == Diagnostic309 &&
              diagPtr->parseFareMarket(trx, *(fu.paxTypeFare()->fareMarket()))))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic309);
    diagEnabled = true;
  }

  if (diagEnabled)
  {
    const PaxTypeFare* ptf = fu.paxTypeFare();
    const FareMarket* fm = nullptr;
    if (ptf)
    {
      fm = ptf->fareMarket();
    }

    (*diagPtr) << "CATEGORY 09 - TRANSFERS DIAGNOSTICS" << std::endl;
    (*diagPtr) << "PHASE: PRICING UNIT FINAL VALIDATION" << std::endl;
    if (fm)
    {
      (*diagPtr) << fm->origin()->loc() << " " << fm->destination()->loc() << " "
                 << ptf->fareClass();
    }

    (*diagPtr) << (!transferFCscopeReal(trx) ? " PU" : " FC");

    if (fu.isInbound())
    {
      (*diagPtr) << "  .IN. ";
    }
    else
    {
      (*diagPtr) << "  .OUT.";
    }
    (*diagPtr) << std::endl << " " << std::endl;

    bool anyMaxExceed = checkMaxExceed(pu);

    std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();

    int16_t fcNum = 0;

    for (; fuI != fuEnd; ++fuI)
    {
      ++fcNum;

      const FareUsage* fareUsage = *fuI;
      const PaxTypeFare* ptf = fareUsage->paxTypeFare();

      (*diagPtr) << "FARECOMP-" << fcNum << " : " << std::setw(12) << std::left << ptf->fareClass()
                 << std::right << "   ";

      (*diagPtr) << (!transferFCscopeReal(trx) && hasPricingUnitScope(fareUsage) ? "PU " : "FC ");

      if (fareUsage->isInbound())
      {
        (*diagPtr) << ".IN.  ";
      }
      else
      {
        (*diagPtr) << ".OUT. ";
      }

      int16_t airSegCount = 0;

      std::vector<TravelSeg*>::const_iterator tsI = fareUsage->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tsEnd = fareUsage->travelSeg().end();

      for (; tsI != tsEnd; ++tsI)
      {
        if (dynamic_cast<const AirSeg*>(*tsI))
        {
          ++airSegCount;
        }
      }

      if (airSegCount > 2)
      {
        (*diagPtr) << std::setw(3) << airSegCount - 1 << " OFFPOINTS";
      }
      else if (airSegCount == 2)
      {
        (*diagPtr) << "  1 OFFPOINT ";
      }
      else
      {
        (*diagPtr) << " NO OFFPOINTS";
      }

      (*diagPtr) << " FROM " << ptf->fareMarket()->origin()->loc() << " TO "
                 << ptf->fareMarket()->destination()->loc() << std::endl;

      for (tsI = fareUsage->travelSeg().begin(); tsI != tsEnd; ++tsI)
      {
        const TravelSeg* ts = *tsI;
        const AirSeg* as = dynamic_cast<const AirSeg*>(ts);

        if (as)
        {
          (*diagPtr) << std::setw(3) << std::right << fp.itin()->segmentOrder(ts) << " "
                     << std::setw(3) << std::left << as->carrier() << std::setw(4) << std::right
                     << as->flightNumber() << std::left << " "
                     << ts->departureDT().dateToString(DDMMM, "") << " " << ts->origin()->loc()
                     << ts->destination()->loc() << "  " << ts->departureDT().timeToString(HHMM, "")
                     << "  " << ts->arrivalDT().timeToString(HHMM, "") << "   ";

          if (needToProcessResults())
          {
            TransfersTravelSegWrapperMapCI ttswIter = _transfersTravelSegWrappers.find(ts);

            if (ttswIter != _transfersTravelSegWrappers.end())
            {
              const TransfersTravelSegWrapper& ttsw = (*ttswIter).second;

              if (ttsw.isTransfer())
              {
                (*diagPtr) << "TFR ";
              }
              else
              {
                (*diagPtr) << "    ";
              }

              if (isCmdPricing && ttsw.maxTRExceeded() == TOTAL_IO_EXCEED)
                (*diagPtr) << "FAIL I/O ONLY  ";
              else
              {
                uint32_t ruleItem = getRuleItemMatch(ts);
                if (ttsw.passedValidation())
                {
                  if (ruleItem == Transfers1::TRANSFER_RULE_ITEM_FOR_NOMATCH)
                    (*diagPtr) << "N/A ";
                  else
                    (*diagPtr) << "PASS";
                }
                else if (anyMaxExceed)
                {
                }
                else if (!ttsw.noMatchValidation() && !ignoreNoMatch())
                {
                  (*diagPtr) << "FAIL";
                }
                else
                {
                  (*diagPtr) << "----";
                }
                if (ruleItem)
                {
                  if (ruleItem == Transfers1::TRANSFER_RULE_ITEM_FOR_NOMATCH)
                    (*diagPtr) << " -----     ";
                  else
                    (*diagPtr) << " " << std::setw(10) << std::left << ruleItem;
                }
                else if (ttsw.passedByLeastRestrictive())
                {
                  (*diagPtr) << " LEAST REST";
                }
                else if (ttsw.maxTRExceeded() == TOTAL_MAX_EXCEED)
                {
                  (*diagPtr) << "FAIL MAX EXCEED ";
                }
                else
                {
                  (*diagPtr) << " NO MATCH  ";
                }
              }
              if (hasPricingUnitScope(fareUsage) && !transferFCscope() &&
                  !ttsw.isRecurringFCScope())
              {
                (*diagPtr) << " PU";
              }
              else
              {
                (*diagPtr) << " FC";
              }
            }
            else
            {
              (*diagPtr) << "    ----";
            }
          }
          (*diagPtr) << std::endl;
        }
        else
        {
          (*diagPtr) << "    ARNK" << std::endl;
        }
      }
      (*diagPtr) << " " << std::endl;
    }
    (*diagPtr) << " " << std::endl;
  }

  bool needToProcessCharges = false;
  if (hasPricingUnitScope(&fu) && !transferFCscope())
  {
    collectMaxTransfersAllow(trx, (const_cast<PricingUnit&>(pu)), fu, isCmdPricing);
  }

  const bool maxFallback = fallback::cat9FixMaxTransfers2(&trx);

  if (maxFallback)
  {
    TransfersTravelSegWrapperMapI tswIter = _transfersTravelSegWrappers.begin();
    TransfersTravelSegWrapperMapI tswIterEnd = _transfersTravelSegWrappers.end();
    // collect transfers first
    for (; tswIter != tswIterEnd; ++tswIter)
    {
      TransfersTravelSegWrapper& ttsw = (*tswIter).second;

      if (LIKELY(ttsw.isTransfer()))
      {
        FareUsage* fu1 = ttsw.fareUsage();

        if (&fu == fu1)
        {
          if (hasPricingUnitScope(&fu) && !transferFCscope())
          {
            collectTransfers(pu);
          }
        }
      }
    }
  }

  if ((needToProcessResults() && (!checkAllPassed())) || (transferFailsPU()) ||
      (maxFallback && (lastFU == &fu) && isCmdPricing && pu.totalTransfers() > 0 &&
       pu.totalTransfers() > pu.mostRestrictiveMaxTransfer()))
  {
    if (isCmdPricing)
    {
      if (UNLIKELY(diagEnabled))
      {
        (*diagPtr) << "TRANSFERS : FAIL - PROCESS CHARGES FOR COMMAND PRICING" << std::endl;
        diagPtr->printLine();
        diagPtr->flushMsg();
      }
      needToProcessCharges = true;
    }
    else
    {
      if (UNLIKELY(diagEnabled))
      {
        (*diagPtr) << "TRANSFERS : FAIL - SEE ABOVE FOR FAILED SEGMENTS" << std::endl;
        diagPtr->printLine();
        diagPtr->flushMsg();
      }
      return FAIL;
    }
  }

  for (auto& ttswI : _transfersTravelSegWrappers)
  {
    TransfersTravelSegWrapper& ttsw = ttswI.second;

    if (LIKELY(ttsw.isTransfer()))
    {
      FareUsage* fu1 = ttsw.fareUsage();

      if (&fu == fu1)
      {
        if (UNLIKELY(needToProcessCharges && (!ttsw.passedValidation() || !ttsw.noMatchValidation())))
        {
          continue;
        }

        if (ttsw.isFareRuleMatch())
        {
          VCTR& vctr = fu1->transfersMatchingVCTR();
          vctr = ttsw.ruleVCTRMatch();
        }
        else
        {
          VCTR& vctr = fu1->transfersMatchingGeneralRuleVCTR();
          vctr = ttsw.ruleVCTRMatch();
        }
      }
    }
  }

  if (maxFallback && (lastFU == &fu) && !isCmdPricing)
  {
    if (pu.totalTransfers() > 0 && pu.totalTransfers() > pu.mostRestrictiveMaxTransfer())
    {
      if (UNLIKELY(diagEnabled))
      {
        (*diagPtr) << "TRANSFERS : FAIL - NUMBER OF TRANSFERS EXCEED MAX ALLOWED" << std::endl;
        diagPtr->printLine();
        diagPtr->flushMsg();
      }
      return FAIL;
    }
  }

  if (needToProcessResults() && processCharges)
  {
    processSurcharges(trx, fp, fu, diagPtr);
  }

  if (UNLIKELY(isCmdPricing))
  {
    if (isRelationAndExists())
    {
      setMostRestrictiveMaxTransferForIndAnd(const_cast<PricingUnit*>(&pu), fu);
    }
    else
    {
      setMostRestrictiveMaxTransfer(trx, const_cast<PricingUnit*>(&pu), _trInfo);
    }
  }

  if (UNLIKELY(needToProcessCharges))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "TRANSFERS : FAIL - NUMBER OF TRANSFERS EXCEED MAX ALLOWED" << std::endl;

      diagPtr->printLine();
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << " " << std::endl << "TRANSFERS : PASS - FINAL VALIDATION" << std::endl;
    diagPtr->printLine();
    diagPtr->flushMsg();
  }
  return PASS;
}

void
TransfersInfoWrapper::collectTransfers(const PricingUnit& pu) const
{
  PricingUnit& pu1 = const_cast<PricingUnit&>(pu);
  pu1.totalTransfers()++;
}

void
TransfersInfoWrapper::processSurcharges(PricingTrx& trx,
                                        FarePath& fp,
                                        const FareUsage& fareUsage,
                                        DiagCollector* diagPtr)
{
  CurrencyConversionFacade ccFacade;

  if (UNLIKELY(diagPtr))
  {
    (*diagPtr) << "---TRANSFER SURCHARGES---" << std::endl;
  }

  TransfersTravelSegWrapperMapI tswIterEnd = _transfersTravelSegWrappers.end();

  std::vector<TravelSeg*>::iterator tsIter = fp.itin()->travelSeg().begin();
  std::vector<TravelSeg*>::iterator tsIterEnd = fp.itin()->travelSeg().end();

  for (; tsIter != tsIterEnd; ++tsIter)
  {
    TransfersTravelSegWrapperMapI tswIter = _transfersTravelSegWrappers.find(*tsIter);

    if (tswIter == tswIterEnd)
    {
      continue;
    }

    TransfersTravelSegWrapper& ttsw = (*tswIter).second;

    // if ( getVCTR() != ttsw.ruleVCTRMatch() ) { continue; }

    TravelSeg* ts = ttsw.travelSeg();
    FareUsage* fu = ttsw.fareUsage();
    // PaxTypeFare* ptf = fu->paxTypeFare();

    if (LIKELY(ttsw.isTransfer()))
    {
      // Add the TravelSeg* to the transfers set in FareUsage.
      fu->transfers().insert(ts);

      if (UNLIKELY(diagPtr))
      {
        AirSeg* as = dynamic_cast<AirSeg*>(ts);

        (*diagPtr) << std::setw(3) << std::right << fp.itin()->segmentOrder(ts) << " ";

        if (as)
        {
          (*diagPtr) << std::setw(3) << std::left << as->carrier() << std::setw(4) << std::right
                     << as->flightNumber() << std::left << " ";
        }
        else
        {
          (*diagPtr) << std::setw(8) << std::left << " ";
        }

        (*diagPtr) << ts->departureDT().dateToString(DDMMM, "") << " " << ts->origin()->loc()
                   << ts->destination()->loc() << "  " << ts->departureDT().timeToString(HHMM, "")
                   << "  " << ts->arrivalDT().timeToString(HHMM, "") << "  ";
      }

      TransfersTravelSegWrapper::SurchargeListCI scIter = ttsw.surcharges().begin();
      TransfersTravelSegWrapper::SurchargeListCI scIterEnd = ttsw.surcharges().end();

      if (UNLIKELY(diagPtr))
      {
        if (scIter == scIterEnd)
        {
          if (fu == &fareUsage)
          {
            (*diagPtr) << " FREE";
          }
          else
          {
            (*diagPtr) << " N/A ";
          }
        }
      }

      if (fu == &fareUsage)
      {
        for (; scIter != scIterEnd; ++scIter)
        {
          const TransfersTravelSegWrapper::Surcharge& sc = *scIter;

          // Add surcharges to the FareUsage

          FareUsage::TransferSurcharge* tfrs = nullptr;
          trx.dataHandle().get(tfrs);

          if (LIKELY(tfrs))
          {
            if (LIKELY(sc.currency() != "NUC"))
            {
              Money nuc("NUC");
              Money scAmt(sc.amount(), sc.currency());
              MoneyAmount convertedAmt = 0.0;

              CurrencyCode calcCur = fp.itin()->calculationCurrency();

              if (UNLIKELY(!ccFacade.convert(nuc,
                                    scAmt,
                                    trx,
                                    calcCur,
                                    convertedAmt,
                                    fp.itin()->useInternationalRounding())))
              {
                LOG4CXX_ERROR(
                    _logger,
                    "TransfersInfoWrapper1::processSurcharges(): Error converting currency");
                return;
              }

              Money calcAmt(convertedAmt, calcCur);

              tfrs->amount() = convertedAmt;
              tfrs->currencyCode() = calcCur;
              tfrs->noDecimals() = calcAmt.noDec();
              tfrs->unconvertedAmount() = sc.amount();
              tfrs->unconvertedCurrencyCode() = sc.currency();
              tfrs->unconvertedNoDecimals() = sc.noDec();
              tfrs->travelSeg() = ts;
              tfrs->isSegmentSpecific() = sc.isSegmentSpecific();
              tfrs->isCharge1() = sc.isCharge1();
              tfrs->matchRuleItemNo() = ttsw.ruleItemMatch();
              tfrs->matchRuleVCTR() = ttsw.ruleVCTRMatch();

              fu->addSurcharge(tfrs, nuc.value());
              fp.increaseTotalNUCAmount(nuc.value());
              fp.plusUpAmount() += nuc.value();
            }
            else
            {
              tfrs->amount() = sc.amount();
              tfrs->currencyCode() = sc.currency();
              tfrs->noDecimals() = sc.noDec();
              tfrs->unconvertedAmount() = sc.amountLocCur();
              tfrs->unconvertedCurrencyCode() = sc.currencyLocCur();
              tfrs->unconvertedNoDecimals() = sc.noDecLocCur();
              tfrs->travelSeg() = ts;
              tfrs->isSegmentSpecific() = sc.isSegmentSpecific();
              tfrs->isCharge1() = sc.isCharge1();
              tfrs->isFirstAmt() = sc.isFirstAmt();
              tfrs->matchRuleItemNo() = ttsw.ruleItemMatch();
              tfrs->matchRuleVCTR() = ttsw.ruleVCTRMatch();

              fu->addSurcharge(tfrs, sc.amount());
              fp.increaseTotalNUCAmount(sc.amount());
              fp.plusUpAmount() += sc.amount();
            }
            fp.plusUpFlag() = true;
          }

          if (UNLIKELY(diagPtr))
          {
            // lint --e{413}
            if (tfrs->amount() == 0)
            {
              (*diagPtr) << " FREE";
            }
            else
            {
              if (!sc.isSegmentSpecific())
              {
                (*diagPtr) << " GEN:";
              }
              else
              {
                (*diagPtr) << " ";
              }

              (*diagPtr) << tfrs->amount() << tfrs->currencyCode();

              //                          if ( tfrs->currencyCode() != sc.currency() )
              if (tfrs->currencyCode() != tfrs->unconvertedCurrencyCode())
              {
                (*diagPtr) << ":"
                           //                                         << sc.amount()
                           //                                         << sc.currency();
                           << tfrs->unconvertedAmount() << tfrs->unconvertedCurrencyCode();
              }
            }
          }
        }
      }

      if (UNLIKELY(diagPtr))
      {
        (*diagPtr) << std::endl;
      }
    }
  }
}

bool
TransfersInfoWrapper::convertDiscountToNUCamounts(PricingTrx& trx,
                                                  MoneyAmount& firstAmt,
                                                  CurrencyCode& firstCur,
                                                  int16_t& firstNoDec,
                                                  MoneyAmount& secondAmt,
                                                  CurrencyCode& secondCur,
                                                  int16_t& secondNoDec,
                                                  PaxTypeFare& fare,
                                                  double& multiplier) const
{
  NUCCurrencyConverter nucConv;

  Money firstAmt1NUC(0, NUC);
  Money secondAmt1NUC(0, NUC);
  Money firstAmt1(firstAmt, firstCur);

  CurrencyConversionRequest firstAmt1ConvReq(firstAmt1NUC,
                                             firstAmt1,
                                             trx.getRequest()->ticketingDT(),
                                             *(trx.getRequest()),
                                             trx.dataHandle(),
                                             fare.isInternational());

  if (!nucConv.convert(firstAmt1ConvReq, nullptr))
  {
    LOG4CXX_ERROR(
        _logger,
        "Rules.StopoversInfoWrapper::convertToNUCamounts(): Error converting currency to NUC");
    return false;
  }

  Money secondAmt1(secondAmt, secondCur);

  CurrencyConversionRequest secondAmt1ConvReq(secondAmt1NUC,
                                              secondAmt1,
                                              trx.getRequest()->ticketingDT(),
                                              *(trx.getRequest()),
                                              trx.dataHandle(),
                                              fare.isInternational());

  if (!nucConv.convert(secondAmt1ConvReq, nullptr))
  {
    LOG4CXX_ERROR(
        _logger,
        "Rules.TransfersInfoWrapper1::convertToNUCamounts(): Error converting currency to NUC");
    return false;
  }

  firstAmt = firstAmt1NUC.value();
  firstCur = firstAmt1NUC.code();
  firstNoDec = firstAmt1NUC.noDec();
  secondAmt = secondAmt1NUC.value();
  secondCur = secondAmt1NUC.code();
  secondNoDec = secondAmt1NUC.noDec();

  CurrencyUtil::truncateNUCAmount(firstAmt);
  CurrencyUtil::truncateNUCAmount(secondAmt);

  firstAmt = firstAmt * multiplier;
  CurrencyUtil::truncateNUCAmount(firstAmt);
  secondAmt = secondAmt * multiplier;
  CurrencyUtil::truncateNUCAmount(secondAmt);
  return true;
}

void
TransfersInfoWrapper::noMatchValidation(const PricingUnit& pu, const FareUsage& currentFu)
{
  if (!needToProcessResults())
    return;

  for (const FareUsage* fu : pu.fareUsage())
  {
    for (const TravelSeg* ts : fu->travelSeg())
    {
      if (!ts->isAir())
        continue;

      const auto ttswI = _transfersTravelSegWrappers.find(ts);

      if (ttswI == _transfersTravelSegWrappers.end())
        continue;

      TransfersTravelSegWrapper& ttsw = ttswI->second;

      if (hasPricingUnitScope(fu) && ttsw.isTransfer() && !ttsw.passedValidation() &&
          ttsw.fareUsage() != &currentFu && ttsw.validateEntireRule())
      {
        ttsw.noMatchValidation() = true;
      }
    }
  }
}

void
TransfersInfoWrapper::collectMaxTransfersAllow(PricingTrx& trx,
                                               PricingUnit& pu,
                                               const FareUsage& fu,
                                               bool isCmdPricing) const
{
  if (LIKELY(_crInfo))
  {
    std::vector<CategoryRuleItemInfoSet*>::const_iterator iter =
        _crInfo->categoryRuleItemInfoSet().begin();

    std::vector<CategoryRuleItemInfoSet*>::const_iterator iterEnd =
        _crInfo->categoryRuleItemInfoSet().end();

    int16_t ruleSetTransferTotal = 0;

    for (; iter != iterEnd; ++iter)
    {
      collectMaxPassedTransfers(trx, **iter, ruleSetTransferTotal);
    }
    if ((pu.mostRestrictiveMaxTransfer() < 0 ||
         pu.mostRestrictiveMaxTransfer() > ruleSetTransferTotal))
    {
      pu.mostRestrictiveMaxTransfer() = ruleSetTransferTotal;
    }

    // TODO - The following code may be removed if the fix for SPR#121007
    //        can populate the correct value in _leastRestrictiveTransfersPermitted
    //        for I/B. Current code, _leastRestrictiveTransfersPermitted
    //        is cleared only for 1st fareusage.
    if (UNLIKELY(isCmdPricing && isRelationAndExists() && hasPricingUnitScope(&fu) && !transferFCscope()))
    {
      if (pu.mostRestrictiveMaxTransfer() < 0 ||
          pu.mostRestrictiveMaxTransfer() > ruleSetTransferTotal)
      {
        pu.mostRestrictiveMaxTransfer() = ruleSetTransferTotal;
      }
    }
    // TODO - End of comment
  }
  return;
}

void
TransfersInfoWrapper::collectMaxPassedTransfers(PricingTrx& trx,
                                                const CategoryRuleItemInfoSet& cRuleItemInfoSet,
                                                int16_t& ruleSetTransferTotal) const
{
  for (const auto& info: cRuleItemInfoSet)
  {
    bool trInfoIsValid = anyTablePasses(trx, info.itemNo());

    if (!trInfoIsValid)
      continue;

    const RuleItemInfo* const ruleItemInfo = getRuleItemInfo(trx, &info);
    //                                  trx.dataHandle().getRuleItemInfo(_crInfo, *criIter);

    const TransfersInfo1* trInfo = dynamic_cast<const TransfersInfo1*>(ruleItemInfo);
    if (LIKELY(trInfo))
    {
      if (LIKELY(!trInfo->noTransfersMax().empty()))
      {
        int16_t transfersMax = 0;
        if (trInfo->noTransfersMax() == Transfers1::NUM_TRANSFERS_UNLIMITED)
          transfersMax = RuleConst::MAX_NUMBER_XX;
        else
          transfersMax = atoi(trInfo->noTransfersMax().c_str());

        if (info.relationalInd() == CategoryRuleItemInfo::AND)
        {
          ruleSetTransferTotal += transfersMax;
        }
        else if (info.relationalInd() == CategoryRuleItemInfo::OR)
        {
          if (transfersMax > ruleSetTransferTotal)
          {
            ruleSetTransferTotal = transfersMax;
          }
        }
        else // THEN, IF
        {
          if (LIKELY(transfersMax > ruleSetTransferTotal))
          {
            ruleSetTransferTotal = transfersMax;
          }
        }
      }
    }
  }
  return;
}

bool
TransfersInfoWrapper::anyTablePasses(PricingTrx& trx, const uint32_t itemNo) const
{
  TransfersTravelSegWrapperMapI tswIter = _transfersTravelSegWrappers.begin();
  TransfersTravelSegWrapperMapI tswIterEnd = _transfersTravelSegWrappers.end();

  for (; tswIter != tswIterEnd; ++tswIter)
  {
    TransfersTravelSegWrapper& ttsw = (*tswIter).second;
    if (ttsw.isTransfer() && itemNo == ttsw.ruleItemMatch())
    {
      return true;
    }
  }
  return false;
}

void
TransfersInfoWrapper::setMostRestrictiveMaxTransfer(PricingTrx& trx,
                                                    PricingUnit* pu,
                                                    const TransfersInfo1* trInfo)
{
  int16_t numTransfersMax = 0;
  bool numTransfersMaxUnlimited = false;

  if (trInfo->noTransfersMax().empty() ||
      trInfo->noTransfersMax() == Transfers1::NUM_TRANSFERS_UNLIMITED)
  {
    numTransfersMaxUnlimited = true;
  }
  else
  {
    numTransfersMax = atoi(trInfo->noTransfersMax().c_str());
  }

  if (numTransfersMaxUnlimited)
  {
    pu->hasTransferFCscope() = true;
  }
  else
  {
    if (pu->mostRestrictiveMaxTransfer() < 0 || pu->mostRestrictiveMaxTransfer() > numTransfersMax)
    {
      pu->mostRestrictiveMaxTransfer() = numTransfersMax;
    }
  }
}

void
TransfersInfoWrapper::setMostRestrictiveMaxTransferForIndAnd(PricingUnit* pu, const FareUsage& fu)
{
  if (transferFCscope() || !hasPricingUnitScope(&fu))
    pu->hasTransferFCscope() = true;
}

void
TransfersInfoWrapper::setMaxExceeded(TravelSeg* ts, FareUsage* fu, const int16_t val) const
{
  if (UNLIKELY(!ts))
    return;

  TransfersTravelSegWrapper& ttsw = getOrCreateTsWrapper(ts, fu);
  if (LIKELY(val > 0))
    ttsw.maxTRExceeded() = val;

  _needToProcessResults = true;
}

bool
TransfersInfoWrapper::checkMaxExceed(const PricingUnit& pu) const
{
  bool anyMaxExceed = false;
  std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();

  for (; fuI != fuEnd; ++fuI)
  {
    const FareUsage* fareUsage = *fuI;

    std::vector<TravelSeg*>::const_iterator tsI = fareUsage->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tsEnd = fareUsage->travelSeg().end();

    for (tsI = fareUsage->travelSeg().begin(); tsI != tsEnd; ++tsI)
    {
      const TravelSeg* ts = *tsI;
      if (needToProcessResults())
      {
        TransfersTravelSegWrapperMapCI ttswIter = _transfersTravelSegWrappers.find(ts);

        if (ttswIter != _transfersTravelSegWrappers.end())
        {
          const TransfersTravelSegWrapper& ttsw = (*ttswIter).second;
          if (ttsw.maxTRExceeded() > 0)
          {
            return anyMaxExceed = true;
          }
        }
      }
    }
  }
  return anyMaxExceed;
}

const RuleItemInfo*
TransfersInfoWrapper::getRuleItemInfo(PricingTrx& trx, const CategoryRuleItemInfo* cri) const
{
  return (trx.dataHandle().getRuleItemInfo(_crInfo, cri));
}

TransfersTravelSegWrapper&
TransfersInfoWrapper::getOrCreateTsWrapper(TravelSeg* ts, FareUsage* fu) const
{
  TransfersTravelSegWrapper& ttsw = _transfersTravelSegWrappers[ts];

  if (!ttsw.travelSeg())
  {
    ttsw.travelSeg() = ts;
    ttsw.fareUsage() = fu;
  }

  return ttsw;
}

TransfersInfoWrapper::Charge::Charge(bool s) : first(0.0), second(0.0), noDec(0), status(s) {}

TransfersInfoWrapper::Charge::Charge(const MoneyAmount& a1,
                                     const MoneyAmount& a2,
                                     const CurrencyCode& c,
                                     const CurrencyNoDec& d,
                                     bool s)
  : first(a1), second(a2), currency(c), noDec(d), status(s)
{
}

void
TransfersInfoWrapper::Charge::assign(const MoneyAmount& a1,
                                     const MoneyAmount& a2,
                                     const CurrencyCode& c,
                                     const CurrencyNoDec& d,
                                     bool s)
{
  first = a1;
  second = a2;
  currency = c;
  noDec = d;
  status = s;
}

} // tse
