
//----------------------------------------------------------------------------
//  File:        Diag203Collector.h
//  Authors:     Quan Ta
//  Created:     Mar 2007
//
//  Description: Diagnostic 203 - Fare Type Pricing
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2007
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

#include "Diagnostic/DiagCollector.h"

#include <vector>

namespace tse
{
class PricingTrx;
class FareTypeQualifier;

class Diag203Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag203Collector(Diagnostic& root) : DiagCollector(root), _userApplType(0), _trx(nullptr) {}
  Diag203Collector() : _userApplType(0), _trx(nullptr) {}

  void process(PricingTrx& trx);
  void displayHeading(PricingTrx& trx);

  std::string showFtqInfo(const FareTypeQualifier& ftq);
  std::string showFtqPsgInfo(const FareTypeQualifier& ftq);
  std::string showFtqMsgInfo(const FareTypeQualifier& ftq);

  Diag203Collector& operator<<(const FareTypeQualifier& ftq);
  Diag203Collector& operator<<(const std::vector<FareTypeQualifier*>& ftqList);

private:
  Indicator _userApplType;
  std::string _userAppl;
  FareType _qualifier;

  PricingTrx* _trx;
};

} // namespace tse

