//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AdvResTktInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

bool
AdvResTktInfo::
operator==(const AdvResTktInfo& rhs) const
{
  return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
          (_expireDate == rhs._expireDate) && (_unavailtag == rhs._unavailtag) &&
          (_advTktDepart == rhs._advTktDepart) && (_geoTblItemNo == rhs._geoTblItemNo) &&
          (_firstResTod == rhs._firstResTod) && (_lastResTod == rhs._lastResTod) &&
          (_advTktTod == rhs._advTktTod) && (_advTktExcpTime == rhs._advTktExcpTime) &&
          (_tktTSI == rhs._tktTSI) && (_resTSI == rhs._resTSI) &&
          (_firstResPeriod == rhs._firstResPeriod) && (_firstResUnit == rhs._firstResUnit) &&
          (_lastResPeriod == rhs._lastResPeriod) && (_lastResUnit == rhs._lastResUnit) &&
          (_permitted == rhs._permitted) && (_ticketed == rhs._ticketed) &&
          (_standby == rhs._standby) && (_confirmedSector == rhs._confirmedSector) &&
          (_eachSector == rhs._eachSector) && (_advTktPeriod == rhs._advTktPeriod) &&
          (_advTktUnit == rhs._advTktUnit) && (_advTktOpt == rhs._advTktOpt) &&
          (_advTktDepartUnit == rhs._advTktDepartUnit) && (_advTktBoth == rhs._advTktBoth) &&
          (_advTktExcpUnit == rhs._advTktExcpUnit) && (_waiverResDate == rhs._waiverResDate) &&
          (_waiverTktDate == rhs._waiverTktDate) && (_inhibit == rhs._inhibit));
}

std::ostream&
dumpObject(std::ostream& os, const AdvResTktInfo& obj)
{
  os << "[";

  dumpObject(os, static_cast<const RuleItemInfo&>(obj));

  os << "|" << obj._createDate << "|" << obj._expireDate << "|" << obj._unavailtag << "|"
     << obj._advTktDepart << "|" << obj._geoTblItemNo << "|" << obj._firstResTod << "|"
     << obj._lastResTod << "|" << obj._advTktTod << "|" << obj._advTktExcpTime << "|" << obj._tktTSI
     << "|" << obj._resTSI << "|" << obj._firstResPeriod << "|" << obj._firstResUnit << "|"
     << obj._lastResPeriod << "|" << obj._lastResUnit << "|" << obj._permitted << "|"
     << obj._ticketed << "|" << obj._standby << "|" << obj._confirmedSector << "|"
     << obj._eachSector << "|" << obj._advTktPeriod << "|" << obj._advTktUnit << "|"
     << obj._advTktOpt << "|" << obj._advTktDepartUnit << "|" << obj._advTktBoth << "|"
     << obj._advTktExcpUnit << "|" << obj._waiverResDate << "|" << obj._waiverTktDate << "|"
     << obj._inhibit << "]";

  return os;
}

void
AdvResTktInfo::dummyData(AdvResTktInfo& obj)
{
  RuleItemInfo::dummyData(obj);

  obj._createDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._unavailtag = 'A';
  obj._advTktDepart = 1;
  obj._geoTblItemNo = 2;
  obj._firstResTod = 3;
  obj._lastResTod = 4;
  obj._advTktTod = 5;
  obj._advTktExcpTime = 6;
  obj._tktTSI = 7;
  obj._resTSI = 8;
  obj._firstResPeriod = "BCD";
  obj._firstResUnit = "EF";
  obj._lastResPeriod = "EFG";
  obj._lastResUnit = "HI";
  obj._permitted = 'J';
  obj._ticketed = 'K';
  obj._standby = 'L';
  obj._confirmedSector = 'M';
  obj._eachSector = 'O';
  obj._advTktPeriod = "PQR";
  obj._advTktUnit = "ST";
  obj._advTktOpt = 'U';
  obj._advTktDepartUnit = 'V';
  obj._advTktBoth = 'W';
  obj._advTktExcpUnit = 'X';
  obj._waiverResDate = time(nullptr);
  obj._waiverTktDate = time(nullptr);
  obj._inhibit = 'Y';
}

void
AdvResTktInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _unavailtag);
  FLATTENIZE(archive, _advTktDepart);
  FLATTENIZE(archive, _geoTblItemNo);
  FLATTENIZE(archive, _firstResTod);
  FLATTENIZE(archive, _lastResTod);
  FLATTENIZE(archive, _advTktTod);
  FLATTENIZE(archive, _advTktExcpTime);
  FLATTENIZE(archive, _tktTSI);
  FLATTENIZE(archive, _resTSI);
  FLATTENIZE(archive, _firstResPeriod);
  FLATTENIZE(archive, _firstResUnit);
  FLATTENIZE(archive, _lastResPeriod);
  FLATTENIZE(archive, _lastResUnit);
  FLATTENIZE(archive, _permitted);
  FLATTENIZE(archive, _ticketed);
  FLATTENIZE(archive, _standby);
  FLATTENIZE(archive, _confirmedSector);
  FLATTENIZE(archive, _eachSector);
  FLATTENIZE(archive, _advTktPeriod);
  FLATTENIZE(archive, _advTktUnit);
  FLATTENIZE(archive, _advTktOpt);
  FLATTENIZE(archive, _advTktDepartUnit);
  FLATTENIZE(archive, _advTktBoth);
  FLATTENIZE(archive, _advTktExcpUnit);
  FLATTENIZE(archive, _waiverResDate);
  FLATTENIZE(archive, _waiverTktDate);
  FLATTENIZE(archive, _inhibit);
}

} // tse
