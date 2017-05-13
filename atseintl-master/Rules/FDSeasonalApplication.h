//-------------------------------------------------------------------
//
//  File:        FDSeasonalApplication.h
//  Authors:     Lipika Bardalai
//  Created:     March 2005
//  Description: This class contains SeasonalApplication validation
//               for Fare Display, reusing existing functionality.
//               Method - validate is overidden to reuse functionality
//               from base class, adding new semantics for
//               Fare Display.
//
//  Copyright Sabre 2001
//              The copyright to the computer program(s) herein
//              is the property of Sabre.
//              The program(s) may be used and/or copied only with
//              the written permission of Sabre or in accordance
//              with the terms and conditions stipulated in the
//              agreement/contract under which the program(s)
//              have been supplied.
//
//---------------------------------------------------------------------

#pragma once

#include "Rules/SeasonalApplication.h"

namespace tse
{
class FareDisplayInfo;

class FDSeasonalApplication : public SeasonalApplication
{
public:
  FDSeasonalApplication(const CategoryRuleItemInfo* rule) : _rule(rule) {}

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& fare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket,
                                      bool isQualifiedCategory,
                                      bool isInbound);

  void updateFareDisplayInfo(const RuleItemInfo* rule, FareDisplayInfo*& fdInfo) const;

  static constexpr Indicator HIGH_SEASON = 'H';
  static constexpr Indicator INBOUND = 'I';
  static constexpr Indicator OUTBOUND = 'O';
  static constexpr Indicator BLANK = ' ';

private:
  void updateFareDisplayInfo(const SeasonalAppl& seasonalRuleInfo, FareDisplayInfo*& fdInfo) const;

  const CategoryRuleItemInfo* _rule;

  static Logger _logger;
};
}
