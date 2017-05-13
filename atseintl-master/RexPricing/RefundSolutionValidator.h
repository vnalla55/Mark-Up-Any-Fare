//-------------------------------------------------------------------
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#pragma once

#include "RexPricing/CommonSolutionValidator.h"

#include <map>
#include <vector>

namespace tse
{

class RefundPricingTrx;
class Diag689Collector;
class FarePath;
class FareUsage;
class FareCompInfo;
class RefundProcessInfo;
class PaxTypeFare;
class RefundPermutation;
class PenaltyCalculator;
class VoluntaryRefundsInfo;

class RefundSolutionValidator : public CommonSolutionValidator
{
  friend class RefundSolutionValidatorTest;

public:
  static const Indicator SAME_FARE_TYPE;
  static const Indicator SAME_FARE_CLASS;
  static const Indicator CHANGE_TO_FARE_BREAKS_NOT_ALLOWED;
  static const Indicator RULE_TARIFF_NO_RESTRICTION;
  static const Indicator RULE_TARIFF_EXACT_TARIFF;
  static const Indicator SAME_RBD_OR_CABIN;

  RefundSolutionValidator(RefundPricingTrx& trx, FarePath& fp);
  virtual ~RefundSolutionValidator();
  bool process();

protected:
  enum Validator
  { FARE_BREAKS = 0,
    RULE_TARIFF,
    RULE,
    FARE_CLASS_FAMILY,
    FARE_TYPE,
    SAME_FARE,
    NORMAL_SPECIAL,
    OWRT,
    FARE_AMOUNT,
    SAME_RBD };

  typedef bool (RefundSolutionValidator::*ValidationMethod)(const FareUsage&,
                                                            const RefundProcessInfo&);

  class CacheKey
  {
  public:
    CacheKey(const FareCompInfo& fareComponent,
             const VoluntaryRefundsInfo& record3,
             const Validator& byteValidator)
      : _fareComponent(&fareComponent), _record3(&record3), _byteValidator(byteValidator)
    {
    }

    bool operator<(const CacheKey& key) const
    {
      return (_fareComponent < key._fareComponent) ||
             (_fareComponent == key._fareComponent && _record3 < key._record3) ||
             (_fareComponent == key._fareComponent && _record3 == key._record3 &&
              _byteValidator < key._byteValidator);
    }

  private:
    const FareCompInfo* _fareComponent;
    const VoluntaryRefundsInfo* _record3;
    const Validator _byteValidator;
  };

  bool process(RefundPermutation& permutation, PenaltyCalculator& penaltyCalc);

  bool hasDiagAndFilterPassed() const;
  void printPermutation(const RefundPermutation& permutation);
  void printValidationResult(unsigned number, bool result) const;
  void printFareBreaksFail(const FareUsage& fareUsage,
                           const RefundProcessInfo& processInfo,
                           const std::string& failPoint);
  const std::vector<FareUsage*>& getRelatedFUs(const FareCompInfo& fareCompInfo);
  bool executeValidation(const RefundPermutation& permutation, const Validator& byteValidator);
  bool executeValidation(const RefundProcessInfo& processInfo,
                         const std::vector<FareUsage*>& mappedFus,
                         const Validator& byteValidator);
  RefundSolutionValidator::ValidationMethod
  getValidationMethod(const Validator& byteValidator);
  bool checkCache(const CacheKey& key, bool& cachedValidationResult) const;

  bool checkFareBreaks(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  bool originChanged(const FareUsage& fareUsage, const RefundProcessInfo& processInfo) const;
  bool destinationChanged(const FareUsage& fareUsage, const RefundProcessInfo& processInfo) const;

  bool
  validateSameFareIndicator(const FareUsage& fareUsage, const RefundProcessInfo& refundProcessInfo);
  virtual std::string getFareBasis(const PaxTypeFare& paxTypeFare) const;

  bool checkRuleTariff(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  bool checkAnyRuleTariff(const PaxTypeFare& ptf, const RefundProcessInfo& processInfo);
  bool checkSameRuleTariff(const PaxTypeFare& ptf, const RefundProcessInfo& processInfo);
  void printSameRuleTariffFail(const PaxTypeFare& ptf,
                               const RefundProcessInfo& processInfo,
                               const std::string& message);
  void printAnyRuleTariffFail(const PaxTypeFare& ptf,
                              const RefundProcessInfo& processInfo,
                              const std::string& message);
  virtual void printCommonFail(const PaxTypeFare& ptf,
                               const RefundProcessInfo& processInfo,
                               const std::string& message);
  void printTariffCat(const int& tariffCat);
  bool repriceIndicatorValidation(const RefundPermutation& perm) const;
  bool checkRule(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  bool checkFareNormalSpecial(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  bool checkOWRT(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  bool checkFareClassCode(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  bool checkFareAmount(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  void setWinnerPermutation(const RefundPermutation& permutation);
  bool checkFareType(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);
  bool checkSameRBD(const FareUsage& fareUsage, const RefundProcessInfo& processInfo);

private:
  RefundPricingTrx& _trx;
  FarePath& _farePath;
  Diag689Collector* _dc;
  std::map<const FareCompInfo*, std::vector<FareUsage*> > _relatedFUsMap;
  std::map<const CacheKey, const bool> _validationCache;

  RefundSolutionValidator(const RefundSolutionValidator&);
  RefundSolutionValidator& operator=(const RefundSolutionValidator&);
};
}

