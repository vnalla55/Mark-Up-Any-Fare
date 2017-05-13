//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AccompaniedTravelInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

bool
AccompaniedTravelInfo::
operator==(const AccompaniedTravelInfo& rhs) const
{
  return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
          (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
          (_segCnt == rhs._segCnt) && (_geoTblItemNoVia1 == rhs._geoTblItemNoVia1) &&
          (_geoTblItemNoVia2 == rhs._geoTblItemNoVia2) && (_minAge == rhs._minAge) &&
          (_maxAge == rhs._maxAge) && (_minNoPsg == rhs._minNoPsg) &&
          (_maxNoPsg == rhs._maxNoPsg) && (_accTvlAllSectors == rhs._accTvlAllSectors) &&
          (_accTvlOut == rhs._accTvlOut) && (_accTvlOneSector == rhs._accTvlOneSector) &&
          (_accTvlSameCpmt == rhs._accTvlSameCpmt) && (_accTvlSameRule == rhs._accTvlSameRule) &&
          (_accPsgAppl == rhs._accPsgAppl) && (_accPsgType == rhs._accPsgType) &&
          (_accPsgId == rhs._accPsgId) && (_fareClassBkgCdInd == rhs._fareClassBkgCdInd) &&
          (_inhibit == rhs._inhibit) && (_fareClassBkgCds == rhs._fareClassBkgCds));
}

void
AccompaniedTravelInfo::dummyData(AccompaniedTravelInfo& obj)
{
  RuleItemInfo::dummyData(obj);

  obj._createDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._unavailTag = 'A';
  obj._segCnt = 1;
  obj._geoTblItemNoVia1 = 2;
  obj._geoTblItemNoVia2 = 3;
  obj._minAge = 4;
  obj._maxAge = 5;
  obj._minNoPsg = 6;
  obj._maxNoPsg = 7;
  obj._accTvlAllSectors = 'B';
  obj._accTvlOut = 'C';
  obj._accTvlOneSector = 'D';
  obj._accTvlSameCpmt = 'E';
  obj._accTvlSameRule = 'F';
  obj._accPsgAppl = 'G';
  obj._accPsgType = "HIJ";
  obj._accPsgId = 'K';
  obj._fareClassBkgCdInd = 'L';
  obj._inhibit = 'M';
  obj._fareClassBkgCds.push_back("aaaaaaaa");
  obj._fareClassBkgCds.push_back("bbbbbbbb");
}

void
AccompaniedTravelInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _unavailTag);
  FLATTENIZE(archive, _segCnt);
  FLATTENIZE(archive, _geoTblItemNoVia1);
  FLATTENIZE(archive, _geoTblItemNoVia2);
  FLATTENIZE(archive, _minAge);
  FLATTENIZE(archive, _maxAge);
  FLATTENIZE(archive, _minNoPsg);
  FLATTENIZE(archive, _maxNoPsg);
  FLATTENIZE(archive, _accTvlAllSectors);
  FLATTENIZE(archive, _accTvlOut);
  FLATTENIZE(archive, _accTvlOneSector);
  FLATTENIZE(archive, _accTvlSameCpmt);
  FLATTENIZE(archive, _accTvlSameRule);
  FLATTENIZE(archive, _accPsgAppl);
  FLATTENIZE(archive, _accPsgType);
  FLATTENIZE(archive, _accPsgId);
  FLATTENIZE(archive, _fareClassBkgCdInd);
  FLATTENIZE(archive, _inhibit);
  FLATTENIZE(archive, _fareClassBkgCds);
}

} // tse
