//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{

class SubArea
{
public:
  bool operator==(const SubArea& rhs) const
  {
    return ((_subArea == rhs._subArea) && (_area == rhs._area) &&
            (_description == rhs._description));
  }

  static void dummyData(SubArea& obj)
  {
    obj._subArea = "aaaaaaaa";
    obj._area = "bbbbbbbb";
    obj._description = "cccccccc";
  }

private:
  IATASubAreaCode _subArea;
  IATAAreaCode _area;
  Description _description;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _subArea);
    FLATTENIZE(archive, _area);
    FLATTENIZE(archive, _description);
  }

  IATASubAreaCode& subArea() { return _subArea; }
  const IATASubAreaCode& subArea() const { return _subArea; }

  IATAAreaCode& area() { return _area; }
  const IATAAreaCode& area() const { return _area; }

  Description& description() { return _description; }
  const Description& description() const { return _description; }
};
}

