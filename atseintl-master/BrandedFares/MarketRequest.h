//-------------------------------------------------------------------
//
//  File:        MarketRequest.h
//  Created:     March 2013
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class MarketCriteria;

class MarketRequest : boost::noncopyable
{
public:
  using CarrierList = std::vector<CarrierCode>;

  int& setMarketID() { return _marketID; }
  int getMarketID() const { return _marketID; }

  MarketCriteria*& marketCriteria() { return _marketCriteria; }
  const MarketCriteria* marketCriteria() const { return _marketCriteria; }

  CarrierList& carrriers() { return _carriers; }
  const CarrierList& carrriers() const { return _carriers; }

private:
  int _marketID = 0;
  MarketCriteria* _marketCriteria = nullptr;
  CarrierList _carriers;
};
} // tse

