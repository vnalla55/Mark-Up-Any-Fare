//-------------------------------------------------------------------
//
//  File:        S8Brand.h
//  Created:     April 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "FareDisplay/S8Brand.h"

#include <boost/noncopyable.hpp>
#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace tse
{
class FareDisplayTrx;
class PricingTrx;
class Group;
class MarketResponse;
class OneProgramOneBrand;

class S8Brand : boost::noncopyable
{
  friend class S8BrandTest;

public:
  S8Brand();
  void initializeS8BrandGroup(FareDisplayTrx& trx, std::vector<Group*>& groups);

private:
  void buildProgramBrandMap(FareDisplayTrx& trx, Group& group);
  void buildOneProgramOneBrand(PricingTrx& trx,
                               MarketResponse& mR,
                               std::vector<OneProgramOneBrand*>& spbVec);
};
} // tse

