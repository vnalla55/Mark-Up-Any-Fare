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
#include "DomainDataObjects/Geo.h"

#include <boost/optional.hpp>
#include <vector>

namespace tax
{

class GeoPath
{
public:
  GeoPath(void);
  virtual ~GeoPath(void);

  type::Index& id()
  {
    return _id;
  };

  const type::Index& id() const
  {
    return _id;
  };

  const std::vector<Geo>& geos() const
  {
    return _geos;
  };

  std::vector<Geo>& geos()
  {
    return _geos;
  };

  virtual bool isJourneyDomestic() const;
  virtual bool isJourneyInternational() const;

  virtual type::Nation getOriginNation() const;
  virtual type::Nation getDestinationNation() const;

  virtual type::CityCode getOriginCity() const;
  virtual type::CityCode getDestinationCity() const;

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::Index _id;
  std::vector<Geo> _geos;
  mutable boost::optional<bool> _isJourneyDomestic;
};
}

