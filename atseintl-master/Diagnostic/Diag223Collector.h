//----------------------------------------------------------------------------
//  File:        Diag223Collector.h
//  Authors:
//  Created:     July 2004
//
//  Description: Diagnostic 223 formatter
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareMarket;

class Diag223Collector : public DiagCollector
{
public:
  explicit Diag223Collector(Diagnostic& root)
    : DiagCollector(root),
      _checkSpecial(false),
      _isSpecial(false),
      _globalDirection(GlobalDirection::ZZ)
  {
  }
  Diag223Collector()
    : _checkSpecial(false), _isSpecial(false), _globalDirection(GlobalDirection::ZZ)
  {
  }

  Diag223Collector& operator<<(const PaxTypeFare& paxFare) override;
  Diag223Collector& operator<<(const Itin& itn) override;
  Diag223Collector& operator<<(const FareMarket& fareMarket) override;
  Diag223Collector& operator<<(const PricingTrx& x) override;

  void parseQualifiers(const PricingTrx& trx);

private:
  enum SeparatorType
  {
    FARE_HEADER = 1
  };

  FareClassCode _fareClass;

  bool _checkSpecial;
  bool _isSpecial;

  GlobalDirection _globalDirection;

  void writeSeparator(SeparatorType);

  bool checkPaxTypeFare(const PaxTypeFare& paxFare);
};

} // namespace tse

