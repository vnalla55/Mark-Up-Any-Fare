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

class MinStayRestriction : public RuleItemInfo
{
public:
  MinStayRestriction()
    : _unavailTag(' '),
      _geoTblItemNoFrom(0),
      _geoTblItemNoTo(0),
      _tod(0),
      _minStayWaiver(' '),
      _earlierLaterInd(' '),
      _waiverEarlierLaterInd(' '),
      _waiverUnit(' '),
      _mileageAppl(' '),
      _mileageGreaterThan(0),
      _mileageLessThan(0),
      _inhibit(' ')
  {
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& minStayDate() { return _minStayDate; }
  const DateTime& minStayDate() const { return _minStayDate; }

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

  std::string& originDow() { return _originDow; }
  const std::string& originDow() const { return _originDow; }

  std::string& minStay() { return _minStay; }
  const std::string& minStay() const { return _minStay; }

  std::string& minStayUnit() { return _minStayUnit; }
  const std::string& minStayUnit() const { return _minStayUnit; }

  Indicator& minStayWaiver() { return _minStayWaiver; }
  const Indicator& minStayWaiver() const { return _minStayWaiver; }

  Indicator& earlierLaterInd() { return _earlierLaterInd; }
  const Indicator& earlierLaterInd() const { return _earlierLaterInd; }

  Indicator& waiverEarlierLaterInd() { return _waiverEarlierLaterInd; }
  const Indicator& waiverEarlierLaterInd() const { return _waiverEarlierLaterInd; }

  std::string& waiverPeriod() { return _waiverPeriod; }
  const std::string& waiverPeriod() const { return _waiverPeriod; }

  Indicator& waiverUnit() { return _waiverUnit; }
  const Indicator& waiverUnit() const { return _waiverUnit; }

  Indicator& mileageAppl() { return _mileageAppl; }
  const Indicator& mileageAppl() const { return _mileageAppl; }

  int& mileageGreaterThan() { return _mileageGreaterThan; }
  const int& mileageGreaterThan() const { return _mileageGreaterThan; }

  int& mileageLessThan() { return _mileageLessThan; }
  const int& mileageLessThan() const { return _mileageLessThan; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const MinStayRestriction& rhs) const
  {
    return (
        (RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_minStayDate == rhs._minStayDate) &&
        (_waiverDate == rhs._waiverDate) && (_unavailTag == rhs._unavailTag) &&
        (_geoTblItemNoFrom == rhs._geoTblItemNoFrom) && (_geoTblItemNoTo == rhs._geoTblItemNoTo) &&
        (_tod == rhs._tod) && (_originDow == rhs._originDow) && (_minStay == rhs._minStay) &&
        (_minStayUnit == rhs._minStayUnit) && (_minStayWaiver == rhs._minStayWaiver) &&
        (_earlierLaterInd == rhs._earlierLaterInd) &&
        (_waiverEarlierLaterInd == rhs._waiverEarlierLaterInd) &&
        (_waiverPeriod == rhs._waiverPeriod) && (_waiverUnit == rhs._waiverUnit) &&
        (_mileageAppl == rhs._mileageAppl) && (_mileageGreaterThan == rhs._mileageGreaterThan) &&
        (_mileageLessThan == rhs._mileageLessThan) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(MinStayRestriction& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._minStayDate = time(nullptr);
    obj._waiverDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._geoTblItemNoFrom = 1;
    obj._geoTblItemNoTo = 2;
    obj._tod = 3;
    obj._originDow = "aaaaaaaa";
    obj._minStay = "bbbbbbbb";
    obj._minStayUnit = "cccccccc";
    obj._minStayWaiver = 'B';
    obj._earlierLaterInd = 'C';
    obj._waiverEarlierLaterInd = 'D';
    obj._waiverPeriod = "dddddddd";
    obj._waiverUnit = 'E';
    obj._mileageAppl = 'F';
    obj._mileageGreaterThan = 4;
    obj._mileageLessThan = 5;
    obj._inhibit = 'G';
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
  DateTime _minStayDate;
  DateTime _waiverDate;
  Indicator _unavailTag;
  int _geoTblItemNoFrom;
  int _geoTblItemNoTo;
  int _tod;
  std::string _originDow; // vc6
  std::string _minStay; // c3
  std::string _minStayUnit; // c2
  Indicator _minStayWaiver;
  Indicator _earlierLaterInd;
  Indicator _waiverEarlierLaterInd;
  std::string _waiverPeriod; // c3
  Indicator _waiverUnit;
  Indicator _mileageAppl;
  int _mileageGreaterThan;
  int _mileageLessThan;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _minStayDate);
    FLATTENIZE(archive, _waiverDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _geoTblItemNoFrom);
    FLATTENIZE(archive, _geoTblItemNoTo);
    FLATTENIZE(archive, _tod);
    FLATTENIZE(archive, _originDow);
    FLATTENIZE(archive, _minStay);
    FLATTENIZE(archive, _minStayUnit);
    FLATTENIZE(archive, _minStayWaiver);
    FLATTENIZE(archive, _earlierLaterInd);
    FLATTENIZE(archive, _waiverEarlierLaterInd);
    FLATTENIZE(archive, _waiverPeriod);
    FLATTENIZE(archive, _waiverUnit);
    FLATTENIZE(archive, _mileageAppl);
    FLATTENIZE(archive, _mileageGreaterThan);
    FLATTENIZE(archive, _mileageLessThan);
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
           & ptr->_minStayDate
           & ptr->_waiverDate
           & ptr->_unavailTag
           & ptr->_geoTblItemNoFrom
           & ptr->_geoTblItemNoTo
           & ptr->_tod
           & ptr->_originDow
           & ptr->_minStay
           & ptr->_minStayUnit
           & ptr->_minStayWaiver
           & ptr->_earlierLaterInd
           & ptr->_waiverEarlierLaterInd
           & ptr->_waiverPeriod
           & ptr->_waiverUnit
           & ptr->_mileageAppl
           & ptr->_mileageGreaterThan
           & ptr->_mileageLessThan
           & ptr->_inhibit;
  }
};
}

