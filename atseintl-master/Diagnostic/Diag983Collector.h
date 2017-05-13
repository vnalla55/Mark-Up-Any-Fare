//----------------------------------------------------------------------------
//  File:        Diag983Collector.h
//  Created:     2010-05-19
//
//  Description: Diagnostic 983 formatter
//
//  Updates:
//
//  Copyright Sabre 2010
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

#include <string>
#include <vector>

namespace tse
{
class PfcItem;
class PricingTrx;
class TaxItem;
class TaxRecord;

class Diag983Collector : public DiagCollector
{
public:
  explicit Diag983Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag983Collector() {}

  virtual Diag983Collector& operator<<(const PricingTrx& shoppingTrx) override;

private:
  void displayItinVec(const std::vector<Itin*>& itinVec, const std::string& separator);
  void displayFarePaths(const Itin& itin);
  void displayTaxResponse(const Itin& itin);
  virtual DiagCollector& operator<<(const TaxItem& x) override;
  virtual DiagCollector& operator<<(const PfcItem& x) override;
  virtual DiagCollector& operator<<(const TaxRecord& x) override;
  void setPrecision(const CurrencyCode& currencyCode);
};

} // namespace tse

