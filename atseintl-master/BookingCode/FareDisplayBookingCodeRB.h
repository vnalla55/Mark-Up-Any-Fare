//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/TseCodeTypes.h"

namespace tse
{
class AirSeg;
class RBData;

//------------------------------------------------------------------
// This is a base class for the Booking Code Display process.
//------------------------------------------------------------------
class FareDisplayBookingCodeRB : public FareDisplayBookingCode
{
  friend class FareDisplayBookingCodeMock;

public:
  FareDisplayBookingCodeRB(RBData* rbData) : _rbData(rbData) {}

private:
  bool isRBReady() override;
  bool isNotRBReady() override;
  bool isSecondary() override;
  bool isGetBookingCodesEmpty() override;
  bool getBookingCodeException(PaxTypeFare& paxTypeFare,
                               VendorCode vendorA,
                               AirSeg* airSeg,
                               std::vector<BookingCode>& bkgCodes,
                               bool convention) override;
  void setAirSeg(AirSeg** as) override;
  void setCarrierMatchedTable990() override;

  // Retrieve the ExceptT999 object from the objectManager and
  // invoke the object's validate method.

  bool validateT999(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes) override;

  bool isRBRequest();

  void setRBBookingCodes(std::vector<BookingCode>& bkgCodes) override;

  // Attributes:
  RBData* _rbData; // For RB rule text
};
} // end tse namespace
