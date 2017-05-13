//----------------------------------------------------------------------------
//  File:        Diag02Collector.h
//  Authors:     Mohammad Hossan, Vadim Nikushin
//  Created:     Feb 2004
//
//  Description: Diagnostic 200 formatter
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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Cat31PseudoDecorator;
class FareMarket;
class Itin;

class Diag200Collector : public DiagCollector
{
  friend class Diag200CollectorTest;

public:
  explicit Diag200Collector(Diagnostic& root) : DiagCollector(root) {}

  Diag200Collector() = default;

  Diag200Collector& operator<<(const PaxTypeFare& paxFare) override;
  Diag200Collector& operator<<(const Itin& itn) override;
  Diag200Collector& operator<<(const FareMarket& fareMarket) override;
  Diag200Collector& operator<<(const PricingTrx& x) override;

  void print(const Itin& itn, const FareMarket& fareMarket);

  bool parseQualifiers(PricingTrx& trx, const FareMarket& fareMarket, const Itin* itin = nullptr);
  void parseQualifiers(PricingTrx& trx);

  void setCat31PseudoDecorator(const Cat31PseudoDecorator* c31pd) { _cat31PseudoDecorator = c31pd; }

protected:
  enum SeparatorType
  {
    FARE_HEADER = 1,
    DIVIDER
  };

  FareClassCode _fareClass;

  bool _checkSpecial = false;
  bool _isSpecial = false;
  bool _info = false;
  bool _allFares = false;
  bool _displayR1 = false;

  bool _ddAllPax = false; // display detail: all pax types
  bool _ddFareCount = false; // display detail: fare count summary

  bool _restrictSecurity = false;

  bool _application = false;

  VendorCode _vendor;
  GlobalDirection _globalDirection = GlobalDirection::ZZ;
  FareTypeDesignator _fareTypeDesignator;
  FareType _fareType;
  FareClassCode _fareBasis;
  TariffNumber _tariffNumber = 0;
  RuleNumber _ruleNumber;

  PricingTrx* _trx = nullptr;
  uint16_t _itinIndex = 0;
  const Cat31PseudoDecorator* _cat31PseudoDecorator = nullptr;
  int _rtwContinentCount = 0;

  void writeSeparator(SeparatorType);

  bool checkPaxTypeFare(const PaxTypeFare& paxFare);

  Diag200Collector& displayFareCount(const FareMarket& fm);
  Diag200Collector& displayAllPax(const FareMarket& fm);
};
} // namespace tse
