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
#include "DataModel/Common/Types.h"

namespace tax
{

struct ServiceBaggageEntry
{
  ServiceBaggageEntry()
    : applTag(type::ServiceBaggageAppl::Blank),
      taxTypeSubcode(""),
      taxCode(UninitializedCode),
      optionalServiceTag(type::OptionalServiceTag::Blank),
      group(""),
      subGroup(""),
      feeOwnerCarrier(UninitializedCode)
  {
  }
  type::ServiceBaggageAppl applTag;
  type::TaxTypeOrSubCode taxTypeSubcode;
  type::TaxCode taxCode;
  type::OptionalServiceTag optionalServiceTag;
  type::ServiceGroupCode group;
  type::ServiceGroupCode subGroup;
  type::CarrierCode feeOwnerCarrier;
};

struct ServiceBaggage
{
  typedef ServiceBaggageEntry entry_type;
  type::Vendor vendor;
  type::Index itemNo;
  boost::ptr_vector<entry_type> entries;
};
}

