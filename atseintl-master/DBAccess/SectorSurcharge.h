//----------------------------------------------------------------------------
//  2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class SectorSurcharge
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& firstTvlDate() { return _firstTvlDate; }
  const DateTime& firstTvlDate() const { return _firstTvlDate; }

  DateTime& lastTvlDate() { return _lastTvlDate; }
  const DateTime& lastTvlDate() const { return _lastTvlDate; }

  int& startTime() { return _startTime; }
  const int& startTime() const { return _startTime; }

  int& stopTime() { return _stopTime; }
  const int& stopTime() const { return _stopTime; }

  int& surchargeNoDec1() { return _surchargeNoDec1; }
  const int& surchargeNoDec1() const { return _surchargeNoDec1; }

  MoneyAmount& surchargeAmt1() { return _surchargeAmt1; }
  const MoneyAmount& surchargeAmt1() const { return _surchargeAmt1; }

  int& surchargeNoDec2() { return _surchargeNoDec2; }
  const int& surchargeNoDec2() const { return _surchargeNoDec2; }

  MoneyAmount& surchargeAmt2() { return _surchargeAmt2; }
  const MoneyAmount& surchargeAmt2() const { return _surchargeAmt2; }

  int& surchargePercentNoDec() { return _surchargePercentNoDec; }
  const int& surchargePercentNoDec() const { return _surchargePercentNoDec; }

  Percent& surchargePercent() { return _surchargePercent; }
  const Percent& surchargePercent() const { return _surchargePercent; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Directionality& directionalInd() { return _directionalInd; }
  const Directionality& directionalInd() const { return _directionalInd; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& surchargeAppl() { return _surchargeAppl; }
  const Indicator& surchargeAppl() const { return _surchargeAppl; }

  LocKey& posLoc() { return _posLoc; }
  const LocKey& posLoc() const { return _posLoc; }

  LocKey& poiLoc() { return _poiLoc; }
  const LocKey& poiLoc() const { return _poiLoc; }

  Indicator& exceptTktgCarrier() { return _exceptTktgCarrier; }
  const Indicator& exceptTktgCarrier() const { return _exceptTktgCarrier; }

  std::string& dow() { return _dow; }
  const std::string& dow() const { return _dow; }

  Indicator& surchargeType() { return _surchargeType; }
  const Indicator& surchargeType() const { return _surchargeType; }

  CurrencyCode& surchargeCur1() { return _surchargeCur1; }
  const CurrencyCode& surchargeCur1() const { return _surchargeCur1; }

  CurrencyCode& surchargeCur2() { return _surchargeCur2; }
  const CurrencyCode& surchargeCur2() const { return _surchargeCur2; }

  Indicator& exclPsgType() { return _exclPsgType; }
  const Indicator& exclPsgType() const { return _exclPsgType; }

  Indicator& psgTypeChild() { return _psgTypeChild; }
  const Indicator& psgTypeChild() const { return _psgTypeChild; }

  Indicator& psgTypeInfant() { return _psgTypeInfant; }
  const Indicator& psgTypeInfant() const { return _psgTypeInfant; }

  std::vector<PaxTypeCode>& psgTypes() { return _psgTypes; }
  const std::vector<PaxTypeCode>& psgTypes() const { return _psgTypes; }

  std::vector<CarrierCode>& tktgCxrs() { return _tktgCxrs; }
  const std::vector<CarrierCode>& tktgCxrs() const { return _tktgCxrs; }

  bool operator==(const SectorSurcharge& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_versionDate == rhs._versionDate) &&
            (_seqNo == rhs._seqNo) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_firstTvlDate == rhs._firstTvlDate) &&
            (_lastTvlDate == rhs._lastTvlDate) && (_startTime == rhs._startTime) &&
            (_stopTime == rhs._stopTime) && (_surchargeNoDec1 == rhs._surchargeNoDec1) &&
            (_surchargeAmt1 == rhs._surchargeAmt1) && (_surchargeNoDec2 == rhs._surchargeNoDec2) &&
            (_surchargeAmt2 == rhs._surchargeAmt2) &&
            (_surchargePercentNoDec == rhs._surchargePercentNoDec) &&
            (_surchargePercent == rhs._surchargePercent) && (_userApplType == rhs._userApplType) &&
            (_userAppl == rhs._userAppl) && (_directionalInd == rhs._directionalInd) &&
            (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) &&
            (_surchargeAppl == rhs._surchargeAppl) && (_posLoc == rhs._posLoc) &&
            (_poiLoc == rhs._poiLoc) && (_exceptTktgCarrier == rhs._exceptTktgCarrier) &&
            (_dow == rhs._dow) && (_surchargeType == rhs._surchargeType) &&
            (_surchargeCur1 == rhs._surchargeCur1) && (_surchargeCur2 == rhs._surchargeCur2) &&
            (_exclPsgType == rhs._exclPsgType) && (_psgTypeChild == rhs._psgTypeChild) &&
            (_psgTypeInfant == rhs._psgTypeInfant) && (_psgTypes == rhs._psgTypes) &&
            (_tktgCxrs == rhs._tktgCxrs));
  }

  static void dummyData(SectorSurcharge& obj)
  {
    obj._carrier = "ABC";
    obj._versionDate = time(nullptr);
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._firstTvlDate = time(nullptr);
    obj._lastTvlDate = time(nullptr);
    obj._startTime = 2;
    obj._stopTime = 3;
    obj._surchargeNoDec1 = 4;
    obj._surchargeAmt1 = 5.55;
    obj._surchargeNoDec2 = 6;
    obj._surchargeAmt2 = 7.77;
    obj._surchargePercentNoDec = 8;
    obj._surchargePercent = 9.999;
    obj._userApplType = 'D';
    obj._userAppl = "EFGH";
    obj._directionalInd = TO;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._surchargeAppl = 'I';

    LocKey::dummyData(obj._posLoc);
    LocKey::dummyData(obj._poiLoc);

    obj._exceptTktgCarrier = 'J';
    obj._dow = "bbbbbbbb";
    obj._surchargeType = 'K';
    obj._surchargeCur1 = "LMN";
    obj._surchargeCur2 = "OPQ";
    obj._exclPsgType = 'R';
    obj._psgTypeChild = 'S';
    obj._psgTypeInfant = 'T';

    obj._psgTypes.push_back("UVW");
    obj._psgTypes.push_back("XYZ");

    obj._tktgCxrs.push_back("abc");
    obj._tktgCxrs.push_back("def");
  }

private:
  CarrierCode _carrier;
  DateTime _versionDate;
  int _seqNo = 0;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _firstTvlDate;
  DateTime _lastTvlDate;
  int _startTime = 0;
  int _stopTime = 0;
  int _surchargeNoDec1 = 0;
  MoneyAmount _surchargeAmt1 = 0;
  int _surchargeNoDec2 = 0;
  MoneyAmount _surchargeAmt2 = 0;
  int _surchargePercentNoDec = 0;
  Percent _surchargePercent = 0;
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  Directionality _directionalInd = Directionality::TERMINATE;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _surchargeAppl = ' ';
  LocKey _posLoc;
  LocKey _poiLoc;
  Indicator _exceptTktgCarrier = ' ';
  std::string _dow;
  Indicator _surchargeType = ' ';
  CurrencyCode _surchargeCur1;
  CurrencyCode _surchargeCur2;
  Indicator _exclPsgType = ' ';
  Indicator _psgTypeChild = ' ';
  Indicator _psgTypeInfant = ' ';
  std::vector<PaxTypeCode> _psgTypes;
  std::vector<CarrierCode> _tktgCxrs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _firstTvlDate);
    FLATTENIZE(archive, _lastTvlDate);
    FLATTENIZE(archive, _startTime);
    FLATTENIZE(archive, _stopTime);
    FLATTENIZE(archive, _surchargeNoDec1);
    FLATTENIZE(archive, _surchargeAmt1);
    FLATTENIZE(archive, _surchargeNoDec2);
    FLATTENIZE(archive, _surchargeAmt2);
    FLATTENIZE(archive, _surchargePercentNoDec);
    FLATTENIZE(archive, _surchargePercent);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _directionalInd);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _surchargeAppl);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _poiLoc);
    FLATTENIZE(archive, _exceptTktgCarrier);
    FLATTENIZE(archive, _dow);
    FLATTENIZE(archive, _surchargeType);
    FLATTENIZE(archive, _surchargeCur1);
    FLATTENIZE(archive, _surchargeCur2);
    FLATTENIZE(archive, _exclPsgType);
    FLATTENIZE(archive, _psgTypeChild);
    FLATTENIZE(archive, _psgTypeInfant);
    FLATTENIZE(archive, _psgTypes);
    FLATTENIZE(archive, _tktgCxrs);
  }
}; // Class
} // NameSpace
