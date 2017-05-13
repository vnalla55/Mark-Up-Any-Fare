//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/FareByRuleItemInfo.h"

#include <vector>

namespace tse
{
class PaxTypeFare;
class PricingTrx;
class FareByRuleApp;
class FareByRuleProcessingInfo;
class FareMarket;
class FarePath;
class PricingUnit;

/** @class FareByRuleValidator
 *  This class processes Record 3 Category 25 Fare By Rule.
 *     This is a class to validate Record 3 Category 25 data
 *  to the user input request.
 *
 */

class FareByRuleValidator
{
  friend class FareByRuleValidatorTest;

public:
  FareByRuleValidator() {};
  virtual ~FareByRuleValidator() {};

  Record3ReturnTypes validate(FareByRuleProcessingInfo& fbrProcessingInfo);

  static bool checkSameTariffRule(const PricingUnit& pricingUnit);
  static bool checkSameTariffRule(const FarePath& farePath);

  static const char NO_DISCOUNT;
  static constexpr Indicator COMBINE_SAME_TARIFF_FARES = 'T';
  static constexpr Indicator COMBINE_SAME_TARIFF_AND_RULE_FARES = 'R';
  static constexpr Indicator COMBINE_PRIVATE_TARIFF_FARES = 'P';
  static constexpr Indicator COMBINE_PUBLIC_TARIFF_FARES = 'Y';

private:
  bool isValid(FareByRuleProcessingInfo& fbrProcessingInfo) const;

  bool matchPaxType(const FareByRuleItemInfo& fbrItemInfo, FareByRuleApp& fbrApp) const;

  bool matchOverrideDateTbl(FareByRuleProcessingInfo& fbrProcessingInfo) const;

  bool matchUnavaiTag(const FareByRuleItemInfo& fbrItemInfo) const;

  bool matchPassengerStatus(const FareByRuleItemInfo& fbrItemInfo,
                            PricingTrx& trx,
                            FareByRuleProcessingInfo& fbrProcessingInfo) const;

  bool matchNbrOfFltSegs(const FareByRuleItemInfo& fbrItemInfo, FareMarket& fareMarket) const;

  bool matchWhollyWithin(PricingTrx& trx,
                         const FareByRuleItemInfo& fbrItemInfo,
                         FareMarket& fareMarket) const;

  void doDiag325Collector(FareByRuleProcessingInfo& fbrProcessingInfo, const char* failCode) const;

  bool checkResultingInfo(FareByRuleProcessingInfo& fbrProcessingInfo) const;

  static bool checkSameTariffRule(const std::vector<const PaxTypeFare*>& paxTypeFares);
  static bool getRuleTariffRestrictions(const std::vector<const PaxTypeFare*>& paxTypeFares,
                                        bool& reqSameRule,
                                        bool& reqSameTariff,
                                        bool& reqPublicTariff,
                                        bool& reqPrivateTariff);
};
}

