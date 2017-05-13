//----------------------------------------------------------------------------
//
//  File:           FareDisplayTypeQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare for FareRule Tariff criterion.
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

#include "FareDisplay/FareDisplayTypeQualifier.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{
FareDisplayTypeQualifier::FareDisplayTypeQualifier() {}

FareDisplayTypeQualifier::~FareDisplayTypeQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareDisplayTypeQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL FARE DISPLAY TYPR");
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayTypeQualifier::qualify()...");

  if (trx.getOptions()->frrDisplayType() != ' ' &&
      trx.getOptions()->frrDisplayType() != ptFare.fcaDisplayCatType())
    return PaxTypeFare::FD_Display_Type;
  else
    return retProc(trx, ptFare);
}

bool
FareDisplayTypeQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
