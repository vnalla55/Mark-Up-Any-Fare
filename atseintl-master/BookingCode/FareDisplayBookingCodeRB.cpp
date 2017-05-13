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
#include "BookingCode/FareDisplayBookingCodeRB.h"

#include "BookingCode/FareDisplayBookingCodeException.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

#include <iomanip>
#include <iostream>
#include <vector>

namespace tse
{
bool
FareDisplayBookingCodeRB::isRBReady()
{
  return (isRBRequest() && (_rbData != nullptr));
}

bool
FareDisplayBookingCodeRB::isNotRBReady()
{
  return (!isRBRequest() || (_rbData != nullptr && !_rbData->isSecondary()));
}

bool
FareDisplayBookingCodeRB::isSecondary()
{
  if (_rbData)
    return _rbData->isSecondary();
  else
    return false;
}

void
FareDisplayBookingCodeRB::setAirSeg(AirSeg** as)
{
  if (isRBReady() && isSecondary())
    *as = _rbData->airSeg();
}

void
FareDisplayBookingCodeRB::setCarrierMatchedTable990()
{
  if (isRBReady())
    _rbData->setCarrierMatchedTable990(true);
}

bool
FareDisplayBookingCodeRB::isGetBookingCodesEmpty()
{
  if (_rbData)
    return _rbData->getBookingCodes().empty();
  else
    return false;
}

bool
FareDisplayBookingCodeRB::getBookingCodeException(PaxTypeFare& paxTypeFare,
                                                  VendorCode vendorA,
                                                  AirSeg* airSeg,
                                                  std::vector<BookingCode>& bkgCodes,
                                                  bool convention)
{
  FareDisplayBookingCodeException fdbce(_trx, &paxTypeFare, _rbData);
  return fdbce.getBookingCodeException(vendorA, bkgCodes, airSeg, convention);
}

// ----------------------------------------------------------------
// Retrieve the ExceptT999 object from the objectManager and invoke
// the object's validate method.
// ----------------------------------------------------------------
bool
FareDisplayBookingCodeRB::validateT999(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCodeRB::validateT999()");
  FareDisplayBookingCodeException fdbce(_trx, &paxTypeFare, _rbData); // BookingCodeExcept object
  return fdbce.getBookingCodeException(bkgCodes);
}

//--------------------------------------------------------------------
// Return true if RB this is transaction
//--------------------------------------------------------------------
bool
FareDisplayBookingCodeRB::isRBRequest()
{
  if (_rbData == nullptr)
    return false;
  return (_trx->getRequest()->requestType() == "RB");
}

void
FareDisplayBookingCodeRB::setRBBookingCodes(std::vector<BookingCode>& bkgCodes)
{
  if (isRBRequest())
  {
    _rbData->setBookingCodes(bkgCodes);
  }
}
}
