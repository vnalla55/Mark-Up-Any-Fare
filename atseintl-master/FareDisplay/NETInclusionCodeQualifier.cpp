//----------------------------------------------------------------------------
//
//  File:           NETInclusionCodeQualifier.cpp
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

#include "FareDisplay/NETInclusionCodeQualifier.h"

#include "Common/FallbackUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"

namespace tse
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
NETInclusionCodeQualifier::NETInclusionCodeQualifier() {}

NETInclusionCodeQualifier::~NETInclusionCodeQualifier() {}

const tse::PaxTypeFare::FareDisplayState
NETInclusionCodeQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL NET INC CODE");

  if (trx.getRequestedInclusionCode() != FD_NET || isNetFareDisabled(trx, ptFare))
  {
    if (ptFare.fareDisplayCat35Type() == RuleConst::NET_FARE ||
        ptFare.fareDisplayCat35Type() == RuleConst::REDISTRIBUTED_FARE)
    {
      return PaxTypeFare::FD_Inclusion_Code;
    }
  }
  else
  {
    if (ptFare.fcaDisplayCatType() == RuleConst::NF_SPECIFIED)
    {
      return PaxTypeFare::FD_Inclusion_Code;
    }
  }
  return retProc(trx, ptFare);
}

bool
NETInclusionCodeQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}

bool
NETInclusionCodeQualifier::isNetFareDisabled(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  const NegPaxTypeFareRuleData* negData = ptFare.getNegRuleData();

  return negData && negData->cat35Level() == NET_LEVEL_NOT_ALLOWED;
}
}
