//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class CustomerActivationControl
{
public:
  CustomerActivationControl() = default;
  CustomerActivationControl(const CustomerActivationControl&) = delete;
  CustomerActivationControl& operator=(const CustomerActivationControl&) = delete;

  struct MultiHostActivation final
  {
    bool operator==(const MultiHostActivation& rhs) const
    {
      return ((_mhCxr == rhs._mhCxr) && (_mhCxrActDate == rhs._mhCxrActDate));
    }

    static void dummyData(MultiHostActivation& obj)
    {
      obj._mhCxr = "XX";
      obj._mhCxrActDate = time(nullptr);
    }

  private:
    CarrierCode _mhCxr;
    DateTime _mhCxrActDate;

  public:
    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _mhCxr);
      FLATTENIZE(archive, _mhCxrActDate);
    }

  public:
    CarrierCode& mhCxr() { return _mhCxr; }
    const CarrierCode& mhCxr() const { return _mhCxr; }

    DateTime& mhCxrActDate() { return _mhCxrActDate; }
    const DateTime& mhCxrActDate() const { return _mhCxrActDate; }
  };

  struct CarrierActivation final
  {
    bool operator==(const CarrierActivation& rhs) const
    {
      return ((_cxr == rhs._cxr) && (_cxrActDate == rhs._cxrActDate));
    }

    static void dummyData(CarrierActivation& obj)
    {
      obj._cxr = "YY";
      obj._cxrActDate = time(nullptr);
    }

  private:
    CarrierCode _cxr;
    DateTime _cxrActDate;

  public:
    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _cxr);
      FLATTENIZE(archive, _cxrActDate);
    }

  public:
    CarrierCode& cxr() { return _cxr; }
    const CarrierCode& cxr() const { return _cxr; }

    DateTime& cxrActDate() { return _cxrActDate; }
    const DateTime& cxrActDate() const { return _cxrActDate; }
  };

  struct GeoActivation final
  {
    bool operator==(const GeoActivation& rhs) const
    {
      return ((_locType == rhs._locType) && (_loc == rhs._loc) &&
              (_locActivationDate == rhs._locActivationDate));
    }

    static void dummyData(GeoActivation& obj)
    {
      obj._locType = 'J';
      obj._loc = "KLMNOPQR";
      obj._locActivationDate = time(nullptr);
    }

  private:
    LocTypeCode _locType = ' ';
    LocCode _loc;
    DateTime _locActivationDate;

  public:
    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _locType);
      FLATTENIZE(archive, _loc);
      FLATTENIZE(archive, _locActivationDate);
    }

  public:
    LocTypeCode& locType() { return _locType; }
    const LocTypeCode& locType() const { return _locType; }

    LocCode& loc() { return _loc; }
    const LocCode& loc() const { return _loc; }

    DateTime& locActivationDate() { return _locActivationDate; }
    const DateTime& locActivationDate() const { return _locActivationDate; }
  };

  std::string& projCode() { return _projCode; }
  const std::string& projCode() const { return _projCode; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  int64_t& seqNo() { return _seqNo; }
  const int64_t& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  std::string& projDesc() { return _projDesc; }
  const std::string& projDesc() const { return _projDesc; }

  Indicator& projActvInd() { return _projActvInd; }
  const Indicator& projActvInd() const { return _projActvInd; }
  bool isActivated() const { return 'X' == _projActvInd; }

  UserApplCode& crsUserAppl() { return _crsUserAppl; }
  const UserApplCode& crsUserAppl() const { return _crsUserAppl; }

  std::vector<MultiHostActivation*>& multiHostActivation() { return _multiHostActivation; }
  const std::vector<MultiHostActivation*>& multiHostActivation() const
  {
    return _multiHostActivation;
  }

  std::vector<CarrierActivation*>& cxrActivation() { return _cxrActivation; }
  const std::vector<CarrierActivation*>& cxrActivation() const { return _cxrActivation; }

  std::vector<GeoActivation*>& geoActivation() { return _geoActivation; }
  const std::vector<GeoActivation*>& geoActivation() const { return _geoActivation; }

  bool operator==(const CustomerActivationControl& second) const
  {
    return (_projCode == second._projCode) && (_versionDate == second._versionDate) &&
           (_seqNo == second._seqNo) && (_createDate == second._createDate) &&
           (_expireDate == second._expireDate) && (_effDate == second._effDate) &&
           (_discDate == second._discDate) && (_pseudoCity == second._pseudoCity) &&
           (_projDesc == second._projDesc) && (_projActvInd == second._projActvInd) &&
           (_crsUserAppl == second._crsUserAppl) &&
           (_multiHostActivation == second._multiHostActivation) &&
           (_cxrActivation == second._cxrActivation) && (_geoActivation == second._geoActivation);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _projCode);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _projDesc);
    FLATTENIZE(archive, _projActvInd);
    FLATTENIZE(archive, _crsUserAppl);
    FLATTENIZE(archive, _multiHostActivation);
    FLATTENIZE(archive, _cxrActivation);
    FLATTENIZE(archive, _geoActivation);
  }

  static void dummyData(CustomerActivationControl& obj)
  {
    obj._projCode = "PROJECTCODE";
    obj._versionDate = time(nullptr);
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._pseudoCity = "STUVW";
    obj._projDesc = "PROJECTDESCRIPTION";
    obj._projActvInd = 'A';
    obj._crsUserAppl = "BCDE";

    MultiHostActivation mha1;
    MultiHostActivation mha2;
    MultiHostActivation::dummyData(mha1);
    MultiHostActivation::dummyData(mha2);
    obj._multiHostActivation.push_back(&mha1);
    obj._multiHostActivation.push_back(&mha2);

    CarrierActivation cxA1;
    CarrierActivation cxA2;
    CarrierActivation::dummyData(cxA1);
    CarrierActivation::dummyData(cxA2);
    obj._cxrActivation.push_back(&cxA1);
    obj._cxrActivation.push_back(&cxA2);

    GeoActivation geoA1;
    GeoActivation geoA2;
    GeoActivation::dummyData(geoA1);
    GeoActivation::dummyData(geoA2);
    obj._geoActivation.push_back(&geoA1);
    obj._geoActivation.push_back(&geoA2);
  }

private:
  std::string _projCode;
  DateTime _versionDate;
  int64_t _seqNo = 0;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  PseudoCityCode _pseudoCity;
  std::string _projDesc;
  Indicator _projActvInd = ' ';
  UserApplCode _crsUserAppl;
  std::string _creatorId;
  std::vector<MultiHostActivation*> _multiHostActivation;
  std::vector<CarrierActivation*> _cxrActivation;
  std::vector<GeoActivation*> _geoActivation;

  friend class SerializationTestBase;
};
}

