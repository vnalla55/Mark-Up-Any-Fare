//----------------------------------------------------------------------------
//
//  File:           FareRuleNumberQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare for FareRule Number criterion.
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

#include "FareDisplay/FareRuleNumberQualifier.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{
FareRuleNumberQualifier::FareRuleNumberQualifier() {}

FareRuleNumberQualifier::~FareRuleNumberQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareRuleNumberQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL FARE RULE NUMBER");
  LOG4CXX_DEBUG(_logger, "Entering FareRuleNumberQualifier::qualify()...");

  if (!trx.getOptions()->frrRuleNumber().empty() &&
      trx.getOptions()->frrRuleNumber() != ptFare.ruleNumber())
    return PaxTypeFare::FD_Rule_Number;
  else
    return retProc(trx, ptFare);
}

bool
FareRuleNumberQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
