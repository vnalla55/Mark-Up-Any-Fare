//-------------------------------------------------------------------
//
//  File:
//  Created:     June 26, 2007
//  Authors:     Simon Li
//
//  Updates:
//
//  Copyright Sabre 2007
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

#include "RexPricing/CheckTrxError.h"

#include "Common/FallbackUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ReissueOptionsMap.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Rules/RuleConst.h"

#include <exception>

namespace tse
{

CheckTrxError::CheckTrxError(RexBaseTrx& trx)
  : _trx(trx),
    _subjectCategory((trx.excTrxType() == PricingTrx::AF_EXC_TRX)
                         ? RuleConst::VOLUNTARY_REFUNDS_RULE
                         : RuleConst::VOLUNTARY_EXCHANGE_RULE),
    _missingR3Error((trx.excTrxType() == PricingTrx::AF_EXC_TRX)
                        ? ErrorResponseException::UNABLE_TO_MATCH_REFUND_RULES
                        : ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES)
{
}

void
CheckTrxError::process() const
{
  if (_trx.exchangeItin().empty())
    throw ErrorResponseException(ErrorResponseException::UNABLE_TO_MATCH_FARE);

  if (_trx.trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE)
    processRepriceExcPhase();

  else if (_trx.trxPhase() == RexBaseTrx::MATCH_EXC_RULE_PHASE)
    processMatchExcRulePhase();
}

void
CheckTrxError::processRepriceExcPhase() const
{
  std::vector<FareCompInfo*>::const_iterator fcIter =
      _trx.exchangeItin().front()->fareComponent().begin();
  const std::vector<FareCompInfo*>::const_iterator fcIterEnd =
      _trx.exchangeItin().front()->fareComponent().end();

  for (; fcIter != fcIterEnd; ++fcIter)
  {
    if (!fareCompMatchedFare(**fcIter))
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_MATCH_FARE);
  }

  fcIter = _trx.exchangeItin().front()->fareComponent().begin();

  for (; fcIter != fcIterEnd; ++fcIter)
  {
    if (!fareCompMatchedReissueRules(**fcIter))
      throw ErrorResponseException(_missingR3Error);
  }
}

void
CheckTrxError::processMatchExcRulePhase() const
{
  if (_trx.exchangeItin().front()->farePath().empty() ||
      _trx.exchangeItin().front()->farePath().front()->pricingUnit().empty())
    throw ErrorResponseException(ErrorResponseException::UNABLE_TO_MATCH_FARE);

  std::vector<PricingUnit*>::const_iterator puIter =
      _trx.exchangeItin().front()->farePath().front()->pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIterEnd =
      _trx.exchangeItin().front()->farePath().front()->pricingUnit().end();

  for (; puIter != puIterEnd; puIter++)
  {
    std::vector<FareUsage*>::const_iterator fuIter = (*puIter)->fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuIterEnd = (*puIter)->fareUsage().end();

    for (; fuIter != fuIterEnd; fuIter++)
    {
      if (_trx.excTrxType() == PricingTrx::AF_EXC_TRX)
        processPaxTypeFareForRefund((*fuIter)->paxTypeFare());
      else
        processPaxTypeFareForReissue((*fuIter)->paxTypeFare());
    }
  }
}

void
CheckTrxError::getBaseFare(const PaxTypeFare*& ptf) const
{
  if (ptf->isFareByRule() && !ptf->isSpecifiedFare())
  {
    const FBRPaxTypeFareRuleData* fbrPaxTypeFareData = ptf->getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (fbrPaxTypeFareData != nullptr)
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFareData->ruleItemInfo());

      if ((fbrItemInfo != nullptr) && fbrItemInfo->ovrdcat33() == 'B')
        ptf = fbrPaxTypeFareData->baseFare();
    }
  }
}

void
CheckTrxError::processPaxTypeFareForRefund(const PaxTypeFare* ptf) const
{
  getBaseFare(ptf);

  RefundPricingTrx& refundTrx = static_cast<RefundPricingTrx&>(_trx);
  if (refundTrx.refundOptions(ptf) == 0)
  {
    bool qualifyingCategoryFailed =
        refundTrx.reachedR3Validation().find(ptf) == refundTrx.reachedR3Validation().end();

    throw ErrorResponseException(
        qualifyingCategoryFailed ? _missingR3Error : ErrorResponseException::REFUND_RULES_FAIL);
  }
}

void
CheckTrxError::processPaxTypeFareForReissue(const PaxTypeFare* ptf) const
{
  std::vector<ReissueOptions::R3WithDateRange> r3v;
  static_cast<RexPricingTrx&>(_trx).reissueOptions().getRec3s(ptf, r3v);

  if (r3v.empty())
  {
    FareCompInfo* fcInfo = _trx.exchangeItin().front()->findFareCompInfoInCache(ptf->fareMarket());
    if (fcInfo == nullptr)
      fcInfo = _trx.exchangeItin().front()->findFareCompInfo(ptf->fareMarket());

    if (fcInfo)
    {
      const std::map<const PaxTypeFare*, bool>& failByPrevReissuedMap =
          fcInfo->failByPrevReissued();
      std::map<const PaxTypeFare*, bool>::const_iterator iter = failByPrevReissuedMap.find(ptf);
      if (iter != failByPrevReissuedMap.end() && iter->second)
        throw ErrorResponseException(ErrorResponseException::NUMBER_OF_REISSUES_RESTRICTED);
    }

    throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
  }
}

void
CheckTrxError::checkPermutations() const
{
  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
      static_cast<RexPricingTrx&>(_trx).processTagPermutations().empty())
    throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
}

bool
CheckTrxError::fareCompMatchedFare(const FareCompInfo& fc) const
{
  return fc.fareMarket() && !fc.getMatchedFares().empty();
}

namespace
{

struct IsSubjectCategoryValid
{
  IsSubjectCategoryValid(uint16_t cat) : _cat(cat) {}

  bool operator()(const FareCompInfo::MatchedFare& fare) const
  {
    return fare.get()->isCategoryValid(_cat);
  }

protected:
  uint16_t _cat;
};

} // namespace

bool
CheckTrxError::fareCompMatchedReissueRules(const FareCompInfo& fc) const
{
  return std::find_if(fc.getMatchedFares().begin(),
                      fc.getMatchedFares().end(),
                      IsSubjectCategoryValid(_subjectCategory)) != fc.getMatchedFares().end();
}

void
CheckTrxError::multiItinProcess() const
{
  if (_trx.exchangeItin().front()->farePath().empty() ||
      _trx.exchangeItin().front()->farePath().front()->pricingUnit().empty())
    throw ErrorResponseException(ErrorResponseException::UNABLE_TO_MATCH_FARE);

  RexExchangeTrx& rexTrx = static_cast<RexExchangeTrx&>(_trx);

  std::vector<PricingUnit*>::const_iterator puIter =
      _trx.exchangeItin().front()->farePath().front()->pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIterEnd =
      _trx.exchangeItin().front()->farePath().front()->pricingUnit().end();

  for (; puIter != puIterEnd; puIter++)
  {
    std::vector<FareUsage*>::const_iterator fuIter = (*puIter)->fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuIterEnd = (*puIter)->fareUsage().end();

    for (; fuIter != fuIterEnd; fuIter++)
    {
      std::vector<ReissueOptions::R3WithDateRange> r3v;

      const PaxTypeFare* ptf = (*fuIter)->paxTypeFare();
      bool reissueOptionExistForAtLeastOneNewItin = false;
      bool numberForReissueRestrictedForAllNewItins = true;

      ExcItin* excItin = _trx.exchangeItin().front();
      FareCompInfo* fcInfo = excItin->findFareCompInfoInCache(ptf->fareMarket());
      if (fcInfo == nullptr)
        fcInfo = excItin->findFareCompInfo(ptf->fareMarket());

      for (uint16_t itinIndex = 0; itinIndex < _trx.newItin().size(); ++itinIndex)
      {
        _trx.setItinIndex(itinIndex);
        
        rexTrx.reissueOptions().getRec3s(ptf, r3v);

        if (!r3v.empty())
        {
          reissueOptionExistForAtLeastOneNewItin = true;
          numberForReissueRestrictedForAllNewItins = false;
        }
        else
        {
          rexTrx.itinStatus() = RexExchangeTrx::NUMBER_OF_REISSUES_RESTRICTED;
          if (fcInfo)
          {
            const FareCompInfo::FailByPrevReissued& failByPrevReissuedMap =
                fcInfo->failByPrevReissued(itinIndex);
            std::map<const PaxTypeFare*, bool>::const_iterator iter =
                failByPrevReissuedMap.find(ptf);
            if (!(iter != failByPrevReissuedMap.end() && iter->second))
            {
              numberForReissueRestrictedForAllNewItins = false;
            }
            else
              rexTrx.itinStatus() = RexExchangeTrx::REISSUE_RULES_FAIL;
          }
        }
      }
      if (numberForReissueRestrictedForAllNewItins)
        throw ErrorResponseException(ErrorResponseException::NUMBER_OF_REISSUES_RESTRICTED);
      if (!reissueOptionExistForAtLeastOneNewItin)
        throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
    }
  }
}

void
CheckTrxError::multiItinCheckPermutations() const
{
  RexExchangeTrx& rexTrx = static_cast<RexExchangeTrx&>(_trx);
  bool processTagPermutationsExistForAtLeastOneNewItin = false;
  for (uint16_t itinIndex = 0; itinIndex < _trx.newItin().size(); ++itinIndex)
  {
    _trx.setItinIndex(itinIndex);
    if (rexTrx.processTagPermutations().empty())
      rexTrx.itinStatus() = RexExchangeTrx::REISSUE_RULES_FAIL;
    else
      processTagPermutationsExistForAtLeastOneNewItin = true;
  }

  if (!processTagPermutationsExistForAtLeastOneNewItin)
    throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
}

} // tse
