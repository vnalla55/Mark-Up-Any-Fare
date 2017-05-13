//----------------------------------------------------------------------------
//
//  File:           FareCatSelectQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare for Fare Cat Select criterion.
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

#include "FareDisplay/FareCatSelectQualifier.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{
FareCatSelectQualifier::FareCatSelectQualifier() {}

FareCatSelectQualifier::~FareCatSelectQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareCatSelectQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL FARE CAT SELECT TYPR");
  LOG4CXX_DEBUG(_logger, "Entering FareCatSelectQualifier::qualify()...");

  if (trx.getOptions()->frrSelectCat35Fares() && !ptFare.isNegotiated())
    return PaxTypeFare::FD_Negotiated_Select;
  else if (trx.getOptions()->frrSelectCat25Fares() && !ptFare.isFareByRule())
    return PaxTypeFare::FD_FBR_Select;
  else if (trx.getOptions()->frrSelectCat15Fares() &&
                (ptFare.isNegotiated()||
                (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE  ||
                 ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
                 ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) ||
                 ptFare.tcrTariffCat() == RuleConst::PUBLIC_TARIFF))
    return PaxTypeFare::FD_Sales_Restriction_Select;
  else
    return retProc(trx, ptFare);
}

bool
FareCatSelectQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
