// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <map>
#include "Taxes/Common/AtpcoTypes.h"

namespace tse {
  class OCFees;
};

namespace tax
{

struct V2TrxMappingDetails
{
  typedef std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > OptionalServicesRefs;
  std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > _ocTaxInclIndMap;
  tax::ItinFarePathMapping _itinFarePathMapping;
  OptionalServicesRefs _optionalServicesRefs;
  tax::FarePathMap _farePathMap;

  bool
  isInTaxInclIndMap(const tax::type::Index& index) const
  {
    return _ocTaxInclIndMap.find(index) != _ocTaxInclIndMap.end();
  }
};

} // namespace tax

