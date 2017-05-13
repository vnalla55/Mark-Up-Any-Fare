//----------------------------------------------------------------------------
//  File:        Diag801Collector.h
//  Authors:     Vladimir Reznikov
//  Created:     Mar 2004
//
//  Description: Diagnostic 801 formatter - Tax Customer Diagnostic Display
//  TaxOut Object is formatted to produce the WPQ/ *201 display.
//  The TaxOut Object will be overriden to allow for this specific
//  format to be produced.
//
//  Updates:
//          03/05/04 - VR - Intitial Development
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class TaxResponse;
class TaxItem;

class Diag801Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag801Collector(Diagnostic& root) : DiagCollector(root), _taxDisplayTotal(0) {}
  Diag801Collector() : _taxDisplayTotal(0) {}

  virtual Diag801Collector& operator<<(const TaxResponse& Data) override;

private:
  uint32_t _taxDisplayTotal;

  enum TaxDisplayHeaderType
  {
    DIAGNOSTIC_HEADER = 1
  };

  void
  buildDiag801Header(const TaxResponse& taxResponse, TaxDisplayHeaderType taxDisplayHeaderType);

  void outputResponseNum(DiagCollector& dc, uint32_t taxResponseNum);
  void outputTaxCode(DiagCollector& dc, const TaxItem* taxItem);
  void outputFailCode(DiagCollector& dc, const TaxItem* taxItem);
  void outputTaxType(DiagCollector& dc, const TaxItem* taxItem);
  void outputTaxAmounts(DiagCollector& dc, const TaxItem* taxItem);
  void outputLocalCities(DiagCollector& dc, const TaxItem* taxItem);
  void outputAbsorbtion(DiagCollector& dc, const TaxItem* taxItem);
  void outputThruCities(DiagCollector& dc, const TaxItem* taxItem);
  void outputDescription(DiagCollector& dc, const TaxItem* taxItem);
  void outputRoundRule(DiagCollector& dc, const TaxItem* taxItem);
  void outputSequnceNumber(DiagCollector& dc, const TaxItem* taxItem);
};
} // namespace tse

