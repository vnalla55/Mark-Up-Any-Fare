//----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "Common/ClassOfService.h"
#include "Common/TseCodeTypes.h"

#include <set>
#include <vector>

namespace tse
{

class DiagManager;

class AvailabilityAdjuster
{
public:
  friend class AvailabilityAdjusterTest;

  struct LessClassOfService
  {
    bool operator()(const ClassOfService* lh, const ClassOfService* rh) const
    {
      return lh->bookingCode() < rh->bookingCode();
    }
  };

  typedef std::set<ClassOfService*, LessClassOfService> ClassOfServiceSet;

  AvailabilityAdjuster(const std::vector<BookingCode>& primary,
                       const std::vector<BookingCode>& secondary)
    : _primaryBookingCodes(primary.begin(), primary.end()),
      _secondaryBookingCodes(secondary.begin(), secondary.end())
  {
  }

  void process(const std::vector<ClassOfService*>& classOfServices, DiagManager& diag) const;

private:
  bool adjustSeats(const std::set<BookingCode>& bookingCodes,
                   const ClassOfServiceSet& classOfServices,
                   DiagManager& diag) const;

  bool isEmpty(const ClassOfServiceSet& set) const;

  std::set<BookingCode> _primaryBookingCodes;
  std::set<BookingCode> _secondaryBookingCodes;
};

} // tse

