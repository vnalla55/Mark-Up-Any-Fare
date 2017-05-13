//----------------------------------------------------------------------------
//
//  File:           FareTypeCodeQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare for FareType code criterion.
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

#include "FareDisplay/FareTypeCodeQualifier.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{
FareTypeCodeQualifier::FareTypeCodeQualifier() {}

FareTypeCodeQualifier::~FareTypeCodeQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareTypeCodeQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL PTF CODE");
  LOG4CXX_DEBUG(_logger, "Entering FareTypeCodeQualifier::qualify()...");

  if (!trx.getOptions()->frrFareTypeCode().empty() &&
      trx.getOptions()->frrFareTypeCode() != ptFare.fcaFareType())
    return PaxTypeFare::FD_Fare_Type_Code;
  else
    return retProc(trx, ptFare);
}

bool
FareTypeCodeQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
