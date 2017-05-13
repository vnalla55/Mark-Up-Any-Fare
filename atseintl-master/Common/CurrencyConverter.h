//----------------------------------------------------------------------------
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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"


namespace tse
{
class Money;
class Agent;
class CurrencyConversionRequest;
class CurrencyCollectionResults;
class CurrencyConversionCache;
class DateTime;

class CurrencyConverter
{
  friend class BSRCurrencyConverterTest;
  friend class CurrencyRoundingUtil;
  friend class CurrencyConverterTest;

public:
  CurrencyConverter();
  virtual ~CurrencyConverter() {}

  virtual bool convert(CurrencyConversionRequest& request, CurrencyCollectionResults* results);
  virtual bool
  roundByRule(Money& amount, const RoundingFactor& roundingFactor, RoundingRule& roundingRule);

  virtual bool roundByRuleCB(Money& target,
                             RoundingFactor roundingFactor,
                             RoundingRule roundingRule);

  virtual bool round(Money& target, RoundingFactor& roundingFactor, RoundingRule& roundingRule);

  bool determineRemainder(Money& amount,
                          const RoundingFactor& roundingFactor,
                          long long truncShiftedOriginalAmount,
                          long modFactor);
  int determineRoundingDecimals(RoundingFactor roundingFactor);
  virtual bool roundNone(Money& amount, const RoundingFactor& roundingFactor);
  virtual bool roundUp(Money& amount, const RoundingFactor& roundingFactor);
  virtual bool roundDown(Money& amount, const RoundingFactor& roundingFactor);
  virtual bool roundNearest(Money& amount, const RoundingFactor& roundingFactor);
  bool isZeroAmount(const Money& source);
  bool isZeroAmount(const double& source);
  virtual bool getNucInfo(const CarrierCode& carrier,
                          const CurrencyCode& currency,
                          const DateTime& ticketDate,
                          ExchRate& nucFactor,
                          RoundingFactor& nucRoundingFactor,
                          RoundingRule& roundingRule,
                          CurrencyNoDec& roundingFactorNoDec,
                          CurrencyNoDec& nucFactorNoDec,
                          DateTime& discontinueDate,
                          DateTime& effectiveDate,
                          CurrencyConversionCache* cache = nullptr);

  const static double ONE;

protected:
  bool
  validateInput(Money& target, const Money& source, const Agent& agent, const DateTime& ticketDate);
  bool validateInput(Money& target, const Money& source);

private:

  class Precision
  {
  public:
    Precision(const Money& money);
    RoundUnitNoDec roundUnitNoDec() const;
    long long truncatedAmount(long long truncShiftedOriginalAmount) const;

  protected:
    RoundUnitNoDec _noDec;
    long long _power;
  };
};
}

