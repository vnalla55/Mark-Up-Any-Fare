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

class Distance
{
public:
  Distance(): _fromGeoRefId(0), _toGeoRefId(0), _miles(0), _globalDirection(type::GlobalDirection::NO_DIR)
  {
  }
  ~Distance()
  {
  }

  type::Index const& fromGeoRefId() const
  {
    return _fromGeoRefId;
  };

  type::Index& fromGeoRefId()
  {
    return _fromGeoRefId;
  };

  type::Index const& toGeoRefId() const
  {
    return _toGeoRefId;
  };

  type::Index& toGeoRefId()
  {
    return _toGeoRefId;
  };

  type::Miles const& miles() const
  {
    return _miles;
  };

  type::Miles& miles()
  {
    return _miles;
  };

  type::GlobalDirection const& globalDirection() const
  {
    return _globalDirection;
  };
 
  type::GlobalDirection& globalDirection()
  {
    return _globalDirection;
  };

private:
  type::Index _fromGeoRefId;
  type::Index _toGeoRefId;
  type::Miles _miles;
  type::GlobalDirection _globalDirection;
};
}
