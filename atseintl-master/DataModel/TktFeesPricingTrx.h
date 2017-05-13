//-------------------------------------------------------------------
//
//  File:        TktFeesPricingTrx.h
//  Authors:
//
//  Description: Ticketing Fees Pricing  Transaction object
//
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
//-------------------------------------------------------------------

#pragma once

#include "DataModel/PricingTrx.h"

namespace tse
{

class TktFeesPricingTrx : public PricingTrx
{
public:
  TktFeesPricingTrx() { setTrxType(PRICING_TRX); }

  virtual bool process(Service& srv) override { return srv.process(*this); }
};
} // tse namespace
