//----------------------------------------------------------------------------
//  File:        Diag970Collector.h
//
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"
#include "FareCalc/FcTaxInfo.h"

namespace tse
{
class PricingTrx;
class FarePath;
class Itin;

class Diag970Collector : public DiagCollector
{
public:
  explicit Diag970Collector(Diagnostic& root) : DiagCollector(root), _itinNo(0) {}
  Diag970Collector() : _itinNo(0) {}

  bool displayHeader(const PricingTrx& trx);
  void displayOption(const Itin* itin);
  void displayTaxesByFareUsage(const FareCalc::FcTaxInfo::TaxesPerFareUsage& taxes,
                               const Itin* itin,
                               const FarePath* farePath);
  void displayTaxesByLeg(const FareCalc::FcTaxInfo::TaxesPerLeg& taxes);

private:
  int _itinNo;
};

} // namespace tse

