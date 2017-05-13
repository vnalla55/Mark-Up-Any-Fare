//----------------------------------------------------------------------------
//
//  File:           FarePrivateIndicatorQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare for Fare Private Indicator criterion.
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

#include "FareDisplay/FarePrivateIndicatorQualifier.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PrivateIndicator.h"

namespace tse
{

FarePrivateIndicatorQualifier::FarePrivateIndicatorQualifier() {}

FarePrivateIndicatorQualifier::~FarePrivateIndicatorQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FarePrivateIndicatorQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL FARE PRIVATE INDICATOR");
  LOG4CXX_DEBUG(_logger, "Entering FarePrivateIndicatorQualifier::qualify()...");

  Indicator privateIndicator = trx.getOptions()->frrPrivateIndicator();

  if (privateIndicator != ' ')
  {
    bool isFQ = true;
    std::string privateFareIndTag = "";
    PrivateIndicator::privateIndicatorOld(ptFare, privateFareIndTag, true, isFQ);

    if (privateFareIndTag.find(privateIndicator) == std::string::npos)
      return PaxTypeFare::FD_Private_Indicator;
  }

  return retProc(trx, ptFare);
}

bool
FarePrivateIndicatorQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
