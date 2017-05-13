//-------------------------------------------------------------------
//
//  File:        AltPricingTrx.h
//  Authors:
//
//  Description: Alternative Pricing (WPA) Transaction object
//
//
//  Copyright Sabre 2006
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
#include "DataModel/AltPricingTrxData.h"

namespace tse
{

class AltPricingTrx : public PricingTrx, public AltPricingTrxData
{
public:
  AltPricingTrx();
  bool process(Service& srv) override;

  bool convert(std::string& response) override;

  const static uint32_t WPA_50_OPTIONS = 50;

  bool isSingleMatch() const override;
};

inline AltPricingTrx::AltPricingTrx()
  : AltPricingTrxData()
{
  setTrxType(PRICING_TRX);
  altTrxType() = WPA;
}
} // tse namespace
