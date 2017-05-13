//-------------------------------------------------------------------
//
//  File:        FDCarrierFareController.h
//  Created:     Aug 10, 2005
//  Authors:     Marco Cartolano
//
//  Description: Carrier fare factory for Fare Display
//
//  Updates:
//          08/10/05 - MC - file created.
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
//-------------------------------------------------------------------

#pragma once

#include "Fares/CarrierFareController.h"

#include <vector>

namespace tse
{
class Fare;

class FDCarrierFareController : public CarrierFareController
{
public:
  FDCarrierFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  bool process(std::vector<Fare*>& constructedFares) override;

private:
  bool isInternationalFare(const Fare& fare) const override;
  void setBoardOffCity(FareMarket& fareMarket, DateTime& travelDate);
  bool resolveTariffInhibits(const Fare& fare) const override;
  void findFares(const CarrierCode& carrier, std::vector<Fare*>& fares) override;
  const std::vector<const FareInfo*>& findCityPairFares(const LocCode& origin,
                                                        const LocCode& destination,
                                                        const CarrierCode& carrier) override;
  bool isProcessMultiAirport() const;
};
} // namespace tse
