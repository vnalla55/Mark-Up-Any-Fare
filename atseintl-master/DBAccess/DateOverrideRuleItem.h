//----------------------------------------------------------------------------
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class DateOverrideRuleItem : public RuleItemInfo
{
public:
  DateOverrideRuleItem() : _inhibit(' ') {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& tvlEffDate() { return _tvlEffDate; }
  const DateTime& tvlEffDate() const { return _tvlEffDate; }

  DateTime& tvlDiscDate() { return _tvlDiscDate; }
  const DateTime& tvlDiscDate() const { return _tvlDiscDate; }

  DateTime& tktEffDate() { return _tktEffDate; }
  const DateTime& tktEffDate() const { return _tktEffDate; }

  DateTime& tktDiscDate() { return _tktDiscDate; }
  const DateTime& tktDiscDate() const { return _tktDiscDate; }

  DateTime& resEffDate() { return _resEffDate; }
  const DateTime& resEffDate() const { return _resEffDate; }

  DateTime& resDiscDate() { return _resDiscDate; }
  const DateTime& resDiscDate() const { return _resDiscDate; }

  const Indicator inhibit() const
  {
    return _inhibit;
  };
  Indicator& inhibit()
  {
    return _inhibit;
  };

  virtual bool operator==(const DateOverrideRuleItem& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_tvlEffDate == rhs._tvlEffDate) &&
            (_tvlDiscDate == rhs._tvlDiscDate) && (_tktEffDate == rhs._tktEffDate) &&
            (_tktDiscDate == rhs._tktDiscDate) && (_resEffDate == rhs._resEffDate) &&
            (_resDiscDate == rhs._resDiscDate) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(DateOverrideRuleItem& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._tvlEffDate = time(nullptr);
    obj._tvlDiscDate = time(nullptr);
    obj._tktEffDate = time(nullptr);
    obj._tktDiscDate = time(nullptr);
    obj._resEffDate = time(nullptr);
    obj._resDiscDate = time(nullptr);
    obj._inhibit = 'A';
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
  DateTime _tvlEffDate;
  DateTime _tvlDiscDate;
  DateTime _tktEffDate;
  DateTime _tktDiscDate;
  DateTime _resEffDate;
  DateTime _resDiscDate;
  Indicator _inhibit; // Inhibit now checked at App Level

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _tvlEffDate);
    FLATTENIZE(archive, _tvlDiscDate);
    FLATTENIZE(archive, _tktEffDate);
    FLATTENIZE(archive, _tktDiscDate);
    FLATTENIZE(archive, _resEffDate);
    FLATTENIZE(archive, _resDiscDate);
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
           & ptr->_tvlEffDate
           & ptr->_tvlDiscDate
           & ptr->_tktEffDate
           & ptr->_tktDiscDate
           & ptr->_resEffDate
           & ptr->_resDiscDate
           & ptr->_inhibit;
  }
};
}
