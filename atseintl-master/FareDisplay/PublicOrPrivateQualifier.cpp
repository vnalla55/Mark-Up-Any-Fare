//----------------------------------------------------------------------------
//
//  File:           PublicOrPrivateQualifier.cpp
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

#include "FareDisplay/PublicOrPrivateQualifier.h"

#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"

namespace tse
{
PublicOrPrivateQualifier::PublicOrPrivateQualifier() {}

PublicOrPrivateQualifier::~PublicOrPrivateQualifier() {}

const tse::PaxTypeFare::FareDisplayState
PublicOrPrivateQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL PUB PVT");

  if (trx.getOptions()->isPublicFares() && ptFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
  {
    return PaxTypeFare::FD_Public;
  }

  if (trx.getOptions()->isPrivateFares() && ptFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF)
  {
    return PaxTypeFare::FD_Private;
  }
  return retProc(trx, ptFare);
}

bool
PublicOrPrivateQualifier::setup(FareDisplayTrx& trx)
{
  // TODO: Add the condition when we need to create these qualifiers
  return true;
}
}
