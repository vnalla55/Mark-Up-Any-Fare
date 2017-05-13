//----------------------------------------------------------------------------
//
//
//  File:     MatchExceptionTable.h
//  Created:  8/20/2004
//  Authors:  Quan Ta
//
//  Description:
//          Function object to match Minimum Fare Application Table.
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

#include <vector>
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"

namespace tse
{

class DateTime;
class DiagCollector;
class FarePath;
class Itin;
class MinFareAppl;
class MinFareDefaultLogic;
class MinimumFare;
class PaxTypeFare;
class PricingTrx;

struct MatchExceptionTable
{
  MatchExceptionTable(MinimumFareModule module,
                      PricingTrx& trx,
                      const FarePath& farePath,
                      const Itin& itin,
                      const PaxTypeFare& paxTypeFare,
                      const DateTime& tvlDate,
                      const CarrierCode& govCxr);
  ~MatchExceptionTable();

  bool operator()();
  bool operator()(const MinFareAppl* rule) const;

  const MinFareAppl* const matchedApplItem() const { return _matchedApplItem; }
  const MinFareDefaultLogic* const matchedDefaultItem() const { return _matchedDefaultItem; }

private:
  void displayApplication(const MinFareAppl& application) const;
  bool matchFareRule(const RuleNumber& sameFareGroupRN, const PaxTypeFare& paxTypeFare) const;
  bool matchFootnote(const Footnote&, const PaxTypeFare&) const;

  MinimumFareModule _module;
  PricingTrx& _trx;
  const FarePath& _farePath;
  const Itin& _itin;
  const PaxTypeFare& _paxTypeFare;
  const DateTime& _tvlDate;
  const CarrierCode& _govCxr;
  DiagCollector* _diag;
  bool _diagEnabled;
  const PaxTypeFare* _fareForCat17;
  bool _compCat17ItemOnly;

  const MinFareAppl* _matchedApplItem;
  const MinFareDefaultLogic* _matchedDefaultItem;
};

}; // tse

