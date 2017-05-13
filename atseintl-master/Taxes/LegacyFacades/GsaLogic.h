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

// GSA stands for "General Sales Agent". This logic deals with substituting validating carriers.

#pragma once
#include "Common/AtpcoTypes.h"
#include "DataModel/RequestResponse/OutputItins.h"
#include <iosfwd>

namespace tse
{
class FarePath;
class PricingTrx;
}

namespace tax
{

class V2TrxMappingDetails;

ItinFarePathMapping buildBestFarePathMap(const V2TrxMappingDetails& mappings,
                                         const OutputItins& solutions,
                                         tse::PricingTrx& trx,
                                         std::ostream* diagnostic = nullptr);
class ForwardFarePath
{
public:
  const tse::FarePath* operator() (const tse::FarePath* input) const { return input; }
};

class GetMainFarePath
{
  const tax::FarePathMap& _farePathMap;
public:
  GetMainFarePath(const tax::FarePathMap& fpm) : _farePathMap(fpm) {}
  const tse::FarePath* operator() (const tse::FarePath* input) const;
};

} // namespace tax

