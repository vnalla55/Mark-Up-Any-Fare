//-------------------------------------------------------------------
//
//  File:        FDIndustryFareController.h
//  Created:     Jan. 13th, 2006
//  Authors:     Daryl Champagne
//
//  Description: Fare Display Industry Fare Controller
//
//  Updates:
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

#include "Fares/IndustryFareController.h"

#include <vector>

namespace tse
{
class FareDisplayTrx;
class Itin;
class FareMarket;
class Fare;

class FDIndustryFareController : public IndustryFareController
{
  FareDisplayTrx& _fdTrx;

public:
  FDIndustryFareController(FareDisplayTrx& trx, Itin& itin, FareMarket& fareMarket);

  virtual bool process(const std::vector<Fare*>* addOnFares = nullptr) override;
};
} // namespace tse
