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

#pragma once

#include "Common/LoggerPtr.h"
#include "DataModel/RefundPenalty.h"

#include <functional>

namespace tse
{
class DataHandle;
class DateTime;
class FareUsage;
class FarePath;
class PaxType;
class PaxTypeFare;
class PenaltyAdjuster;
class PricingTrx;
class PricingUnit;
class RefundDiscountApplier;
class RefundPermutation;
class RefundProcessInfo;
class RefundPricingTrx;
class VoluntaryRefundsInfo;

class PenaltyCalculator
{
  friend class PenaltyCalculatorTest;

public:
  PenaltyCalculator(RefundPricingTrx& trx,
                    const RefundDiscountApplier& discountApp);

  PenaltyCalculator(PricingTrx& trx,
                    const FarePath& farePath,
                    const CurrencyCode calculationCurrency,
                    const RefundDiscountApplier& discountApp);

  virtual ~PenaltyCalculator() {}

  void calculate(RefundPermutation& permutation);

  static constexpr Indicator REISSUEFEE_FC = 'F';
  static constexpr Indicator REISSUEFEE_PU = 'P';
  static constexpr Indicator CALCOPTION_A = 'A';
  static constexpr Indicator CALCOPTION_B = 'B';
  static constexpr Indicator HUNDRED_PERCENT_PENALTY = 'X';
  static constexpr Indicator HIGH = 'H';
  static constexpr Indicator LOW = 'L';

private:
  RefundPenalty* calculatePenalty(const PricingUnit& pu);
  RefundPenalty* calculateInFcScope(const PricingUnit& pu);
  RefundPenalty* calculateInPuScope(const PricingUnit& pu);
  RefundPenalty* calculateInMixedScope(const PricingUnit& pu);
  const PenaltyAdjuster getAdjuster(const PricingUnit& pu) const;

  enum Scope
  {
    FC_SCOPE,
    PU_SCOPE,
    MX_SCOPE
  };
  enum CalcMethod
  {
    ZERO_MTH,
    HNDR_MTH,
    SPEC_MTH,
    PERC_MTH,
    HILO_MTH
  };

  Scope determineScope(const PricingUnit& pu) const;
  CalcMethod determineMethod(const VoluntaryRefundsInfo& r3) const;

  class VoluntaryRefundsArray
      : public std::map<const PricingUnit*, std::map<const FareUsage*, const VoluntaryRefundsInfo*>>
  {
  public:
    VoluntaryRefundsArray(){}

    void create(const FarePath& fp);
    bool update(RefundPermutation& permutation);

  };

  struct CalculationFee : public RefundPenalty::Fee
  {
    explicit CalculationFee(const Money& money,
                            const MoneyAmount& camt = 0.0,
                            bool discount = false,
                            bool noRefundable = false)
      : RefundPenalty::Fee(money, discount, noRefundable), _convertedAmount(camt)
    {
    }

    bool operator<(const CalculationFee& fee) const;

    MoneyAmount _convertedAmount;
  };

public:
  typedef std::vector<CalculationFee> FeeVec;
  typedef VoluntaryRefundsArray::mapped_type PuItems;

  typedef std::pair<CalculationFee, RefundPenalty::Scope> ScopedFee;
  typedef std::vector<ScopedFee> ScopedFees;

  virtual CurrencyCode getOriginCurrency(const FareUsage& fu, const PuItems& items);
  CurrencyCode getOriginCurrencyFromCommencement() const;

private:
  RefundPenalty* createPenalty(const FeeVec& fees, RefundPenalty::Scope scope) const;

  Money getSpecifiedAmount(const VoluntaryRefundsInfo& rec3, const CurrencyCode& subjectCurr) const;

  CalculationFee getSpecifiedFee(const VoluntaryRefundsInfo& rec3,
                                 const CurrencyCode& subjectCurr,
                                 const PaxTypeFare& ptf) const;

  CalculationFee getPercentageFee(const VoluntaryRefundsInfo& rec3,
                                  const MoneyAmount& amount,
                                  const CurrencyCode& subjectCurr) const;

  CalculationFee getFee(const Money& m, bool discount = false, bool noRefundable = false) const;

  CalculationFee getFee(const MoneyAmount& amount,
                        const CurrencyCode& curr,
                        bool discount = false,
                        bool noRefundable = false) const;

  const CalculationFee& chooseHighLowFee(Indicator hiLoByte,
                                         const CalculationFee& left,
                                         const CalculationFee& right) const;

  CalculationFee determineFee(const VoluntaryRefundsInfo& rec3,
                              MoneyAmount totalAmount,
                              const CurrencyCode& currency,
                              const PaxTypeFare& ptf) const;

  void setTotalPenalty(RefundPermutation& permutation) const;
  void setHighestPenalty(RefundPermutation& permutation) const;
  void setWaivedPenalty(RefundPermutation& permutation) const;
  void setMinimumPenalty(RefundPermutation& permutation) const;

  DataHandle& _dataHandle;
  const FarePath& _farePath;
  DateTime _nationCurrencyTicketingDate;
  std::function<Money(const Money& source, const CurrencyCode& targetCurr)> _convertCurrency;
  std::function<bool(const RefundProcessInfo*)> _isWaived;
  bool _arePenaltiesAndFCsEqualToSumFromFareCalc;
  VoluntaryRefundsArray _vrArray;
  CurrencyCode _calcCurr;
  CurrencyCode _originCurrency;
  const RefundDiscountApplier& _discountApplier;
  PricingTrx& _trx;
};
}

