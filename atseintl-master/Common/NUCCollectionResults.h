//----------------------------------------------------------------------------
//
//  File: NUCCollectionResults.h
//
//  Created: June 2004
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
#pragma once

#include "Common/CurrencyCollectionResults.h"
#include "Common/DateTime.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class DateTime;

/**
*   @class NUCCollectionResults
*
*   Description:
*   Collects the NUC ROE , NUC Discontinue date and NUC
*   Effective Date.
*/
class NUCCollectionResults : public CurrencyCollectionResults
{
public:
  NUCCollectionResults() : _discontinueDate(tse::pos_infin), _effectiveDate(tse::pos_infin) {}

  /**
  *   @method nucAmount
  *
  *   Description: Get converted nuc amount
  *
  *   @return MoneyAmount - reference to  nuc amount
  *
  */
  MoneyAmount& nucAmount() { return _nucAmount; }
  const MoneyAmount& nucAmount() const { return _nucAmount; }

  /**
  *   @method discontinueDate
  *
  *   Description: Get discontinue date
  *
  *   @return DateTime - const reference to  discontinue date
  *
  */
  DateTime& discontinueDate() { return _discontinueDate; }
  const DateTime& discontinueDate() const { return _discontinueDate; }

  /**
  *   @method effectiveDate
  *
  *   Description: Get effective date
  *
  *   @return DateTime - const reference to  effective date
  *
  */
  DateTime& effectiveDate() { return _effectiveDate; }
  const DateTime& effectiveDate() const { return _effectiveDate; }

  /**
  *   @method exchangeRate
  *
  *   Description: Get exchange rate
  *
  *   @return ExchRate - const reference to  exchange rate
  *
  */
  ExchRate& exchangeRate() { return _exchangeRate; }
  const ExchRate& exchangeRate() const { return _exchangeRate; }

  /**
  *   @method exchangeRateNoDec
  *
  *   Description: Get/Set number of decimals in nuc exchange rate
  *
  *   @return ExchRate - const reference to  exchange rate
  *
  */
  CurrencyNoDec& exchangeRateNoDec() { return _exchangeRateNoDec; }
  const CurrencyNoDec& exchangeRateNoDec() const { return _exchangeRateNoDec; }

  RoundingFactor& roundingFactor() { return _roundingFactor; }
  const RoundingFactor& roundingFactor() const { return _roundingFactor; }

  RoundingRule& roundingRule() { return _roundingRule; }
  const RoundingRule& roundingRule() const { return _roundingRule; }

private:
  MoneyAmount _nucAmount = 0;
  DateTime _discontinueDate;
  DateTime _effectiveDate;
  ExchRate _exchangeRate = 0;
  CurrencyNoDec _exchangeRateNoDec = 0;
  RoundingFactor _roundingFactor = 0;
  RoundingRule _roundingRule = RoundingRule::EMPTY;
};
}

