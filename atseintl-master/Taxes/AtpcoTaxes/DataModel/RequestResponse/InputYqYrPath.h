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

#include "DataModel/RequestResponse/InputYqYrUsage.h"

namespace tax
{

struct InputYqYrPath
{
  InputYqYrPath() : _id(0), _totalAmount(0) {}

  type::Index _id;
  boost::ptr_vector<InputYqYrUsage> _yqYrUsages;
  type::MoneyAmount _totalAmount;
};

} // namespace tax
