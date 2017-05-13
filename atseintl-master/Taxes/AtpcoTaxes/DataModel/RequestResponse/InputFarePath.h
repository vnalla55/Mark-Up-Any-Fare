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

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/RequestResponse/InputFareUsage.h"

namespace tax
{

struct InputFarePath
{
  InputFarePath() : _id(0), _totalAmount(0), _totalMarkupAmount(0), _totalAmountBeforeDiscount(0) {}

  type::Index _id;
  boost::ptr_vector<InputFareUsage> _fareUsages;
  type::MoneyAmount _totalAmount;
  type::MoneyAmount _totalMarkupAmount;
  type::MoneyAmount _totalAmountBeforeDiscount;
  type::CarrierCode _validatingCarrier;
  type::PassengerCode _outputPtc;
};

} // namespace tax
