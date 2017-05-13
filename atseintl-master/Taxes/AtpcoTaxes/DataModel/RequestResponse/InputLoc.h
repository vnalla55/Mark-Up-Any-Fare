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

class InputLoc
{
public:
  InputLoc(void) : _taxPointTag{type::TaxPointTag::Sale}, _inBufferZone{false} {}

  type::AirportCode& code() { return _code; }
  type::AirportCode const& code() const { return _code; }

  type::TaxPointTag& tag() { return _taxPointTag; }
  type::TaxPointTag const& tag() const { return _taxPointTag; }

  type::Nation& nation() { return _nation; }
  const type::Nation& nation() const { return _nation; }

  type::CityCode& cityCode() { return _cityCode; }
  const type::CityCode& cityCode() const { return _cityCode; }

  bool& inBufferZone() { return _inBufferZone; }
  const bool& inBufferZone() const { return _inBufferZone; }

private:
  type::AirportCode _code;
  type::TaxPointTag _taxPointTag;
  type::Nation _nation;
  type::CityCode _cityCode;
  bool _inBufferZone;
};
}

