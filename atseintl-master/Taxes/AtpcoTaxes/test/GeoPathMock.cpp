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
#include "test/GeoPathMock.h"

#include <stdexcept>

namespace tax
{

GeoPathMock::GeoPathMock()
{
}

GeoPathMock::~GeoPathMock()
{
}

bool GeoPathMock::isJourneyDomestic() const
{
  if (_maybeIsJourneyDomestic)
    return *_maybeIsJourneyDomestic;
  else
    throw std::runtime_error("GeoPath::isJourneyDomestic() should not be called!");
}

type::CityCode GeoPathMock::getOriginCity() const
{
  if (_maybeOriginCity)
    return *_maybeOriginCity;
  else
    throw std::runtime_error("GeoPath::getOriginCity() should not be called!");
}

type::CityCode GeoPathMock::getDestinationCity() const
{
  if (_maybeDestinationCity)
    return *_maybeDestinationCity;
  else
    throw std::runtime_error("GeoPath::getDestinationCity() should not be called!");
}

type::Nation GeoPathMock::getOriginNation() const
{
  if (_maybeOriginNation)
    return *_maybeOriginNation;
  else
    throw std::runtime_error("GeoPath::getOriginNation() should not be called!");
}

type::Nation GeoPathMock::getDestinationNation() const
{
  if (_maybeDestinationNation)
    return *_maybeDestinationNation;
  else
    throw std::runtime_error("GeoPath::getDestinationNation() should not be called!");
}

void GeoPathMock::setIsJourneyDomestic(bool isJourneyDomestic)
{
  _maybeIsJourneyDomestic = isJourneyDomestic;
}

void GeoPathMock::setOriginCity(type::CityCode originCity)
{
  _maybeOriginCity = originCity;
}

void GeoPathMock::setDestinationCity(type::CityCode destinationCity)
{
  _maybeDestinationCity = destinationCity;
}

void GeoPathMock::setOriginNation(type::Nation originNation)
{
  _maybeOriginNation = originNation;
}

void GeoPathMock::setDestinationNation(type::Nation destinationNation)
{
  _maybeDestinationNation = destinationNation;
}

} // namespace tax
