// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

struct InputOptionalService
{
  InputOptionalService() : _id(0), _amount(0), _type(type::OptionalServiceTag::Blank),
      _taxInclInd(false), _quantity(1) {}

  type::Index _id;
  type::MoneyAmount _amount;
  type::MoneyAmount _feeAmountInSellingCurrencyPlusTaxes;
  type::OcSubCode _subCode;
  type::ServiceGroupCode _serviceGroup;
  type::ServiceGroupCode _serviceSubGroup;
  type::OptionalServiceTag _type;
  type::CarrierCode _ownerCarrier;
  type::AirportCode _pointOfDeliveryLoc;
  type::PassengerCode _outputPtc;
  bool _taxInclInd;
  unsigned int _quantity;
};

} // namespace tax

