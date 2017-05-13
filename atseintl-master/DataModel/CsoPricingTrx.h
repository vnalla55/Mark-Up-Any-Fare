//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "DataModel/PricingTrx.h"

namespace tse
{

class RexPricingTrx;

class CsoPricingTrx : public PricingTrx
{
public:
  CsoPricingTrx() { _excTrxType = PricingTrx::CSO_EXC_TRX; }

  bool initialize(RexPricingTrx& trx, bool runForDiagnostic);
  void cloneClassOfService();

private:
  void cloneTravelSegments(const std::vector<TravelSeg*>& rexTvlSegs);
  void clonePlusUpPricing(const Itin& itin);

  friend class CsoPricingTrxTest;
};

} // end of namespace tse

