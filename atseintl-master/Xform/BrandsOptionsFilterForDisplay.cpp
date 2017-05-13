#include "Xform/BrandsOptionsFilterForDisplay.h"
#include "Common/TNBrands/ItinBranding.h"

#include <map>

namespace tse
{

BrandsOptionsFilterForDisplay::BrandingSpacesWithSoldoutVector
BrandsOptionsFilterForDisplay::collectOptionsForDisplay(Itin& itin) const
{
  skipper::ItinBranding& itineraryBranding = itin.getItinBranding();
  const BrandingOptionSpaceIndex spacesCount = itineraryBranding.getBrandingOptionSpacesCount();
  const std::set<BrandingOptionSpaceIndex> pricedBrandCombIdxs = itin.getPricedBrandCombinationIndexes();
  BrandingSpacesWithSoldoutVector resultVector;

  for(BrandingOptionSpaceIndex poptSpaceIndex = 0; poptSpaceIndex < spacesCount; ++poptSpaceIndex)
  {
    SoldoutStatus soldout = getSoldOutStatus(pricedBrandCombIdxs, itineraryBranding, poptSpaceIndex);

    if (soldout && soldout.get() == IbfErrorMessage::IBF_EM_NO_FARE_FOUND)
      continue;

    resultVector.emplace_back(poptSpaceIndex, soldout);
  }

  if (pricingTrx_.getNumberOfBrands() != static_cast<size_t>(TnShoppingBrandsLimit::UNLIMITED_BRANDS) &&
      resultVector.size() > pricingTrx_.getNumberOfBrands())
  {
    removeFirstSoldout(resultVector);
    TSE_ASSERT(resultVector.size() <= pricingTrx_.getNumberOfBrands());
  }

  return resultVector;
}

void
BrandsOptionsFilterForDisplay::removeFirstSoldout(
    BrandingSpacesWithSoldoutVector& brandingSpacesWithSoldoutVector) const
{
  auto soldout = std::find_if(brandingSpacesWithSoldoutVector.begin(),
                              brandingSpacesWithSoldoutVector.end(),
                              [](const BrandingOptionWithSoldout& option)
                              { return static_cast<bool>(option.second); });

  if (soldout != brandingSpacesWithSoldoutVector.end())
    brandingSpacesWithSoldoutVector.erase(soldout);
}

BrandsOptionsFilterForDisplay::SoldoutStatus
BrandsOptionsFilterForDisplay::getSoldOutStatus(
  const std::set<BrandingOptionSpaceIndex>& pricedBrandCombIdxs,
  skipper::ItinBranding& itineraryBranding,
  BrandingOptionSpaceIndex spaceIndex) const
{
  if (!pricedBrandCombIdxs.count(spaceIndex))
    return IbfAvailabilityTools::translateForOutput(
             itineraryBranding.getItinSoldoutStatusForSpace(spaceIndex));
  return SoldoutStatus();
}

}
