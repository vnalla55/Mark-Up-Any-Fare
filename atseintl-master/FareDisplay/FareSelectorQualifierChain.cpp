//////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------
//
//  File:        FareSelectorQualifierChain.cpp
//  Created:     Feb 6, 2006
//  Authors:     J.D. Batchelor
//
//  Updates:
//
//  Copyright Sabre 2006
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/FareSelectorQualifierChain.h"

#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "FareDisplay/BookingCodeQualifier.h"
#include "FareDisplay/FareTypeCodeQualifier.h"
#include "FareDisplay/FareRuleTariffQualifier.h"
#include "FareDisplay/FareRuleNumberQualifier.h"
#include "FareDisplay/FareDisplayTypeQualifier.h"
#include "FareDisplay/FareCatSelectQualifier.h"
#include "FareDisplay/FarePrivateIndicatorQualifier.h"
#include "FareDisplay/DirectionalityQualifier.h"
#include "FareDisplay/FareClassAppSegInfoQualifier.h"
#include "FareDisplay/FareClassAppUnavailTagQualifier.h"
#include "FareDisplay/FareClassQualifier.h"
#include "FareDisplay/InclusionCodeQualifier.h"
#include "FareDisplay/NETInclusionCodeQualifier.h"
#include "FareDisplay/OWRTQualifier.h"
#include "FareDisplay/PublicOrPrivateQualifier.h"
#include "FareDisplay/TicketDesignatorQualifier.h"
#include "FareDisplay/TravelDateQualifier.h"

#include "Common/FallbackUtil.h"

#include <vector>

namespace tse
{

static Logger
logger("atseintl.FareDisplay.FareSelectorQualifierChain");

FareSelectorQualifierChain::FareSelectorQualifierChain() : _itsQualifier(nullptr) {}

bool
FareSelectorQualifierChain::buildChain(FareDisplayTrx& trx, bool useAll)
{
  LOG4CXX_DEBUG(logger, "Entering FareSelectorQualifierChain::buildChain()...");
  if (_itsQualifier)
  {
    // Must have a chain.  Need to demolish it first.
    demolishChain();
  }
  Qualifier* q = nullptr;

  q = setSuccessor<OWRTQualifier>(q, trx);
  if (useAll)
    q = setSuccessor<NETInclusionCodeQualifier>(q, trx);
  q = setSuccessor<FareClassAppUnavailTagQualifier>(q, trx);
  q = setSuccessor<PublicOrPrivateQualifier>(q, trx);
  if (useAll)
    q = setSuccessor<TicketDesignatorQualifier>(q, trx);
  q = setSuccessor<DirectionalityQualifier>(q, trx);
  q = setSuccessor<InclusionCodeQualifier>(q, trx);
  q = setSuccessor<TravelDateQualifier>(q, trx);
  q = setSuccessor<FareClassAppSegInfoQualifier>(q, trx);
  q = setSuccessor<BookingCodeQualifier>(q, trx);
  q = setSuccessor<FareTypeCodeQualifier>(q, trx);
  q = setSuccessor<FareRuleTariffQualifier>(q, trx);
  q = setSuccessor<FareRuleNumberQualifier>(q, trx);
  q = setSuccessor<FareDisplayTypeQualifier>(q, trx);
  q = setSuccessor<FareCatSelectQualifier>(q, trx);
  q = setSuccessor<FarePrivateIndicatorQualifier>(q, trx);
  if (useAll)
    q = setSuccessor<FareClassQualifier>(q, trx);

  return true;
}

const tse::PaxTypeFare::FareDisplayState
FareSelectorQualifierChain::qualifyPaxTypeFare(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL PAXTYPEFARE");

  LOG4CXX_DEBUG(logger, "Entering FareSelectorQualifierChain::qualifyPaxTypeFare()...");
  if (_itsQualifier)
  {
    LOG4CXX_DEBUG(logger, "Calling qualify()...");
    tse::PaxTypeFare::FareDisplayState ret = _itsQualifier->qualify(trx, ptFare);
    if (ret == tse::PaxTypeFare::FD_Valid)
    {
      LOG4CXX_INFO(logger, "This PaxTypeFare is valid...");
    }
    else
    {
      LOG4CXX_INFO(logger, "This PaxTypeFare is INVALID for reason= " << ret);
    }
    return ret;
  }
  else
  {
    LOG4CXX_ERROR(logger, "returning FD_Pax_Type_Code, no Qualify Chain!!!...");
    return PaxTypeFare::FD_Pax_Type_Code;
  }
}
}
