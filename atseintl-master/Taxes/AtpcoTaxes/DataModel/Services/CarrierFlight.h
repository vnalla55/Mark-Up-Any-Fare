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
#include "CarrierFlightSegment.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

struct CarrierFlight
{
  type::Vendor vendor;
  type::Index itemNo;
  boost::ptr_vector<CarrierFlightSegment> segments;
};
}
