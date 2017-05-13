//----------------------------------------------------------------------------
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class GeoRuleItem : public RuleItemInfo
{
public:
  GeoRuleItem() : _tsi(0), _inhibit(' ') {}

  virtual ~GeoRuleItem() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  TSICode& tsi() { return _tsi; }
  const TSICode& tsi() const { return _tsi; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  const Indicator inhibit() const
  {
    return _inhibit;
  };
  Indicator& inhibit()
  {
    return _inhibit;
  };

  virtual bool operator==(const GeoRuleItem& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_tsi == rhs._tsi) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(GeoRuleItem& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._tsi = 1;
    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);
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
  TSICode _tsi;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _inhibit; // Inhibit now checked at App Level

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _tsi);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
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
           & ptr->_tsi
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_inhibit;
  }
};
}
