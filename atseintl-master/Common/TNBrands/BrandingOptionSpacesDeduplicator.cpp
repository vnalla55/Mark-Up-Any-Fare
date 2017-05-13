//-------------------------------------------------------------------
//
//  Authors:     Artur de Sousa Rocha
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

#include "Common/BrandingUtil.h"
#include "Common/TNBrands/BrandingOptionSpacesDeduplicator.h"
#include "Common/TNBrands/ItinBranding.h"

#include "Common/TNBrands/ItinBranding.h"

namespace tse
{

namespace skipper
{


GoverningCarrierBrand FareUsageBrandFinder::findBrand(const PricingTrx& trx,
                                                      const Itin& itin,
                                                      const FarePath& farePath,
                                                      const FareUsage& fu) const
{
  const uint16_t brandIndex = farePath.brandIndex();
  const CarrierCode& carrierCode = fu.paxTypeFare()->fareMarket()->governingCarrier();

  if (brandIndex == CHEAPEST_OPTION_INDEX)
  {
    BrandCode brandCode = fu.paxTypeFare()->getFirstValidBrand(trx, fu.getFareUsageDirection());
    return std::make_pair(carrierCode, brandCode.empty() ? NO_BRAND : brandCode);
  }

  const ItinBranding& itinBranding = itin.getItinBranding();
  const uint16_t segmentIndex = itin.segmentOrder(fu.travelSeg().front()) - 1;

  Direction direction = Direction::BOTHWAYS;
  if (BrandingUtil::isDirectionalityToBeUsed(trx))
    direction = fu.paxTypeFare()->getDirection();
  BrandCode brandCode;
  brandCode = itinBranding.getBrandCode(
    brandIndex, segmentIndex, carrierCode, direction, fu.getFareUsageDirection());
  return std::make_pair(carrierCode, brandCode);
}


} /* namespace skipper */

} /* namespace tse */
