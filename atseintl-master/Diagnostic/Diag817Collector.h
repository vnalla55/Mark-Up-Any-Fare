//----------------------------------------------------------------------------
//  File:        Diag817Collector.h
//  Authors:     Piotr Badarycz
//  Created:     Oct 2008
//
//  Description: Basic tax diagnostic
//
//  Updates:
//          10/1/2008 - VR - Intitial Development
//
//  Copyright Sabre 2008
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

namespace tse
{
class TaxResponse;
class BSRCollectionResults;

class Diag817Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag817Collector(Diagnostic& root) : DiagCollector(root), _taxDisplayTotal(0) {}
  Diag817Collector() : _taxDisplayTotal(0) {}

  virtual Diag817Collector& operator<<(const TaxResponse& taxResponse) override;
  void displayInfo(void) override;

  void accept(DiagVisitor& diagVisitor) override
  {
    diagVisitor.visit(*this);
  }

private:
  uint32_t _taxDisplayTotal;

  enum TaxDisplayHeaderType
  {
    DIAGNOSTIC_HEADER = 1
  };

  void buildDiag817Header(const TaxResponse& taxResponse,
                          TaxDisplayHeaderType taxDisplayHeaderType,
                          bool utc_config);

  void displayDiag817(std::vector<TaxItem*>& taxItemVector,
                      DiagCollector& dc,
                      uint32_t& taxResponseNum,
                      bool utc_display);
  void buildDiag817(const TaxResponse& taxResponse);

  void buildDiag817OcHeader(const PaxTypeCode& pax);
  void buildDiag817Oc(const TaxResponse& taxResponse);

  void buildDiag817DcHeader(const TaxCode& taxCode,
                            const uint32_t& taxResponseNum,
                            const CarrierCode& cxr);

  void buildDiag817Dc(const TaxResponse& taxResponse);

  const std::string roundingToString(const RoundingFactor& roundingFactor,
                                     const CurrencyNoDec& roundingFactorNoDec,
                                     const RoundingRule& roundingRule) const;
  const std::string
  roundingToString(const BSRCollectionResults& results, const uint32_t& roundingNo) const;
  void printExchange(const BSRCollectionResults& results);
};
} // namespace tse

