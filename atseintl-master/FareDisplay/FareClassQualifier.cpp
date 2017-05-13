//----------------------------------------------------------------------------
//
//  File:           FareClassQualifier.cpp
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

#include "FareDisplay/FareClassQualifier.h"

#include "Common/PaxTypeUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "Rules/RuleConst.h"

namespace tse
{
FareClassQualifier::FareClassQualifier() {}

FareClassQualifier::~FareClassQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareClassQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL FARE CLASS");

  const FareDisplayRequest* request = trx.getRequest();

  if (!request->fareBasisCode().empty())
  {
    if (!qualifyFareClass(trx, request->fareBasisCode(), ptFare))
    {
      return PaxTypeFare::FD_FareClass_Match;
    }
    // Eliminate discounted Child and Infant fares with tkt designator if applicable
    if (removeDiscountedChildAndInfantFares(trx, ptFare))
    {
      return PaxTypeFare::FD_Pax_Type_Code;
    }
  }
  return retProc(trx, ptFare);
}

bool
FareClassQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}

bool
FareClassQualifier::qualifyFareClass(FareDisplayTrx& trx,
                                     const FareClassCode& requestedFareClassCode,
                                     const PaxTypeFare& ptFare) const
{
  // Create PaxTypeFare Fare Basis for Comparison
  std::string ptFareFC = ptFare.createFareBasisCodeFD(trx);

  // Remove Tkt Designator from paxTypeFare's Fare Basis if one exists
  std::string paxTypeFareFC = ptFareFC.substr(0, ptFareFC.find('/'));

  // Remove Tkt Designator from requestedFareClassCode if one exists (short entries)
  std::string requestedFC =
      requestedFareClassCode.substr(0, requestedFareClassCode.find('/')).c_str();

  return paxTypeFareFC == requestedFC;
}

// -------------------------------------------------------------------
// @MethodName  FareSelector::removeDiscountedChildAndInfantFares()
//
// Remove discounted child and infant fares with tkt designator
//
// @return bool
// -------------------------------------------------------------------------
bool
FareClassQualifier::removeDiscountedChildAndInfantFares(FareDisplayTrx& trx,
                                                        const PaxTypeFare& ptFare) const
{
  return trx.getRequest()->ticketDesignator().empty() &&
         trx.getRequest()->requestedInclusionCode() != ALL_FARES &&
         trx.getOptions()->lineNumber() == 0 && // is not short RD
         !trx.isERD() && // is not  ERD
         ptFare.isDiscounted() &&
         (PaxTypeUtil::isChild(ptFare.actualPaxType()->paxType(), ptFare.vendor()) ||
          PaxTypeUtil::isInfant(ptFare.actualPaxType()->paxType(), ptFare.vendor())) &&
         // Create fare basis code to check if a tkt designator is present
         ptFare.createFareBasisCodeFD(trx).find('/') != std::string::npos;
}
}
