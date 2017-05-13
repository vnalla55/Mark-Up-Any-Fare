// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "DataModel/Common/Types.h"

namespace tax
{
bool
compareYqYrType(const type::TaxTypeOrSubCode& taxType, const type::YqYrType& yqYrType);

class YqYr
{
public:
  YqYr(void);
  ~YqYr(void);

  type::SeqNo& seqNo() { return _seqNo; }
  const type::SeqNo& seqNo() const { return _seqNo; }

  type::TaxCode& code() { return _code; }
  const type::TaxCode& code() const { return _code; }

  type::YqYrType& type() { return _type; }
  const type::YqYrType& type() const { return _type; }

  type::MoneyAmount& amount() { return _amount; }
  const type::MoneyAmount& amount() const { return _amount; }

  type::MoneyAmount& originalAmount() { return _originalAmount; }
  const type::MoneyAmount& originalAmount() const { return _originalAmount; }

  type::CurrencyCode& originalCurrency() { return _orignalCurrency; }
  const type::CurrencyCode& originalCurrency() const { return _orignalCurrency; }

  type::CarrierCode& carrierCode() { return _carrierCode; }
  const type::CarrierCode& carrierCode() const { return _carrierCode; }

  bool& taxIncluded() { return _taxIncluded; }
  const bool& taxIncluded() const { return _taxIncluded; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::SeqNo _seqNo;
  type::MoneyAmount _amount;
  type::MoneyAmount _originalAmount;
  type::CurrencyCode _orignalCurrency;
  type::CarrierCode _carrierCode;
  type::TaxCode _code;
  type::YqYrType _type;
  bool _taxIncluded;
};

struct TaxableYqYr
{
  TaxableYqYr();
  TaxableYqYr(const YqYr&);
  TaxableYqYr(const type::TaxCode&, const type::YqYrType&, bool, const type::MoneyAmount&);
  type::TaxCode _code;
  type::YqYrType _type;
  bool _taxIncluded;
  type::MoneyAmount _amount;
};
} // namespace tax
