//-------------------------------------------------------------------
//
//  File:        AncillaryPricingTrx.h
//  Authors:
//
//  Description: Ancillary Pricing (AE* or WP-AE) Transaction object
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

#include <map>
#include <ostream>
#include <vector>

namespace tse
{
class BaggageTravel;

class AncillaryPricingTrx : public PricingTrx
{
public:
  using ItinToBaggageTravelMap = std::map<const Itin*, std::vector<BaggageTravel*>>;

  AncillaryPricingTrx();

  std::ostringstream& response() { return _response; }
  ItinToBaggageTravelMap& baggageTravels() { return _baggageTravels; }
  const ItinToBaggageTravelMap& baggageTravels() const { return _baggageTravels; }

  virtual bool process(Service& srv) override { return srv.process(*this); }
  void checkCurrencyAndSaleLoc();
  bool isAB240AncillaryRequest();
  bool isAddPadisData(bool isPadisAllowed, const ServiceGroup& serviceGroup);
  bool isSecondCallForMonetaryDiscount();

private:
  std::ostringstream _response;
  ItinToBaggageTravelMap _baggageTravels;
};
} // tse namespace
