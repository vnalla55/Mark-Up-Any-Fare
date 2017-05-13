//----------------------------------------------------------------------------
// � 2005, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class RuleCatAlphaCode
{
public:
  RuleCatAlphaCode() : _displayCategory(0) {}
  AlphaCode& alphaRepresentation() { return _alphaRepresentation; }
  const AlphaCode& alphaRepresentation() const { return _alphaRepresentation; }

  DisplayCategory& displayCategory() { return _displayCategory; }
  const DisplayCategory& displayCategory() const { return _displayCategory; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  bool operator==(const RuleCatAlphaCode& rhs) const
  {
    return ((_alphaRepresentation == rhs._alphaRepresentation) &&
            (_displayCategory == rhs._displayCategory) && (_createDate == rhs._createDate));
  }

  static void dummyData(RuleCatAlphaCode& obj)
  {
    obj._alphaRepresentation = "AB";
    obj._displayCategory = 1;
    obj._createDate = time(nullptr);
  }

private:
  AlphaCode _alphaRepresentation;
  DisplayCategory _displayCategory;
  DateTime _createDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _alphaRepresentation);
    FLATTENIZE(archive, _displayCategory);
    FLATTENIZE(archive, _createDate);
  }

};
}
