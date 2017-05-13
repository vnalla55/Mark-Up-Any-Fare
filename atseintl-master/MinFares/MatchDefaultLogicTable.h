//----------------------------------------------------------------------------
//
//
//  File:     MatchDefaultLogicTable.h
//  Created:  8/20/2004
//  Authors:  Quan Ta
//
//  Description:
//          Function object to match Minimum Fare Default Logic Table.
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

namespace tse
{

class MinimumFare;
class PricingTrx;
class Itin;
class PaxTypeFare;
class DiagCollector;
class MinFareDefaultLogic;

class MatchDefaultLogicTable
{
public:
  MatchDefaultLogicTable(MinimumFareModule module,
                         PricingTrx& trx,
                         const Itin& itin,
                         const PaxTypeFare& paxTypeFare);
  ~MatchDefaultLogicTable();

  bool operator()();
  bool operator()(const MinFareDefaultLogic* rule) const;

  const MinFareDefaultLogic* const matchedDefaultItem() const { return _matchedDefaultItem; }

private:
  void displayDefaultAppl(const MinFareDefaultLogic& defaultAppl) const;

  MinimumFareModule _module;
  PricingTrx& _trx;
  const Itin& _itin;
  const PaxTypeFare& _paxTypeFare;
  DiagCollector* _diag;
  bool _diagEnabled;

  const MinFareDefaultLogic* _matchedDefaultItem;
};

}; // tse

