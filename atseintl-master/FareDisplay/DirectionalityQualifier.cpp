//----------------------------------------------------------------------------
//
//  File:           DirectionalityQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare for directionality criterion.
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

#include "FareDisplay/DirectionalityQualifier.h"

#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/InclusionCodeConsts.h"

namespace tse
{
DirectionalityQualifier::DirectionalityQualifier() {}

DirectionalityQualifier::~DirectionalityQualifier() {}

const tse::PaxTypeFare::FareDisplayState
DirectionalityQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL BY DIR");

  if (trx.isSameCityPairRqst())
  {
    return retProc(trx, ptFare);
  }
  if (ptFare.directionality() == TO && !trx.getOptions()->swappedDirectionality())
  {
    LOG4CXX_DEBUG(_logger, "Directionality invalidating: " << ptFare.fareClass());
    return PaxTypeFare::FD_Not_Valid_For_Direction;
  }
  return retProc(trx, ptFare);
}

bool
DirectionalityQualifier::setup(FareDisplayTrx& trx)
{
  return !trx.isSameCityPairRqst();
}
}
