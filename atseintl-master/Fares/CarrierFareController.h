//-------------------------------------------------------------------
//
//  File:        CarrierFareController.h
//  Created:     Apr 09, 2004
//  Authors:     Abu Islam, Mark Kasprowicz, Vadim Nikushin
//
//  Description: Carrier fare factory
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

#include "Fares/FareController.h"

#include <vector>

namespace tse
{
class Fare;

class CarrierFareController : public FareController
{
public:
  CarrierFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  virtual bool process(std::vector<Fare*>& constructedFares);

  bool processESV(std::vector<Fare*>& publishedFares);

  void selectFareForShortRequest(std::vector<Fare*>& publishedFares);

private:
  void findFaresMultiAirport(std::vector<Fare*>& fares);
};
} // namespace tse
