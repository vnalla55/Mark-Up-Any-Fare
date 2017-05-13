//----------------------------------------------------------------------------
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class AccompaniedTravelInfo : public RuleItemInfo
{
public:
  AccompaniedTravelInfo()
    : _unavailTag(' '),
      _segCnt(0),
      _geoTblItemNoVia1(0),
      _geoTblItemNoVia2(0),
      _minAge(0),
      _maxAge(0),
      _minNoPsg(0),
      _maxNoPsg(0),
      _accTvlAllSectors(' '),
      _accTvlOut(' '),
      _accTvlOneSector(' '),
      _accTvlSameCpmt(' '),
      _accTvlSameRule(' '),
      _accPsgAppl(' '),
      _accPsgId(' '),
      _fareClassBkgCdInd(' '),
      _inhibit(' ')
  {
  }

  virtual ~AccompaniedTravelInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  int& geoTblItemNoVia1() { return _geoTblItemNoVia1; }
  const int& geoTblItemNoVia1() const { return _geoTblItemNoVia1; }

  int& geoTblItemNoVia2() { return _geoTblItemNoVia2; }
  const int& geoTblItemNoVia2() const { return _geoTblItemNoVia2; }

  int& minAge() { return _minAge; }
  const int& minAge() const { return _minAge; }

  int& maxAge() { return _maxAge; }
  const int& maxAge() const { return _maxAge; }

  int& minNoPsg() { return _minNoPsg; }
  const int& minNoPsg() const { return _minNoPsg; }

  int& maxNoPsg() { return _maxNoPsg; }
  const int& maxNoPsg() const { return _maxNoPsg; }

  Indicator& accTvlAllSectors() { return _accTvlAllSectors; }
  const Indicator& accTvlAllSectors() const { return _accTvlAllSectors; }

  Indicator& accTvlOut() { return _accTvlOut; }
  const Indicator& accTvlOut() const { return _accTvlOut; }

  Indicator& accTvlOneSector() { return _accTvlOneSector; }
  const Indicator& accTvlOneSector() const { return _accTvlOneSector; }

  Indicator& accTvlSameCpmt() { return _accTvlSameCpmt; }
  const Indicator& accTvlSameCpmt() const { return _accTvlSameCpmt; }

  Indicator& accTvlSameRule() { return _accTvlSameRule; }
  const Indicator& accTvlSameRule() const { return _accTvlSameRule; }

  Indicator& accPsgAppl() { return _accPsgAppl; }
  const Indicator& accPsgAppl() const { return _accPsgAppl; }

  PaxTypeCode& accPsgType() { return _accPsgType; }
  const PaxTypeCode& accPsgType() const { return _accPsgType; }

  Indicator& accPsgId() { return _accPsgId; }
  const Indicator& accPsgId() const { return _accPsgId; }

  Indicator& fareClassBkgCdInd() { return _fareClassBkgCdInd; }
  const Indicator& fareClassBkgCdInd() const { return _fareClassBkgCdInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<FareClassCode>& fareClassBkgCds() { return _fareClassBkgCds; }
  const std::vector<FareClassCode>& fareClassBkgCds() const { return _fareClassBkgCds; }

  virtual bool operator==(const AccompaniedTravelInfo& rhs) const;

  static void dummyData(AccompaniedTravelInfo& obj);

  virtual void flattenize(Flattenizable::Archive& archive) override;

private:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailTag;
  int _segCnt;
  int _geoTblItemNoVia1;
  int _geoTblItemNoVia2;
  int _minAge;
  int _maxAge;
  int _minNoPsg;
  int _maxNoPsg;
  Indicator _accTvlAllSectors;
  Indicator _accTvlOut;
  Indicator _accTvlOneSector;
  Indicator _accTvlSameCpmt;
  Indicator _accTvlSameRule;
  Indicator _accPsgAppl;
  PaxTypeCode _accPsgType;
  Indicator _accPsgId;
  Indicator _fareClassBkgCdInd;
  Indicator _inhibit;
  std::vector<FareClassCode> _fareClassBkgCds;

};

} // tse

