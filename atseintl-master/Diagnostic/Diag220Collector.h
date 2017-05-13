//----------------------------------------------------------------------------
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
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PaxType;

class Diag220Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag220Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag220Collector() {}

  virtual void printHeader() override;
  void writeHeader();
  void displayPaxTypes(PricingTrx& trx);

private:
  void displayCxrPaxTypes(const CarrierCode& carrier, const PaxType& paxType);
};

} // namespace tse

