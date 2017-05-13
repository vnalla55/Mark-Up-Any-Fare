//----------------------------------------------------------------------------
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
class Itin;
class PricingTrx;

class Diag192Collector : public DiagCollector
{
public:
  friend class Diag192CollectorTest;

  void printTrx();
private:
  void printTravelSegments(PricingTrx& prTrx, const Itin& itin);
  void printRtwArunk(const PricingTrx& prTrx, const Itin& itin);
  void printRtwFareMarkets(PricingTrx& prTrx, const Itin& itin);
  void printItin(Itin& itin);
};

} // namespace tse

