//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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

#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"

#include <boost/utility.hpp>

#include <map>
#include <set>
#include <string>

namespace tse
{
class Service;

using ProgramIdSet = std::set<std::string>;
using ProgramsForBrandMap = std::map<std::string, ProgramIdSet>;

struct ItinNumLess
{
  bool operator()(const Itin* left, const Itin* right) const
  {
    return left->itinNum() < right->itinNum();
  }
};

using BrandingResponseType = std::map<const Itin*, ProgramsForBrandMap, ItinNumLess>;

class BrandingTrx : public PricingTrx, boost::noncopyable
{
public:
  virtual bool process(Service& srv) override;
};

std::ostream& operator<<(std::ostream& out, const BrandingResponseType& m);
}

