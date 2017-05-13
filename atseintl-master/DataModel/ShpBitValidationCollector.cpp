// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "DataModel/ShpBitValidationCollector.h"

#include "BookingCode/BCETuning.h"
#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "DataModel/FareMarket.h"
#include "Fares/RoutingController.h"

#include <set>

namespace tse
{

void
ShpBitValidationCollector::FMValidationSharedData::updateFareMarketData(FareMarket* fareMarket,
                                                                        const uint32_t bit)
{
  fareMarket->mileageInfo(true) = _sharedData[bit].getMileageInfOutbound();
  fareMarket->mileageInfo(false) = _sharedData[bit].getMileageInfInbound();
}

void
ShpBitValidationCollector::FMValidationSharedData::collectFareMarketData(FareMarket* fareMarket,
                                                                         const uint32_t bit)
{
  _sharedData[bit].setMileageInfOutbound(fareMarket->mileageInfo(true));
  _sharedData[bit].setMileageInfInbound(fareMarket->mileageInfo(false));
}

ShoppingRtgMap*
ShpBitValidationCollector::FMValidationSharedData::getRoutingMapForBit(const uint32_t bit)
{
  return &_sharedData[bit].getRoutingMap();
}

std::vector<BCETuning>*
ShpBitValidationCollector::FMValidationSharedData::getBCEData(const uint32_t bit)
{
  return &_sharedData[bit].getBCEData();
}

ShpBitValidationCollector::FMValidationSharedData*
ShpBitValidationCollector::getFMSharedData(const ItinIndex::Key carrierKey,
                                           const FareMarket* fareMarket)
{
  return &_cxrValidationShareMap[carrierKey][fareMarket];
}

} /* namespace tse */
