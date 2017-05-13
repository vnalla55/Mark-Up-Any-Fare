//----------------------------------------------------------------------------
//
//  File:               CurrencyConverter.C
//  Description:        A CurrencyConverter is responsible for performing conversions
//                      between Currencies.
//
//  Created:            02/20/2004
//  Authors:            Doug Steeb
//
//  Description:        see .C module
//
//  Return types:
//
//  Copyright Sabre 2003
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
#include "Common/CurrencyConverter.h"

#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionCache.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TSSCacheCommon.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/NUCInfo.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace tse
{

static Logger
logger("atseintl.Common.CurrencyConverter");

const double CurrencyConverter::ONE = (1.00 - EPSILON);

CurrencyConverter::CurrencyConverter() {}

bool
CurrencyConverter::convert(CurrencyConversionRequest& request, CurrencyCollectionResults* results)
{
  return true;
}

bool
CurrencyConverter::roundUp(Money& money, const RoundingFactor& roundingFactor)
{
  Precision precision(money);

  long truncatedAmount = 0;
  RoundUnitNoDec roundUnitNoDec = 0;

  LOG4CXX_DEBUG(logger, "Rounding up");
  LOG4CXX_DEBUG(logger, "Amount : " << money.value());
  LOG4CXX_DEBUG(logger, "Rounding factor : " << roundingFactor);

  if (isZeroAmount(money) || isZeroAmount(roundingFactor))
    return true;

  if (roundingFactor >= ONE)
    roundUnitNoDec = precision.roundUnitNoDec();
  else
    roundUnitNoDec = determineRoundingDecimals(roundingFactor);

  LOG4CXX_DEBUG(logger, "NUMBER OF ROUNDING UNIT DECIMALS: " << roundUnitNoDec);
  LOG4CXX_DEBUG(logger, "ROUNDING FACTOR: " << roundingFactor);

  double shiftedOriginalAmount = (money.value() * static_cast<double>(pow(10.0, (roundUnitNoDec))));
  LOG4CXX_DEBUG(logger, "shifted original amount: " << shiftedOriginalAmount);

  long long truncShiftedOriginalAmount = static_cast<long long>(shiftedOriginalAmount);

  LOG4CXX_DEBUG(logger, "truncated shifted original amount: " << truncShiftedOriginalAmount);

  long decPoint = static_cast<long>(roundUnitNoDec - precision.roundUnitNoDec());

  long decFactor = static_cast<long>(log10(roundingFactor));
  LOG4CXX_DEBUG(logger, "decFactor  " << decFactor);

  long intRoundFactor = static_cast<long>(roundingFactor);
  if (intRoundFactor == 0)
    decPoint = -decPoint;

  long modAmt = decFactor + roundUnitNoDec;
  LOG4CXX_DEBUG(logger, "Mod Amount  " << modAmt);
  long modFactor = static_cast<long>(std::pow(10.0, static_cast<double>(modAmt)));
  double adjustDecNo = std::pow(10.0, static_cast<double>(decPoint));
  LOG4CXX_DEBUG(logger, "adjustDecNo = " << std::setprecision(12) << adjustDecNo);

  truncatedAmount = precision.truncatedAmount(truncShiftedOriginalAmount);

  // Need to compare with original amount
  // to a certain precision
  //
  bool remainder = determineRemainder(money, roundingFactor, truncShiftedOriginalAmount, modFactor);

  LOG4CXX_DEBUG(logger, "Remainder = " << remainder);

  if (remainder)
  {
    if (intRoundFactor == 1)
    {
      truncatedAmount++;
    }
    else
    {
      long tenth(1);
      if (intRoundFactor == 0)
      {
        tenth = static_cast<long>(pow(10.0, (roundUnitNoDec - 1)));
        intRoundFactor = static_cast<long>(roundingFactor * static_cast<double>(tenth));
      }
      else
      {
        RoundUnitNoDec roundUpdigit = 1;
        long intDigit = intRoundFactor;
        intDigit = intDigit / 10;
        while (intDigit > 0)
        {
          roundUpdigit++;
          intDigit = intDigit / 10;
        }
        tenth = static_cast<long>(pow(10.0, roundUpdigit));
      }
      long noToRoundUp = truncatedAmount % tenth;
      truncatedAmount /= tenth;
      long stepUpNo = noToRoundUp / intRoundFactor;
      long addUp = stepUpNo * intRoundFactor;
      addUp += intRoundFactor;
      truncatedAmount *= tenth;
      truncatedAmount += addUp;
    }

    money.value() = static_cast<double>(truncatedAmount) * adjustDecNo;
  }
  else
  {
    money.value() = static_cast<double>(truncatedAmount) * adjustDecNo;
    roundNone(money, roundingFactor);
  }

  LOG4CXX_DEBUG(logger, "Original amount: " << std::setprecision(12) << money.value());

  return true;
}

bool
CurrencyConverter::roundDown(Money& money, const RoundingFactor& roundingFactor)
{
  double tmpAmount = 0.0;
  long truncatedAmount = 0;

  LOG4CXX_DEBUG(logger, "Amount: " << money.value());
  LOG4CXX_DEBUG(logger, "Rounding factor: " << roundingFactor);

  if (isZeroAmount(money) || isZeroAmount(roundingFactor))
    return true;

  tmpAmount = (money.value() / roundingFactor);

  truncatedAmount = static_cast<long>(tmpAmount);

  tmpAmount = static_cast<double>(truncatedAmount) * roundingFactor;

  money.value() = tmpAmount;

  return true;
}

bool
CurrencyConverter::roundNearest(Money& money, const RoundingFactor& roundingFactor)
{
  double tmpAmount = 0.0;
  long truncatedAmount = 0;

  LOG4CXX_DEBUG(logger, "Amount : " << money.value());
  LOG4CXX_DEBUG(logger, "Rounding factor : " << roundingFactor);

  if (isZeroAmount(money) || isZeroAmount(roundingFactor))
    return true;

  tmpAmount = ((money.value() / roundingFactor) + .5) + EPSILON;

  truncatedAmount = static_cast<long>(tmpAmount);

  tmpAmount = static_cast<double>(truncatedAmount) * roundingFactor;

  money.value() = tmpAmount;

  return true;
}

bool
CurrencyConverter::roundNone(Money& money, const RoundingFactor& roundingFactor)
{
  int64_t truncatedAmount = 0;

  LOG4CXX_INFO(logger, "Entered CurrencyConverter::roundNone");

  LOG4CXX_DEBUG(logger, "Amount : " << money.value());
  LOG4CXX_DEBUG(logger, "Rounding factor : " << roundingFactor);

  if (isZeroAmount(money))
    return true;

  const CurrencyNoDec& noDec = money.noDec();

  double truncationFactor = static_cast<double>(pow(10.0, noDec));

  LOG4CXX_DEBUG(logger, "Truncation factor : " << truncationFactor);

  double tmpAmount = ((money.value() + EPSILON) * truncationFactor);
  truncatedAmount = static_cast<int64_t>(tmpAmount);
  LOG4CXX_DEBUG(logger, "Truncated amount : " << truncatedAmount);

  money.value() = (static_cast<double>(truncatedAmount) / truncationFactor);

  LOG4CXX_DEBUG(logger, "Truncated original amount : " << money.value());

  LOG4CXX_INFO(logger, "Leaving CurrencyConverter::roundNone");

  return true;
}

bool
CurrencyConverter::getNucInfo(const CarrierCode& carrier,
                              const CurrencyCode& currency,
                              const DateTime& ticketDate,
                              ExchRate& nucFactor,
                              RoundingFactor& nucRoundingFactor,
                              RoundingRule& roundingRule,
                              CurrencyNoDec& roundingFactorNoDec,
                              CurrencyNoDec& nucFactorNoDec,
                              DateTime& discontinueDate,
                              DateTime& effectiveDate,
                              CurrencyConversionCache* cache)
{

  DataHandle dataHandle(ticketDate);

  LOG4CXX_DEBUG(logger, "Entered getNucInfo");

  const NUCInfo* nucInfo = nullptr;

  if (cache)
  {
    nucInfo = cache->getNUCInfo(currency, carrier, ticketDate);
  }
  else
  {
    nucInfo = dataHandle.getNUCFirst(currency, carrier, ticketDate);
  }

  LOG4CXX_DEBUG(logger, "Returned from db getNUC method");

  if (!nucInfo)
  {
    LOG4CXX_ERROR(logger, "No NUC information retrieved from DBAccess layer");
    return false;
  }
  else
  {
    nucFactor = nucInfo->_nucFactor;

    LOG4CXX_DEBUG(logger, "Nuc Factor: " << nucFactor);

    nucRoundingFactor = nucInfo->_roundingFactor;

    LOG4CXX_DEBUG(logger, "Nuc rounding Factor: " << nucRoundingFactor);

    roundingRule = nucInfo->_roundingRule;

    LOG4CXX_DEBUG(logger, "Nuc rounding rule: " << roundingRule);

    roundingFactorNoDec = nucInfo->_roundingFactorNodec;
    nucFactorNoDec = nucInfo->_nucFactorNodec;
    LOG4CXX_DEBUG(logger, "Nuc roe no dec : " << nucFactorNoDec);

    discontinueDate = nucInfo->_discDate;
    effectiveDate = nucInfo->_effDate;
    LOG4CXX_DEBUG(logger, "Nuc effective date: " << effectiveDate.toSimpleString());
    LOG4CXX_DEBUG(logger, "Nuc ticket date: " << ticketDate.toSimpleString());
  }

  return true;
}

bool
CurrencyConverter::round(Money& target, RoundingFactor& roundingFactor, RoundingRule& roundingRule)
{
  if (target.isApplyNonIATARounding())
  {
    roundingFactor = 1.0;
    roundingRule = UP;
  }

  return roundByRule(target, roundingFactor, roundingRule);
}

bool
CurrencyConverter::roundByRule(Money& target,
                               const RoundingFactor& roundingFactor,
                               RoundingRule& roundingRule)
{
  return tsscache::ccRoundByRule(*this, target, roundingFactor, roundingRule);
}

bool CurrencyConverter::roundByRuleCB(Money& target,
                                      RoundingFactor roundingFactor,
                                      RoundingRule roundingRule)
{
  switch (roundingRule)
  {
  case UP:
    LOG4CXX_DEBUG(logger, "Invoking round up");
    return roundUp(target, roundingFactor);

  case DOWN:
    LOG4CXX_DEBUG(logger, "Invoking round down");
    return roundDown(target, roundingFactor);

  case NEAREST:
    LOG4CXX_DEBUG(logger, "Invoking round nearest");
    return roundNearest(target, roundingFactor);

  case NONE:
    LOG4CXX_DEBUG(logger, "Invoking round none");
    return roundNone(target, roundingFactor);

  default:
    return false;
  }
}

bool
CurrencyConverter::validateInput(Money& target,
                                 const Money& source,
                                 const Agent& agent,
                                 const DateTime& ticketDate)
{
  if (UNLIKELY(source.isNuc() || target.isNuc()))
  {
    LOG4CXX_ERROR(logger,
        "Invalid Input: Source/Target currency code is NUC, use BSRCurrencyConverter.");
    return false;
  }

  if (UNLIKELY(source.code().empty()))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Source currency code is empty");
    return false;
  }

  if (UNLIKELY(target.code().empty()))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Target currency code is empty");
    return false;
  }

  if (UNLIKELY((source.value() <= 0.0) && (-source.value() > EPSILON)))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Source value is " << source.value());
    return false;
  }

  if (UNLIKELY(isnan(source.value())))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Source value is  NaN");
    return false;
  }

  if (UNLIKELY(isinf(source.value())))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Source value is  infinite value");
    return false;
  }

  if (UNLIKELY(!(agent.agentLocation())))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Agent location is nil");
    return false;
  }

  DataHandle dataHandle;
  const Currency* targetcurrency = dataHandle.getCurrency( target.code() );
  if (UNLIKELY( !targetcurrency ))
  {
    LOG4CXX_ERROR(logger, "Invalid Target Currency ");
    return false;
  }

  const Currency* sourceCurrency = dataHandle.getCurrency( source.code() );
  if (UNLIKELY( !sourceCurrency ))
  {
    LOG4CXX_ERROR(logger, "Invalid Source Currency ");
    return false;
  }

  return true;
}

bool
CurrencyConverter::validateInput(Money& target, const Money& source)
{
  if (UNLIKELY((!source.isNuc()) && (!target.isNuc())))
  {
    LOG4CXX_ERROR(logger, "Invalid Currency code: Source or Target currency code must be NUC");
    return false;
  }

  if (UNLIKELY(source.value() < -EPSILON))
  {
    LOG4CXX_ERROR(logger, "Invalid Input amount: Source value is " << source.value());
    return false;
  }

  if (UNLIKELY(isnan(source.value())))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Source value is  NaN");
    return false;
  }

  if (UNLIKELY(isinf(source.value())))
  {
    LOG4CXX_ERROR(logger, "Invalid Input: Source value is  infinite value");
    return false;
  }

  return true;
}

bool
CurrencyConverter::isZeroAmount(const Money& source)
{
  return source.isZeroAmount();
}

bool
CurrencyConverter::isZeroAmount(const double& source)
{
  return Money::isZeroAmount(source);
}

bool
CurrencyConverter::determineRemainder(Money& money,
                                      const RoundingFactor& roundingFactor,
                                      long long truncShiftedOriginalAmount,
                                      long modFactor)
{
  LOG4CXX_DEBUG(logger, "ENTERING determineRemainder");
  long remainder = 0;

  remainder = truncShiftedOriginalAmount % modFactor;

  if (remainder == 0)
  {
    long rFactor = static_cast<long>(roundingFactor);
    LOG4CXX_DEBUG(logger, "rFactor: " << rFactor);

    long long truncOriginalAmount = static_cast<long long>(money.value());
    LOG4CXX_DEBUG(logger, "truncateded original amount: " << truncOriginalAmount);

    if (rFactor > 0)
      remainder = truncOriginalAmount % rFactor;

    LOG4CXX_DEBUG(logger, "remainder: " << remainder);

    if (remainder > 0)
      return true;

    return false;
  }

  return true;
}

int
CurrencyConverter::determineRoundingDecimals(RoundingFactor roundingFactor)
{
  double shiftFactor = 10.00;
  unsigned int i = 0;

  if ((roundingFactor > -EPSILON) && (roundingFactor < EPSILON))
    LOG4CXX_ERROR(logger, "Rounding Factor is zero" << roundingFactor);

  LOG4CXX_DEBUG(logger, "roundingFactor : " << roundingFactor);

  for (i = 1; !((roundingFactor > -EPSILON) && (roundingFactor < EPSILON)); i++)
  {
    roundingFactor *= shiftFactor;
    LOG4CXX_DEBUG(logger, "tmpRoundingFactor : " << roundingFactor);

    if (roundingFactor >= ONE)
      break;
  }

  int roundingUnitNoDec = (i + 1);

  LOG4CXX_DEBUG(logger, "Number of decimals to be used in rounding: " << roundingUnitNoDec);

  return roundingUnitNoDec;
}

CurrencyConverter::Precision::Precision(const Money& money)
{
  if (money.code().equalToConst("MXN")) // Hardcoded because there is no difference in database for MXN
  {
    _noDec = 2;
    _power = 100;
  }
  else if (money.isApplyNonIATARounding())
  {
    _noDec = 2;
    _power = 100;
  }
  else
  {
    _noDec = 1;
    _power = 10;
  }
}

RoundUnitNoDec
CurrencyConverter::Precision::roundUnitNoDec() const
{
  return _noDec;
}

long long
CurrencyConverter::Precision::truncatedAmount(long long truncShiftedOriginalAmount) const
{
  return truncShiftedOriginalAmount / _power;
}
}
