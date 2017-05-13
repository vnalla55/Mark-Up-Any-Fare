//----------------------------------------------------------------
//
//  File:              FDFareMarketRuleController.cpp
//
//  Author:      Daryl Champagne
//  Created:     April 5, 2006
//  Description: FDFareMarketRuleController class for Fare Display
//
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------
#include "Rules/FDFareMarketRuleController.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareByRuleApp.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

namespace tse
{
namespace
{
Logger
logger("atseintl.Rules.FDFareMarketRuleController");
ConfigurableValue<bool>
fdGetBkgcodeDuringFareval("FARESV_SVC", "FD_GET_BKGCODE_DURING_FAREVAL", true);
}

//----------------------------------------------------------------------------
// validate(): what the outside world calls...
//----------------------------------------------------------------------------
bool
FDFareMarketRuleController::validate(PricingTrx& trx, Itin& itin, PaxTypeFare& paxTypeFare)
{
  FareDisplayTrx* fdTrx = nullptr;
  if (!FareDisplayUtil::getFareDisplayTrx(&trx, fdTrx))
  {
    LOG4CXX_ERROR(logger, "validate(): Null FareDisplayTrx!");
    return false;
  }

  bool ret = FareMarketRuleController::validate(*fdTrx, itin, paxTypeFare);

  // ensure all invalid fares have FD error code
  if (!paxTypeFare.isValidForPricing())
  {
    // if ( fdErr.isNull() )
    //    fdErr = new err code
  }

  // have to wait after rule validation to see if acct/corp option applies
  if (fdTrx->getOptions()->isUniqueAccountCode() || fdTrx->getOptions()->isUniqueCorporateID())
  {
    if (!usedAcctOrCorp(paxTypeFare))
      paxTypeFare.invalidateFare(PaxTypeFare::FD_Corp_ID_AccCode);
  }

  if (paxTypeFare.fare()->isMissingFootnote())
  {
    paxTypeFare.invalidateFare(PaxTypeFare::FD_Missing_Footnote_Data);
  }

  // TODO - Remove this once we determine if we can get the booking
  //        code safely from FVO instead of from FareDisplay Service.
  // Method to determine if the booking code is to be retrieved from FVO
  // instead of FareDisplay service where it can potentially execute threaded.
  if (fdGetBkgcodeDuringFareval.getValue())
  {
    if (!fdTrx->isFT())
    {
      if (paxTypeFare.bookingCode().empty())
      {
#ifdef DEBUG_PERF
        TSELatencyData tld(trx, "FDFMRC GET BOOKING CODE");
#endif
        FareDisplayBookingCode fdbc;
        fdbc.getBookingCode(*fdTrx, paxTypeFare, paxTypeFare.bookingCode());
      }
    }
  }

  if (!isValidForCWTuser(trx, paxTypeFare))
  {
    paxTypeFare.invalidateFare(PaxTypeFare::FD_Invalid_For_CWT);
  }

  return ret;
}

bool
FDFareMarketRuleController::usedAcctOrCorp(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.matchedCorpID())
    return true;

  // when a FBR w/ non-empty acctCode gets through rule validation, we know it was used
  if (paxTypeFare.isFareByRule())
    return !paxTypeFare.getFbrRuleData()->fbrApp()->accountCode().empty();

  return false;
}

bool
FDFareMarketRuleController::isValidForCWTuser(PricingTrx& trx, PaxTypeFare& paxTypeFare)
{
  bool ret = true;

  // The market fare is not valid for CWT user
  // The market fare: 1. Private tariff,
  //                  2. PTC is ADT/NEG,
  //                  3. 'Nation France' is coded in cat15/Cat35 security;
  //                  4. No matched Corp_id/account code in Cat1/Rec8(Cat25)/Cat35

  if (trx.getRequest()->ticketingAgent()->cwtUser() &&
      paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && (!usedAcctOrCorp(paxTypeFare)) &&
      PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType(paxTypeFare.fcasPaxType()))
  {
    if ((paxTypeFare.fare()->isNationFRInCat15()) || (paxTypeFare.isNationFRInCat35()))
    {
      ret = false;
    }

    else if (paxTypeFare.isFareByRule()) // need to check the calculated FBR
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(paxTypeFare.getFbrRuleData()->ruleItemInfo());
      if (fbrItemInfo == nullptr || paxTypeFare.isSpecifiedFare())
        ret = true;

      Indicator i = fbrItemInfo->ovrdcat15();
      if (i == 'B' || i == ' ') // need to check cat15 for the base fare
      {
        PaxTypeFare* bFare = paxTypeFare.baseFare(25);
        if (bFare && bFare->fare()->isNationFRInCat15())
          ret = false;
      }
    }
  }

  return ret;
}
}
