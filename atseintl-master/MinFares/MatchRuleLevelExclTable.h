//----------------------------------------------------------------------------
//
//
//  File:     MatchRuleLevelExclTable.h
//  Created:  8/20/2004
//  Authors:  Quan Ta
//
//  Description:
//          Function object to match Minimum Fare Rule Level Exclusion Table.
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

class DateTime;
class DiagCollector;
class Itin;
class MinFareRuleLevelExcl;
class MinimumFare;
class PaxTypeFare;
class PricingTrx;

struct MatchRuleLevelExclTable
{
  MatchRuleLevelExclTable(MinimumFareModule module,
                          PricingTrx& trx,
                          const Itin& itin,
                          const PaxTypeFare& paxTypeFare,
                          const DateTime& tvlDate,
                          const bool checkSameFareGroup = false);

  ~MatchRuleLevelExclTable();

  bool operator()();
  bool operator()(const MinFareRuleLevelExcl* rule);

  const MinFareRuleLevelExcl* const matchedRuleItem() const { return _matchedRuleItem; }

  void displayRuleLevel(const MinFareRuleLevelExcl& ruleLevelExcl) const;
  bool checkRuleLevelExclusionSameFareGroupCheck(const MinFareRuleLevelExcl& ruleLevelExcl,
                                                 const PaxTypeFare& paxTypeFare);
  bool passRuleLevelExclusionSameGroup(const MinFareRuleLevelExcl& ruleLevelExcl,
                                       const PaxTypeFare& paxTypeFare,
                                       bool ifThruFare);
  bool matchSameGroup(const FareClassCode& sameFareGroupFC,
                      const FareType& sameFareGroupFT,
                      const RuleNumber& sameFareGroupRN,
                      const TariffNumber& sameFareGroupRT,
                      const PaxTypeFare& paxTypeFare);

  bool matchFareType(const FareType& sameFareGroupFT, const PaxTypeFare& paxTypeFare) const;
  bool matchFareClass(const FareClassCode& sameFareGroupFC, const PaxTypeFare& paxTypeFare) const;
  bool matchFareRule(const RuleNumber& sameFareGroupRN, const PaxTypeFare& paxTypeFare) const;
  bool matchFootnote(const Footnote&, const PaxTypeFare&) const;
  bool matchRuleTariff(const TariffNumber& sameFareGroupRT, const PaxTypeFare& paxTypeFare) const;

  DiagCollector*& diagCollector() { return _diag; }

private:
  MinimumFareModule _module;
  PricingTrx& _trx;
  const Itin& _itin;
  const PaxTypeFare& _paxTypeFare;
  const DateTime& _tvlDate;
  DiagCollector* _diag;
  bool _diagEnabled;
  const PaxTypeFare* _fareForCat17;
  bool _compCat17ItemOnly;

  const MinFareRuleLevelExcl* _matchedRuleItem;
  SetNumber _setvalue;
  bool _checkSameFareGroup;
};

}; // tse

