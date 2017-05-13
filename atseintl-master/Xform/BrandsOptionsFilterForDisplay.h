#pragma once

#include "Common/TseEnums.h"

#include <set>
#include <boost/optional.hpp>

namespace tse
{
class PricingTrx;
class Itin;

namespace skipper
{
class ItinBranding;
}

class BrandsOptionsFilterForDisplay
{
public:
  using BrandingOptionSpaceIndex = uint16_t;
  using SoldoutStatus = boost::optional<IbfErrorMessage>;
  using BrandingOptionWithSoldout = std::pair<BrandingOptionSpaceIndex, SoldoutStatus>;
  using BrandingSpacesWithSoldoutVector = std::vector<BrandingOptionWithSoldout>;

  BrandsOptionsFilterForDisplay(const PricingTrx& pricingTrx): pricingTrx_(pricingTrx)
  {
  }

  BrandingSpacesWithSoldoutVector collectOptionsForDisplay(Itin& itin) const;

private:

  void removeFirstSoldout(BrandingSpacesWithSoldoutVector& brandingSpacesWithSoldoutVector) const;

  SoldoutStatus getSoldOutStatus(const std::set<BrandingOptionSpaceIndex>& pricedBrandCombIdxs,
                                 skipper::ItinBranding& itineraryBranding,
                                 BrandingOptionSpaceIndex spaceIndex) const;

  const PricingTrx& pricingTrx_;
};


} // End namespace tse

