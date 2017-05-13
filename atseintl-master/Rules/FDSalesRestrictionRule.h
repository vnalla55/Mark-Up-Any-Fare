//----------------------------------------------------------------
//
//  File:	       FDSalesRestrictionRule.h
//
//  Authors:     Marco Cartolano
//  Created:     March 10, 2005
//  Description: FDSalesRestrictionRule class for Fare Display
//
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------
#pragma once

#include "DBAccess/Record2Types.h"
#include "Rules/SalesRestrictionRule.h"


namespace tse
{

class PaxTypeFare;
class PricingTrx;
class FareDisplayInfo;
class Itin;
class RuleItemInfo;
class SalesRestriction;
class FareUsage;

class FDSalesRestrictionRule : public SalesRestrictionRule
{
  friend class FDSalesRestrictionRuleTest;

public:
  Record3ReturnTypes validate(PricingTrx&,
                              Itin&,
                              FareUsage*,
                              PaxTypeFare&,
                              const CategoryRuleInfo&,
                              const CategoryRuleItemInfo*,
                              const SalesRestriction*,
                              bool,
                              bool&,
                              bool) override;

  bool checkTicketElectronic(bool reqEtkt, Indicator canEtkt) const override;

private:

  void updateFareDisplayInfo(const SalesRestriction*& salesRestrictionRule,
                             FareDisplayInfo*& fdInfo,
                             const DateTime& ticketingDate) const;

  void updateDiagnostic315(PricingTrx& trx,
                           const PaxTypeFare& paxTypeFare,
                           const CategoryRuleInfo& cri,
                           const CategoryRuleItemInfo* rule,
                           const SalesRestriction* salesRestrictionRule,
                           Record3ReturnTypes retCode,
                           Cat15FailReasons failReason,
                           bool skipCat15SecurityCheck);
};

} // namespace tse

