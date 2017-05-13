//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "RexPricing/PenaltyCalculator.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TseConsts.h"
#include "DataModel/Billing.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/PenaltyAdjuster.h"
#include "RexPricing/RefundDiscountApplier.h"
#include <boost/range/adaptor/map.hpp>
#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(cat33AmountSelection);
FALLBACK_DECL(smpCat33DoubleConversionFix)
FALLBACK_DECL(cat33DoubleConversionFix)
FALLBACK_DECL(cat33DoubleConversionFixForAll)

const Indicator PenaltyCalculator::REISSUEFEE_FC;
const Indicator PenaltyCalculator::REISSUEFEE_PU;
const Indicator PenaltyCalculator::CALCOPTION_A;
const Indicator PenaltyCalculator::CALCOPTION_B;
const Indicator PenaltyCalculator::HUNDRED_PERCENT_PENALTY;
const Indicator PenaltyCalculator::HIGH;
const Indicator PenaltyCalculator::LOW;

typedef std::vector<PricingUnit*> PuVec;
typedef std::vector<PricingUnit*>::const_iterator PuIt;
typedef std::vector<FareUsage*>::const_iterator FuIt;

namespace
{

struct ScopedFeeComparator
{
  bool operator()(const PenaltyCalculator::ScopedFee& l, const PenaltyCalculator::ScopedFee& r)
  {
    return (l.first < r.first);
  }
};
}

PenaltyCalculator::PenaltyCalculator(RefundPricingTrx& trx,
                                     const RefundDiscountApplier& discountApp)
  : _dataHandle(trx.dataHandle()),
    _farePath(*trx.exchangeItin().front()->farePath().front()),
    _nationCurrencyTicketingDate(trx.originalTktIssueDT()),
    _arePenaltiesAndFCsEqualToSumFromFareCalc(trx.arePenaltiesAndFCsEqualToSumFromFareCalc()),
    _calcCurr(trx.exchangeItinCalculationCurrency()),
    _discountApplier(discountApp),
    _trx(trx)
{
  _vrArray.create(_farePath);

  _convertCurrency = [&trx](const Money& source, const CurrencyCode& targetCurr)
  { return trx.convertCurrency(source, targetCurr); };

  _isWaived = [&trx](const RefundProcessInfo* rpi)
  { return trx.waivedRecord3().find(&rpi->record3()) != trx.waivedRecord3().end(); };
}

PenaltyCalculator::PenaltyCalculator(PricingTrx& trx,
                                     const FarePath& farePath,
                                     const CurrencyCode calculationCurrency,
                                     const RefundDiscountApplier& discountApp)
  : _dataHandle(trx.dataHandle()),
    _farePath(farePath),
    _nationCurrencyTicketingDate(trx.ticketingDate()),
    _arePenaltiesAndFCsEqualToSumFromFareCalc(false),
    _calcCurr(calculationCurrency),
    _discountApplier(discountApp),
    _trx(trx)
{
  _vrArray.create(_farePath);

  _convertCurrency = [&trx](const Money& source, const CurrencyCode& targetCurr)
  {
    if (source.code() != targetCurr)
    {
      Money target(targetCurr);
      CurrencyConversionFacade currConvFacade;
      currConvFacade.convert(target, source, trx, trx.itin().front()->useInternationalRounding(), ConversionType::NO_ROUNDING);
      return target;
    }
    else
    {
      return source;
    }
  };
}

void
PenaltyCalculator::calculate(RefundPermutation& permutation)
{
  if (!permutation.penaltyFees().empty())
    return;

  if (!_vrArray.update(permutation))
    return;

  for (const PricingUnit* pu : _farePath.pricingUnit())
    permutation.penaltyFees()[pu] = calculatePenalty(*pu);

  setTotalPenalty(permutation);
  setHighestPenalty(permutation);
  setWaivedPenalty(permutation);
  setMinimumPenalty(permutation);
}

RefundPenalty*
PenaltyCalculator::calculatePenalty(const PricingUnit& pu)
{
  RefundPenalty* resultPenalty = nullptr;
  switch (determineScope(pu))
  {
  case FC_SCOPE:
    resultPenalty = calculateInFcScope(pu);
    break;
  case PU_SCOPE:
    resultPenalty = calculateInPuScope(pu);
    break;
  case MX_SCOPE:
    resultPenalty = calculateInMixedScope(pu);
    break;
  }

  _originCurrency.clear();
  return resultPenalty;
}

namespace
{

struct HasReissueFeeInd
{
  HasReissueFeeInd(const Indicator pufc) : _reissueFeeInd(pufc) {}

  bool operator()(const PenaltyCalculator::PuItems::value_type& puItem) const
  {
    return puItem.second->reissueFeeInd() != _reissueFeeInd;
  }

  Indicator _reissueFeeInd;
};

struct HasCalcOptionA
{
  bool operator()(const PenaltyCalculator::PuItems::value_type& puItem) const
  {
    return puItem.second->calcOption() == PenaltyCalculator::CALCOPTION_A;
  }
};
}

PenaltyCalculator::Scope
PenaltyCalculator::determineScope(const PricingUnit& pu) const
{
  const PuItems& puItems = _vrArray.find(&pu)->second;

  PuItems::const_iterator b = puItems.begin();
  Indicator ind = b->second->reissueFeeInd();

  if (std::find_if(++b, puItems.end(), HasReissueFeeInd(ind)) == puItems.end())
    return ind == REISSUEFEE_FC ? FC_SCOPE : PU_SCOPE;

  if (std::find_if(puItems.begin(), puItems.end(), HasCalcOptionA()) != puItems.end())
    return PU_SCOPE;

  return MX_SCOPE;
}

PenaltyCalculator::CalcMethod
PenaltyCalculator::determineMethod(const VoluntaryRefundsInfo& r3) const
{
  if (r3.cancellationInd() == HUNDRED_PERCENT_PENALTY)
    return HNDR_MTH;

  bool hasPerc = r3.penaltyPercent() > EPSILON;
  bool hasSpec = !r3.penalty1Cur().empty() || !r3.penalty2Cur().empty();

  if (hasPerc && hasSpec && (r3.highLowInd() == HIGH || r3.highLowInd() == LOW))
    return HILO_MTH;

  if (hasPerc && !hasSpec)
    return PERC_MTH;

  if (!hasPerc && hasSpec)
    return SPEC_MTH;

  return ZERO_MTH;
}

inline PenaltyCalculator::CalculationFee
PenaltyCalculator::getFee(const Money& m, bool discount, bool noRefundable) const
{
  return CalculationFee(m, _convertCurrency(m, _calcCurr).value(), discount, noRefundable);
}

inline PenaltyCalculator::CalculationFee
PenaltyCalculator::getFee(const MoneyAmount& amount,
                          const CurrencyCode& curr,
                          bool discount,
                          bool noRefundable) const
{
  Money m = _convertCurrency(Money(amount, _calcCurr), curr);
  return CalculationFee(m, amount, discount, noRefundable);
}

PenaltyCalculator::CalculationFee
PenaltyCalculator::getPercentageFee(const VoluntaryRefundsInfo& rec3,
                                    const MoneyAmount& amount,
                                    const CurrencyCode& subjectCurr) const
{
  Money fee(amount * 1E-2 * rec3.penaltyPercent(), _calcCurr);
  bool isDiscounted = _discountApplier.apply(fee, rec3);
  return getFee(fee.value(), subjectCurr, isDiscounted);
}

Money
PenaltyCalculator::getSpecifiedAmount(const VoluntaryRefundsInfo& rec3,
                                      const CurrencyCode& subjectCurr) const
{
  if (fallback::fixed::cat33AmountSelection())
  {
    Money amt = Money(rec3.penalty1Amt(), rec3.penalty1Cur());

    if (rec3.penalty1Amt() < EPSILON)
      return amt;

    if (rec3.penalty1Cur() == subjectCurr || rec3.penalty2Amt() < EPSILON)
      return amt;

    if (rec3.penalty2Cur() == subjectCurr)
      return Money(rec3.penalty2Amt(), rec3.penalty2Cur());

    return amt;
  }
  else
  {
    if (rec3.penalty1Cur() == subjectCurr ||
        rec3.penalty1Amt() < EPSILON ||
        rec3.penalty2Cur() == NULL_CODE)
    {
      return Money(rec3.penalty1Amt(), rec3.penalty1Cur());
    }

    if (rec3.penalty2Cur() == subjectCurr)
    {
      return Money(rec3.penalty2Amt(), rec3.penalty2Cur());
    }

    MoneyAmount penalty1 = CurrencyUtil::convertMoneyAmount(
        rec3.penalty1Amt(), rec3.penalty1Cur(), NUC, _trx, ConversionType::NO_ROUNDING);
    MoneyAmount penalty2 = CurrencyUtil::convertMoneyAmount(
        rec3.penalty2Amt(), rec3.penalty2Cur(), NUC, _trx, ConversionType::NO_ROUNDING);

    if (penalty2 >= penalty1)
    {
      return Money(rec3.penalty1Amt(), rec3.penalty1Cur());
    }
    else
    {
      return Money(rec3.penalty2Amt(), rec3.penalty2Cur());
    }
  }
}

PenaltyCalculator::CalculationFee
PenaltyCalculator::getSpecifiedFee(const VoluntaryRefundsInfo& rec3,
                                   const CurrencyCode& subjectCurr,
                                   const PaxTypeFare& ptf) const
{
  Money fee = getSpecifiedAmount(rec3, subjectCurr);
  bool isDiscounted = _discountApplier.apply(fee, ptf, rec3);
  return getFee(fee, isDiscounted);
}

PenaltyCalculator::CalculationFee
PenaltyCalculator::determineFee(const VoluntaryRefundsInfo& rec3,
                                MoneyAmount totalAmount,
                                const CurrencyCode& currency,
                                const PaxTypeFare& ptf) const
{
  CurrencyCode calculationCurrency = _farePath.calculationCurrency();
  // in some cases totalAmount is stored in NUC, hence the need for conversion
  if (calculationCurrency.empty())
    calculationCurrency = NUC;
  totalAmount = _convertCurrency(Money(totalAmount, calculationCurrency), _calcCurr).value();

  switch (determineMethod(rec3))
  {
  case ZERO_MTH:
    return getFee(Money(0.0, currency));

  case HNDR_MTH:
    return getFee(totalAmount, currency, false, true);

  case SPEC_MTH:
    return getSpecifiedFee(rec3, currency, ptf);

  case PERC_MTH:
    return getPercentageFee(rec3, totalAmount, currency);

  case HILO_MTH:
    return chooseHighLowFee(rec3.highLowInd(),
                            getSpecifiedFee(rec3, currency, ptf),
                            getPercentageFee(rec3, totalAmount, currency));
  default:
    ;
  }

  return getFee(Money(0.0, currency));
}

const PenaltyCalculator::CalculationFee&
PenaltyCalculator::chooseHighLowFee(Indicator hiLoByte,
                                    const CalculationFee& left,
                                    const CalculationFee& right) const
{
  switch (hiLoByte)
  {
  case HIGH:
    return std::max(left, right);

  case LOW:
    return std::min(left, right);

  default:
    ;
  }
  return left;
}

RefundPenalty*
PenaltyCalculator::calculateInFcScope(const PricingUnit& pu)
{
  const PuItems& items = _vrArray[&pu];
  FeeVec fees;
  const PenaltyAdjuster adjuster = getAdjuster(pu);

  for (const auto& item : items)
  {
    const FareUsage& fu = *item.first;
    const VoluntaryRefundsInfo& rec3 = *item.second;

    fees.push_back(determineFee(
        rec3, adjuster.adjustedFuAmt(fu), getOriginCurrency(fu, items), *fu.paxTypeFare()));
  }

  return createPenalty(fees, RefundPenalty::FC);
}

RefundPenalty*
PenaltyCalculator::calculateInPuScope(const PricingUnit& pu)
{
  const PuItems& items = _vrArray[&pu];

  CurrencyCode puCurr = getOriginCurrency(*(items.begin()->first), items);
  FeeVec fees;
  const PenaltyAdjuster adjuster = getAdjuster(pu);

  for (const auto& item : items)
  {
    const FareUsage& fu = *item.first;
    const VoluntaryRefundsInfo& rec3 = *item.second;
    fees.push_back(determineFee(rec3, adjuster.adjustedPuAmt(), puCurr, *fu.paxTypeFare()));
  }

  FeeVec::const_iterator max = std::max_element(fees.begin(), fees.end());

  return createPenalty(FeeVec(1, *max), RefundPenalty::PU);
}

RefundPenalty*
PenaltyCalculator::calculateInMixedScope(const PricingUnit& pu)
{
  const PuItems& items = _vrArray[&pu];
  const PenaltyAdjuster adjuster = getAdjuster(pu);

  CurrencyCode puCurr = getOriginCurrency(*(items.begin()->first), items);
  ScopedFees scopedFees;

  for (PuItems::const_iterator i = items.begin(); i != items.end(); ++i)
  {
    const FareUsage& fu = *(i->first);
    const VoluntaryRefundsInfo& rec3 = *(i->second);

    if (rec3.reissueFeeInd() == PenaltyCalculator::REISSUEFEE_PU)
    {
      scopedFees.push_back(
          std::make_pair(determineFee(rec3, adjuster.adjustedPuAmt(), puCurr, *fu.paxTypeFare()),
                         RefundPenalty::PU));
    }
    else
    {
      CurrencyCode fcCurr = (i != items.begin()) ? getOriginCurrency(fu, items) : puCurr;

      scopedFees.push_back(
          std::make_pair(determineFee(rec3, adjuster.adjustedFuAmt(fu), fcCurr, *fu.paxTypeFare()),
                         RefundPenalty::FC));
    }
  }

  ScopedFees::const_iterator max =
      std::max_element(scopedFees.begin(), scopedFees.end(), ScopedFeeComparator());

  return createPenalty(FeeVec(1, max->first), max->second);
}

const PenaltyAdjuster
PenaltyCalculator::getAdjuster(const PricingUnit& pu) const
{
  if (_arePenaltiesAndFCsEqualToSumFromFareCalc)
    return PenaltyAdjuster(pu, PenaltyAdjuster::SUMARIZE_FC);

  return PenaltyAdjuster(pu, PenaltyAdjuster::SUMARIZE_FU, 0.0);
}

CurrencyCode
PenaltyCalculator::getOriginCurrencyFromCommencement() const
{
  for (const PricingUnit* pu : _farePath.pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
      if (fu->paxTypeFare()->isInternational())
        return fu->paxTypeFare()->currency();
  return _farePath.pricingUnit().front()->fareUsage().front()->paxTypeFare()->currency();
}

CurrencyCode
PenaltyCalculator::getOriginCurrency(const FareUsage& fu, const PuItems& items)
{
  bool isSMP =
      !fallback::smpCat33DoubleConversionFix(&_trx) && _farePath.paxType()->maxPenaltyInfo();
  bool anyRec3Chg1AndChg2All =
      !fallback::cat33DoubleConversionFixForAll(&_trx) &&
      (!_originCurrency.empty() ||
       std::any_of(items.begin(),
                   items.end(),
                   [](auto pair)
                   { return pair.second->penalty1Amt() && pair.second->penalty2Amt(); }));
  bool anyRec3Chg1AndChg2WS =
      !fallback::cat33DoubleConversionFix(&_trx) && _trx.billing()->partitionID() == CARRIER_WS &&
      (!_originCurrency.empty() ||
       std::any_of(items.begin(),
                   items.end(),
                   [](auto pair)
                   { return pair.second->penalty1Amt() && pair.second->penalty2Amt(); }));

  if (isSMP || anyRec3Chg1AndChg2All || anyRec3Chg1AndChg2WS)
  {
    if (_originCurrency.empty())
    {
      _originCurrency = getOriginCurrencyFromCommencement();
    }
    return _originCurrency;
  }

  CurrencyCode originCurr(NUC);
  NationCode originNation = fu.isOutbound()
                                ? fu.paxTypeFare()->fareMarket()->origin()->nation()
                                : fu.paxTypeFare()->fareMarket()->destination()->nation();
  CurrencyUtil::getNationCurrency(originNation, originCurr, _nationCurrencyTicketingDate);
  return originCurr;
}

RefundPenalty*
PenaltyCalculator::createPenalty(const FeeVec& fees, RefundPenalty::Scope scope) const
{
  RefundPenalty* p = _dataHandle.create<RefundPenalty>();
  std::vector<RefundPenalty::Fee> f(fees.begin(), fees.end());
  p->assign(f, scope);
  return p;
}

void
PenaltyCalculator::setTotalPenalty(RefundPermutation& permutation) const
{
  MoneyAmount amount = 0.0;
  for (const RefundPenalty* penalty : permutation.penaltyFees() | boost::adaptors::map_values)
  {
    for (const RefundPenalty::Fee& fee : penalty->fee())
      amount += _convertCurrency(fee.amount(), _calcCurr).value();
  }

  permutation.totalPenalty() = Money(amount, _calcCurr);
}

void
PenaltyCalculator::setHighestPenalty(RefundPermutation& permutation) const
{
  auto comparator = [&](const RefundPenalty::Fee& fee1, const RefundPenalty::Fee& fee2)
  {
    return _convertCurrency(fee1.amount(), _calcCurr).value() <
           _convertCurrency(fee2.amount(), _calcCurr).value();
  };

  auto highestFee = permutation.penaltyFees().begin()->second->fee().begin();

  for (RefundPenalty* penalty : permutation.penaltyFees() | boost::adaptors::map_values)
  {
    auto localMax = std::max_element(penalty->fee().begin(), penalty->fee().end(), comparator);

    if (comparator(*highestFee, *localMax))
      highestFee = localMax;
  }

  highestFee->highest() = true;
}

void
PenaltyCalculator::setWaivedPenalty(RefundPermutation& permutation) const
{
  if (_isWaived)
  {
    if (std::any_of(
            permutation.processInfos().begin(), permutation.processInfos().end(), _isWaived))
    {
      permutation.waivedPenalty() = true;
    }
  }
}

void
PenaltyCalculator::setMinimumPenalty(RefundPermutation& perm) const
{
  typedef std::vector<RefundProcessInfo*>::const_iterator It;
  for (It i = perm.processInfos().begin(); i != perm.processInfos().end(); ++i)
  {
    const VoluntaryRefundsInfo& rec3 = (*i)->record3();
    if (rec3.minimumAmtCur().empty())
      continue;

    Money mny(rec3.minimumAmt(), rec3.minimumAmtCur());

    _discountApplier.apply(mny, (*i)->paxTypeFare(), rec3);

    if (perm.totalPenalty().value() < _convertCurrency(mny, _calcCurr).value())
      perm.minimumPenalty() = mny;
  }
}

// --== CalculationFee ==--

bool
PenaltyCalculator::CalculationFee::
operator<(const CalculationFee& fee) const
{
  return (std::fabs(_convertedAmount - fee._convertedAmount) < EPSILON
              ? nonRefundable() < fee.nonRefundable()
              : _convertedAmount < fee._convertedAmount);
}

// --== VoluntaryRefundsArray ==--

void
PenaltyCalculator::VoluntaryRefundsArray::create(const FarePath& fp)
{
  clear();
  for (const auto pu : fp.pricingUnit())
    for (const auto fu : pu->fareUsage())
      (*this)[pu][fu] = nullptr;
}

bool
PenaltyCalculator::VoluntaryRefundsArray::update(RefundPermutation& permutation)
{
  typedef mapped_type::const_iterator It;
  for (const_iterator pu = begin(); pu != end(); ++pu)
    for (It fu = pu->second.begin(); fu != pu->second.end(); ++fu)
    {
      std::vector<RefundProcessInfo*>::const_iterator it =
          permutation.find(fu->first->paxTypeFare());
      if (it != permutation.processInfos().end())
        (*this)[pu->first][fu->first] = &(*it)->record3();
      else
      {
        return false;
      }
    }
  return true;
}
}
