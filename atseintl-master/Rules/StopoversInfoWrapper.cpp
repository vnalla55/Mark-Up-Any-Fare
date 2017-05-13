//-------------------------------------------------------------------
//
//  File:        StopoversInfoWrapper.cpp
//  Created:     August 3, 2004
//  Authors:     Andrew Ahmad
//
//  Description:
//
//
//  Copyright Sabre 2004
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

#include "Rules/StopoversInfoWrapper.h"

#include "Common/ChargeSODirect.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
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
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/StopoversInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleSetPreprocessor.h"
#include "Rules/Stopovers.h"
#include "Rules/StopoversTravelSegWrapper.h"
#include "Util/BranchPrediction.h"

#include <map>


namespace tse
{
using namespace std;

static Logger
logger("atseintl.Rules.StopoversInfoWrapper");

const int16_t StopoversInfoWrapper::TOTAL_MAX_EXECEED = 1;
const int16_t StopoversInfoWrapper::OUT_MAX_EXECEED = 2;
const int16_t StopoversInfoWrapper::IN_MAX_EXECEED = 3;
const int16_t StopoversInfoWrapper::TO_MANY_SO = 4;

void
StopoversInfoWrapper::soInfo(const StopoversInfo* soInfo)
{
  _soInfo = soInfo;

  if (LIKELY(_soInfo))
  {
    copyFrom(*_soInfo);

    std::string charge1FirstNoStr;
    std::string charge1AddNoStr;
    std::string charge2FirstNoStr;
    std::string charge2AddNoStr;

    charge1FirstNoStr = _soInfo->charge1First();
    charge1AddNoStr = _soInfo->charge1AddNo();
    charge2FirstNoStr = _soInfo->charge1First();
    charge2AddNoStr = _soInfo->charge1AddNo();

    _charge1FirstUnlimited =
        (charge1FirstNoStr == Stopovers::NUM_STOPS_UNLIMITED) || (charge1FirstNoStr.empty());

    _charge2FirstUnlimited =
        (charge2FirstNoStr == Stopovers::NUM_STOPS_UNLIMITED) || (charge2FirstNoStr.empty());

    _charge1AddUnlimited =
        (charge1AddNoStr == Stopovers::NUM_STOPS_UNLIMITED) || (charge1AddNoStr.empty());

    _charge2AddUnlimited =
        (charge2AddNoStr == Stopovers::NUM_STOPS_UNLIMITED) || (charge2AddNoStr.empty());

    if (!_charge1FirstUnlimited)
    {
      _charge1FirstNo = atoi(charge1FirstNoStr.c_str());
    }
    else
    {
      _charge1FirstNo = 0;
    }

    if (!_charge2FirstUnlimited)
    {
      _charge2FirstNo = atoi(charge2FirstNoStr.c_str());
    }
    else
    {
      _charge2FirstNo = 0;
    }

    if (!_charge1AddUnlimited)
    {
      _charge1AddNo = atoi(charge1AddNoStr.c_str());
    }
    else
    {
      _charge1AddNo = 0;
    }

    if (!_charge2AddUnlimited)
    {
      _charge2AddNo = atoi(charge2AddNoStr.c_str());
    }
    else
    {
      _charge2AddNo = 0;
    }
  }
}

void
StopoversInfoWrapper::crInfo(const CategoryRuleInfo* crInfo)
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
StopoversInfoWrapper::copyFrom(const RuleItemInfo& ruleItemInfo)
{
  vendor() = ruleItemInfo.vendor();
  itemNo() = ruleItemInfo.itemNo();
  textTblItemNo() = ruleItemInfo.textTblItemNo();
  overrideDateTblItemNo() = ruleItemInfo.overrideDateTblItemNo();
}

bool
StopoversInfoWrapper::checkAllPassed()
{
  if (ignoreNoMatch())
  {
    return true;
  }

  bool applyLeastRestrictive = applyLeastRestrictiveProvision();
  bool numStopsUnlimited = leastRestrictiveStopoversUnlimited();
  int16_t numStopsMax = leastRestrictiveStopoversPermitted();

  int16_t stopCount = 0;

  StopoversTravelSegWrapperMapI iter = _stopoversTravelSegWrappers.begin();
  StopoversTravelSegWrapperMapI iterEnd = _stopoversTravelSegWrappers.end();

  for (; iter != iterEnd; ++iter)
  {
    StopoversTravelSegWrapper& stsw = (*iter).second;
    if (stsw.isStopover())
    {
      if (!stsw.passedValidation() || !checkArnkMatch(stsw))
      {
        // we check arunkTravelSeg for non-null since we do forced match
        // and itemNo may not be null.
        if (stsw.fareUsage() != this->fareUsage() &&
            (stsw.arunkTravelSeg() || !stsw.ruleItemMatch()))
        {
          continue; // not NULL but exists arunkTravelSeg while ArnkMatch failed
        }
        return false;
      }

      if (UNLIKELY(!applyLeastRestrictive && (!stsw.arunkTravelSeg() && !stsw.ruleItemMatch())))
      {
        return false;
      }

      if (applyLeastRestrictive)
      {
        if (!numStopsUnlimited)
        {
          ++stopCount;
          if (stopCount > numStopsMax)
          {
            return false;
          }
        }
      }
    }
  }
  return true;
}

void
StopoversInfoWrapper::setIsStopover(TravelSeg* ts, FareUsage* fu) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::setIsStopover(): TravelSeg=0");
    return;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (tswIter == _stopoversTravelSegWrappers.end())
  {
    StopoversTravelSegWrapper stsw;
    stsw.travelSeg() = ts;
    stsw.fareUsage() = fu;

    StopoversTravelSegWrapperMap::value_type value(ts, stsw);

    tswIter = (_stopoversTravelSegWrappers.insert(value)).first;
  }

  StopoversTravelSegWrapper& stsw = (*tswIter).second;

  stsw.isStopover() = true;
  _needToProcessResults = true;
}

bool
StopoversInfoWrapper::checkIsStopover(TravelSeg* ts) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::checkIsStopover(): TravelSeg =0");
    return false;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (tswIter == _stopoversTravelSegWrappers.end())
  {
    return false;
  }
  StopoversTravelSegWrapper& stsw = (*tswIter).second;

  return stsw.isStopover();
}

void
StopoversInfoWrapper::setPassedValidation(TravelSeg* ts,
                                          FareUsage* fu,
                                          bool passedByLeastRestrictive,
                                          TravelSeg* arunk) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::setPassedValidation(): TravelSeg =0");
    return;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (UNLIKELY(tswIter == _stopoversTravelSegWrappers.end()))
  {
    StopoversTravelSegWrapper stsw;
    stsw.travelSeg() = ts;
    stsw.fareUsage() = fu;

    StopoversTravelSegWrapperMap::value_type value(ts, stsw);

    tswIter = (_stopoversTravelSegWrappers.insert(value)).first;
  }

  StopoversTravelSegWrapper& stsw = (*tswIter).second;

  stsw.passedValidation() = true;
  stsw.passedByLeastRestrictive() = passedByLeastRestrictive;
  stsw.arunkTravelSeg() = arunk;
  _needToProcessResults = true;
}

bool
StopoversInfoWrapper::checkPassedValidation(TravelSeg* ts) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::checkPassedValidation(): TravelSeg =0");
    return false;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (UNLIKELY(tswIter == _stopoversTravelSegWrappers.end()))
  {
    return false;
  }
  StopoversTravelSegWrapper& stsw = (*tswIter).second;
  return stsw.passedValidation();
}

void
StopoversInfoWrapper::setRuleItemMatch(TravelSeg* ts,
                                       FareUsage* fu,
                                       const uint32_t itemNo,
                                       const bool isTentativeMatch,
                                       bool surfaceNegativeMatch) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::setRuleItemMatch(): TravelSeg =0");
    return;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (tswIter == _stopoversTravelSegWrappers.end())
  {
    StopoversTravelSegWrapper stsw;
    stsw.travelSeg() = ts;
    stsw.fareUsage() = fu;

    StopoversTravelSegWrapperMap::value_type value(ts, stsw);

    tswIter = (_stopoversTravelSegWrappers.insert(value)).first;
  }

  StopoversTravelSegWrapper& stsw = (*tswIter).second;

  if (UNLIKELY(stsw.ruleItemMatch()))
  {
    if ((stsw.isTentativeMatch() && !isTentativeMatch) || stsw.arunkTravelSeg())
    {
      stsw.clearSurcharges();
      stsw.ruleItemMatch() = itemNo;
      stsw.ruleVCTRMatch() = getVCTR();
      stsw.isFareRuleMatch() = _isFareRule;
      stsw.isTentativeMatch() = isTentativeMatch;
      stsw.passedValidation() = false;
    }
  }
  else
  {
    if (LIKELY(!surfaceNegativeMatch))
    {
      stsw.ruleItemMatch() = itemNo;
    }
    else
    {
      StopoversTravelSegWrapperMapI iter = _stopoversTravelSegWrappers.begin();
      StopoversTravelSegWrapperMapI iterEnd = _stopoversTravelSegWrappers.end();

      for (; iter != iterEnd; ++iter)
      {
        StopoversTravelSegWrapper& stsW = (*iter).second;
        if (fu && stsW.isStopover() && stsW.arunkTravelSeg() == ts)
        {
          stsW.ruleItemMatch() = itemNo;
          break;
        }
      }
    }
    stsw.ruleVCTRMatch() = getVCTR();
    stsw.isFareRuleMatch() = _isFareRule;
    stsw.isTentativeMatch() = isTentativeMatch;
    stsw.passedValidation() = false;
  }

  _needToProcessResults = true;
}

uint32_t
StopoversInfoWrapper::getRuleItemMatch(const TravelSeg* ts) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::getRuleItemMatch(): TravelSeg =0");
    return 0;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (UNLIKELY(tswIter == _stopoversTravelSegWrappers.end()))
  {
    return 0;
  }
  StopoversTravelSegWrapper& stsw = (*tswIter).second;
  return stsw.ruleItemMatch();
}

uint32_t
StopoversInfoWrapper::getRuleItemMatch(const TravelSeg* ts, bool& isTentativeMatch) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::getRuleItemMatch(): TravelSeg =0");
    return 0;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (tswIter == _stopoversTravelSegWrappers.end())
    return 0;

  StopoversTravelSegWrapper& stsw = (*tswIter).second;
  isTentativeMatch = stsw.isTentativeMatch();
  return stsw.ruleItemMatch();
}

const VCTR&
StopoversInfoWrapper::getVCTR() const
{
  return _vctr;
}

void
StopoversInfoWrapper::doRuleSetPreprocessing(PricingTrx& trx,
                                             const RuleSetPreprocessor& rsp,
                                             const PricingUnit& pu)
{
  applyLeastRestrictiveProvision() = rsp.applyLeastRestrictiveStopovers();

  leastRestrictiveStopoversUnlimited() = rsp.leastRestrictiveStopoversUnlimited();

  leastRestrictiveStopoversPermitted() = rsp.leastRestrictiveStopoversPermitted();

  crInfo(rsp.categoryRuleInfo());

  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

  for (; fuIter != fuIterEnd; ++fuIter)
  {
    FareUsage* fu = *fuIter;

    if (!fu->stopoverByDir().empty())
    {
      FareUsage::StopoverInfoByDirectionMapCI ifu = fu->stopoverByDir().begin();

      for (; ifu != fu->stopoverByDir().end(); ++ifu)
      {
        _infoSegsDirectionalityCharge.insert(std::make_pair(ifu->first, ifu->second));
      }
    }
  }
}

bool
StopoversInfoWrapper::selectApplCharge(PricingTrx& trx,
                                       int16_t& chargeNum,
                                       MoneyAmount& firstAmt,
                                       CurrencyCode& firstCur,
                                       int16_t& firstNoDec,
                                       MoneyAmount& addAmt,
                                       CurrencyCode& addCur,
                                       int16_t& addNoDec,
                                       const PaxTypeFare& fare,
                                       const PaxType& paxType) const
{
  const DateTime& ticketingDate = trx.ticketingDate();

  firstAmt = 0;
  firstCur = "";
  firstNoDec = 0;
  addAmt = 0;
  addCur = "";
  addNoDec = 0;

  chargeNum = 1;

  // If only one charge has a valid currency, then use it
  //
  if (!_soInfo->charge1Cur().empty() && _soInfo->charge2Cur().empty())
  {
    chargeNum = 1;

    if (UNLIKELY(!chargeForPaxType(trx, _soInfo->charge1Appl(), fare, paxType)))
    {
      // The stopover surcharge does not apply for this pax type.
      //  i.e. The stopover surcharge is 0.
      return true;
    }

    firstAmt = _soInfo->charge1FirstAmt();
    firstCur = _soInfo->charge1Cur();
    firstNoDec = _soInfo->charge1NoDec();
    addAmt = _soInfo->charge1AddAmt();
    addCur = _soInfo->charge1Cur();
    addNoDec = _soInfo->charge1NoDec();

    return true;
  }
  else if (_soInfo->charge1Cur().empty() && !_soInfo->charge2Cur().empty())
  {
    chargeNum = 2;

    if (!chargeForPaxType(trx,
                         _soInfo->charge1Appl(),
                          fare,
                          paxType))
    {
      // The stopover surcharge does not apply for this pax type.
      //  i.e. The stopover surcharge is 0.
      return true;
    }

    firstAmt = _soInfo->charge2FirstAmt();
    firstCur = _soInfo->charge2Cur();
    firstNoDec = _soInfo->charge2NoDec();
    addAmt = _soInfo->charge2AddAmt();
    addCur = _soInfo->charge2Cur();
    addNoDec = _soInfo->charge2NoDec();

    return true;
  }
  else if (_soInfo->charge1Cur().empty() && _soInfo->charge2Cur().empty())
  {
    // Both currencies are blank. There is no surcharge to apply.
    return true;
  }

  // Both charges have a valid currency, so now determine which one to use.
  //

  if ( !chargeForPaxType(trx, _soInfo->charge1Appl(), fare, paxType) )
  {
    // Neither stopover surcharge applies for this pax type.
    //  i.e. The stopover surcharge is 0.
    return true;
  }

  if (fare.currency() == _soInfo->charge1Cur())
  {
    chargeNum = 1;

    firstAmt = _soInfo->charge1FirstAmt();
    firstCur = _soInfo->charge1Cur();
    firstNoDec = _soInfo->charge1NoDec();
    addAmt = _soInfo->charge1AddAmt();
    addCur = _soInfo->charge1Cur();
    addNoDec = _soInfo->charge1NoDec();

    return true;
  }
  else if (fare.currency() == _soInfo->charge2Cur())
  {
    chargeNum = 2;

    firstAmt = _soInfo->charge2FirstAmt();
    firstCur = _soInfo->charge2Cur();
    firstNoDec = _soInfo->charge2NoDec();
    addAmt = _soInfo->charge2AddAmt();
    addCur = _soInfo->charge2Cur();
    addNoDec = _soInfo->charge2NoDec();

    return true;
  }
  else
  {
    // Convert both currencies to NUC and decide which one to use.
    //
    NUCCurrencyConverter nucConv;

    Money firstAmt1NUC(0, NUC);
    Money firstAmt2NUC(0, NUC);
    Money addAmt1NUC(0, NUC);
    Money addAmt2NUC(0, NUC);

    if (_soInfo->charge1FirstAmt() != 0)
    {
      Money firstAmt1(_soInfo->charge1FirstAmt(), _soInfo->charge1Cur());

      CurrencyConversionRequest firstAmt1ConvReq(firstAmt1NUC,
                                                 firstAmt1,
                                                 trx.getRequest()->ticketingDT(),
                                                 *(trx.getRequest()),
                                                 trx.dataHandle(),
                                                 fare.isInternational());

      if (!nucConv.convert(firstAmt1ConvReq, nullptr))
      {
        LOG4CXX_ERROR(
            logger,
            "Rules.StopoversInfoWrapper::selectApplCharge(): Error converting currency to NUC");
        return false;
      }
    }

    if (_soInfo->charge1AddAmt() != 0)
    {
      Money addAmt1(_soInfo->charge1AddAmt(), _soInfo->charge1Cur());

      CurrencyConversionRequest addAmt1ConvReq(addAmt1NUC,
                                               addAmt1,
                                               trx.getRequest()->ticketingDT(),
                                               *(trx.getRequest()),
                                               trx.dataHandle(),
                                               fare.isInternational());

      if (!nucConv.convert(addAmt1ConvReq, nullptr))
      {
        LOG4CXX_ERROR(
            logger,
            "Rules.StopoversInfoWrapper::selectApplCharge(): Error converting currency to NUC");
        return false;
      }
    }

    if (_soInfo->charge2FirstAmt() != 0)
    {
      Money firstAmt2(_soInfo->charge2FirstAmt(), _soInfo->charge2Cur());

      CurrencyConversionRequest firstAmt2ConvReq(firstAmt2NUC,
                                                 firstAmt2,
                                                 trx.getRequest()->ticketingDT(),
                                                 *(trx.getRequest()),
                                                 trx.dataHandle(),
                                                 fare.isInternational());

      if (!nucConv.convert(firstAmt2ConvReq, nullptr))
      {
        LOG4CXX_ERROR(
            logger,
            "Rules.StopoversInfoWrapper::selectApplCharge(): Error converting currency to NUC");
        return false;
      }
    }

    if (_soInfo->charge2AddAmt() != 0)
    {
      Money addAmt2(_soInfo->charge2AddAmt(), _soInfo->charge2Cur());

      CurrencyConversionRequest addAmt2ConvReq(addAmt2NUC,
                                               addAmt2,
                                               trx.getRequest()->ticketingDT(),
                                               *(trx.getRequest()),
                                               trx.dataHandle(),
                                               fare.isInternational());

      if (!nucConv.convert(addAmt2ConvReq, nullptr))
      {
        LOG4CXX_ERROR(
            logger,
            "Rules.StopoversInfoWrapper::selectApplCharge(): Error converting currency to NUC");
        return false;
      }
    }

    if (firstAmt1NUC.value() < firstAmt2NUC.value())
    {
      // Use charge1
      chargeNum = 1;

      firstAmt = firstAmt1NUC.value();
      firstCur = firstAmt1NUC.code();
      firstNoDec = firstAmt1NUC.noDec(ticketingDate);
      addAmt = addAmt1NUC.value();
      addCur = addAmt1NUC.code();
      addNoDec = addAmt1NUC.noDec(ticketingDate);
    }
    else if (firstAmt2NUC.value() < firstAmt1NUC.value())
    {
      // Use charge2
      chargeNum = 2;

      firstAmt = firstAmt2NUC.value();
      firstCur = firstAmt2NUC.code();
      firstNoDec = firstAmt2NUC.noDec(ticketingDate);
      addAmt = addAmt2NUC.value();
      addCur = addAmt2NUC.code();
      addNoDec = addAmt2NUC.noDec(ticketingDate);
    }
    else if (addAmt1NUC.value() < addAmt2NUC.value())
    {
      // Use charge1
      chargeNum = 1;

      firstAmt = firstAmt1NUC.value();
      firstCur = firstAmt1NUC.code();
      firstNoDec = firstAmt1NUC.noDec(ticketingDate);
      addAmt = addAmt1NUC.value();
      addCur = addAmt1NUC.code();
      addNoDec = addAmt1NUC.noDec(ticketingDate);
    }
    else if (addAmt2NUC.value() < addAmt1NUC.value())
    {
      // Use charge2
      chargeNum = 2;

      firstAmt = firstAmt2NUC.value();
      firstCur = firstAmt2NUC.code();
      firstNoDec = firstAmt2NUC.noDec(ticketingDate);
      addAmt = addAmt2NUC.value();
      addCur = addAmt2NUC.code();
      addNoDec = addAmt2NUC.noDec(ticketingDate);
    }
    else
    {
      // Use charge1
      chargeNum = 1;

      firstAmt = firstAmt1NUC.value();
      firstCur = firstAmt1NUC.code();
      firstNoDec = firstAmt1NUC.noDec(ticketingDate);
      addAmt = addAmt1NUC.value();
      addCur = addAmt1NUC.code();
      addNoDec = addAmt1NUC.noDec(ticketingDate);
    }
  }

  return true;
}

bool
StopoversInfoWrapper::applyPaxTypeDiscount(PricingTrx& trx,
                                           MoneyAmount& firstAmt,
                                           CurrencyCode& firstCur,
                                           int16_t& firstNoDec,
                                           MoneyAmount& addAmt,
                                           CurrencyCode& addCur,
                                           int16_t& addNoDec,
                                           const FareUsage& fareUsage,
                                           const PaxType& paxType,
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

  if (PaxTypeUtil::isInfant(trx, paxType.paxType(), fare->vendor()) &&
      (appl == RuleConst::CHARGE_PAX_ADULT_CHILD_INFANT_FREE ||
       appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE))
  {
    firstAmt = 0;
    addAmt = 0;
    return true;
  }

  if (fare->discountInfo().farecalcInd() != RuleConst::CALCULATED)
    return true;

  MoneyAmount discPercent = fare->discountInfo().discPercent();

  // discPercent is not the discount percentage, it is the percentage to charge
  /*double*/ multiplier = discPercent / 100.0;

  if (PaxTypeUtil::isChild(trx, paxType.paxType(), fare->vendor()))
  {
    if (appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC ||
        appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC ||
        appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE)
    {
      firstAmt = firstAmt * multiplier;
      addAmt = addAmt * multiplier;
    }
  }
  else if (PaxTypeUtil::isInfant(trx, paxType.paxType(), fare->vendor()))
  {
    if (appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC)
    {
      firstAmt = firstAmt * multiplier;
      addAmt = addAmt * multiplier;
    }
  }
  return true;
}

bool
StopoversInfoWrapper::chargeForPaxType(PricingTrx& trx,
                                       const Indicator& chargeAppl,
                                       const PaxTypeFare& fare,
                                       const PaxType& paxType,
                                       const bool step) const
{
  if (UNLIKELY(!step && fare.fareAmount() == 0 &&
      PaxTypeUtil::isInfant(trx, paxType.paxType(), fare.vendor())))
  {
    // Do not apply stopover surcharges for infant if fare amount is zero.
    return false;
  }

  if (chargeAppl == RuleConst::CHARGE_PAX_ANY)
  {
    return true;
  }

  if (PaxTypeUtil::isChild(trx, paxType.paxType(), fare.vendor()))
  {
    if (chargeAppl == RuleConst::CHARGE_PAX_ADULT || chargeAppl == RuleConst::CHARGE_PAX_INFANT)
    {
      return false;
    }
  }
  else if (PaxTypeUtil::isInfant(trx, paxType.paxType(), fare.vendor()))
  {
    if (chargeAppl == RuleConst::CHARGE_PAX_ADULT || chargeAppl == RuleConst::CHARGE_PAX_CHILD ||
        chargeAppl == RuleConst::CHARGE_PAX_ADULT_CHILD ||
        chargeAppl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC)
    {
      return false;
    }
    else if ((chargeAppl == RuleConst::CHARGE_PAX_INFANT) &&
             (paxType.paxTypeInfo().numberSeatsReq() != 0))
    {
      return false;
    }
  }
  else if (LIKELY(PaxTypeUtil::isAdult(trx, paxType.paxType(), fare.vendor())))
  {
    if (UNLIKELY(chargeAppl == RuleConst::CHARGE_PAX_CHILD || chargeAppl == RuleConst::CHARGE_PAX_INFANT))
    {
      return false;
    }
  }
  return true;
}

void
StopoversInfoWrapper::clearResults(bool skipInd)
{
  if (!skipInd)
  {
    _stopoversTravelSegWrappers.clear();
  }
  else if (!stopoversTravelSegWrappers().empty())
  {
    StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.begin();
    for (; tswIter != _stopoversTravelSegWrappers.end(); tswIter++)
    {
      StopoversTravelSegWrapper& stsw = (*tswIter).second;
      stsw.ruleItemMatch() = 0;
      stsw.isFareRuleMatch() = false;
      stsw.isTentativeMatch() = false;
      stsw.passedValidation() = false;
      stsw.passedByLeastRestrictive() = false;
      stsw.clearSurcharges();
    }
  }
  _charge1FirstAmtCount = 0;
  _charge1AddAmtCount = 0;
  _charge2FirstAmtCount = 0;
  _charge2AddAmtCount = 0;

  _maxStopoversPermitted = 0;
  _maxStopoversPermittedUnlimited = false;
  if (!skipInd)
  {
    _needToProcessResults = false;
  }
  _ignoreNoMatch = false;
  _stopoverMaxExceeed.reset();
}

// called by FareMarketRuleController
Record3ReturnTypes
StopoversInfoWrapper::processResults(PricingTrx& trx, PaxTypeFare& ptf)
{
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  const FareMarket* fm = ptf.fareMarket();
  bool maybeSoftPass = (fm->direction() == FMDirection::UNKNOWN);

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic308))
  {
    if (diagPtr->parseFareMarket(trx, *(ptf.fareMarket())))
    {
      factory = DCFactory::instance();
      diagPtr = factory->create(trx);
      diagPtr->enable(Diagnostic308);
      diagEnabled = true;
    }
  }

  if (diagEnabled)
  {
    (*diagPtr) << "CATEGORY 08 - STOPOVERS DIAGNOSTICS" << endl;
    (*diagPtr) << "PHASE: FARE FINAL VALIDATION" << endl << " " << endl;

    (*diagPtr) << "FARECOMP-1 : " << setw(12) << std::left << ptf.fareClass() << std::right
               << "   ";

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
      (*diagPtr) << setw(3) << airSegCount - 1 << " OFFPOINTS";
    }
    else if (airSegCount == 2)
    {
      (*diagPtr) << "  1 OFFPOINT ";
    }
    else
    {
      (*diagPtr) << " NO OFFPOINTS";
    }

    (*diagPtr) << " FROM " << fm->origin()->loc() << " TO " << fm->destination()->loc() << endl;

    int16_t segNumber = 1;

    for (tsI = fm->travelSeg().begin(); tsI != tsEnd; ++tsI, ++segNumber)
    {
      const TravelSeg* ts = *tsI;
      const AirSeg* as = dynamic_cast<const AirSeg*>(ts);

      if (as)
      {
        (*diagPtr) << setw(3) << right << segNumber << " " << setw(3) << left << as->carrier()
                   << setw(4) << right << as->flightNumber() << left << " "
                   << ts->departureDT().dateToString(DDMMM, "") << " " << ts->origin()->loc()
                   << ts->destination()->loc() << "  " << ts->departureDT().timeToString(HHMM, "")
                   << "  " << ts->arrivalDT().timeToString(HHMM, "") << "   ";

        if (needToProcessResults())
        {
          StopoversTravelSegWrapperMapCI stswIter = _stopoversTravelSegWrappers.find(ts);

          if (stswIter != _stopoversTravelSegWrappers.end())
          {
            const StopoversTravelSegWrapper& stsw = (*stswIter).second;

            if (stsw.isStopover())
            {
              (*diagPtr) << "SO ";
            }
            else
            {
              (*diagPtr) << "   ";
            }

            if (stsw.passedValidation() && checkArnkMatch(stsw))
            {
              (*diagPtr) << "PASS";
            }
            else if (!ignoreNoMatch() && !maybeSoftPass)
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
            else if (stsw.passedByLeastRestrictive())
            {
              (*diagPtr) << " LEAST REST";
            }
            else if (!maybeSoftPass)
            {
              (*diagPtr) << " NO MATCH";
            }
          }
          else
          {
            (*diagPtr) << "   PASS";
          }
        }
        (*diagPtr) << endl;
      }
      else
      {
        (*diagPtr) << "    ARNK" << endl;
      }
    }
    (*diagPtr) << " " << endl;
  }

  if (needToProcessResults() && (!checkAllPassed()) && !maybeSoftPass)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "STOPOVERS : FAIL - SEE ABOVE FOR FAILED SEGMENTS" << endl;

      diagPtr->printLine();
      diagPtr->flushMsg();
    }
    return tse::FAIL;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << " " << endl << "STOPOVERS : SOFTPASS - FINAL VALIDATION" << endl;

    diagPtr->printLine();
    diagPtr->flushMsg();
  }

  if (_maxStopoversPermittedUnlimited)
  {
    ptf.maxStopoversPermitted() = PaxTypeFare::MAX_STOPOVERS_UNLIMITED;
  }
  else
  {
    ptf.maxStopoversPermitted() = _maxStopoversPermitted;
  }

  return tse::SOFTPASS;
}

// called by PricingMarketRuleController
Record3ReturnTypes
StopoversInfoWrapper::processResults(PricingTrx& trx,
                                     FarePath& fp,
                                     const PricingUnit& pu,
                                     const FareUsage& fu,
                                     const bool processCharges)
{
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic308))
  {
    if (diagPtr->parseFareMarket(trx, *(fu.paxTypeFare()->fareMarket())))
    {
      factory = DCFactory::instance();
      diagPtr = factory->create(trx);
      diagPtr->enable(Diagnostic308);
      diagEnabled = true;
    }
  }

  bool anyMaxExceed = false;

  if (needToProcessResults())
  {
    anyMaxExceed = _stopoverMaxExceeed.newCheckMaxExceed(pu, trx, _stopoversTravelSegWrappers,
                                                         fu);
  }

  if (UNLIKELY(diagEnabled))
  {
    const PaxTypeFare* ptf = fu.paxTypeFare();
    const FareMarket* fm = nullptr;
    if (ptf)
    {
      fm = ptf->fareMarket();
    }

    (*diagPtr) << "CATEGORY 08 - STOPOVERS DIAGNOSTICS" << endl;
    (*diagPtr) << "PHASE: PRICING UNIT FINAL VALIDATION" << endl;
    if (fm)
    {
      (*diagPtr) << fm->origin()->loc() << " " << fm->destination()->loc() << " "
                 << ptf->fareClass();
    }
    if (fu.isInbound())
    {
      (*diagPtr) << "  .IN. ";
    }
    else
    {
      (*diagPtr) << "  .OUT.";
    }
    (*diagPtr) << endl << " " << endl;

    std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();

    int16_t fcNum = 0;

    for (; fuI != fuEnd; ++fuI)
    {
      ++fcNum;

      const FareUsage* fareUsage = *fuI;
      const PaxTypeFare* ptf = fareUsage->paxTypeFare();

      (*diagPtr) << "FARECOMP-" << fcNum << " : " << setw(12) << std::left << ptf->fareClass()
                 << std::right << "   ";

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
        (*diagPtr) << setw(3) << airSegCount - 1 << " OFFPOINTS";
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
                 << ptf->fareMarket()->destination()->loc() << endl;

      for (tsI = fareUsage->travelSeg().begin(); tsI != tsEnd; ++tsI)
      {
        const TravelSeg* ts = *tsI;
        const AirSeg* as = dynamic_cast<const AirSeg*>(ts);

        if (as)
        {
          (*diagPtr) << setw(3) << right << fp.itin()->segmentOrder(ts) << " " << setw(3) << left
                     << as->carrier() << setw(4) << right << as->flightNumber() << left << " "
                     << ts->departureDT().dateToString(DDMMM, "") << " " << ts->origin()->loc()
                     << ts->destination()->loc() << "  " << ts->departureDT().timeToString(HHMM, "")
                     << "  " << ts->arrivalDT().timeToString(HHMM, "") << "   ";

          if (needToProcessResults())
          {
            StopoversTravelSegWrapperMapCI stswIter = _stopoversTravelSegWrappers.find(ts);

            if (stswIter != _stopoversTravelSegWrappers.end())
            {
              const StopoversTravelSegWrapper& stsw = (*stswIter).second;

              if (stsw.isStopover())
              {
                (*diagPtr) << "SO ";
              }
              else
              {
                (*diagPtr) << "   ";
              }

              if (stsw.passedValidation() && checkArnkMatch(stsw))
              {
                (*diagPtr) << "PASS";
              }
              else if (anyMaxExceed)
              {
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
              else if (stsw.passedByLeastRestrictive())
              {
                (*diagPtr) << " LEAST REST";
              }
              else if (stsw.maxSOExceeded() == TOTAL_MAX_EXECEED)
              {
                (*diagPtr) << " FAIL MAX EXCEED";
              }
              else if (stsw.maxSOExceeded() == OUT_MAX_EXECEED)
              {
                (*diagPtr) << " FAIL MAX OUT. EXCEED";
              }
              else if (stsw.maxSOExceeded() == IN_MAX_EXECEED)
              {
                (*diagPtr) << " FAIL MAX IN. EXCEED";
              }
              else if (stsw.maxSOExceeded() == TO_MANY_SO)
              {
                (*diagPtr) << "FAIL TO MANY STOPOVER";
              }
              else if (!anyMaxExceed)
              {
                (*diagPtr) << " NO MATCH";
              }
            }
            else
            {
              (*diagPtr) << "   ----";
            }
          }
          (*diagPtr) << endl;
        }
        else
        {
          (*diagPtr) << "    ARNK" << endl;
        }
      }
      (*diagPtr) << " " << endl;
    }
    (*diagPtr) << " " << endl;
  }

  bool needToProcessCharges = false;
  bool isCmdPricing = false;

  if (UNLIKELY(((fu.paxTypeFare()->isFareByRule() || !fu.cat25Fare()) && fu.paxTypeFare()->isCmdPricing()) ||
      (fu.cat25Fare() && fu.cat25Fare()->isCmdPricing())))
  {
    isCmdPricing = true;
  }

  if ((needToProcessResults() && (!checkAllPassed())) ||
      (!applyLeastRestrictiveProvision() && anyMaxExceed))
  {
    if (isCmdPricing)
    {
      if (UNLIKELY(diagEnabled))
      {
        (*diagPtr) << "STOPOVERS : FAIL - PROCESS CHARGES FOR COMMAND PRICING" << endl;
        diagPtr->printLine();
        diagPtr->flushMsg();
      }
      needToProcessCharges = true;
    }
    else
    {
      if (UNLIKELY(diagEnabled))
      {
        (*diagPtr) << "STOPOVERS : FAIL - SEE ABOVE FOR FAILED SEGMENTS" << endl;

        diagPtr->printLine();
        diagPtr->flushMsg();
      }
      return tse::FAIL;
    }
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.begin();
  StopoversTravelSegWrapperMapI tswIterEnd = _stopoversTravelSegWrappers.end();

  for (; tswIter != tswIterEnd; ++tswIter)
  {
    StopoversTravelSegWrapper& stsw = (*tswIter).second;

    if (stsw.isStopover())
    {
      FareUsage* fu1 = stsw.fareUsage();

      if (&fu == fu1)
      {
        if (UNLIKELY(needToProcessCharges && (!stsw.passedValidation() || !checkArnkMatch(stsw))))
          continue;
      }
      if (stsw.isFareRuleMatch())
      {
        VCTR& vctr = fu1->stopoversMatchingVCTR();
        vctr = stsw.ruleVCTRMatch();
      }
      else
      {
        VCTR& vctr = fu1->stopoversMatchingGeneralRuleVCTR();
        vctr = stsw.ruleVCTRMatch();
      }
    }
  }

  if (needToProcessResults() && processCharges)
  {
    processSurcharges(trx, fp, fu, diagPtr, isCmdPricing);
  }

  if (UNLIKELY(isCmdPricing))
  {
    setMostRestrictiveMaxStop(const_cast<PricingUnit*>(&pu), _soInfo);
  }

  if (UNLIKELY(needToProcessCharges))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " " << endl << "STOPOVERS : FAIL - SEE ABOVE FOR FAILED SEGMENTS" << endl;

      diagPtr->printLine();
      diagPtr->flushMsg();
    }
    return tse::FAIL;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << " " << endl << "STOPOVERS : PASS - FINAL VALIDATION" << endl;

    diagPtr->printLine();
    diagPtr->flushMsg();
  }
  return tse::PASS;
}

void
StopoversInfoWrapper::processSurcharges(PricingTrx& trx,
                                        FarePath& fp,
                                        const FareUsage& fareUsage,
                                        DiagCollector* diagPtr,
                                        bool cmdPricing)
{
  CurrencyConversionFacade ccFacade;

  if (UNLIKELY(diagPtr))
  {
    (*diagPtr) << "---STOPOVER SURCHARGES---" << endl;
  }

  bool forcedRecord3Fail = false;
  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.begin();
  StopoversTravelSegWrapperMapI tswIterEnd = _stopoversTravelSegWrappers.end();
  for (; tswIter != tswIterEnd; ++tswIter)
  {
    StopoversTravelSegWrapper& stsw = (*tswIter).second;
    if (stsw.isStopover())
    {
      TravelSeg* ts = stsw.travelSeg();
      FareUsage* fu = stsw.fareUsage();

      // Add the TravelSeg* to the stopovers set in FareUsage.
      fu->stopovers().insert(ts);
      if (UNLIKELY(diagPtr))
      {
        AirSeg* as = dynamic_cast<AirSeg*>(ts);
        (*diagPtr) << setw(3) << right << fp.itin()->segmentOrder(ts) << " ";

        if (as)
        {
          (*diagPtr) << setw(3) << left << as->carrier() << setw(4) << right << as->flightNumber()
                     << left << " ";
        }
        else
        {
          (*diagPtr) << setw(8) << left << " ";
        }

        (*diagPtr) << ts->departureDT().dateToString(DDMMM, "") << " " << ts->origin()->loc()
                   << ts->destination()->loc() << "  " << ts->departureDT().timeToString(HHMM, "")
                   << "  " << ts->arrivalDT().timeToString(HHMM, "") << "  ";
      }

      // Checking whether surchage is valid since we may forced pass travel segment with arunk
      if (UNLIKELY(stsw.arunkTravelSeg() && !checkArnkMatch(stsw) && !cmdPricing))
        forcedRecord3Fail = true;
      else
        forcedRecord3Fail = false;

      StopoversTravelSegWrapper::SurchargeListCI scIter = stsw.surcharges().begin();
      StopoversTravelSegWrapper::SurchargeListCI scIterEnd = stsw.surcharges().end();

      if (UNLIKELY(diagPtr))
      {
        if (stsw.surcharges().empty() || forcedRecord3Fail)
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

      // No need to process surcharge since forced Arunk did not match
      if (UNLIKELY(forcedRecord3Fail))
      {
        if (UNLIKELY(diagPtr))
          (*diagPtr) << endl;

        continue;
      }

      if (fu == &fareUsage)
      {
        if (!_infoSegsDirectionalityCharge.empty())
        {
          SoMapSegmentDirectionalityChargeI iterI = _infoSegsDirectionalityCharge.begin();
          SoMapSegmentDirectionalityChargeI iterE = _infoSegsDirectionalityCharge.end();
          for (; iterI != iterE; ++iterI)
          {
            if (fu->stopoverByDir().find(iterI->first) == fu->stopoverByDir().end())
            {
              FareUsage::StopoverInfoByDirectionMap::value_type value(iterI->first, iterI->second);
              fu->stopoverByDir().insert(value);
            }
          }
        }

        for (; scIter != scIterEnd; ++scIter)
        {
          const StopoversTravelSegWrapper::Surcharge& sc = *scIter;

          // Add surcharges to the FareUsage

          FareUsage::StopoverSurcharge* sos = nullptr;
          trx.dataHandle().get(sos);

          if (LIKELY(sos))
          {
            if (!sc.currency().equalToConst("NUC"))
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
                    logger,
                    "StopoversInfoWrapper::processSurcharges(): Error converting currency");
                return;
              }

              Money calcAmt(convertedAmt, calcCur);

              sos->amount() = convertedAmt;
              sos->currencyCode() = calcCur;
              sos->noDecimals() = calcAmt.noDec(trx.ticketingDate());
              sos->unconvertedAmount() = sc.amount();
              sos->unconvertedCurrencyCode() = sc.currency();
              sos->unconvertedNoDecimals() = sc.noDec();
              sos->travelSeg() = ts;
              sos->isSegmentSpecific() = sc.isSegmentSpecific();
              sos->isCharge1() = sc.isCharge1();
              sos->isFirstAmt() = sc.isFirstAmt();
              sos->matchRuleItemNo() = stsw.ruleItemMatch();
              sos->matchRuleVCTR() = stsw.ruleVCTRMatch();
              sos->chargeFromFirstInbound() = sc.chargeFromFirstInbound();

              fu->addSurcharge(sos, nuc.value());
              fp.increaseTotalNUCAmount(nuc.value());
              fp.plusUpAmount() += nuc.value();
            }
            else
            {
              sos->amount() = sc.amount();
              sos->currencyCode() = sc.currency();
              sos->noDecimals() = sc.noDec();
              sos->unconvertedAmount() = sc.amountLocCur();
              sos->unconvertedCurrencyCode() = sc.currencyLocCur();
              sos->unconvertedNoDecimals() = sc.noDecLocCur();
              sos->travelSeg() = ts;
              sos->isSegmentSpecific() = sc.isSegmentSpecific();
              sos->isCharge1() = sc.isCharge1();
              sos->isFirstAmt() = sc.isFirstAmt();
              sos->matchRuleItemNo() = stsw.ruleItemMatch();
              sos->matchRuleVCTR() = stsw.ruleVCTRMatch();
              sos->chargeFromFirstInbound() = sc.chargeFromFirstInbound();

              fu->addSurcharge(sos, sc.amount());
              fp.increaseTotalNUCAmount(sc.amount());
              fp.plusUpAmount() += sc.amount();
            }
            fp.plusUpFlag() = true;
          }
          if (UNLIKELY(diagPtr))
          {
            // lint --e{413}
            if (sos->amount() == 0)
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

              (*diagPtr) << sos->amount() << sos->currencyCode();

              if (sos->currencyCode() != sos->unconvertedCurrencyCode())
              {
                (*diagPtr) << ":" << sos->unconvertedAmount() << sos->unconvertedCurrencyCode();
              }
            }
          }
        }
      }

      if (UNLIKELY(diagPtr))
        (*diagPtr) << endl;
    }
  }
}

bool
StopoversInfoWrapper::convertDiscountToNUCamounts(PricingTrx& trx,
                                                  MoneyAmount& firstAmt,
                                                  CurrencyCode& firstCur,
                                                  int16_t& firstNoDec,
                                                  MoneyAmount& addAmt,
                                                  CurrencyCode& addCur,
                                                  int16_t& addNoDec,
                                                  PaxTypeFare& fare,
                                                  double& multiplier) const
{
  const DateTime& ticketingDate = trx.ticketingDate();

  NUCCurrencyConverter nucConv;

  Money firstAmt1NUC(0, NUC);
  Money addAmt1NUC(0, NUC);
  Money firstAmt1(firstAmt, firstCur);
  Money addAmt1(addAmt, addCur);

  if (firstCur == NUC)
  {
    firstAmt1NUC.value() = firstAmt;
  }
  else
  {
    CurrencyConversionRequest firstAmt1ConvReq(firstAmt1NUC,
                                               firstAmt1,
                                               trx.getRequest()->ticketingDT(),
                                               *(trx.getRequest()),
                                               trx.dataHandle(),
                                               fare.isInternational());

    if (!nucConv.convert(firstAmt1ConvReq, nullptr))
    {
      LOG4CXX_ERROR(
          logger,
          "Rules.StopoversInfoWrapper::convertToNUCamounts(): Error converting currency to NUC");
      return false;
    }
  }

  if (addCur == NUC)
  {
    addAmt1NUC.value() = addAmt;
  }
  else
  {
    CurrencyConversionRequest addAmt1ConvReq(addAmt1NUC,
                                             addAmt1,
                                             trx.getRequest()->ticketingDT(),
                                             *(trx.getRequest()),
                                             trx.dataHandle(),
                                             fare.isInternational());

    if (!nucConv.convert(addAmt1ConvReq, nullptr))
    {
      LOG4CXX_ERROR(
          logger,
          "Rules.StopoversInfoWrapper::convertToNUCamounts(): Error converting currency to NUC");
      return false;
    }
  }

  firstAmt = firstAmt1NUC.value();
  firstCur = firstAmt1NUC.code();
  firstNoDec = firstAmt1NUC.noDec(ticketingDate);
  addAmt = addAmt1NUC.value();
  addCur = addAmt1NUC.code();
  addNoDec = addAmt1NUC.noDec(ticketingDate);

  CurrencyUtil::truncateNUCAmount(firstAmt);
  CurrencyUtil::truncateNUCAmount(addAmt);

  firstAmt = firstAmt * multiplier;
  CurrencyUtil::truncateNUCAmount(firstAmt);
  addAmt = addAmt * multiplier;
  CurrencyUtil::truncateNUCAmount(addAmt);
  return true;
}

void
StopoversInfoWrapper::setMaxExceeded(TravelSeg* ts, FareUsage* fu, const int16_t& val) const
{
  if (!ts)
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::setMaxExceeded(): TravelSeg=0");
    return;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (tswIter == _stopoversTravelSegWrappers.end())
  {
    StopoversTravelSegWrapper stsw;
    stsw.travelSeg() = ts;
    stsw.fareUsage() = fu;

    StopoversTravelSegWrapperMap::value_type value(ts, stsw);

    tswIter = (_stopoversTravelSegWrappers.insert(value)).first;
  }

  StopoversTravelSegWrapper& stsw = (*tswIter).second;
  if (val > 0)
    stsw.maxSOExceeded() = val;

  _needToProcessResults = true;
}

bool
StopoversInfoWrapper::checkMatched(TravelSeg* ts) const
{
  if (!ts)
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::checkMatched(): TravelSeg =0");
    return false;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (tswIter != _stopoversTravelSegWrappers.end())
    return tswIter->second.isMatched();

  return false;
}

void
StopoversInfoWrapper::setMatched(TravelSeg* ts,
                                 FareUsage* fu,
                                 StopoversTravelSegWrapper::MatchType val) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::setMatched(): TravelSeg=0");
    return;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (UNLIKELY(tswIter == _stopoversTravelSegWrappers.end()))
  {
    StopoversTravelSegWrapper stsw;
    stsw.travelSeg() = ts;
    stsw.fareUsage() = fu;

    StopoversTravelSegWrapperMap::value_type value(ts, stsw);
    tswIter = (_stopoversTravelSegWrappers.insert(value)).first;
  }

  StopoversTravelSegWrapper& stsw = (*tswIter).second;
  stsw.setMatched(val);
  _needToProcessResults = true;
}

void
StopoversInfoWrapper::setMostRestrictiveMaxStop(PricingUnit* pu, const StopoversInfo* soInfo)
{
  int16_t numStopsMax = 0;
  bool numStopsUnlimited = false;

  if (soInfo->noStopsMax().empty() || soInfo->noStopsMax() == Stopovers::NUM_STOPS_UNLIMITED)
  {
    numStopsUnlimited = true;
  }
  else
  {
    numStopsMax = atoi(soInfo->noStopsMax().c_str());
  }

  if (!numStopsUnlimited &&
      (pu->mostRestrictiveMaxStop() < 0 || pu->mostRestrictiveMaxStop() > numStopsMax))
  {
    pu->mostRestrictiveMaxStop() = numStopsMax;
  }
}

bool
StopoversInfoWrapper::addSurcharge(PricingTrx& trx,
                                   TravelSeg* ts,
                                   FareUsage* fu,
                                   const FarePath& fp,
                                   const StopoversInfoSeg* soInfoSeg,
                                   const bool isSegmentSpecific,
                                   const bool forceAddAmt,
                                   const bool chargeFromFirstInbound,
                                   const bool forceFirstAmt,
                                   const bool doAdd) const
{
  if (UNLIKELY(!ts))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::addSurcharge(): TravelSeg =0");
    return false;
  }

  if (UNLIKELY(!fu))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::addSurcharge(): FareUsage =0");
    return false;
  }

  // lint -e{530}
  const PaxType* paxType = fp.paxType();

  if (UNLIKELY(!paxType))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::addSurcharge(): FarePath.paxType() =0");
    return false;
  }

  StopoversTravelSegWrapperMapI tswIter = _stopoversTravelSegWrappers.find(ts);
  if (UNLIKELY(tswIter == _stopoversTravelSegWrappers.end()))
  {
    StopoversTravelSegWrapper stsw;
    stsw.travelSeg() = ts;
    stsw.fareUsage() = fu;

    StopoversTravelSegWrapperMap::value_type value(ts, stsw);

    tswIter = (_stopoversTravelSegWrappers.insert(value)).first;
  }

  StopoversTravelSegWrapper& stsw = (*tswIter).second;

  if (UNLIKELY(!stsw.fareUsage()))
  {
    stsw.fareUsage() = fu;
  }

  PaxTypeFare* fare = fu->paxTypeFare();

  if (UNLIKELY(!fare))
  {
    LOG4CXX_ERROR(logger, "Rules.StopoversInfoWrapper::addSurcharge(): PaxTypeFare = 0");
    return false;
  }

  int16_t chargeNum;
  MoneyAmount firstAmt, firstAmtLc;
  CurrencyCode firstCur, firstCurLc;
  int16_t firstNoDec, firstNoDecLc;
  MoneyAmount addAmt, addAmtLc;
  CurrencyCode addCur, addCurLc;
  int16_t addNoDec, addNoDecLc;

  if (UNLIKELY(!selectApplCharge(trx,
                        chargeNum,
                        firstAmtLc,
                        firstCurLc,
                        firstNoDecLc,
                        addAmtLc,
                        addCurLc,
                        addNoDecLc,
                        *fare,
                        *paxType)))
  {
    return false;
  }

  const Indicator appl = _soInfo->charge1Appl();

  firstAmt = firstAmtLc;
  firstCur = firstCurLc;
  firstNoDec = firstNoDecLc;
  addAmt = addAmtLc;
  addCur = addCurLc;
  addNoDec = addNoDecLc;
  double multiplier = 0.0;

  if (doAdd)
  {
    if (UNLIKELY(!applyPaxTypeDiscount(trx,
                              firstAmtLc,
                              firstCurLc,
                              firstNoDecLc,
                              addAmtLc,
                              addCurLc,
                              addNoDecLc,
                              *fu,
                              *paxType,
                              appl,
                              multiplier)))
    {
      return false;
    }

    if (multiplier != 0.0 && (firstAmt != firstAmtLc || addAmt != addAmtLc))
    {
      if (firstAmtLc == 0 && addAmtLc == 0)
      {
        multiplier = 0.0;
      }
      if (!convertDiscountToNUCamounts(
              trx, firstAmt, firstCur, firstNoDec, addAmt, addCur, addNoDec, *fare, multiplier))
      {
        return false;
      }
    }
  } // doAdd

  FMDirection dir = fu->isInbound() ? FMDirection::INBOUND : FMDirection::OUTBOUND;

  if (chargeNum == 1)
  {
    if ((forceAddAmt) ||
        (!forceFirstAmt && (!_charge1FirstUnlimited) &&
         (_charge1FirstAmtCount == _charge1FirstNo) && !numberOfChargesOneOrTwoApplied(chargeNum)))
    {
      if (_charge1AddUnlimited || (_charge1AddAmtCount < _charge1AddNo) ||
          (soInfoSeg && checkIfChargeIsApplicable(soInfoSeg, chargeNum, dir, forceAddAmt)))
      {
        if (!addCur.empty())
        {
          ++_charge1AddAmtCount;

          if (doAdd)
          {
            StopoversTravelSegWrapper::Surcharge chrg(addAmt,
                                                      addCur,
                                                      addNoDec,
                                                      isSegmentSpecific,
                                                      true,
                                                      false,
                                                      addAmtLc,
                                                      addCurLc,
                                                      addNoDecLc,
                                                      chargeFromFirstInbound);

            stsw.surcharges().push_back(chrg);
          }
          updateDirectionalityChargeIfApplicable(trx, soInfoSeg, chargeNum, dir, forceAddAmt);
        }
      }
    }
    else
    {
      if (!firstCur.empty())
      {
        ++_charge1FirstAmtCount;

        if (doAdd)
        {
          StopoversTravelSegWrapper::Surcharge chrg(firstAmt,
                                                    firstCur,
                                                    firstNoDec,
                                                    isSegmentSpecific,
                                                    true,
                                                    true,
                                                    firstAmtLc,
                                                    firstCurLc,
                                                    firstNoDecLc,
                                                    chargeFromFirstInbound);

          stsw.surcharges().push_back(chrg);
        }
        updateDirectionalityChargeIfApplicable(trx, soInfoSeg, chargeNum, dir);
      }
    }
  }
  else
  {
    if ((forceAddAmt) ||
        (!forceFirstAmt && (!_charge2FirstUnlimited) &&
         (_charge2FirstAmtCount == _charge2FirstNo) && !numberOfChargesOneOrTwoApplied(chargeNum)))
    {
      if (_charge2AddUnlimited || (_charge2AddAmtCount < _charge2AddNo) ||
          (soInfoSeg && checkIfChargeIsApplicable(soInfoSeg, chargeNum, dir, forceAddAmt)))
      {
        if (!addCur.empty())
        {
          ++_charge2AddAmtCount;

          if (doAdd)
          {
            StopoversTravelSegWrapper::Surcharge chrg(addAmt,
                                                      addCur,
                                                      addNoDec,
                                                      isSegmentSpecific,
                                                      false,
                                                      false,
                                                      addAmtLc,
                                                      addCurLc,
                                                      addNoDecLc,
                                                      chargeFromFirstInbound);

            stsw.surcharges().push_back(chrg);
          }
          updateDirectionalityChargeIfApplicable(trx, soInfoSeg, chargeNum, dir, forceAddAmt);
        }
      }
    }
    else
    {
      if (!firstCur.empty())
      {
        ++_charge2FirstAmtCount;

        if (doAdd)
        {
          StopoversTravelSegWrapper::Surcharge chrg(firstAmt,
                                                    firstCur,
                                                    firstNoDec,
                                                    isSegmentSpecific,
                                                    false,
                                                    true,
                                                    firstAmtLc,
                                                    firstCurLc,
                                                    firstNoDecLc,
                                                    chargeFromFirstInbound);

          stsw.surcharges().push_back(chrg);
        }
        updateDirectionalityChargeIfApplicable(trx, soInfoSeg, chargeNum, dir);
      }
    }
  }
  _needToProcessResults = true;
  return true;
}

bool
StopoversInfoWrapper::checkIfChargeIsApplicable(const StopoversInfoSeg* soInfoSeg,
                                                const int16_t chargeNum,
                                                const FMDirection dir,
                                                const bool forceAddAmt) const
{
  if (chargeNum < 1 || chargeNum > 2)
    return false;

  if (!_infoSegsDirectionalityCharge.empty() &&
      (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_OUT ||
       soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_IN ||
       soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_EITHER))
  {
    SoMapSegmentDirectionalityChargeIC iter = _infoSegsDirectionalityCharge.find(soInfoSeg);
    if (iter == _infoSegsDirectionalityCharge.end())
      return true;

    if (!iter->second)
      return false;

    if (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_EITHER)
    {
      if (iter->second->direction() == 'O' && dir != FMDirection::OUTBOUND)
        return false;
      if (iter->second->direction() == 'I' && dir == FMDirection::OUTBOUND)
        return false;
    }
    if (chargeNum == 1)
    {
      if (forceAddAmt)
      {
        if (iter->second->charge1_FirstAdd() >= _charge1AddNo)
          return false;
      }
      else if (iter->second->charge1_FirstAmt() >= _charge1FirstNo)
        return false;
    }
    else
    {
      if (forceAddAmt)
      {
        if (iter->second->charge2_FirstAdd() >= _charge2AddNo)
          return false;
      }
      else if (iter->second->charge2_FirstAmt() >= _charge2FirstNo)
        return false;
    }
    return true;
  }
  return false;
}

void
StopoversInfoWrapper::updateDirectionalityChargeIfApplicable(PricingTrx& trx,
                                                             const StopoversInfoSeg* soInfoSeg,
                                                             const int16_t chargeNum,
                                                             const FMDirection dir,
                                                             const bool forceAddAmt) const
{
  if (!soInfoSeg)
    return;

  if (chargeNum < 1 || chargeNum > 2)
    return;

  if (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_OUT ||
      soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_IN ||
      soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_EITHER)
  {
    SoMapSegmentDirectionalityChargeIC iter = _infoSegsDirectionalityCharge.find(soInfoSeg);
    if (iter == _infoSegsDirectionalityCharge.end())
    {
      ChargeSODirect* chDirSo = nullptr;
      trx.dataHandle().get(chDirSo);
      accumulateDirCharge(chDirSo, chargeNum, forceAddAmt, dir);
      _infoSegsDirectionalityCharge.insert(std::make_pair(soInfoSeg, chDirSo));
    }
    else
    {
      if (!iter->second)
        return;

      ChargeSODirect* chDirSo = iter->second;
      accumulateDirCharge(chDirSo, chargeNum, forceAddAmt, dir);
    }
  }
}

void
StopoversInfoWrapper::accumulateDirCharge(ChargeSODirect* chDirSo,
                                          const int16_t chargeNum,
                                          const bool forceAddAmt,
                                          const FMDirection dir) const
{
  chDirSo->ruleItemNumber() = itemNo();
  if (dir == FMDirection::OUTBOUND)
    chDirSo->direction() = 'O';
  else
    chDirSo->direction() = 'I';

  if (chargeNum == 1)
  {
    if (forceAddAmt)
    {
      ++(chDirSo->charge1_FirstAdd());
    }
    else
    {
      ++(chDirSo->charge1_FirstAmt());
    }
  }
  else
  {
    if (forceAddAmt)
    {
      ++(chDirSo->charge2_FirstAdd());
    }
    else
    {
      ++(chDirSo->charge2_FirstAmt());
    }
  }
}

bool
StopoversInfoWrapper::numberOfChargesOneOrTwoApplied(const int16_t chargeNum) const
{
  if (!_infoSegsDirectionalityCharge.empty())
  {
    SoMapSegmentDirectionalityChargeIC iter = _infoSegsDirectionalityCharge.begin();
    int32_t count = 0;
    for (; iter != _infoSegsDirectionalityCharge.end(); ++iter)
    {
      if (iter->second->ruleItemNumber() == itemNo())
      {
        if (chargeNum == 1)
        {
          if (iter->second->charge1_FirstAmt() > 0)
            count += iter->second->charge1_FirstAmt();
        }
        else
        {
          if (iter->second->charge2_FirstAmt() > 0)
            count += iter->second->charge2_FirstAmt();
        }
      }
    }

    if (chargeNum == 1)
      return (count < _charge1FirstNo);
    else
      return (count < _charge2FirstNo);
  }
  return false;
}

// Check forceArnkPass tvl for final validation
bool
StopoversInfoWrapper::checkArnkMatch(const StopoversTravelSegWrapper& stsw) const
{
  if (!stsw.arunkTravelSeg() || !stsw.travelSeg())
    return true;

  StopoversTravelSegWrapperMapCI iter = _stopoversTravelSegWrappers.find(stsw.arunkTravelSeg());

  if (iter == _stopoversTravelSegWrappers.end() || iter->second.ruleItemMatch() == 0)
  {
    // FINAL-ARUNK-VALIDATION: has arunk but did not match
    return false;
  }

  // FINAL-ARUNK-VALIDATION: ARNK MATCH
  return true;
}

// return true only if it is forced pass or itemNo(s) did not match (inclusive check).
bool
StopoversInfoWrapper::checkArunkForcedPass(const TravelSeg* ts) const
{
  if (!ts)
    return false;

  StopoversTravelSegWrapperMapI segIter = _stopoversTravelSegWrappers.find(ts);
  if (segIter == _stopoversTravelSegWrappers.end() || !segIter->second.arunkTravelSeg())
    return false;

  // it is forced-passed...
  uint32_t itemNo = 0;
  StopoversTravelSegWrapper& stsw = (*segIter).second;
  StopoversTravelSegWrapperMapI arunkIter = _stopoversTravelSegWrappers.find(stsw.arunkTravelSeg());
  if (arunkIter != _stopoversTravelSegWrappers.end())
  {
    // Arunk matched.
    itemNo = arunkIter->second.ruleItemMatch();
    if (itemNo && itemNo == stsw.ruleItemMatch())
      return false;
  }

  return true;
}

void
StopoversInfoWrapper::sumNumStopoversMax(const uint32_t relationalInd, const StopoversInfo* soInfo)
{
  _stopoverMaxExceeed.sumNumStopoversMax(relationalInd, soInfo);
}

} // tse
