//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class SalesNationRestr
{
public:
  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

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

  int& specialProcessNo() { return _specialProcessNo; }
  const int& specialProcessNo() const { return _specialProcessNo; }

  Indicator& restriction() { return _restriction; }
  const Indicator& restriction() const { return _restriction; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& exceptGoverningCxr() { return _exceptGoverningCxr; }
  const Indicator& exceptGoverningCxr() const { return _exceptGoverningCxr; }

  Indicator& exceptTicketingCxr() { return _exceptTicketingCxr; }
  const Indicator& exceptTicketingCxr() const { return _exceptTicketingCxr; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  Indicator& exceptLoc() { return _exceptLoc; }
  const Indicator& exceptLoc() const { return _exceptLoc; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  Indicator& travelType() { return _travelType; }
  const Indicator& travelType() const { return _travelType; }

  LocKey& viaLoc() { return _viaLoc; }
  const LocKey& viaLoc() const { return _viaLoc; }

  Indicator& curRestrAppl() { return _curRestrAppl; }
  const Indicator& curRestrAppl() const { return _curRestrAppl; }

  Indicator& posExceptInd() { return _posExceptInd; }
  const Indicator& posExceptInd() const { return _posExceptInd; }

  LocKey& posLoc() { return _posLoc; }
  const LocKey& posLoc() const { return _posLoc; }

  Indicator& poiExceptInd() { return _poiExceptInd; }
  const Indicator& poiExceptInd() const { return _poiExceptInd; }

  LocKey& poiLoc() { return _poiLoc; }
  const LocKey& poiLoc() const { return _poiLoc; }

  std::vector<CurrencyCode>& curRstrs() { return _curRstrs; }
  const std::vector<CurrencyCode>& curRstrs() const { return _curRstrs; }

  std::vector<CarrierCode>& govCxrs() { return _govCxrs; }
  const std::vector<CarrierCode>& govCxrs() const { return _govCxrs; }

  std::vector<std::string>& txtSegs() { return _txtSegs; }
  const std::vector<std::string>& txtSegs() const { return _txtSegs; }

  std::vector<CarrierCode>& tktgCxrs() { return _tktgCxrs; }
  const std::vector<CarrierCode>& tktgCxrs() const { return _tktgCxrs; }

  bool operator==(const SalesNationRestr& rhs) const
  {
    return (
        (_nation == rhs._nation) && (_versionDate == rhs._versionDate) && (_seqNo == rhs._seqNo) &&
        (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
        (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
        (_firstTvlDate == rhs._firstTvlDate) && (_lastTvlDate == rhs._lastTvlDate) &&
        (_specialProcessNo == rhs._specialProcessNo) && (_restriction == rhs._restriction) &&
        (_vendor == rhs._vendor) && (_userApplType == rhs._userApplType) &&
        (_userAppl == rhs._userAppl) && (_exceptGoverningCxr == rhs._exceptGoverningCxr) &&
        (_exceptTicketingCxr == rhs._exceptTicketingCxr) &&
        (_directionality == rhs._directionality) && (_exceptLoc == rhs._exceptLoc) &&
        (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_globalDir == rhs._globalDir) &&
        (_travelType == rhs._travelType) && (_viaLoc == rhs._viaLoc) &&
        (_curRestrAppl == rhs._curRestrAppl) && (_posExceptInd == rhs._posExceptInd) &&
        (_posLoc == rhs._posLoc) && (_poiExceptInd == rhs._poiExceptInd) &&
        (_poiLoc == rhs._poiLoc) && (_curRstrs == rhs._curRstrs) && (_govCxrs == rhs._govCxrs) &&
        (_txtSegs == rhs._txtSegs) && (_tktgCxrs == rhs._tktgCxrs));
  }

  static void dummyData(SalesNationRestr& obj)
  {
    obj._nation = "ABCD";
    obj._versionDate = time(nullptr);
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._firstTvlDate = time(nullptr);
    obj._lastTvlDate = time(nullptr);
    obj._specialProcessNo = 2;
    obj._restriction = 'E';
    obj._vendor = "FGHI";
    obj._userApplType = 'J';
    obj._userAppl = "KLMN";
    obj._exceptGoverningCxr = 'O';
    obj._exceptTicketingCxr = 'P';
    obj._directionality = WITHIN;
    obj._exceptLoc = 'Q';

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._globalDir = GlobalDirection::US;
    obj._travelType = 'R';

    LocKey::dummyData(obj._viaLoc);

    obj._curRestrAppl = 'S';
    obj._posExceptInd = 'T';

    LocKey::dummyData(obj._posLoc);

    obj._poiExceptInd = 'U';

    LocKey::dummyData(obj._poiLoc);

    obj._curRstrs.push_back("VWX");
    obj._curRstrs.push_back("YZa");
    obj._govCxrs.push_back("bcd");
    obj._govCxrs.push_back("efg");
    obj._txtSegs.push_back("aaaaaaaa");
    obj._txtSegs.push_back("bbbbbbbb");
    obj._tktgCxrs.push_back("hij");
    obj._tktgCxrs.push_back("klm");
  }

private:
  NationCode _nation;
  DateTime _versionDate;
  int _seqNo = 0;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _firstTvlDate;
  DateTime _lastTvlDate;
  int _specialProcessNo = 0;
  Indicator _restriction = ' ';
  VendorCode _vendor;
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  Indicator _exceptGoverningCxr = ' ';
  Indicator _exceptTicketingCxr = ' ';
  Directionality _directionality = Directionality::TERMINATE;
  Indicator _exceptLoc = ' ';
  LocKey _loc1;
  LocKey _loc2;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  Indicator _travelType = ' ';
  LocKey _viaLoc;
  Indicator _curRestrAppl = ' ';
  Indicator _posExceptInd = ' ';
  LocKey _posLoc;
  Indicator _poiExceptInd = ' ';
  LocKey _poiLoc;
  std::vector<CurrencyCode> _curRstrs;
  std::vector<CarrierCode> _govCxrs;
  std::vector<std::string> _txtSegs;
  std::vector<CarrierCode> _tktgCxrs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _firstTvlDate);
    FLATTENIZE(archive, _lastTvlDate);
    FLATTENIZE(archive, _specialProcessNo);
    FLATTENIZE(archive, _restriction);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _exceptGoverningCxr);
    FLATTENIZE(archive, _exceptTicketingCxr);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _exceptLoc);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _travelType);
    FLATTENIZE(archive, _viaLoc);
    FLATTENIZE(archive, _curRestrAppl);
    FLATTENIZE(archive, _posExceptInd);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _poiExceptInd);
    FLATTENIZE(archive, _poiLoc);
    FLATTENIZE(archive, _curRstrs);
    FLATTENIZE(archive, _govCxrs);
    FLATTENIZE(archive, _txtSegs);
    FLATTENIZE(archive, _tktgCxrs);
  }
};
}
