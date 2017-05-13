// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include "DomainDataObjects/GeoPathMapping.h"

namespace tax
{
class GeoPathMappingBuilder
{
  GeoPathMapping* _geoPathMapping;

public:
  GeoPathMappingBuilder() : _geoPathMapping(new GeoPathMapping) {}

  GeoPathMappingBuilder& addMap(type::Index mappingId, type::Index geoId)
  {
    if (_geoPathMapping->mappings().size() == mappingId)
      _geoPathMapping->mappings().push_back(Mapping());

    _geoPathMapping->mappings()[mappingId].maps().push_back(Map());
    _geoPathMapping->mappings()[mappingId].maps().back().index() = geoId;

    return *this;
  }

  GeoPathMapping* build() { return _geoPathMapping; }
};
}

