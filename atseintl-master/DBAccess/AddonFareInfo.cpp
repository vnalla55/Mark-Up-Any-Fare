//----------------------------------------------------------------------------
// 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#include "DBAccess/AddonFareInfo.h"

#include "DBAccess/DataHandle.h"

namespace tse
{

AddonFareInfo*
AddonFareInfo::clone(DataHandle& dataHandle) const
{
  // lint --e{413}
  AddonFareInfo* cloneObj = nullptr;

  dataHandle.get(cloneObj);

  clone(*cloneObj);

  return cloneObj;
}

void
AddonFareInfo::clone(AddonFareInfo& cloneObj) const
{
  _effInterval.cloneDateInterval(cloneObj._effInterval);

  cloneObj._gatewayMarket = _gatewayMarket;
  cloneObj._interiorMarket = _interiorMarket;
  cloneObj._carrier = _carrier;
  cloneObj._fareClass = _fareClass;
  cloneObj._addonTariff = _addonTariff;
  cloneObj._vendor = _vendor;
  cloneObj._cur = _cur;
  cloneObj._lastModDate = _lastModDate;
  cloneObj._noDec = _noDec;
  cloneObj._fareAmt = _fareAmt;
  cloneObj._hashKey = _hashKey;
  cloneObj._footNote1 = _footNote1;
  cloneObj._footNote2 = _footNote2;
  cloneObj._owrt = _owrt;
  cloneObj._routing = _routing;
  cloneObj._directionality = _directionality;
  cloneObj._arbZone = _arbZone;
  cloneObj._inhibit = _inhibit;
  cloneObj._classFareBasisInd = _classFareBasisInd;
  cloneObj._globalDir = _globalDir;
  cloneObj._routeCode = _routeCode;
  cloneObj._linkNo = _linkNo;
  cloneObj._seqNo = _seqNo;
}

bool
AddonFareInfo::
operator==(const AddonFareInfo& rhs) const
{
  return ((_effInterval == rhs._effInterval) && (_gatewayMarket == rhs._gatewayMarket) &&
          (_interiorMarket == rhs._interiorMarket) && (_carrier == rhs._carrier) &&
          (_fareClass == rhs._fareClass) && (_addonTariff == rhs._addonTariff) &&
          (_vendor == rhs._vendor) && (_cur == rhs._cur) && (_lastModDate == rhs._lastModDate) &&
          (_noDec == rhs._noDec) && (_fareAmt == rhs._fareAmt) && (_hashKey == rhs._hashKey) &&
          (_footNote1 == rhs._footNote1) && (_footNote2 == rhs._footNote2) &&
          (_owrt == rhs._owrt) && (_routing == rhs._routing) &&
          (_directionality == rhs._directionality) && (_arbZone == rhs._arbZone) &&
          (_inhibit == rhs._inhibit) && (_classFareBasisInd == rhs._classFareBasisInd) &&
          (_globalDir == rhs._globalDir) && (_routeCode == rhs._routeCode) &&
          (_linkNo == rhs._linkNo) && (_seqNo == rhs._seqNo));
}

void
AddonFareInfo::dummyData(AddonFareInfo& obj)
{
  TSEDateInterval::dummyData(obj._effInterval);

  obj._gatewayMarket = "ABCDEFGH";
  obj._interiorMarket = "IJKLMNOP";
  obj._carrier = "QRS";
  obj._fareClass = "TUVWXYZa";
  obj._addonTariff = 1;
  obj._vendor = "bcde";
  obj._cur = "fgh";
  obj._lastModDate = time(nullptr);
  obj._noDec = 2;
  obj._fareAmt = 3.33;
  obj._hashKey = "aaaaaaaa";
  obj._footNote1 = "ij";
  obj._footNote2 = "kl";
  obj._owrt = 'm';
  obj._routing = "nopq";
  obj._directionality = 'r';
  obj._arbZone = 4;
  obj._inhibit = 's';
  obj._classFareBasisInd = 't';
  obj._globalDir = GlobalDirection::US;
  obj._routeCode = "uv";
  obj._linkNo = 5;
  obj._seqNo = 6;
}

void
AddonFareInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _effInterval);
  FLATTENIZE(archive, _gatewayMarket);
  FLATTENIZE(archive, _interiorMarket);
  FLATTENIZE(archive, _carrier);
  FLATTENIZE(archive, _fareClass);
  FLATTENIZE(archive, _addonTariff);
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _cur);
  FLATTENIZE(archive, _lastModDate);
  FLATTENIZE(archive, _noDec);
  FLATTENIZE(archive, _fareAmt);
  FLATTENIZE(archive, _hashKey);
  FLATTENIZE(archive, _footNote1);
  FLATTENIZE(archive, _footNote2);
  FLATTENIZE(archive, _owrt);
  FLATTENIZE(archive, _routing);
  FLATTENIZE(archive, _directionality);
  FLATTENIZE(archive, _arbZone);
  FLATTENIZE(archive, _inhibit);
  FLATTENIZE(archive, _classFareBasisInd);
  FLATTENIZE(archive, _globalDir);
  FLATTENIZE(archive, _routeCode);
  FLATTENIZE(archive, _linkNo);
  FLATTENIZE(archive, _seqNo);
}

} // tse
