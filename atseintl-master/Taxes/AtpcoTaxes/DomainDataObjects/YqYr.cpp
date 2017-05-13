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
#include "DomainDataObjects/YqYr.h"
#include "AtpcoTaxes/DataModel/Common/CodeIO.h"

namespace tax
{
bool
compareYqYrType(const type::TaxTypeOrSubCode& taxType, const type::YqYrType& yqYrType)
{
  return taxType.size() > 0 && taxType[0] == yqYrType;
}

YqYr::YqYr(void) :
    _seqNo(0),
    _amount(0),
    _originalAmount(0),
    _orignalCurrency(),
    _carrierCode(),
    _code(),
    _type(),
    _taxIncluded(false)
{}

YqYr::~YqYr(void) {}

std::ostream&
YqYr::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "SEQNO: " << _seqNo << "\n";
  out << "AMOUNT: " << _amount << "\n";
  out << "CODE: " << _code << "\n";
  out << "TYPE: " << _type << "\n";
  out << "AMOUNT: " << _amount << "\n";
  out << "ORIGNALAMOUNT: " << _originalAmount << "\n";
  out << "ORIGNALCURRENCY: " << _orignalCurrency << "\n";
  out << "CARRIER: " << _carrierCode << "\n";
  out << "TAXINCLUDED: " << _taxIncluded << "\n";

  return out;
}

TaxableYqYr::TaxableYqYr() : _code(UninitializedCode), _type(0), _taxIncluded(false), _amount(0)
{
}

TaxableYqYr::TaxableYqYr(const YqYr& yqYr)
  : _code(yqYr.code()), _type(yqYr.type()), _taxIncluded(yqYr.taxIncluded()), _amount(yqYr.amount())
{
}

TaxableYqYr::TaxableYqYr(const type::TaxCode& code,
                         const type::YqYrType& type,
                         bool taxIncluded,
                         const type::MoneyAmount& amount)
  : _code(code), _type(type), _taxIncluded(taxIncluded), _amount(amount)
{
}
} // namespace tax
