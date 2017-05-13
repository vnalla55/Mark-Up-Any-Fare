//----------------------------------------------------------------------------
//  File:        Diag23XCollector.h
//  Authors:     Artur Krezel
//  Created:     Feb 2004
//
//  Description: Diagnostic 231 & 233 formatter
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
#include "DataModel/FareCompInfo.h"
#include "Diagnostic/DiagCollector.h"

#include <string>

namespace tse
{

class Diag23XCollector : public DiagCollector
{
  friend class Diag23XCollectorTest;

public:
  explicit Diag23XCollector(Diagnostic& root)
    : DiagCollector(root), _allFares(false), _ddAllPax(false), _variance(0.0), _trx(nullptr)
  {
  }
  Diag23XCollector() : _allFares(false), _ddAllPax(false), _variance(0), _trx(nullptr) {}

  double& variance() { return _variance; }
  const double variance() const { return _variance; }

  Diag23XCollector& operator<<(RexBaseTrx& trx);
  Diag23XCollector& operator<<(const FareCompInfo& fc);
  Diag23XCollector& operator<<(const FareMarket& fareMarket) override;
  Diag23XCollector& operator<<(const PaxTypeFare& paxFare) override;
  Diag23XCollector& operator<<(const FareCompInfo::MatchedFare& fare);

  void printSeparator();
  void parseQualifiers(RexBaseTrx& trx);

private:
  bool _allFares;
  bool _ddAllPax; // display detail: all pax types
  VendorCode _vendor;
  FareType _fareType;
  double _variance;

  RexBaseTrx* _trx;
  std::string _printType;

  void printFareAmount(const FareCompInfo& fc);
  void printAmountWithVariance(const PaxTypeFare& ptf);
  void printConvertedAmount(const MoneyAmount& amt, int width);
  void printNetAmountLine(const PaxTypeFare& ptf);
};

} // tse

