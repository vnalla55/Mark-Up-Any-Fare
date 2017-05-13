//----------------------------------------------------------------------------
//
//  File:           FareClassAppUnavailTagQualifier.cpp
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

#include "FareDisplay/FareClassAppUnavailTagQualifier.h"

#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/InclusionCodeConsts.h"

namespace tse
{
FareClassAppUnavailTagQualifier::FareClassAppUnavailTagQualifier() {}

FareClassAppUnavailTagQualifier::~FareClassAppUnavailTagQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareClassAppUnavailTagQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL F CLASS APP UNAVAIL");

  if (trx.getRequestedInclusionCode() != ASEAN_ADDON_FARES)
  {
    // Check for Unavailable Tag 'Z' in FareClassApp (SITA value)
    if (ptFare.fcaUnavailTag() == FareClassAppUnavailTagQualifier::FARE_NOT_TO_BE_DISPLAYED)
    {
      LOG4CXX_DEBUG(_logger, "FareClassAppUnavail Tag invalidating: " << ptFare.fareClass());
      return PaxTypeFare::FD_FareClassApp_Unavailable;
    }
  }
  return retProc(trx, ptFare);
}

bool
FareClassAppUnavailTagQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
