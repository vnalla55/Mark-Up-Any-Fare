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

struct InputYqYr
{
  InputYqYr() :
    _id(0),
    _seqNo(0),
    _amount(0),
    _originalAmount(0),
    _originalCurrency(),
    _carrierCode(),
    _code(),
    _type(' '),
    _taxIncluded(false)
  {}

  bool operator==(const InputYqYr& rhs) const
  {
    return ((_seqNo == rhs._seqNo) && (_amount == rhs._amount) &&
            (_originalAmount == rhs._originalAmount) &&
            (_originalCurrency == rhs._originalCurrency) &&
            (_carrierCode == rhs._carrierCode) &&
            (_code == rhs._code) && (_type == rhs._type) &&
            (_taxIncluded == rhs._taxIncluded));
  }

  type::Index _id;
  type::SeqNo _seqNo;
  type::MoneyAmount _amount;
  type::MoneyAmount _originalAmount;
  type::CurrencyCode _originalCurrency;
  type::CarrierCode _carrierCode;
  type::TaxCode _code;
  type::YqYrType _type;
  bool _taxIncluded;
};

} // namespace tax
