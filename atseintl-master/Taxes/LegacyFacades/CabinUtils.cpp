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


#include "Common/CabinType.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "Taxes/LegacyFacades/CabinUtils.h"

namespace tse
{

tax::type::CabinCode
CabinUtils::translateCabin(const CabinType& cabin)
{
  typedef tax::type::CabinCode CabinCode;
  if (cabin.isEconomyClass())
    return CabinCode(CabinCode::Economy);
  if (cabin.isPremiumEconomyClass())
    return CabinCode(CabinCode::PremiumEconomy);
  if (cabin.isBusinessClass())
    return CabinCode(CabinCode::Business);
  if (cabin.isPremiumBusinessClass())
    return CabinCode(CabinCode::PremiumBusiness);
  if (cabin.isFirstClass())
    return CabinCode(CabinCode::First);
  if (cabin.isPremiumFirstClass())
    return CabinCode(CabinCode::PremiumFirst);

  return CabinCode(CabinCode::Blank);
}

std::pair<BookingCode, tax::type::CabinCode>
CabinUtils::getBookingCodeAndCabin(const Itin& itin,
                                   const FareUsage& fareUsage,
                                   const TravelSeg* travelSegment)
{
  size_t i = 0;
  int segOrder = itin.segmentOrder(travelSegment);
  for(const TravelSeg* fuTravelSegment : fareUsage.travelSeg())
  {
    if (itin.segmentOrder(fuTravelSegment) == segOrder)
    {
      if (i < fareUsage.paxTypeFare()->segmentStatus().size())
      {
        const PaxTypeFare::SegmentStatus& segmentStatus = fareUsage.paxTypeFare()->segmentStatus()[i];
        if (!segmentStatus._bkgCodeReBook.empty() &&
            segmentStatus._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          return std::make_pair(segmentStatus._bkgCodeReBook,
                                translateCabin(segmentStatus._reBookCabin));
        }
      }
    }
    ++i;
  }

  return std::make_pair(travelSegment->getBookingCode(),
                        translateCabin(travelSegment->bookedCabin()));
}

}
