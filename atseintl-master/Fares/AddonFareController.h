//-------------------------------------------------------------------
//
//  File:        AddonFareController.h
//  Created:     Apr 09, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Add-on construction factory
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

namespace tse
{

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

class SpecifiedFareCache;


class AddonFareController : public FareController
{
public:
  AddonFareController(PricingTrx& trx,
                      Itin& itin,
                      FareMarket& fareMarket,
                      SpecifiedFareCache* specFareCache);

  bool process(std::vector<Fare*>& cxrFares, std::vector<Fare*>& yFares);

private:
  template <class ContainerType>
  void postprocess(std::vector<Fare*>& cxrFares,
                   std::vector<Fare*>& yFares,
                   ContainerType& responseHashSet);

  SpecifiedFareCache* _specFareCache;
};

#else

class AddonFareController : public FareController
{
public:
  AddonFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  bool process(std::vector<Fare*>& cxrFares, std::vector<Fare*>& yFares);

private:
  template <class ContainerType>
  void postprocess(std::vector<Fare*>& cxrFares,
                   std::vector<Fare*>& yFares,
                   ContainerType& responseHashSet);
};

#endif

} // namespace tse
