// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------
#include "Pricing/MaximumPenaltyValidator.h"

#include "Common/Config/ConfigMan.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/Cat16MaxPenaltyCalculator.h"
#include "RexPricing/FarePathChangeDetermination.h"
#include "RexPricing/PenaltyCalculator.h"
#include "RexPricing/RefundDiscountApplier.h"
#include "RexPricing/RefundPermutationGenerator.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <boost/format.hpp>
#include <memory>

namespace tse
{
FALLBACK_DECL(sfrPenaltyCurrency)

MaximumPenaltyValidator::MaximumPenaltyValidator(PricingTrx& trx)
  : _trx(trx),
    _diagEnabled(_trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, "MAXPEN")),
    _failedFaresThreshold(TrxUtil::getMaxPenaltyFailedFaresThreshold(trx))
{
}

std::pair<bool, std::string>
MaximumPenaltyValidator::validateFarePath(FarePath& farePath) const
{
  std::pair<bool, std::string> result{true, std::string()};

  const MaxPenaltyInfo* maxPenalty = farePath.paxType()->maxPenaltyInfo();

  if (LIKELY(!maxPenalty || maxPenalty->_mode == smp::INFO))
  {
    // validation isn't necessary
    return result;
  }

  printDiagHeader(farePath.paxType()->paxType());

  MaxPenaltyStats& stats = farePath.itin()->maxPenaltyStats();
  MaximumPenaltyCalculator calculator(_trx, farePath);
  MaxPenaltyResponse::Fees changePenalty;
  MaxPenaltyResponse::Fees refundPenalty;

  if (maxPenalty->_changeFilter._query || maxPenalty->_changeFilter._maxFee)
  {
    changePenalty = calculator.changePenalty(maxPenalty->_changeFilter._departure,
                                             getPenaltyCurrency(maxPenalty->_changeFilter));
    result = validateFilter(maxPenalty->_changeFilter, changePenalty);
  }

  if (!result.first && maxPenalty->_mode == smp::AND)
  {
    printValidationResult(result);
    updateFailStats(stats, *maxPenalty);
    return result;
  }

  if (!(result.first && maxPenalty->_mode == smp::OR) &&
      (maxPenalty->_refundFilter._query || maxPenalty->_refundFilter._maxFee))
  {
    refundPenalty = calculator.refundPenalty(maxPenalty->_refundFilter._departure,
                                             getPenaltyCurrency(maxPenalty->_refundFilter));
    result = validateFilter(maxPenalty->_refundFilter, refundPenalty);
  }

  if (!result.first)
  {
    updateFailStats(stats, *maxPenalty);
  }

  printValidationResult(result);

  return result;
}

CurrencyCode
MaximumPenaltyValidator::getPenaltyCurrency(const MaxPenaltyInfo::Filter& filter) const
{
  if(!fallback::sfrPenaltyCurrency(&_trx))
  {
    if (filter._maxFee)
    {
      return filter._maxFee->code();
    }
    else if (!_trx.getOptions()->currencyOverride().empty())
    {
      return _trx.getOptions()->currencyOverride();
    }
    else if (!_trx.getRequest()->ticketingAgent()->currencyCodeAgent().empty())
    {
      return _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
    }
  }
  else
  {
    if (filter._maxFee)
    {
      return filter._maxFee->code();
    }
    else if (!_trx.getRequest()->ticketingAgent()->currencyCodeAgent().empty())
    {
      return _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
    }
    else if (!_trx.getOptions()->currencyOverride().empty())
    {
      return _trx.getOptions()->currencyOverride();
    }
  }

  throw ErrorResponseException(ErrorResponseException::FEE_CURRENCY_CONVERSION_FAILED);
}

bool
MaximumPenaltyValidator::validateQuery(const smp::ChangeQuery& query,
                                       const smp::RecordApplication& departure,
                                       const MaxPenaltyResponse::Fees& fees) const
{
  bool beforeResult, afterResult;

  if (query == smp::CHANGEABLE)
  {
    beforeResult = (departure & smp::BEFORE) && !fees._before.isFullyNon();
    afterResult = (departure & smp::AFTER) && !fees._after.isFullyNon();
  }
  else // only nonchangleable or nonrefundable
  {
    beforeResult = (departure & smp::BEFORE) && fees._before.isFullyNon();
    afterResult = (departure & smp::AFTER) && fees._after.isFullyNon();
  }

  return beforeResult || afterResult;
}

bool
MaximumPenaltyValidator::validateAmount(const Money& maxPenalty,
                                        const smp::RecordApplication& departure,
                                        const MaxPenaltyResponse::Fees& fees) const
{
  bool result = true;
  if (departure & smp::BEFORE)
  {
    result = (fees._before._fee && fees._before._fee.get() <= maxPenalty);
  }

  if (result && (departure & smp::AFTER))
  {
    result = (fees._after._fee && fees._after._fee.get() <= maxPenalty);
  }

  return result;
}

std::pair<bool, std::string>
MaximumPenaltyValidator::validateFilter(const MaxPenaltyInfo::Filter& filter,
                                        const MaxPenaltyResponse::Fees& fees) const
{
  bool isValid = true;
  std::stringstream diagnosticLogs;

  if (filter._query)
  {
    isValid = validateQuery(filter._query.get(), filter._departure, fees);

    if (_diagEnabled && !isValid)
    {
      diagnosticLogs << "QUERY FAILED: " << fees;
    }
  }
  else
  {
    isValid = validateAmount(filter._maxFee.get(), filter._departure, fees);

    if (_diagEnabled && !isValid)
    {
      diagnosticLogs << "AMOUNT FAILED: " << fees;
    }
  }

  return std::make_pair(isValid, diagnosticLogs.str());
}

void
MaximumPenaltyValidator::updateFailStats(MaxPenaltyStats& stats, const MaxPenaltyInfo& maxPenalty)
    const
{
  ++stats._failedFares;

  if (stats._failedFares >= _failedFaresThreshold)
  {
    const std::string msg = getFailedFaresDiagnostics(maxPenalty, stats);
    throw ErrorResponseException(ErrorResponseException::MAXIMUM_PENALTY_TOO_RESTRICTIVE,
                                 !msg.empty() ? msg.c_str() : nullptr);
  }
}

void
MaximumPenaltyValidator::completeResponse(FarePath& farePath) const
{
  if (MaxPenaltyInfo* maxPenalty = farePath.paxType()->maxPenaltyInfo())
  {
    const CurrencyCode changeCur = getPenaltyCurrency(maxPenalty->_changeFilter);
    const CurrencyCode refundCur = getPenaltyCurrency(maxPenalty->_refundFilter);

    MaxPenaltyResponse& response = getMaxPenaltyResponse(farePath);
    MaximumPenaltyCalculator calc(_trx, farePath);

    if (_diagEnabled)
    {
      DiagManager diag(_trx, Diagnostic555);
      diag << "********************MAX PENALTY INFORMATION********************\n"
           << "PASSENGER TYPE: " << farePath.paxType()->paxType() << "\n"
           << "--------------MAX CHANGE PENALTY --------------\n";
    }

    response._changeFees = calc.changePenalty(smp::BOTH, changeCur);

    if (_diagEnabled)
    {
      DiagManager diag(_trx, Diagnostic555);
      diag << "--------------MAX REFUND PENALTY --------------\n";
    }

    response._refundFees = calc.refundPenalty(smp::BOTH, refundCur);
  }
}

void
MaximumPenaltyValidator::printDiagHeader(const PaxTypeCode& paxType) const
{
  DiagManager diag(_trx, Diagnostic555);
  diag << "-------------------MAXIMUM PENALTY VALIDATION------------------\n"
       << "PASSENGER TYPE: " << paxType << "\n";
}

void
MaximumPenaltyValidator::printValidationResult(const std::pair<bool, std::string>& result) const
{
  DiagManager diag(_trx, Diagnostic555);
  diag << '\n';
  if (result.first)
    diag << " - PASS -\n";
  else
    diag << " - FAIL - " << result.second << '\n';
}

namespace
{
class NoDiag
{
protected:
  void printNon(PricingTrx& trx, const bool nonRefChg) const {}

  void printAmt(PricingTrx& trx,
                const MaxPenaltyResponse::Fees& fees,
                const MaxPenaltyInfo::Filter& filter) const
  {
  }

  void
  printFare(PricingTrx& trx, const MaxPenaltyResponse::Fees& fees, const PaxTypeFare& ptf) const
  {
  }

  void printResult(PricingTrx& trx, const PaxTypeFare& ptf) const {}
};

class ActiveDiag
{
protected:
  void printNon(PricingTrx& trx, const bool nonRefChg) const
  {
    DiagManager diag(trx, Diagnostic316);
    diag << "SMP" << (nonRefChg ? " NON" : " REF/CHG");
  }

  void printAmt(PricingTrx& trx,
                const MaxPenaltyResponse::Fees& fees,
                const MaxPenaltyInfo::Filter& filter) const
  {
    DiagManager diag(trx, Diagnostic316);
    diag << "SMP";

    if (filter._departure & smp::BEFORE)
    {
      if (fees._before.isFullyNon())
        diag << " NON";
      else if (!fees._before._fee)
        diag << " MDT";
      else
        diag << " " << fees._before._fee->value();

      diag << "/B";
    }

    if (filter._departure & smp::AFTER)
    {
      if (fees._after.isFullyNon())
        diag << " NON";
      else if (!fees._after._fee)
        diag << " MDT";
      else
        diag << " " << fees._after._fee->value();

      diag << "/A";
    }
  }

  void
  printFare(PricingTrx& trx, const MaxPenaltyResponse::Fees& fees, const PaxTypeFare& ptf) const
  {
    DiagManager diag(trx, Diagnostic316);
    diag << " " << ptf.fareMarket()->toString() << " ";
    smp::printDiagnosticFareFees(diag, ptf, fees, false);
  }

  void printResult(PricingTrx& trx, const PaxTypeFare& ptf) const
  {
    DiagManager diag(trx, Diagnostic316);
    diag << (ptf.isCategoryValid(RuleConst::PENALTIES_RULE) ? "   PASS" : "   FAIL") << "\n";
  }
};

template <class DiagC>
class SMP16prevalidator : public DiagC
{
  bool failedNon(const bool nonRefChg, const smp::ChangeQuery& filter) const
  {
    this->printNon(_trx, nonRefChg);
    return nonRefChg ? (filter == smp::CHANGEABLE) : (filter == smp::NONCHANGEABLE);
  }

  bool failedAmt(const MaxPenaltyInfo::Filter& filter) const
  {
    auto isNotMDTAndFailedAmt = [](const boost::optional<Money>& fee,
                                   const boost::optional<Money>& maxFee)
    { return fee && (fee->value() > maxFee->value()); };

    this->printAmt(_trx, _fees, filter);

    return (((filter._departure & smp::BEFORE) &&
            (_fees._before.isFullyNon() ||
            isNotMDTAndFailedAmt(_fees._before._fee, filter._maxFee))) ||
            ((filter._departure & smp::AFTER) &&
            (_fees._after.isFullyNon() || isNotMDTAndFailedAmt(_fees._after._fee, filter._maxFee))));
  }

  bool failedImpl(const MaxPenaltyInfo::Filter& filter,
                  const Cat16MaxPenaltyCalculator::PenaltyType& penaltyType)
  {
    _fees = _calc.calculateMaxPenalty(
        _ptf.getPenaltyInfo(),
        filter._maxFee ? filter._maxFee->code()
                       : CurrencyCode(_trx.getRequest()->ticketingAgent()->currencyCodeAgent()),
        _ptf,
        nullptr,
        filter._departure,
        penaltyType);
    _fees.finalRounding(_trx);

    if (filter._query)
    {
      bool nonDetected = ((filter._departure & smp::BEFORE) && _fees._before.isFullyNon()) ||
                         ((filter._departure & smp::AFTER) && _fees._after.isFullyNon());

      return failedNon(nonDetected, filter._query.get());
    }

    return failedAmt(filter);
  }

  bool changeableFailed()
  {
    return (_query->_changeFilter._query || _query->_changeFilter._maxFee) &&
           failedImpl(_query->_changeFilter, Cat16MaxPenaltyCalculator::CHANGE_PEN);
  }

  bool refundableFailed()
  {
    return (_query->_refundFilter._query || _query->_refundFilter._maxFee) &&
           failedImpl(_query->_refundFilter, Cat16MaxPenaltyCalculator::REFUND_PEN);
  }

  PricingTrx& _trx;
  PaxTypeFare& _ptf;
  const MaxPenaltyInfo* _query;

  Cat16MaxPenaltyCalculator _calc;
  MaxPenaltyResponse::Fees _fees;

public:
  SMP16prevalidator(PricingTrx& trx, PaxTypeFare& ptf)
    : _trx(trx),
      _ptf(ptf),
      _query(nullptr),
      _calc(trx,
            !_trx.diagnostic().diagParamIsSet(Diagnostic::RULE_PHASE, Diagnostic::PREVALIDATION))
  {
    if (_ptf.isValid())
    {
      TSE_ASSERT(ptf.paxType());
      _query = ptf.paxType()->maxPenaltyInfo();
    }
  }

  ~SMP16prevalidator()
  {
    if (_query && _query->_mode != smp::INFO)
    {
      this->printFare(_trx, _fees, _ptf);
      this->printResult(_trx, _ptf);
    }
  }

  void prevalidate()
  {
    if (!_query || _query->_mode == smp::INFO)
      return;

    if ((_query->_mode == smp::AND && (changeableFailed() || refundableFailed())) ||
        (_query->_mode == smp::OR && (changeableFailed() && refundableFailed())))
    {
      _ptf.setCategoryValid(RuleConst::PENALTIES_RULE, false);
      _ptf.setCategoryProcessed(RuleConst::PENALTIES_RULE, true);
    }
  }
};
}

void
MaximumPenaltyValidator::prevalidate16(FareMarket& fareMarket) const
{
  if (_diagEnabled && _trx.diagnostic().diagnosticType() == Diagnostic316)
  {
    for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
    {
      SMP16prevalidator<ActiveDiag> pvr(_trx, *ptf);
      pvr.prevalidate();
    }
  }
  else
  {
    for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
    {
      SMP16prevalidator<NoDiag> pvr(_trx, *ptf);
      pvr.prevalidate();
    }
  }
}

MaxPenaltyResponse&
MaximumPenaltyValidator::getMaxPenaltyResponse(FarePath& farePath) const
{
  if (!farePath.maxPenaltyResponse())
  {
    farePath.maxPenaltyResponse() = _trx.dataHandle().create<MaxPenaltyResponse>();
  }

  return *farePath.maxPenaltyResponse();
}

std::string
MaximumPenaltyValidator::getFailedFaresDiagnostics(const MaxPenaltyInfo& maxPenalty,
                                                   const MaxPenaltyStats& stats)
{
  std::string changeMsg;
  std::string refundMsg;

  if (stats._changeFilter._minPenalty > 0.0 || stats._refundFilter._minPenalty > 0.0)
  {
    if (stats._changeFilter._minPenalty > 0.0 && maxPenalty._changeFilter._maxFee)
    {
      changeMsg = boost::str(
          boost::format("NO FARES WITH WITH CHANGE PENALTY LESS THAN %1%") %
          Money(stats._changeFilter._minPenalty, maxPenalty._changeFilter._maxFee->code()));
    }

    if (stats._refundFilter._minPenalty > 0.0 && maxPenalty._refundFilter._maxFee)
    {
      refundMsg = boost::str(
          boost::format("NO FARES WITH WITH REFUND PENALTY LESS THAN %1%") %
          Money(stats._refundFilter._minPenalty, maxPenalty._refundFilter._maxFee->code()));
    }
  }
  else if (stats._changeFilter._queryFailed || stats._refundFilter._queryFailed)
  {
    if (stats._changeFilter._queryFailed && maxPenalty._changeFilter._query)
    {
      const char* query =
          (maxPenalty._changeFilter._query.get() == smp::CHANGEABLE ? "CHANGEABLE"
                                                                    : "NONCHANGEABLE");
      changeMsg = boost::str(boost::format("NO %1% FARES") % query);
    }

    if (stats._refundFilter._queryFailed && maxPenalty._refundFilter._query)
    {
      const char* query =
          (maxPenalty._refundFilter._query.get() == smp::CHANGEABLE ? "REFUNDABLE"
                                                                    : "NONREFUNDABLE");
      refundMsg = boost::str(boost::format("NO %1% FARES") % query);
    }
  }

  std::string details;
  if (!changeMsg.empty() && !refundMsg.empty())
  {
    details = boost::str(boost::format("%1% %2% %3%") % changeMsg %
                         (maxPenalty._mode == smp::OR ? "AND" : "OR") % refundMsg);
  }
  else if (!changeMsg.empty())
  {
    details = changeMsg;
  }
  else if (!refundMsg.empty())
  {
    details = refundMsg;
  }

  if (!details.empty())
  {
    return boost::str(boost::format("MAXIMUM PENALTY IS TOO RESTRICTIVE. %1%") % details);
  }
  else
  {
    return std::string();
  }
}

} /* namespace tse */
