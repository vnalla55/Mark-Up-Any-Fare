//-------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include <vector>
#include <tuple>

#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputItin.h"

namespace tse
{
class FarePath;
class Itin;
}

namespace tax
{

typedef std::tuple<tse::Itin*, tse::FarePath*, type::CarrierCode, type::Index> ItinFarePathKey;
typedef std::vector<ItinFarePathKey> ItinFarePathMapping;

struct FarePathLink
{
  const tse::FarePath* altPath;
  tse::FarePath* mainPath;
  type::CarrierCode validatingCarrier;
  type::Index index; // sometimes we have the same altPath and mainPath: then index helps disambiguate

  FarePathLink(const tse::FarePath* a, tse::FarePath* m, const type::CarrierCode& v, type::Index i)
  : altPath(a), mainPath(m), validatingCarrier(v), index(i) {}
};

typedef std::vector<FarePathLink> FarePathMap;

} // namespace tax

