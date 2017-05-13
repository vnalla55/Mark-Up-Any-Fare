//-------------------------------------------------------------------
//
//  File:        FareUtil.cpp
//  Created:     Feb 25, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Misc. util functions to process constructed fares
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "AddonConstruction/FareUtil.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/NUCCurrencyConverter.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/FareInfo.h"

namespace tse
{

static Logger
logger("atseintl.AddonConstruction.FareUtil");

/////////////////////////////////////////////////////////////////////
// total amount & currency

bool
FareUtil::calculateTotalAmount(ConstructedFareInfo& cfi, ConstructionJob& cj)
{
  cfi.constructedNucAmount() = 0;
  cfi.constructedSecondNucAmount() = 0;

  try
  {
    // specified fare

    FareInfo& sfi = cfi.fareInfo();
    if (UNLIKELY(sfi.currency().empty()))
    {
      LOG4CXX_ERROR(logger, "Specified Fare Currency is empty.");

      return false;
    }

    if (UNLIKELY(!addNucAmount(cfi.specifiedFareAmount(),
                               sfi.currency(),
                               sfi.owrt(),
                               cj,
                               cfi.constructedNucAmount(),
                               cfi.constructedSecondNucAmount())))
      return false;

    // origin add-on

    if (cfi.origAddonAmount() != 0)
    {
      if (UNLIKELY(cfi.origAddonCurrency().empty()))
      {
        LOG4CXX_ERROR(logger, "Origin Add-on Currency is empty.");

        return false;
      }

      if (UNLIKELY(!addNucAmount(cfi.origAddonAmount(),
                                  cfi.origAddonCurrency(),
                                  sfi.owrt(),
                                  cj,
                                  cfi.constructedNucAmount(),
                                  cfi.constructedSecondNucAmount())))
        return false;
    }

    // destination add-on

    if (cfi.destAddonAmount() != 0)
    {
      if (UNLIKELY(cfi.destAddonCurrency().empty()))
      {
        LOG4CXX_ERROR(logger, "Destination Add-on Currency is empty.");

        return false;
      }

      if (UNLIKELY(!addNucAmount(cfi.destAddonAmount(),
                   cfi.destAddonCurrency(),
                   sfi.owrt(),
                   cj,
                   cfi.constructedNucAmount(),
                   cfi.constructedSecondNucAmount())))
        return false;
    }
  }
  catch (ErrorResponseException& ex) { return false; }

  return true;
}

bool
FareUtil::addNucAmount(const MoneyAmount amount,
                       const CurrencyCode& currency,
                       const Indicator specifiedOWRT,
                       ConstructionJob& cj,
                       MoneyAmount& constructedNucAmount,
                       MoneyAmount& constructedSecondNucAmount)
{
  MoneyAmount alwaysPositiveAmount = (amount > 0 ? amount : -amount);
  // We should use the original amount in the calculations
  if (specifiedOWRT == ROUND_TRIP_MAYNOT_BE_HALVED && !cj.trx().getOptions()->isRtw())
    alwaysPositiveAmount *= 2.0;

  Money fareCurrency(alwaysPositiveAmount, currency);

  Money nuc("NUC");
  CurrencyConversionRequest ccRequest(nuc,
                                      fareCurrency,
                                      cj.ticketingDate(),
                                      *(cj.trx().getRequest()),
                                      cj.trx().dataHandle(),
                                      true,
                                      CurrencyConversionRequest::OTHER,
                                      false,
                                      cj.trx().getOptions(),
                                      true);

  NUCCurrencyConverter ncc;
  if (LIKELY(ncc.convert(ccRequest, nullptr)))
  {
    if (amount > 0)
      constructedNucAmount += nuc.value();
    else
      constructedNucAmount -= nuc.value();
  }
  else
    return false;

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(cj.trx()) &&
      static_cast<RexPricingTrx&>(cj.trx()).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(cj.trx()).newItinSecondROEConversionDate().isEmptyDate()))
  {
    CurrencyConversionRequest ccRequestSecond(
        nuc,
        fareCurrency,
        static_cast<RexBaseTrx&>(cj.trx()).newItinSecondROEConversionDate(),
        *(cj.trx().getRequest()),
        cj.trx().dataHandle(),
        true,
        CurrencyConversionRequest::OTHER,
        false,
        cj.trx().getOptions(),
        true);

    if (ncc.convert(ccRequestSecond, nullptr))
    {
      if (amount > 0)
        constructedSecondNucAmount += nuc.value();
      else
        constructedSecondNucAmount -= nuc.value();
    }
    else
      return false;
  }

  return true;
}

bool
FareUtil::nucToFareCurrency(ConstructedFareInfo& cfi, ConstructionJob& cjob)
{
  FareInfo& sfi = cfi.fareInfo();

  try
  {
    NUCCurrencyConverter ncc;

    Money nucSourceAmount(cfi.constructedNucAmount(), "NUC");
    Money targetAmount(sfi.currency());

    CurrencyConversionRequest ccr(targetAmount,
                                  nucSourceAmount,
                                  cjob.ticketingDate(),
                                  *(cjob.trx().getRequest()),
                                  cjob.trx().dataHandle(),
                                  true,
                                  CurrencyConversionRequest::OTHER,
                                  false,
                                  cjob.trx().getOptions(),
                                  true);

    if (LIKELY(ncc.convert(ccr, nullptr)))
    {
      sfi.fareAmount() = sfi.originalFareAmount() = targetAmount.value();

      if (sfi.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
          !cjob.trx().getOptions()->isRtw())
      {
        sfi.fareAmount() /= 2.0;
      }

      return true;
    }

    return false;
  }
  catch (ErrorResponseException& ex) { return false; }
}

bool
FareUtil::calculateTotalAmount(const ConstructedFare& cf,
                               ConstructionJob& cj,
                               MoneyAmount& totalAmount)
{
  MoneyAmount wrkAmount;
  MoneyAmount nucAmount = 0;
  MoneyAmount secondNucAmount = 0;

  // first to calculate NUC amount

  try
  {
    // specified fare

    const FareInfo& sfi = *cf.specifiedFare();
    if (sfi.currency().empty())
      return false;

    if (!addNucAmount(sfi.fareAmount(), sfi.currency(), sfi.owrt(), cj, nucAmount, secondNucAmount))
      return false;

    // origin add-on

    if (cf.origAddon() != nullptr)
    {
      const AddonFareInfo& oa = *(cf.origAddon()->addonFare());
      if (oa.fareAmt() != 0)
      {
        if (oa.cur().empty())
          return false;

        if (oa.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
          wrkAmount = oa.fareAmt() / 2.0;
        else
          wrkAmount = oa.fareAmt();

        if (!addNucAmount(wrkAmount, oa.cur(), sfi.owrt(), cj, nucAmount, secondNucAmount))
          return false;
      }
    }

    // destination add-on

    if (cf.destAddon() != nullptr)
    {
      const AddonFareInfo& da = *(cf.destAddon()->addonFare());
      if (da.fareAmt() != 0)
      {
        if (da.cur().empty())
          return false;

        if (da.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
          wrkAmount = da.fareAmt() / 2.0;
        else
          wrkAmount = da.fareAmt();

        if (!addNucAmount(wrkAmount, da.cur(), sfi.owrt(), cj, nucAmount, secondNucAmount))
          return false;
      }
    }

    // second to convert NUC amount to fare currency

    NUCCurrencyConverter ncc;

    Money nucSourceAmount(nucAmount, "NUC");
    Money targetAmount(sfi.currency());

    CurrencyConversionRequest ccr(targetAmount,
                                  nucSourceAmount,
                                  cj.ticketingDate(),
                                  *(cj.trx().getRequest()),
                                  cj.trx().dataHandle(),
                                  true,
                                  CurrencyConversionRequest::OTHER,
                                  false,
                                  cj.trx().getOptions(),
                                  true);

    if (!ncc.convert(ccr, nullptr))
      return false;

    totalAmount = targetAmount.value();
  }
  catch (ErrorResponseException& ex) { return false; }

  return true;
}
}
