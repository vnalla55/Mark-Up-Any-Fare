#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class PricingTrx;
class FareByRuleItemInfo;
class GeneralFareRuleInfo;
class PaxTypeFare;

class Rec2Wrapper
{
public:
  Rec2Wrapper(const GeneralFareRuleInfo* const rec2,
              const bool isLocationSwapped,
              const bool conditional)
    : _rec2(rec2), _isLocationSwapped(isLocationSwapped), _isConditional(conditional)
  {
  }

  bool isLocationSwapped() const { return _isLocationSwapped; }
  bool isConditional() const { return _isConditional; }
  const GeneralFareRuleInfo& getRec2() const { return *_rec2; }

private:
  const GeneralFareRuleInfo* const _rec2;
  const bool _isLocationSwapped;
  const bool _isConditional;
};

class FareByRuleOverrideCategoryMatcher
{
  friend class FareByRuleOverrideCategoryMatcherTest;

public:
  FareByRuleOverrideCategoryMatcher(PricingTrx& trx,
                                    const FareByRuleItemInfo& fbrItemInfo,
                                    const PaxTypeFare& dummyPtFare);

  Rec2Wrapper* tryMatchRec2(const GeneralFareRuleInfo& rec2);

private:
  bool matchFareClass(const FareClassCode& fbrFareClassName,
                      const FareClassCode& rec2FareClassName) const;

  bool isFieldConditional(const Indicator fbrField, const Indicator rec2Field) const;

  bool isFieldConditional(const FareType& fbrField, const FareType& rec2Field) const;

  bool isFieldConditional(const RoutingNumber& fbrField, const RoutingNumber& rec2Field) const;

  bool isFareClassNameConditional(const FareClassCode& fbrFareClassName,
                                  const FareClassCode& rec2FareClassName) const;

  bool isRec2Conditional(const GeneralFareRuleInfo& rec2) const;

  Rec2Wrapper* createRec2Wrapper(const GeneralFareRuleInfo& rec2, const bool isLocationSwapped);

  PricingTrx& _trx;
  const FareByRuleItemInfo& _fbrItemInfo;
  const PaxTypeFare& _dummyPtFare;
  bool _isMultiAirport;

  static const Indicator BLANK;
  static const Indicator ASTERISK;
  static const Indicator HYPHEN;
  static const std::string NULL_CATEGORY_NAME;
  static const std::size_t MAX_FARE_CLASS_NAME_SIZE;
};
}

