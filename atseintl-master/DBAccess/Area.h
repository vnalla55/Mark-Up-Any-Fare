//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class Area
{
public:
  bool operator==(const Area& rhs) const
  {
    return ((_area == rhs._area) && (_description == rhs._description));
  }

private:
  IATAAreaCode _area;
  Description _description;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _area);
    FLATTENIZE(archive, _description);
  }

  IATAAreaCode& area() { return _area; }
  const IATAAreaCode& area() const { return _area; }

  Description& description() { return _description; }
  const Description& description() const { return _description; }
};
}

