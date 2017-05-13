//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class ZoneInfo
{
public:
  ZoneInfo() : _zoneType(' '), _isUniform(false) {}

  struct ZoneSeg
  {
  public:
    ZoneSeg() : _locType(' '), _directionalQualifier(' '), _inclExclInd(' ') {}

    // accessors from Heck

    Indicator& locType() { return _locType; }
    const Indicator& locType() const { return _locType; }

    LocCode& loc() { return _loc; }
    const LocCode& loc() const { return _loc; }

    Indicator& directionalQualifier() { return _directionalQualifier; }
    const Indicator& directionalQualifier() const { return _directionalQualifier; }

    Indicator& inclExclInd() { return _inclExclInd; }
    const Indicator& inclExclInd() const { return _inclExclInd; }

    bool operator==(const ZoneSeg& rhs) const
    {
      return ((_loc == rhs._loc) && (_locType == rhs._locType) &&
              (_directionalQualifier == rhs._directionalQualifier) &&
              (_inclExclInd == rhs._inclExclInd));
    }

    static void dummyData(ZoneSeg& obj)
    {
      obj._loc = "ABCDEFGH";
      obj._locType = 'I';
      obj._directionalQualifier = 'J';
      obj._inclExclInd = 'K';
    }

  protected:
    LocCode _loc;
    Indicator _locType;
    Indicator _directionalQualifier;
    Indicator _inclExclInd;

  public:
    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _loc);
      FLATTENIZE(archive, _locType);
      FLATTENIZE(archive, _directionalQualifier);
      FLATTENIZE(archive, _inclExclInd);
    }

    WBuffer& write(WBuffer& os) const { return convert(os, *this); }

    RBuffer& read(RBuffer& is) { return convert(is, *this); }

    template <typename B, typename T>
    static B& convert(B& buffer, T& obj)
    {
      return buffer & obj._loc & obj._locType & obj._directionalQualifier & obj._inclExclInd;
    }

  protected:
  };

  // more accessors from Heck

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Zone& zone() { return _zone; }
  const Zone& zone() const { return _zone; }

  Indicator& zoneType() { return _zoneType; }
  const Indicator& zoneType() const { return _zoneType; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  void setDescription(const std::string& description) { _description = description; }
  const std::string& getDescription() const { return _description; }

  std::vector<std::vector<ZoneSeg> >& sets() { return _sets; }
  const std::vector<std::vector<ZoneSeg> >& sets() const { return _sets; }

  void setUniform(bool isUniform = true) { _isUniform = isUniform; }
  bool isUniform() const { return _isUniform; }

  bool operator==(const ZoneInfo& rhs) const
  {
    return ((_zoneType == rhs._zoneType) && (_isUniform == rhs._isUniform) && 
            (_vendor == rhs._vendor) && (_zone == rhs._zone) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_description == rhs._description) && (_sets == rhs._sets));
  }

  static void dummyData(ZoneInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._zone = "EFGHIJK";
    obj._zoneType = 'L';
    obj._isUniform = false;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._description = "desc";

    ZoneSeg zs1;
    ZoneSeg zs2;
    ZoneSeg zs3;
    ZoneSeg zs4;

    ZoneSeg::dummyData(zs1);
    ZoneSeg::dummyData(zs2);
    ZoneSeg::dummyData(zs2);
    ZoneSeg::dummyData(zs3);

    std::vector<ZoneSeg> zsv1;
    std::vector<ZoneSeg> zsv2;

    zsv1.push_back(zs1);
    zsv1.push_back(zs2);
    zsv2.push_back(zs3);
    zsv2.push_back(zs4);

    obj._sets.push_back(zsv1);
    obj._sets.push_back(zsv2);
  }

protected:
  VendorCode _vendor;
  Zone _zone;
  Indicator _zoneType;
  bool _isUniform;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _description;
  std::vector<std::vector<ZoneSeg> > _sets;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _zone);
    FLATTENIZE(archive, _zoneType);
    FLATTENIZE(archive, _isUniform);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _sets);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_zone & ptr->_zoneType & ptr->_isUniform & ptr->_expireDate &
           ptr->_createDate & ptr->_effDate & ptr->_discDate & ptr->_description & ptr->_sets;
  }
};

template<> struct cdu_pod_traits<ZoneInfo::ZoneSeg>: std::true_type{};

struct ZoneSegCmpByLoc
{
  bool operator()(const ZoneInfo::ZoneSeg& s1,
                  const ZoneInfo::ZoneSeg& s2) const
  {
    return s1.loc() < s2.loc();
  }
  bool operator()(const ZoneInfo::ZoneSeg& seg,
                  const LocCode& loc) const
  {
    return seg.loc() < loc;
  }
  bool operator()(const LocCode& loc,
                  const ZoneInfo::ZoneSeg& seg) const
  {
    return loc < seg.loc();
  }
};

}

