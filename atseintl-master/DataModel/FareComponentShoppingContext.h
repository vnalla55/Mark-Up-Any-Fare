//-------------------------------------------------------------------
//
//  File:        FareComponentShoppingContext.h
//  Created:     Apr 29, 2015
//  Authors:     Artur de Sousa Rocha
//
//  Description: Per-fare component data for Shopping Context
//
//  Copyright Sabre 2015
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

#include "Common/TseStringTypes.h"
#include "Common/VCTR.h"

#include <boost/optional.hpp>

#include <map>

namespace tse
{

namespace skipper
{

class FareComponentShoppingContext
{
public:
  virtual ~FareComponentShoppingContext() {}

  std::string fareBasisCode;
  std::string fareCalcFareAmt;
  boost::optional<VCTR> vctr;
  BrandCode brandCode;

  // Returns true if at least some values were filled
  bool isValid()
  {
    return !fareBasisCode.empty() || !fareCalcFareAmt.empty() || bool(vctr) || !brandCode.empty();
  }
};

// Key: fare component number from request (Q6D)
typedef std::map<int, FareComponentShoppingContext*> FareComponentShoppingContextsByFareCompNo;

// Key: TravelSeg::pnrSegment()
typedef std::map<int16_t, FareComponentShoppingContext*> FareComponentShoppingContextsForSegments;

} // skipper namespace
} // tse namespace

