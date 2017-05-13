//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
class ClassOfService;

class AvailabilityChecker
{
public:
  template <typename CosVec>
  bool checkAvailability(const uint16_t numSeatsRequired,
                         const BookingCode& bookingCode,
                         const std::vector<CosVec>& cosVecs,
                         const std::vector<TravelSeg*>& travelSegments) const;

  template <typename CosVec, typename SegStatus>
  bool checkBookedAvailability(const uint16_t numSeatsRequired,
                               const std::vector<SegStatus>& segStatusVec,
                               const std::vector<CosVec>& cosVecs,
                               const std::vector<TravelSeg*>& travelSegments) const;

private:
  template <typename T>
  const T& getRef(T* t) const
  {
    TSE_ASSERT(t);
    return *t;
  }

  template <typename T>
  const T& getRef(const T& t) const
  {
    return t;
  }

  template <typename T>
  bool isNull(T* t) const
  {
    return t == nullptr;
  }

  template <typename T>
  bool isNull(const T& t) const
  {
    return false;
  }

  template <typename CosT>
  bool checkAvail(const BookingCode& bookingCode,
                  const std::vector<CosT>& cosVec,
                  const uint16_t numSeatsRequired) const;
};

// ----------------------------------------------------------------------------
// @return true, if the booking code is available on all flights for all
//               carriers within the fare component being priced.
//         false, otherwise
// ----------------------------------------------------------------------------
template <typename CosVec>
bool
AvailabilityChecker::checkAvailability(const uint16_t numSeatsRequired,
                                       const BookingCode& bookingCode,
                                       const std::vector<CosVec>& cosVecs,
                                       const std::vector<TravelSeg*>& travelSegments) const
{
  if (cosVecs.size() < travelSegments.size())
    return false;

  std::vector<TravelSeg*>::const_iterator tvlSegIter = travelSegments.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegEnd = travelSegments.end();
  uint16_t airIndex = 0;

  for (; tvlSegIter != tvlSegEnd; tvlSegIter++, airIndex++)
  {
    if (UNLIKELY(isNull(cosVecs[airIndex])))
      return false;
    if (!checkAvail(bookingCode, getRef(cosVecs[airIndex]), numSeatsRequired))
      return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
// @return true, if rebooked booking codes are available on all flights for all
//               carriers within the fare component being priced.
//         false, otherwise
// ----------------------------------------------------------------------------
template <typename CosVec, typename SegStatus>
bool
AvailabilityChecker::checkBookedAvailability(
    const uint16_t numSeatsRequired,
    const std::vector<SegStatus>& segStatusVec,
    const std::vector<CosVec>& cosVecs,
    const std::vector<TravelSeg*>& travelSegments) const
{
  TSE_ASSERT(cosVecs.size() == travelSegments.size());

  std::vector<TravelSeg*>::const_iterator tvlSegIter = travelSegments.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegEnd = travelSegments.end();
  typename std::vector<SegStatus>::const_iterator segStatIter = segStatusVec.begin();
  uint16_t airIndex = 0;

  //TODO(andrzej.fediuk)FL: verify it this check is needed, if not, change to assertion
  if (segStatusVec.size() < travelSegments.size())
  {
    for (; tvlSegIter != tvlSegEnd; tvlSegIter++, airIndex++)
    {
      if (_UNLIKELY(isNull(cosVecs[airIndex])))
        return false;

      const BookingCode& bookingCode = (*tvlSegIter)->getBookingCode();
      if (!checkAvail(bookingCode, getRef(cosVecs[airIndex]), numSeatsRequired))
        return false;

      return true;
    }
  }

  for (; tvlSegIter != tvlSegEnd; tvlSegIter++, segStatIter++, airIndex++)
  {
    if (_UNLIKELY(isNull(cosVecs[airIndex])))
      return false;

    const SegStatus& segStatus = *segStatIter;
    const BookingCode& bookingCode =
      (segStatus._bkgCodeReBook.empty() || !segStatus._reBookCabin.isValidCabin()) ?
        (*tvlSegIter)->getBookingCode() : segStatus._bkgCodeReBook;
    if (!checkAvail(bookingCode, getRef(cosVecs[airIndex]), numSeatsRequired))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
// @return true, if the booking code is available.
//         false, otherwise
//-----------------------------------------------------------------------------
template <typename CosT>
bool
AvailabilityChecker::checkAvail(const BookingCode& bookingCode,
                                const std::vector<CosT>& cosVec,
                                const uint16_t numSeatsRequired) const
{
  for (const CosT& cs : cosVec)
  {
    if (getRef(cs).isAvailable(bookingCode, numSeatsRequired))
      return true;
  }

  return false;
}
}

