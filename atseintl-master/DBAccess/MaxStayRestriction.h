//----------------------------------------------------------------------------
// (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{
class MaxStayRestriction : public RuleItemInfo
{
public:
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& maxStayDate() { return _maxStayDate; }
  const DateTime& maxStayDate() const { return _maxStayDate; }

  DateTime& waiverDate() { return _waiverDate; }
  const DateTime& waiverDate() const { return _waiverDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& geoTblItemNoFrom() { return _geoTblItemNoFrom; }
  const int& geoTblItemNoFrom() const { return _geoTblItemNoFrom; }

  int& geoTblItemNoTo() { return _geoTblItemNoTo; }
  const int& geoTblItemNoTo() const { return _geoTblItemNoTo; }

  int& tod() { return _tod; }
  const int& tod() const { return _tod; }

  Indicator& returnTrvInd() { return _returnTrvInd; }
  const Indicator& returnTrvInd() const { return _returnTrvInd; }

  std::string& maxStay() { return _maxStay; }
  const std::string& maxStay() const { return _maxStay; }

  std::string& maxStayUnit() { return _maxStayUnit; }
  const std::string& maxStayUnit() const { return _maxStayUnit; }

  Indicator& tktIssueInd() { return _tktIssueInd; }
  const Indicator& tktIssueInd() const { return _tktIssueInd; }

  Indicator& maxStayWaiver() { return _maxStayWaiver; }
  const Indicator& maxStayWaiver() const { return _maxStayWaiver; }

  Indicator& earlierLaterInd() { return _earlierLaterInd; }
  const Indicator& earlierLaterInd() const { return _earlierLaterInd; }

  Indicator& waiverEarlierLaterInd() { return _waiverEarlierLaterInd; }
  const Indicator& waiverEarlierLaterInd() const { return _waiverEarlierLaterInd; }

  std::string& waiverPeriod() { return _waiverPeriod; }
  const std::string& waiverPeriod() const { return _waiverPeriod; }

  Indicator& waiverUnit() { return _waiverUnit; }
  const Indicator& waiverUnit() const { return _waiverUnit; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  bool operator==(const MaxStayRestriction& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_maxStayDate == rhs._maxStayDate) &&
            (_waiverDate == rhs._waiverDate) && (_unavailTag == rhs._unavailTag) &&
            (_geoTblItemNoFrom == rhs._geoTblItemNoFrom) &&
            (_geoTblItemNoTo == rhs._geoTblItemNoTo) && (_tod == rhs._tod) &&
            (_returnTrvInd == rhs._returnTrvInd) && (_maxStay == rhs._maxStay) &&
            (_maxStayUnit == rhs._maxStayUnit) && (_tktIssueInd == rhs._tktIssueInd) &&
            (_maxStayWaiver == rhs._maxStayWaiver) && (_earlierLaterInd == rhs._earlierLaterInd) &&
            (_waiverEarlierLaterInd == rhs._waiverEarlierLaterInd) &&
            (_waiverPeriod == rhs._waiverPeriod) && (_waiverUnit == rhs._waiverUnit) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(MaxStayRestriction& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._maxStayDate = time(nullptr);
    obj._waiverDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._geoTblItemNoFrom = 1;
    obj._geoTblItemNoTo = 2;
    obj._tod = 3;
    obj._returnTrvInd = 'B';
    obj._maxStay = "aaaaaaaa";
    obj._maxStayUnit = "bbbbbbbb";
    obj._tktIssueInd = 'C';
    obj._maxStayWaiver = 'D';
    obj._earlierLaterInd = 'E';
    obj._waiverEarlierLaterInd = 'F';
    obj._waiverPeriod = "cccccccc";
    obj._waiverUnit = 'G';
    obj._inhibit = 'H';
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _maxStayDate;
  DateTime _waiverDate;
  Indicator _unavailTag = ' ';
  int _geoTblItemNoFrom = 0;
  int _geoTblItemNoTo = 0;
  int _tod = 0;
  Indicator _returnTrvInd = ' ';
  std::string _maxStay; // c3
  std::string _maxStayUnit; // c2
  Indicator _tktIssueInd = ' ';
  Indicator _maxStayWaiver = ' ';
  Indicator _earlierLaterInd = ' ';
  Indicator _waiverEarlierLaterInd = ' ';
  std::string _waiverPeriod; // c3
  Indicator _waiverUnit = ' ';
  Indicator _inhibit = ' ';

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _maxStayDate);
    FLATTENIZE(archive, _waiverDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _geoTblItemNoFrom);
    FLATTENIZE(archive, _geoTblItemNoTo);
    FLATTENIZE(archive, _tod);
    FLATTENIZE(archive, _returnTrvInd);
    FLATTENIZE(archive, _maxStay);
    FLATTENIZE(archive, _maxStayUnit);
    FLATTENIZE(archive, _tktIssueInd);
    FLATTENIZE(archive, _maxStayWaiver);
    FLATTENIZE(archive, _earlierLaterInd);
    FLATTENIZE(archive, _waiverEarlierLaterInd);
    FLATTENIZE(archive, _waiverPeriod);
    FLATTENIZE(archive, _waiverUnit);
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
           & ptr->_maxStayDate
           & ptr->_waiverDate
           & ptr->_unavailTag
           & ptr->_geoTblItemNoFrom
           & ptr->_geoTblItemNoTo
           & ptr->_tod
           & ptr->_returnTrvInd
           & ptr->_maxStay
           & ptr->_maxStayUnit
           & ptr->_tktIssueInd
           & ptr->_maxStayWaiver
           & ptr->_earlierLaterInd
           & ptr->_waiverEarlierLaterInd
           & ptr->_waiverPeriod
           & ptr->_waiverUnit
           & ptr->_inhibit;
  }
};
}
