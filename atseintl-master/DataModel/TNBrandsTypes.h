//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
//
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

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace tse
{

class FareMarket;

namespace skipper
{

typedef std::set<const FareMarket*> FareMarkets;
typedef std::map<BrandCode, FareMarkets> FareMarketsPerBrandCode;

typedef std::set<BrandCode> UnorderedBrandCodes;
typedef std::vector<BrandCode> OrderedBrandCodes;

typedef std::map<const FareMarket*, UnorderedBrandCodes> BrandCodesPerFareMarket;

typedef std::set<int> QualifiedBrandIndices;

typedef std::set<ProgramID> ProgramIds;

struct BrandCodeDirection {
  BrandCode brandCode;
  Direction direction;
  BrandCodeDirection(BrandCode br, Direction dir) : brandCode(br), direction(dir) {}
  inline bool operator==(const BrandCodeDirection& rhs) const
  {
    if ((brandCode == rhs.brandCode) && (direction == rhs.direction))
      return true;
    return false;
  }
  inline bool operator <(const BrandCodeDirection& rhs) const
  {
    if (brandCode == rhs.brandCode)
      return (direction < rhs.direction);
    else
      return (brandCode < rhs.brandCode);
  }
};

struct CarrierDirection {
  CarrierCode carrier;
  Direction direction;
  CarrierDirection(CarrierCode cr, Direction dir) : carrier(cr), direction(dir) {}
  inline bool operator==(const CarrierDirection& rhs) const
  {
    if ((carrier == rhs.carrier) && (direction == rhs.direction))
      return true;
    return false;
  }
  inline bool operator <(const CarrierDirection& rhs) const
  {
    if (carrier == rhs.carrier)
      return (direction < rhs.direction);
    else
      return (carrier < rhs.carrier);
  }
};
typedef std::map<CarrierDirection, UnorderedBrandCodes> BrandCodesPerCarrier;
typedef std::map<CarrierDirection, OrderedBrandCodes> BrandCodeArraysPerCarrier;
typedef std::vector<BrandCodesPerCarrier> SegmentOrientedBrandCodesPerCarrier;
typedef std::vector<BrandCodeArraysPerCarrier> SegmentOrientedBrandCodeArraysPerCarrier;

typedef std::map<size_t, SegmentOrientedBrandCodesPerCarrier> SegmentOrientedBrandCodesPerCarrierInCabin;

typedef std::map<CarrierDirection, BrandCode> CarrierBrandPairs;
typedef std::vector<CarrierBrandPairs> BrandingOptionSpace;
typedef std::vector<BrandingOptionSpace> BrandingOptionSpaces;

typedef std::map<size_t, BrandingOptionSpaces> BrandingOptionSpacesPerCabin;

typedef std::map<LegId, IbfErrorMessage> SoldoutStatusPerLeg;
typedef std::pair<IbfErrorMessage, SoldoutStatusPerLeg> SoldoutStatusInSpace;
typedef std::map<size_t, SoldoutStatusInSpace> SoldoutStatusPerSpace;

typedef std::pair<CarrierCode, BrandCode> GoverningCarrierBrand;

} /* namespace skipper */

} /* namespace tse */

