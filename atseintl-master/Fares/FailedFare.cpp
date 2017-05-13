//--------------------------------------------------------------------
//
//  File:        FailedFare.cpp
//
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//--------------------------------------------------------------------

#include "Fares/FailedFare.h"

#include "Common/Vendor.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FailedFare::FailedFare(PricingTrx& trx, DiagCollector& diag, bool specialRtgFound)
  : _trx(trx), _diag(diag), _specialRtgFound(specialRtgFound)
{
}

bool
FailedFare::
operator()(const PaxTypeFare* ptFare)
{
  // unlikely, but no reason to keep this bad pointer, return true
  if (UNLIKELY(!ptFare))
  {
    return true;
  }

  bool result = checkFare(ptFare);

  if (UNLIKELY((true == result) && (true == ptFare->isKeepForRoutingValidation())))
  {
    return false;
  }

  return result;
}

bool
FailedFare::isValidForCategory35(const PaxTypeFare* ptFare) const
{
  const NegPaxTypeFareRuleData* negPaxTypeFareRuleData = ptFare->getNegRuleData();

  bool isCategoryAndJalAxcess =
      !ptFare->isCategoryValid(RuleConst::NEGOTIATED_RULE) && !ptFare->failedByJalAxessRequest();
  bool paxTypeRuleAndCat35 =
      (negPaxTypeFareRuleData == nullptr) || !negPaxTypeFareRuleData->rexCat35FareUsingPrevAgent();

  if (isCategoryAndJalAxcess && paxTypeRuleAndCat35)
    return false;

  return true;
}

bool
FailedFare::isNotSpecialRoutingAndDataMissing(const PaxTypeFare* ptFare) const
{
  // Do not release Cat 25 fares with no PTC matched here, if fare with special routing exists
  // in the fare market. They will be released in FVO.
  bool noDtMissing = ptFare->isNoDataMissing();

  if (!_specialRtgFound && !noDtMissing)
    return true;

  return false;
}

bool
FailedFare::isValidForOtherCats(const PaxTypeFare* ptFare)
{
  if (_trx.isShopping())
  {
    ShoppingTrx& shoppingTrx = static_cast<ShoppingTrx&>(_trx);

    if (shoppingTrx.isSumOfLocalsProcessingEnabled() || ptFare->isValid())
    {
      const uint32_t szFCOValidationCatList[] = { 1, 15 };
      for (const auto cat : szFCOValidationCatList)
      {
        if (false == ptFare->isCategoryValid(cat))
        {
          if (UNLIKELY(_diagEnabled))
          {
            _diag << toString(*ptFare) << " CAT" << cat << "\n\n";
          }
          return false;
        }
      }
    }
  }

  return true;
}

bool
FailedFare::checkFare(const PaxTypeFare* ptFare)
{
  if (ptFare->cat25BasePaxFare())
  {
    return false;
  }

  _diagEnabled = _diag.isActive();

  // Check for CAT 35
  if (!isValidForCategory35(ptFare))
  {
    if (UNLIKELY(_diagEnabled))
    {
      _diag << toString(*ptFare) << " CAT35"
            << "\n\n";
    }
    return true;
  }

  // Check for WPNETT
  if (UNLIKELY(_trx.getRequest()->isWpNettRequested()))
  {
    return false;
  }

  if (isNotSpecialRoutingAndDataMissing(ptFare))
  {
    if (UNLIKELY(_diagEnabled))
    {
      _diag << toString(*ptFare) << " CAT25"
            << "\n\n";
    }
    return true;
  }

  // Check whether the Fare is valid. Since Cat25 and Cat35 special cases are addressed above,
  // we can safely release any invalid fare at this point.
  if (!isValidForOtherCats(ptFare))
  {
    return true;
  }

  return false;
}

std::string
FailedFare::toString(const PaxTypeFare& paxFare)
{
  std::ostringstream dc;

  std::string gd;
  globalDirectionToStr(gd, paxFare.fare()->globalDirection());

  std::string fareBasis = paxFare.createFareBasis(_trx, false);
  if (fareBasis.size() > 12)
    fareBasis = fareBasis.substr(0, 12) + "*";

  dc << std::setw(2) << _diag.cnvFlags(paxFare) << std::setw(3) << gd << std::setw(2)
     << (paxFare.vendor() == Vendor::ATPCO ? "A" : (paxFare.vendor() == Vendor::SITA ? "S" : "?"))
     << std::setw(5) << paxFare.ruleNumber() << std::setw(13) << fareBasis << std::setw(4)
     << paxFare.fareTariff() << std::setw(2)
     << (paxFare.owrt() == ONE_WAY_MAY_BE_DOUBLED
             ? "X"
             : (paxFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED
                    ? "R"
                    : (paxFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED ? "O" : " "))) << std::setw(2)
     << (paxFare.directionality() == FROM ? "O" : (paxFare.directionality() == TO ? "I" : " "))
     << std::setw(12) << Money(paxFare.fareAmount(), paxFare.currency());

  return dc.str();
}
}
