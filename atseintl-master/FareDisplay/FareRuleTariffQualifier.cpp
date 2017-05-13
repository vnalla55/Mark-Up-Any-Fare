//----------------------------------------------------------------------------
//
//  File:           FareRuleTariffQualifier.cpp
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

#include "FareDisplay/FareRuleTariffQualifier.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{
FareRuleTariffQualifier::FareRuleTariffQualifier() {}

FareRuleTariffQualifier::~FareRuleTariffQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareRuleTariffQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL FARE RULE TARIFF");
  LOG4CXX_DEBUG(_logger, "Entering FareRuleTariffQualifier::qualify()...");

  if (trx.getOptions()->frrTariffNumber() != 0 &&
      trx.getOptions()->frrTariffNumber() != ptFare.tcrRuleTariff())
    return PaxTypeFare::FD_Rule_Tariff;
  else
    return retProc(trx, ptFare);
}

bool
FareRuleTariffQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
