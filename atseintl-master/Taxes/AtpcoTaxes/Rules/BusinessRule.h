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
#include "Rules/RawPayments.h"
#include "PaymentDetail.h"
#include "Rules/RawPayments.h"

#include <string>

namespace tax
{

class Services;

class BusinessRule
{
public:
  BusinessRule() : _id(0) {}
  virtual ~BusinessRule() {}

  virtual std::string getDescription(Services& services) const = 0;

  type::Index getId() const { return _id; }
  void setId(type::Index id) { _id = id; }

  type::Index _id;
};
}

