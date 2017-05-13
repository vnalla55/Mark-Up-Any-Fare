//----------------------------------------------------------------------------
// � 2005, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareDispTemplate
{

public:
  FareDispTemplate() : _templateID(0), _templateType(' '), _lineStyle(' ') {}

  int& templateID() { return _templateID; }
  const int& templateID() const { return _templateID; }

  Indicator& templateType() { return _templateType; }
  const Indicator& templateType() const { return _templateType; }

  Indicator& lineStyle() { return _lineStyle; }
  const Indicator& lineStyle() const { return _lineStyle; }

  bool operator==(const FareDispTemplate& rhs) const
  {
    return ((_templateID == rhs._templateID) && (_templateType == rhs._templateType) &&
            (_lineStyle == rhs._lineStyle));
  }

  static void dummyData(FareDispTemplate& obj)
  {
    obj._templateID = 1;
    obj._templateType = 'A';
    obj._lineStyle = 'B';
  }

private:
  int _templateID;
  Indicator _templateType;
  Indicator _lineStyle;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _templateID);
    FLATTENIZE(archive, _templateType);
    FLATTENIZE(archive, _lineStyle);
  }

};
}

