//----------------------------------------------------------------------------
//
//  File:           OWRTQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/OWRTQualifier.h"

#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/InclusionCodeConsts.h"

namespace tse
{
OWRTQualifier::OWRTQualifier() {}

OWRTQualifier::~OWRTQualifier() {}

bool
OWRTQualifier::setup(FareDisplayTrx& trx)
{
  if (!trx.isRD())
  {
    bool qualifierRequested =
        (trx.getOptions()->oneWayFare() || trx.getOptions()->roundTripFare() ||
         trx.getOptions()->halfRoundTripFare());
    return qualifierRequested;
  }
  return false;
}

const tse::PaxTypeFare::FareDisplayState
OWRTQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  if ((trx.getOptions()->isOneWayFare()) && (ptFare.isRoundTrip()))
  {
    return PaxTypeFare::FD_OW_RT;
  }
  else if ((trx.getOptions()->isRoundTripFare()) && (ptFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED))
  {
    return PaxTypeFare::FD_OW_RT;
  }
  else if ((trx.getOptions()->isHalfRoundTripFare()) && (!ptFare.isRoundTrip()) &&
           (ptFare.owrt() != ONE_WAY_MAY_BE_DOUBLED))
  {
    return PaxTypeFare::FD_OW_RT;
  }
  return retProc(trx, ptFare);
}
}
