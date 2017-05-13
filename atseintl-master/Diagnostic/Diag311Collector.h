//----------------------------------------------------------------------------
//  File:        Diag311Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 311 formatter
//
//  Updates:
//          date - initials - description.
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

#include "Common/TseStringTypes.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class BlackoutDates;
class PaxTypeFare;

class Diag311Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag311Collector(Diagnostic& root) : DiagCollector(root), _geoMatched(false) {}
  Diag311Collector() : _geoMatched(false) {}

  void diag311Collector(const PaxTypeFare& paxFare,
                        const CategoryRuleInfo& ruleInfo,
                        const BlackoutDates& blackoutDates,
                        const Record3ReturnTypes status,
                        const int phase,
                        PricingTrx& trx,
                        bool isInbound = false,
                        const PricingUnit* pricingUnit = nullptr);

  void geoMatched(bool status = true) { _geoMatched = status; }

private:
  bool _geoMatched;
};

} // namespace tse

