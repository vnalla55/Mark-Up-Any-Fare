//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class TravelRestriction : public RuleItemInfo
{
public:
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  int& stopTime() { return _stopTime; }
  const int& stopTime() const { return _stopTime; }

  DateTime& earliestTvlStartDate() { return _earliestTvlStartDate; }
  const DateTime& earliestTvlStartDate() const { return _earliestTvlStartDate; }

  DateTime& latestTvlStartDate() { return _latestTvlStartDate; }
  const DateTime& latestTvlStartDate() const { return _latestTvlStartDate; }

  DateTime& stopDate() { return _stopDate; }
  const DateTime& stopDate() const { return _stopDate; }

  Indicator& returnTvlInd() { return _returnTvlInd; }
  const Indicator& returnTvlInd() const { return _returnTvlInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const TravelRestriction& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
            (_geoTblItemNo == rhs._geoTblItemNo) && (_stopTime == rhs._stopTime) &&
            (_earliestTvlStartDate == rhs._earliestTvlStartDate) &&
            (_latestTvlStartDate == rhs._latestTvlStartDate) && (_stopDate == rhs._stopDate) &&
            (_returnTvlInd == rhs._returnTvlInd) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(TravelRestriction& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._geoTblItemNo = 1;
    obj._stopTime = 2;
    obj._earliestTvlStartDate = time(nullptr);
    obj._latestTvlStartDate = time(nullptr);
    obj._stopDate = time(nullptr);
    obj._returnTvlInd = 'B';
    obj._inhibit = 'C';
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailTag = ' ';
  int _geoTblItemNo = 0;
  int _stopTime = 0;
  DateTime _earliestTvlStartDate;
  DateTime _latestTvlStartDate;
  DateTime _stopDate;
  Indicator _returnTvlInd = ' ';
  Indicator _inhibit = ' ';

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _geoTblItemNo);
    FLATTENIZE(archive, _stopTime);
    FLATTENIZE(archive, _earliestTvlStartDate);
    FLATTENIZE(archive, _latestTvlStartDate);
    FLATTENIZE(archive, _stopDate);
    FLATTENIZE(archive, _returnTvlInd);
    FLATTENIZE(archive, _inhibit);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_unavailTag
           & ptr->_geoTblItemNo
           & ptr->_stopTime
           & ptr->_earliestTvlStartDate
           & ptr->_latestTvlStartDate
           & ptr->_stopDate
           & ptr->_returnTvlInd
           & ptr->_inhibit;
  }

};
}
