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

class FareDispTemplateSeg
{

public:
  FareDispTemplateSeg()
    : _templateID(0),
      _templateType(' '),
      _columnElement(0),
      _elementStart(0),
      _elementLength(0),
      _elementJustify(' '),
      _elementDateFormat(' '),
      _headerStart(0)
  {
  }

  int16_t& templateID() { return _templateID; }
  const int16_t& templateID() const { return _templateID; }

  Indicator& templateType() { return _templateType; }
  const Indicator& templateType() const { return _templateType; }

  int16_t& columnElement() { return _columnElement; }
  const int16_t& columnElement() const { return _columnElement; }

  int16_t& elementStart() { return _elementStart; }
  const int16_t& elementStart() const { return _elementStart; }

  int16_t& elementLength() { return _elementLength; }
  const int16_t& elementLength() const { return _elementLength; }

  Indicator& elementJustify() { return _elementJustify; }
  const Indicator& elementJustify() const { return _elementJustify; }

  Indicator& elementDateFormat() { return _elementDateFormat; }
  const Indicator& elementDateFormat() const { return _elementDateFormat; }

  std::string& header() { return _header; }
  const std::string& header() const { return _header; }

  int16_t& headerStart() { return _headerStart; }
  const int16_t& headerStart() const { return _headerStart; }

  bool operator==(const FareDispTemplateSeg& rhs) const
  {
    return ((_templateID == rhs._templateID) && (_templateType == rhs._templateType) &&
            (_columnElement == rhs._columnElement) && (_elementStart == rhs._elementStart) &&
            (_elementLength == rhs._elementLength) && (_elementJustify == rhs._elementJustify) &&
            (_elementDateFormat == rhs._elementDateFormat) && (_header == rhs._header) &&
            (_headerStart == rhs._headerStart));
  }

  static void dummyData(FareDispTemplateSeg& obj)
  {
    obj._templateID = 1;
    obj._templateType = 'A';
    obj._columnElement = 2;
    obj._elementStart = 3;
    obj._elementLength = 4;
    obj._elementJustify = 'B';
    obj._elementDateFormat = 'C';
    obj._header = "aaaaaaaa";
    obj._headerStart = 5;
  }

private:
  int16_t _templateID;
  Indicator _templateType;
  int16_t _columnElement;
  int16_t _elementStart;
  int16_t _elementLength;
  Indicator _elementJustify;
  Indicator _elementDateFormat;
  std::string _header;
  int16_t _headerStart;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _templateID);
    FLATTENIZE(archive, _templateType);
    FLATTENIZE(archive, _columnElement);
    FLATTENIZE(archive, _elementStart);
    FLATTENIZE(archive, _elementLength);
    FLATTENIZE(archive, _elementJustify);
    FLATTENIZE(archive, _elementDateFormat);
    FLATTENIZE(archive, _header);
    FLATTENIZE(archive, _headerStart);
  }

};
}

