#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class GlobalDirSeg
{
private:
  std::string _globalDir;
  DateTime _createDate;
  Indicator _directionality = ' ';
  LocType _loc1Type = LocType::UNKNOWN_LOC;
  LocCode _loc1;
  LocType _loc2Type = LocType::UNKNOWN_LOC;
  LocCode _loc2;
  std::string _allTvlOnCarrier;
  DateTime _saleEffDate;
  DateTime _saleDiscDate;
  Indicator _flightTrackingInd = ' ';
  Indicator _mustBeViaLocInd = ' ';
  LocType _mustBeViaLocType = LocType::UNKNOWN_LOC;
  LocCode _mustBeViaLoc;
  LocType _viaInterLoc1Type = LocType::UNKNOWN_LOC;
  LocCode _viaInterLoc1;
  LocType _viaInterLoc2Type = LocType::UNKNOWN_LOC;
  LocCode _viaInterLoc2;
  Indicator _mustNotBeViaLocInd = ' ';
  LocType _mustNotBeViaLocType = LocType::UNKNOWN_LOC;
  LocCode _mustNotBeViaLoc;
  LocType _notViaInterLoc1Type = LocType::UNKNOWN_LOC;
  LocCode _notViaInterLoc1;
  LocType _notViaInterLoc2Type = LocType::UNKNOWN_LOC;
  LocCode _notViaInterLoc2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1Type);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2Type);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _allTvlOnCarrier);
    FLATTENIZE(archive, _saleEffDate);
    FLATTENIZE(archive, _saleDiscDate);
    FLATTENIZE(archive, _flightTrackingInd);
    FLATTENIZE(archive, _mustBeViaLocInd);
    FLATTENIZE(archive, _mustBeViaLocType);
    FLATTENIZE(archive, _mustBeViaLoc);
    FLATTENIZE(archive, _viaInterLoc1Type);
    FLATTENIZE(archive, _viaInterLoc1);
    FLATTENIZE(archive, _viaInterLoc2Type);
    FLATTENIZE(archive, _viaInterLoc2);
    FLATTENIZE(archive, _mustNotBeViaLocInd);
    FLATTENIZE(archive, _mustNotBeViaLocType);
    FLATTENIZE(archive, _mustNotBeViaLoc);
    FLATTENIZE(archive, _notViaInterLoc1Type);
    FLATTENIZE(archive, _notViaInterLoc1);
    FLATTENIZE(archive, _notViaInterLoc2Type);
    FLATTENIZE(archive, _notViaInterLoc2);
  }

  std::string& globalDir() { return _globalDir; }
  const std::string& globalDir() const { return _globalDir; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  LocType& loc1Type() { return _loc1Type; }
  const LocType& loc1Type() const { return _loc1Type; }

  LocCode& loc1() { return _loc1; }
  const LocCode& loc1() const { return _loc1; }

  LocType& loc2Type() { return _loc2Type; }
  const LocType& loc2Type() const { return _loc2Type; }

  LocCode& loc2() { return _loc2; }
  const LocCode& loc2() const { return _loc2; }

  std::string& allTvlOnCarrier() { return _allTvlOnCarrier; }
  const std::string& allTvlOnCarrier() const { return _allTvlOnCarrier; }

  DateTime& saleEffDate() { return _saleEffDate; }
  const DateTime& saleEffDate() const { return _saleEffDate; }

  DateTime& saleDiscDate() { return _saleDiscDate; }
  const DateTime& saleDiscDate() const { return _saleDiscDate; }

  Indicator& flightTrackingInd() { return _flightTrackingInd; }
  const Indicator& flightTrackingInd() const { return _flightTrackingInd; }

  Indicator& mustBeViaLocInd() { return _mustBeViaLocInd; }
  const Indicator& mustBeViaLocInd() const { return _mustBeViaLocInd; }

  LocType& mustBeViaLocType() { return _mustBeViaLocType; }
  const LocType& mustBeViaLocType() const { return _mustBeViaLocType; }

  LocCode& mustBeViaLoc() { return _mustBeViaLoc; }
  const LocCode& mustBeViaLoc() const { return _mustBeViaLoc; }

  LocType& viaInterLoc1Type() { return _viaInterLoc1Type; }
  const LocType& viaInterLoc1Type() const { return _viaInterLoc1Type; }

  LocCode& viaInterLoc1() { return _viaInterLoc1; }
  const LocCode& viaInterLoc1() const { return _viaInterLoc1; }

  LocType& viaInterLoc2Type() { return _viaInterLoc2Type; }
  const LocType& viaInterLoc2Type() const { return _viaInterLoc2Type; }

  LocCode& viaInterLoc2() { return _viaInterLoc2; }
  const LocCode& viaInterLoc2() const { return _viaInterLoc2; }

  Indicator& mustNotBeViaLocInd() { return _mustNotBeViaLocInd; }
  const Indicator& mustNotBeViaLocInd() const { return _mustNotBeViaLocInd; }

  LocType& mustNotBeViaLocType() { return _mustNotBeViaLocType; }
  const LocType& mustNotBeViaLocType() const { return _mustNotBeViaLocType; }

  LocCode& mustNotBeViaLoc() { return _mustNotBeViaLoc; }
  const LocCode& mustNotBeViaLoc() const { return _mustNotBeViaLoc; }

  LocType& notViaInterLoc1Type() { return _notViaInterLoc1Type; }
  const LocType& notViaInterLoc1Type() const { return _notViaInterLoc1Type; }

  LocCode& notViaInterLoc1() { return _notViaInterLoc1; }
  const LocCode& notViaInterLoc1() const { return _notViaInterLoc1; }

  LocType& notViaInterLoc2Type() { return _notViaInterLoc2Type; }
  const LocType& notViaInterLoc2Type() const { return _notViaInterLoc2Type; }

  LocCode& notViaInterLoc2() { return _notViaInterLoc2; }
  const LocCode& notViaInterLoc2() const { return _notViaInterLoc2; }

  bool operator==(const GlobalDirSeg& rhs) const
  {
    return ((_globalDir == rhs._globalDir) && (_createDate == rhs._createDate) &&
            (_directionality == rhs._directionality) && (_loc1Type == rhs._loc1Type) &&
            (_loc1 == rhs._loc1) && (_loc2Type == rhs._loc2Type) && (_loc2 == rhs._loc2) &&
            (_allTvlOnCarrier == rhs._allTvlOnCarrier) && (_saleEffDate == rhs._saleEffDate) &&
            (_saleDiscDate == rhs._saleDiscDate) &&
            (_flightTrackingInd == rhs._flightTrackingInd) &&
            (_mustBeViaLocInd == rhs._mustBeViaLocInd) &&
            (_mustBeViaLocType == rhs._mustBeViaLocType) && (_mustBeViaLoc == rhs._mustBeViaLoc) &&
            (_viaInterLoc1Type == rhs._viaInterLoc1Type) && (_viaInterLoc1 == rhs._viaInterLoc1) &&
            (_viaInterLoc2Type == rhs._viaInterLoc2Type) && (_viaInterLoc2 == rhs._viaInterLoc2) &&
            (_mustNotBeViaLocInd == rhs._mustNotBeViaLocInd) &&
            (_mustNotBeViaLocType == rhs._mustNotBeViaLocType) &&
            (_mustNotBeViaLoc == rhs._mustNotBeViaLoc) &&
            (_notViaInterLoc1Type == rhs._notViaInterLoc1Type) &&
            (_notViaInterLoc1 == rhs._notViaInterLoc1) &&
            (_notViaInterLoc2Type == rhs._notViaInterLoc2Type) &&
            (_notViaInterLoc2 == rhs._notViaInterLoc2));
  }

  static void dummyData(GlobalDirSeg& obj)
  {
    obj._globalDir = "aaaaaaaa";
    obj._createDate = time(nullptr);
    obj._directionality = 'A';
    obj._loc1Type = NATION;
    obj._loc1 = "bbbbbbbb";
    obj._loc2Type = ZONE;
    obj._loc2 = "cccccccc";
    obj._allTvlOnCarrier = "dddddddd";
    obj._saleEffDate = time(nullptr);
    obj._saleDiscDate = time(nullptr);
    obj._flightTrackingInd = 'B';
    obj._mustBeViaLocInd = 'C';
    obj._mustBeViaLocType = MARKET;
    obj._mustBeViaLoc = "eeeeeeee";
    obj._viaInterLoc1Type = STATE_PROVINCE;
    obj._viaInterLoc1 = "ffffffff";
    obj._viaInterLoc2Type = SUBAREA;
    obj._viaInterLoc2 = "gggggggg";
    obj._mustNotBeViaLocInd = 'D';
    obj._mustNotBeViaLocType = IATA_AREA;
    obj._mustNotBeViaLoc = "hhhhhhhh";
    obj._notViaInterLoc1Type = UNKNOWN_LOC;
    obj._notViaInterLoc1 = "iiiiiiii";
    obj._notViaInterLoc2Type = NATION;
    obj._notViaInterLoc2 = "jjjjjjjj";
  }
};
}

