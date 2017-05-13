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

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include "DataModel/Common/Types.h"
#include "DataModel/Common/CompactOptional.h"

namespace tax
{
struct InputItin
{
  InputItin()
    : _id(0),
      _geoPathRefId(std::numeric_limits<type::Index>::max()),
      _farePathRefId(std::numeric_limits<type::Index>::max()),
      _passengerRefId(std::numeric_limits<type::Index>::max()),
      _pointOfSaleRefId(0),
      _flightPathRefId(std::numeric_limits<type::Index>::max()),
      _travelOriginDate(),
      _label()
  {
  }

  typedef CompactOptional<type::Index> OptionalIndex;

  type::Index _id;
  type::Index _geoPathRefId;
  type::Index _farePathRefId;
  OptionalIndex _yqYrPathRefId;
  OptionalIndex _optionalServicePathRefId;
  type::Index _passengerRefId;
  type::Index _pointOfSaleRefId;

  OptionalIndex _farePathGeoPathMappingRefId;
  OptionalIndex _yqYrPathGeoPathMappingRefId;
  OptionalIndex _optionalServicePathGeoPathMappingRefId;
  type::Index _flightPathRefId;
  OptionalIndex _changeFeeRefId;
  OptionalIndex _ticketingFeeRefId;

  boost::gregorian::date _travelOriginDate;

  std::string _label;
};

} // namespace tax

