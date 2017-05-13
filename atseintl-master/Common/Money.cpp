//---------------------------------------------------------------------------- //
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
#include "Common/Money.h"

#include "Common/FallbackUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>


namespace tse
{

/**
*   <pre>
*
*   @method operator<<
*
*   Description:  Writes the Money value to a stream
*                 using the number of decimals for precision if available.
*
*   @return ostream&  - stream reference
*
*   </pre>
*/
std::ostream&
operator<<(std::ostream& os, const Money& money)
{
  CurrencyCode currencyCode = NUC;
  int noDec = 2;

  if (LIKELY(money.code() != NUC))
  {
    DataHandle dataHandle;
    const Currency* currency = nullptr;
    currency = dataHandle.getCurrency( money.code() );

    if (UNLIKELY(currency == nullptr))
      return os;
    currencyCode = currency->cur();
    noDec = currency->noDec();
  }

  std::ios::fmtflags fmt =
      os.setf(std::ios::right | std::ios::fixed, std::ios::adjustfield | std::ios::floatfield);
  const std::streamsize prec = os.precision(noDec);
  os << money.value() << " ";
  os.setf(std::ios::left, std::ios::adjustfield);
  os << std::setw(3) << currencyCode;
  os.precision(prec);
  os.setf(fmt, std::ios::adjustfield | std::ios::floatfield);

  return os;
}

/**
*   Description: convert the Money to a string
*
*   @return string
*/
std::string
Money::toString() const
{
  return toString(DateTime::localTime());
}

std::string
Money::toString(const DateTime& ticketingDate) const
{
  if (UNLIKELY(isNuc()))
    return toStringNuc();

  const Currency* currency = nullptr;
  DataHandle dataHandle;
  currency = dataHandle.getCurrency( _currencyCode );

  if (LIKELY(currency))
    return toStringCurr(currency->noDec());
  return EMPTY_STRING();
}

std::string
Money::toString(const CurrencyNoDec noDec) const
{
  if (isNuc())
    return toStringNuc();
  return toStringCurr(noDec);
}

std::string
Money::toStringNuc() const
{
  std::ostringstream os;
  os << std::setw(3) << NUC;
  os << std::left << std::fixed << std::setprecision(2);
  os << value();
  return os.str();
}

std::string
Money::toStringCurr(const CurrencyNoDec noDec) const
{
  std::ostringstream os;
  os << std::setw(3) << _currencyCode;
  os << std::left << std::fixed << std::setprecision(noDec);
  os << value();
  return os.str();
}

//---------------------------------------------------------------
//
//   @method noDec
//
//   Description: Returns the number of decimals for this currency.
//                If it is a NUC it will return 2 decimals.
//                If  none of the above are true it returns -1 .
//                This is an error and the user should check for this.
//
//   @return CurrencyNoDec  - constant reference to currency number
//                            of decimals.
//
//   </pre>
//---------------------------------------------------------------
const CurrencyNoDec
Money::noDec(const DateTime& ticketingDate) const
{
  if (!isNuc())
  {
    const Currency* currency = nullptr;
    DataHandle dataHandle;
    currency = dataHandle.getCurrency( _currencyCode );

    if (currency)
      return currency->noDec();
  }
  else if (LIKELY(isNuc()))
    return NUC_DECIMALS;

  return INVALID_NUM_DECIMALS;
}

Money
Money::operator+(const Money& op2) const
{
  if (code() != op2.code())
    throw InvalidOperation("Money::operator+ supports operands only the same currency. Used "
                            + code() + " and " + op2.code());

  return Money(value() + op2.value(), code());
}

Money
Money::
operator-(Money rhs) const
{
  rhs.value() = -rhs.value();
  return *this + rhs;
}

bool
Money::operator<(const Money& op2) const
{
  return *this != op2 &&
         value() < op2.value();
}

bool
Money::operator>(const Money& op2) const
{
  return op2 < (*this);
}

bool
Money::operator <=(const Money& op2) const
{
  return !(*this > op2);
}

bool
Money::operator==(const Money& op2) const
{
  if (code() != op2.code())
    throw InvalidOperation("Money::operator== supports operands in the same currency only");

  return std::fabs(value() - op2.value()) <
         epsilon(); // to guarantee architecture independent behavior
}

bool
Money::isZeroAmount(const MoneyAmount& amount)
{
  return amount > -EPSILON && amount < EPSILON;
}

bool
Money::isZeroAmount() const
{
  return isZeroAmount(value());
}

// for now this one is used in Money comparison operations only
MoneyAmount
Money::epsilon() const
{
  CurrencyNoDec noDec = this->noDec(DateTime::localTime());
  // implemented just as in operator<<   ^^^^

  if (INVALID_NUM_DECIMALS == noDec)
    return EPSILON; // base epsilon

  MoneyAmount result = 0.5 * std::pow(10., -noDec);
  return result;
}
} // tse namespace
