//-------------------------------------------------------------------
//
//  File:        Group.h
//  Created:     June 19, 2005
//  Authors:     Abu
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "BrandedFares/OneProgramOneBrand.h"
#include "BrandingService/BrandResponseItem.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

/**
*   @class Group
*   The basic container class for Grouping and Sorting.
*   Holds all information that needed for a specific grouping and sorting.
*
*/

class FareDisplaySort;
class BrandedFareApp;
class Comparator;

class Group final
{

public:
  enum GroupType
  {
    GROUP_BY_CARRIER = 1,
    GROUP_BY_ROUTING,
    GROUP_BY_GLOBAL_DIR,
    GROUP_BY_PSG_TYPE,
    GROUP_BY_FARE_BASIS_CODE,
    GROUP_BY_FARE_BASIS_CHAR_COMB,
    GROUP_BY_OW_RT,
    GROUP_BY_PUBLIC_PRIVATE,
    GROUP_BY_NORMAL_SPECIAL,
    GROUP_BY_FARE_AMOUNT,
    GROUP_BY_TRAVEL_DISCONTINUE_DATE,
    GROUP_BY_SCHEDULE_COUNT,
    GROUP_BY_MULTITRANSPORT,
    GROUP_BY_BRAND,
    GROUP_BY_S8BRAND,
    GROUP_BY_CABIN,
    GROUP_NOT_REQUIRED
  };

  static constexpr char ASCENDING = 'A';
  static constexpr char DESCENDING = 'D';
  static constexpr char APPLY_PRIORITY_LIST = 'L';
  static constexpr char NORMAL_OVER_SPECIAL = 'N';
  static constexpr char SPECIAL_OVER_NORMAL = 'S';
  static constexpr char ONE_WAY_OVER_ROUND_TRIP = 'O';
  static constexpr char ROUND_TRIP_OVER_ONE_WAY = 'R';
  static constexpr char PRIVATE_OVER_PUBLIC = 'R';
  static constexpr char PUBLIC_OVER_PRIVATE = 'P';
  static constexpr char FARE_BASIS_CODE = 'Y';
  static constexpr char FARE_BASIS_COMB_CODE = 'Y';

  GroupType& groupType() { return _groupType; }
  const GroupType& groupType() const { return _groupType; }

  Indicator& sortType() { return _sortType; }
  const Indicator& sortType() const { return _sortType; }

  FareDisplaySort*& sortData() { return _sortData; }
  const FareDisplaySort* sortData() const { return _sortData; }

  Comparator*& comparator() { return _comparator; }
  const Comparator* comparator() const { return _comparator; }

  std::map<BookingCode, BrandCode>& brandMap() { return _brandMap; }
  const std::map<BookingCode, BrandCode> brandMap() const { return _brandMap; }

  std::vector<BrandedFareApp*>& brandedFareApps() { return _brandedFareApps; }
  const std::vector<BrandedFareApp*> brandedFareApps() const { return _brandedFareApps; }

  std::vector<BrandResponseItem*>& brandResponseItemVec() { return _brandResponseItemVec; }
  const std::vector<BrandResponseItem*> brandResponseItemVec() const
  {
    return _brandResponseItemVec;
  }

  using OneProgramOneBrandMap = std::map<CarrierCode, std::vector<OneProgramOneBrand*>>;
  OneProgramOneBrandMap& programBrandMap() { return _singleProgBrandMap; }
  const OneProgramOneBrandMap& programBrandMap() const { return _singleProgBrandMap; }

private:
  GroupType _groupType = GroupType::GROUP_NOT_REQUIRED;
  Indicator _sortType = ' ';
  char _blank = ' ';
  FareDisplaySort* _sortData = nullptr;
  Comparator* _comparator = nullptr;

  // for Brand grouping
  std::map<BookingCode, BrandCode> _brandMap;
  std::vector<BrandedFareApp*> _brandedFareApps;

  // new from brand service
  std::vector<BrandResponseItem*> _brandResponseItemVec;

  // Branded Fares 2013
  OneProgramOneBrandMap _singleProgBrandMap;
};
}
