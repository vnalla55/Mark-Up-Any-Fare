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

#include <boost/optional.hpp>
#include "DomainDataObjects/GeoPath.h"

namespace tax
{

class GeoPathMock : public GeoPath
{
public:
  GeoPathMock();
  virtual ~GeoPathMock();

  virtual bool isJourneyDomestic() const;
  virtual type::CityCode getOriginCity() const;
  virtual type::CityCode getDestinationCity() const;
  virtual type::Nation getOriginNation() const;
  virtual type::Nation getDestinationNation() const;

  void setIsJourneyDomestic(bool isJourneyDomestic);
  void setOriginCity(type::CityCode originCity);
  void setDestinationCity(type::CityCode destinationCity);
  void setOriginNation(type::Nation originNation);
  void setDestinationNation(type::Nation destinationNation);

private:
  boost::optional<bool> _maybeIsJourneyDomestic;
  boost::optional<type::CityCode> _maybeOriginCity;
  boost::optional<type::CityCode> _maybeDestinationCity;
  boost::optional<type::Nation> _maybeOriginNation;
  boost::optional<type::Nation> _maybeDestinationNation;
};

} // namespace tax
