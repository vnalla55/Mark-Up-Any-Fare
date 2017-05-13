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

namespace tax
{

struct PaxTypeMappingItem
{
  type::PassengerCode paxFrom;
  type::PassengerCode paxTo;
};

struct PaxTypeMapping
{
  type::Vendor vendor;
  type::CarrierCode validatingCarrier;
  boost::ptr_vector<PaxTypeMappingItem> items;
};
}
