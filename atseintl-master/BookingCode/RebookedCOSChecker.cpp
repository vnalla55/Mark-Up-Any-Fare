//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include "BookingCode/RebookedCOSChecker.h"

#include "Common/ClassOfService.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"

namespace tse
{

bool
RebookedCOSChecker::checkRebookedCOS()
{
  // Verified by caller but let's make sure someone doesn't break it later.
  TSE_ASSERT(_mkt.classOfServiceVec().size() == _mkt.travelSeg().size());

  const uint16_t numSeats = getNumSeats();

  for (size_t segIdx = 0; segIdx < _mkt.travelSeg().size(); ++segIdx)
  {
    AirSeg* airSeg = _mkt.travelSeg()[segIdx]->toAirSeg();

    if (airSeg == nullptr)
      continue;

    PaxTypeFare::SegmentStatus& segStat = getSegmentStatus(segIdx);

    const std::vector<ClassOfService*>* cosVec = getCosVec(airSeg, segStat, segIdx);

    BookingCode foundBkgCode = getBookingCode(airSeg, segStat);

    for (const auto coService : *cosVec)
    {
      ClassOfService& cOs = *coService;
      if (foundBkgCode == cOs.bookingCode())
      {
        if (cOs.numSeats() < numSeats)
          return false;
        break;
      }
    }
  }

  return true;
}

uint16_t
RebookedCOSChecker::getNumSeats()
{
  return PaxTypeUtil::numSeatsForFare(_trx, _paxTfare);
}

PaxTypeFare::SegmentStatus&
RebookedCOSChecker::getSegmentStatus(size_t segIdx)
{
  if (_fu)
  {
    TSE_ASSERT(segIdx < _fu->segmentStatus().size());
    return _fu->segmentStatus()[segIdx];
  }

  if (_statusTypeNotFlowForLocalJrnyCxr)
  {
    TSE_ASSERT(segIdx < _paxTfare.segmentStatus().size());
    return _paxTfare.segmentStatus()[segIdx];
  }
  else
  {
    TSE_ASSERT(segIdx < _paxTfare.segmentStatusRule2().size());
    return _paxTfare.segmentStatusRule2()[segIdx];
  }
}

const std::vector<ClassOfService*>*
RebookedCOSChecker::getCosVec(AirSeg* airSeg, PaxTypeFare::SegmentStatus& segStat, size_t segIdx)
{
  const std::vector<ClassOfService*>* cosVec =
      _fbcValidator.getAvailability(airSeg, segStat, _paxTfare, segIdx);

  if (cosVec)
    return cosVec;

  return _mkt.classOfServiceVec()[segIdx];
}

BookingCode
RebookedCOSChecker::getBookingCode(AirSeg* airSeg, PaxTypeFare::SegmentStatus& segStat)
{
  if (!segStat._bkgCodeReBook.empty() &&
      segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
  {
    return segStat._bkgCodeReBook;
  }

  return airSeg->getBookingCode();
}

} // namespace tse
