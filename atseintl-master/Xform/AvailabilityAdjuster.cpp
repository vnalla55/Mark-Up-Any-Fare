//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#include "Xform/AvailabilityAdjuster.h"

#include "Diagnostic/DiagManager.h"

#include <algorithm>
#include <iterator>

namespace tse
{

namespace
{

struct LessBookingCode
{
  bool operator()(const ClassOfService* lh, const BookingCode& rh) const
  {
    return lh->bookingCode() < rh;
  }

  bool operator()(const BookingCode& lh, const ClassOfService* rh) const
  {
    return lh < rh->bookingCode();
  }

  // required by libstdc++ in debug mode to check if range is sorted
  bool operator()(const ClassOfService* lh, const ClassOfService* rh) const
  {
    return lh->bookingCode() < rh->bookingCode();
  }

  // required by libstdc++ in debug mode to check if range is sorted
  bool operator()(const BookingCode& lh, const BookingCode& rh) const { return lh < rh; }
};

template <class IterableClassOfService>
void
resetSeats(const IterableClassOfService& toReset, DiagManager& diag)
{
  for (ClassOfService* item : toReset)
  {
    diag << "BRANDED CHK - SETTING THE AVAILABILITY OF BKCODE " << item->bookingCode()
         << " TO 0.\n";
    item->numSeats() = 0;
  }
}

} // namespace

void
AvailabilityAdjuster::process(const std::vector<ClassOfService*>& classOfServices,
                              DiagManager& diag) const
{
  ClassOfServiceSet cos(classOfServices.begin(), classOfServices.end());

  if (!(adjustSeats(_primaryBookingCodes, cos, diag) ||
        adjustSeats(_secondaryBookingCodes, cos, diag)))
    resetSeats(classOfServices, diag);
}

bool
AvailabilityAdjuster::isEmpty(const ClassOfServiceSet& set) const
{
  for (ClassOfService* item : set)
    if (item->numSeats() > 0)
      return false;

  return true;
}

bool
AvailabilityAdjuster::adjustSeats(const std::set<BookingCode>& bookingCodes,
                                  const ClassOfServiceSet& classOfServices,
                                  DiagManager& diag) const
{
  ClassOfServiceSet intersection;

  std::set_intersection(classOfServices.begin(),
                        classOfServices.end(),
                        bookingCodes.begin(),
                        bookingCodes.end(),
                        std::inserter(intersection, intersection.begin()),
                        LessBookingCode());

  if (intersection.empty())
    return false;

  ClassOfServiceSet toReset;
  std::set_difference(classOfServices.begin(),
                      classOfServices.end(),
                      intersection.begin(),
                      intersection.end(),
                      std::inserter(toReset, toReset.begin()),
                      LessClassOfService());

  resetSeats(toReset, diag);

  return true;
}

} // tse
