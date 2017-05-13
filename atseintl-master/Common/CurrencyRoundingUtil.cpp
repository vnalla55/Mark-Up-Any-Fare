//----------------------------------------------------------------------------
//
//  File:               CurrencyRoundingUtil.cpp
//
//  Description:        A CurrencyRoundingUtil is responsible for
//                      rounding amounts for a specific currency.
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "Common/CurrencyRoundingUtil.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/Currency.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace tse
{
static Logger logger("atseintl.Common.CurrencyRoundingUtil");

CurrencyRoundingUtil::CurrencyRoundingUtil() {}

bool
CurrencyRoundingUtil::round(MoneyAmount& targetAmount,
                            const CurrencyCode& currency,
                            PricingTrx& trx)
{
  bool roundRC = false;
  double nucFactor = 0.0;
  double roundingFactor = 0.0;
  RoundingRule roundingRule;
  CarrierCode carrier;
  CurrencyNoDec roundingFactorNoDec;
  CurrencyNoDec nucFactorNoDec;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);

  if (currency == NUC)
    return true;

  Money target(targetAmount, currency);

  if (!isValid(target))
    return false;

  bool nucRC = _ccConverter.getNucInfo(carrier,
                                       currency,
                                       trx.getRequest()->ticketingDT(),
                                       nucFactor,
                                       roundingFactor,
                                       roundingRule,
                                       roundingFactorNoDec,
                                       nucFactorNoDec,
                                       discontinueDate,
                                       effectiveDate);

  if (!nucRC)
  {
    LOG4CXX_ERROR(logger,
                  "No NUC information retrieved from DBAccess layer for Currency: " << currency);
    LOG4CXX_ERROR(logger, "Ticketing Date : " << trx.getRequest()->ticketingDT().toSimpleString());
    return false;
  }

  if (roundingRule == UP)
  {
    roundRC = _ccConverter.roundUp(target, roundingFactor);
  }
  else if (roundingRule == DOWN)
  {
    LOG4CXX_DEBUG(logger, "Invoking round down");
    roundRC = _ccConverter.roundDown(target, roundingFactor);
  }
  else if (roundingRule == NEAREST)
  {
    LOG4CXX_DEBUG(logger, "Invoking round nearest");
    roundRC = _ccConverter.roundNearest(target, roundingFactor);
  }
  else if (roundingRule == NONE)
  {
    LOG4CXX_DEBUG(logger, "Invoking round nearest");
    roundRC = _ccConverter.roundNone(target, roundingFactor);
  }

  targetAmount = target.value();

  return roundRC;
}

bool
CurrencyRoundingUtil::round(Money& target, PricingTrx& trx)
{
  bool roundRC = false;
  double nucFactor = 0.0;
  double roundingFactor = 0.0;
  RoundingRule roundingRule;
  CarrierCode carrier;
  CurrencyNoDec roundingFactorNoDec;
  CurrencyNoDec nucFactorNoDec;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);

  if (!isValid(target))
    return false;

  if (target.code() == NUC)
    return true;

  bool nucRC = _ccConverter.getNucInfo(carrier,
                                       target.code(),
                                       trx.getRequest()->ticketingDT(),
                                       nucFactor,
                                       roundingFactor,
                                       roundingRule,
                                       roundingFactorNoDec,
                                       nucFactorNoDec,
                                       discontinueDate,
                                       effectiveDate);

  if (!nucRC)
  {
    LOG4CXX_ERROR(
        logger,
        "No NUC information retrieved from DBAccess layer for Currency: " << target.code());
    LOG4CXX_ERROR(logger, "Ticketing Date : " << trx.getRequest()->ticketingDT().toSimpleString());
    return false;
  }

  if (roundingRule == UP)
  {
    roundRC = _ccConverter.roundUp(target, roundingFactor);
  }
  else if (roundingRule == DOWN)
  {
    LOG4CXX_DEBUG(logger, "Invoking round down");
    roundRC = _ccConverter.roundDown(target, roundingFactor);
  }
  else if (roundingRule == NEAREST)
  {
    LOG4CXX_DEBUG(logger, "Invoking round nearest");
    roundRC = _ccConverter.roundNearest(target, roundingFactor);
  }
  else if (roundingRule == NONE)
  {
    LOG4CXX_DEBUG(logger, "Invoking round nearest");
    roundRC = _ccConverter.roundNone(target, roundingFactor);
  }

  return roundRC;
}

bool
CurrencyRoundingUtil::round(Money& target, PricingTrx& trx, bool useInternationalRounding)
{
  bool roundRC = false;
  double nucFactor = 0.0;
  double roundingFactor = 0.0;
  RoundingRule roundingRule;
  CarrierCode carrier;
  CurrencyNoDec roundingFactorNoDec;
  CurrencyNoDec nucFactorNoDec;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);

  bool nucRC = _ccConverter.getNucInfo(carrier,
                                       target.code(),
                                       trx.getRequest()->ticketingDT(),
                                       nucFactor,
                                       roundingFactor,
                                       roundingRule,
                                       roundingFactorNoDec,
                                       nucFactorNoDec,
                                       discontinueDate,
                                       effectiveDate);

  if (!nucRC)
  {
    LOG4CXX_ERROR(
        logger,
        "No NUC information retrieved from DBAccess layer for Currency: " << target.code());
    LOG4CXX_ERROR(logger, "Ticketing Date : " << trx.getRequest()->ticketingDT().toSimpleString());
    return false;
  }

  if (!useInternationalRounding)
  {
    const Currency* currency = nullptr;
    DataHandle dataHandle;
    currency = dataHandle.getCurrency( target.code() );

    if (currency)
    {
      if (currency->domRoundingFactor() > 0)
      {
        LOG4CXX_DEBUG(logger, "Domestic rounding factor is not zero");
        roundingFactor = currency->domRoundingFactor();

        LOG4CXX_DEBUG(logger, "Domestic NUC Rounding Factor: " << roundingFactor);
      }
      else if (_ccConverter.isZeroAmount(currency->domRoundingFactor()))
      {
        LOG4CXX_DEBUG(logger, "Domestic rounding factor is zero");
        roundingRule = NONE;
        roundingFactor = currency->domRoundingFactor();
      }
    }
  }

  roundRC = _ccConverter.round(target, roundingFactor, roundingRule);
  return roundRC;
}

bool
CurrencyRoundingUtil::isValid(Money& amount)
{

  if (amount.code().empty())
    return false;
  else if (amount.value() <= 0)
  {
    LOG4CXX_ERROR(logger, "Invalid Input amount: Source value is " << amount.value());
    return false;
  }
  else if (isnan(amount.value()))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: value is  NaN");
    return false;
  }
  else if (isinf(amount.value()))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: value is  infinite value");
    return false;
  }

  return true;
}

MoneyAmount
CurrencyRoundingUtil::roundMoneyAmount(const MoneyAmount amount,
                                       const CurrencyCode& displayCurrency,
                                       const CurrencyCode& paxTypeFareCurrency,
                                       FareDisplayTrx& trx)
{
  if (displayCurrency != paxTypeFareCurrency)
    return amount;

  Money roundedFare(amount, paxTypeFareCurrency);
  if (roundedFare.value() == 0)
    return amount;

  Itin* itin = trx.itin().front();
  bool useIntlRounding = itin->useInternationalRounding();
  round(roundedFare, trx, useIntlRounding);
  return roundedFare.value();
}

bool
CurrencyRoundingUtil::applyNonIATARounding(const PricingTrx& trx,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const RuleNumber& ruleNumber)
{
  if (trx.dataHandle().getVendorType(vendor) != 'T') // Sabre MyFare vendor RuleConst::SMF_VENDOR
    return false;

  const std::vector<ContractPreference*>& contractPrefVec =
      trx.dataHandle().getContractPreferences(vendor, carrier, ruleNumber, trx.ticketingDate());

  return !contractPrefVec.empty() && contractPrefVec.front()->applyRoundingException() == 'X';
}
}
