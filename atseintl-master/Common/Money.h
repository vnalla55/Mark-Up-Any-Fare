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
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <stdexcept>
#include <string>

namespace tse
{
class Currency;

class Money
{
public:
  /**
   * This exception is only thrown from operators '+' '&lt;' '==' '!=' only,
   * in the case when operands have different currency code.
   *
   * This exception is not simply wrapper over std::runtime_error, but is rather to catch it when
   * we want to distinguish currency conversion issue.
   *
   * Also this exception
   * can be thrown by other classes if currency conversion error (example is
   *SoloCarnivalFCUtil::getAmountInCurrency).
   */
  class InvalidOperation : public std::runtime_error
  {
  public:
    InvalidOperation(const std::string& what_arg) : std::runtime_error(what_arg) {}
  };

  /**
  *   <pre>
  *   @method Money
  *   Description: Constructor
  *   @param CurrencyCode   - 3 character code that represents a currency
  *                           Retrieves all of the currency information
  *                           from the factory.
  *   @param double         - amount
  *   </pre>
  */
  Money(const CurrencyCode& code) : _currencyCode(code), _isNuc(NUC == _currencyCode) {}

  Money(const double amount, const CurrencyCode& code)
    : _amount(amount),
      _currencyCode(code),
      _isNuc(NUC == _currencyCode)
  {
  }

  virtual ~Money() = default;

  const CurrencyCode& code() const { return _currencyCode; }
  void setCode(const CurrencyCode& code)
  {
    _currencyCode = code;
    setNucFlag();
  }

  /**
  *   <pre>
  *
  *   @method noDec
  *
  *   Description: Returns the number of decimals for a currency.
  *
  *   @return CurrencyNoDec  - number of decimals in currency
  *
  *   </pre>
  */
  const CurrencyNoDec noDec(const DateTime& ticketingDate = DateTime::localTime()) const;

  std::string toString() const;
  std::string toString(const DateTime& ticketingDate) const;
  std::string toString(const CurrencyNoDec noDec) const;
  std::string toStringNuc() const;
  std::string toStringCurr(const CurrencyNoDec noDec) const;

  /**
  *   <pre>
  *
  *   @method operator<<
  *
  *   Description:  stream operator to write a Money to
  *                 a stream using the number of decimals for
  *                 precision.
  *
  *   @return ostream  - ostream reference
  *
  *   </pre>
  */
  friend std::ostream& operator<<(std::ostream& os, const Money& money);

  // operators throws Money::InvalidOperation if operand is in different currency
  Money operator+(const Money& op2) const;
  Money operator-(Money rhs) const;
  bool operator<(const Money& op2) const;
  bool operator<=(const Money& op2) const;
  bool operator>(const Money& op2) const;
  bool operator==(const Money& op2) const;
  bool operator!=(const Money& op2) const { return !(op2 == *this); }

  bool isNuc() const { return _isNuc; }

  static constexpr int NUC_DECIMALS = 2;
  static constexpr int INVALID_NUM_DECIMALS = -1;

  MoneyAmount& value() { return _amount; }
  const MoneyAmount& value() const { return _amount; }

  bool isApplyNonIATARounding() const { return _applyNonIATARounding; }
  void setApplyNonIATARounding() { _applyNonIATARounding = true; }

  static bool isZeroAmount(const MoneyAmount& amount);
  bool isZeroAmount() const;

protected:
  MoneyAmount _amount = 0;
  CurrencyCode _currencyCode;
  bool _applyNonIATARounding = false;
  bool _isNuc;

  void setNucFlag() { _isNuc = (_currencyCode == NUC); }
  MoneyAmount epsilon() const;
};
} // tse namespace
