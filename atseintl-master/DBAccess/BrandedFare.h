#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/BrandedFareSeg.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{
typedef std::vector<BrandedFareSeg*> BrandedFareSegments;

class BrandedFare // S8
{
public:
  BrandedFare()
    : _source(' '),
      _seqNo(-1),
      _validityInd('Y'),
      _publicPrivateInd(' '),
      _tvlFirstYear(0),
      _tvlFirstMonth(0),
      _tvlFirstDay(0),
      _tvlLastYear(0),
      _tvlLastMonth(0),
      _tvlLastDay(0),
      _svcFeesAccountCodeTblItemNo(-1),
      _svcFeesSecurityTblItemNo(-1),
      _directionality(' '),
      _globalInd(GlobalDirection::NO_DIR),
      _oneMatrix(' '),
      _svcFeesFeatureTblItemNo(-1),
      _taxTextTblItemNo(-1),
      _segCount(0)
  {
  }
  ~BrandedFare()
  {
    BrandedFareSegments::const_iterator it(_segments.begin()), itend(_segments.end());
    for (; it != itend; ++it)
    {
      delete *it;
    }
  }
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }
  Indicator& source() { return _source; }
  Indicator source() const { return _source; }
  SequenceNumberLong& seqNo() { return _seqNo; }
  SequenceNumberLong seqNo() const { return _seqNo; }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }
  Indicator& validityInd() { return _validityInd; }
  Indicator validityInd() const { return _validityInd; }
  Indicator& publicPrivateInd() { return _publicPrivateInd; }
  Indicator publicPrivateInd() const { return _publicPrivateInd; }
  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }
  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }
  int& tvlFirstYear() { return _tvlFirstYear; }
  int tvlFirstYear() const { return _tvlFirstYear; }
  int& tvlFirstMonth() { return _tvlFirstMonth; }
  int tvlFirstMonth() const { return _tvlFirstMonth; }
  int& tvlFirstDay() { return _tvlFirstDay; }
  int tvlFirstDay() const { return _tvlFirstDay; }
  int& tvlLastYear() { return _tvlLastYear; }
  int tvlLastYear() const { return _tvlLastYear; }
  int& tvlLastMonth() { return _tvlLastMonth; }
  int tvlLastMonth() const { return _tvlLastMonth; }
  int& tvlLastDay() { return _tvlLastDay; }
  int tvlLastDay() const { return _tvlLastDay; }
  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }
  long& svcFeesAccountCodeTblItemNo() { return _svcFeesAccountCodeTblItemNo; }
  long svcFeesAccountCodeTblItemNo() const { return _svcFeesAccountCodeTblItemNo; }
  long& svcFeesSecurityTblItemNo() { return _svcFeesSecurityTblItemNo; }
  long svcFeesSecurityTblItemNo() const { return _svcFeesSecurityTblItemNo; }
  Indicator& directionality() { return _directionality; }
  Indicator directionality() const { return _directionality; }
  LocKey& locKey1() { return _locKey1; }
  const LocKey& locKey1() const { return _locKey1; }
  Zone& loc1ZoneTblItemNo() { return _loc1ZoneTblItemNo; }
  const Zone& loc1ZoneTblItemNo() const { return _loc1ZoneTblItemNo; }
  LocKey& locKey2() { return _locKey2; }
  const LocKey& locKey2() const { return _locKey2; }
  Zone& loc2ZoneTblItemNo() { return _loc2ZoneTblItemNo; }
  const Zone& loc2ZoneTblItemNo() const { return _loc2ZoneTblItemNo; }
  GlobalDirection& globalInd() { return _globalInd; }
  GlobalDirection globalInd() const { return _globalInd; }
  Indicator& oneMatrix() { return _oneMatrix; }
  Indicator oneMatrix() const { return _oneMatrix; }
  std::string& programCode() { return _programCode; }
  const std::string& programCode() const { return _programCode; }
  std::string& programText() { return _programText; }
  const std::string& programText() const { return _programText; }
  long& svcFeesFeatureTblItemNo() { return _svcFeesFeatureTblItemNo; }
  long svcFeesFeatureTblItemNo() const { return _svcFeesFeatureTblItemNo; }
  long& taxTextTblItemNo() { return _taxTextTblItemNo; }
  long taxTextTblItemNo() const { return _taxTextTblItemNo; }
  int& segCount() { return _segCount; }
  int segCount() const { return _segCount; }
  BrandedFareSegments& segments() { return _segments; }
  const BrandedFareSegments& segments() const { return _segments; }

  bool operator==(const BrandedFare& rhs) const
  {
    bool eq(_vendor == rhs._vendor && _carrier == rhs._carrier && _source == rhs._source &&
            _seqNo == rhs._seqNo && _createDate == rhs._createDate &&
            _expireDate == rhs._expireDate && _validityInd == rhs._validityInd &&
            _publicPrivateInd == rhs._publicPrivateInd && _effDate == rhs._effDate &&
            _discDate == rhs._discDate && _tvlFirstYear == rhs._tvlFirstYear &&
            _tvlFirstMonth == rhs._tvlFirstMonth && _tvlFirstDay == rhs._tvlFirstDay &&
            _tvlLastYear == rhs._tvlLastYear && _tvlLastMonth == rhs._tvlLastMonth &&
            _tvlLastDay == rhs._tvlLastDay && _psgType == rhs._psgType &&
            _svcFeesAccountCodeTblItemNo == rhs._svcFeesAccountCodeTblItemNo &&
            _svcFeesSecurityTblItemNo == rhs._svcFeesSecurityTblItemNo &&
            _directionality == rhs._directionality && _locKey1 == rhs._locKey1 &&
            _loc1ZoneTblItemNo == rhs._loc1ZoneTblItemNo && _locKey2 == rhs._locKey2 &&
            _loc2ZoneTblItemNo == rhs._loc2ZoneTblItemNo && _globalInd == rhs._globalInd &&
            _oneMatrix == rhs._oneMatrix && _programCode == rhs._programCode &&
            _programText == rhs._programText &&
            _svcFeesFeatureTblItemNo == rhs._svcFeesFeatureTblItemNo &&
            _taxTextTblItemNo == rhs._taxTextTblItemNo && _segCount == rhs._segCount);
    eq = eq && _segments.size() == rhs._segments.size();
    for (size_t i = 0; i < _segments.size() && eq; ++i)
    {
      eq = *_segments[i] == *rhs._segments[i];
    }
    return eq;
  }

  static void dummyData(BrandedFare& obj)
  {
    obj._vendor = "ATP";
    obj._carrier = "AA";
    obj._source = 'S';
    obj._seqNo = 1111111111;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'Y';
    obj._publicPrivateInd = ' ';
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._tvlFirstYear = 1;
    obj._tvlFirstMonth = 2;
    obj._tvlFirstDay = 3;
    obj._tvlLastYear = 4;
    obj._tvlLastMonth = 5;
    obj._tvlLastDay = 6;
    obj._psgType = "ADT";
    obj._svcFeesAccountCodeTblItemNo = 22222;
    obj._svcFeesSecurityTblItemNo = 33333;
    obj._directionality = '3';
    obj._locKey1.locType() = 'C';
    obj._locKey1.loc() = "DFW";
    obj._loc1ZoneTblItemNo = "0000000";
    obj._locKey2.locType() = 'B';
    obj._locKey2.loc() = "NYC";
    obj._loc2ZoneTblItemNo = "0000000";
    obj._globalInd = GlobalDirection::ZZ;
    obj._oneMatrix = 'X';
    obj._programCode = "FFA";
    obj._programText = "NORMAL DOMESTIC";
    obj._svcFeesFeatureTblItemNo = 66666;
    obj._taxTextTblItemNo = 77777;
    obj._segCount = 1;
    BrandedFareSeg* seg(new BrandedFareSeg);
    BrandedFareSeg::dummyData(*seg);
    obj._segments.push_back(seg);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _source);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _publicPrivateInd);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _tvlFirstYear);
    FLATTENIZE(archive, _tvlFirstMonth);
    FLATTENIZE(archive, _tvlFirstDay);
    FLATTENIZE(archive, _tvlLastYear);
    FLATTENIZE(archive, _tvlLastMonth);
    FLATTENIZE(archive, _tvlLastDay);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _svcFeesAccountCodeTblItemNo);
    FLATTENIZE(archive, _svcFeesSecurityTblItemNo);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _locKey1);
    FLATTENIZE(archive, _loc1ZoneTblItemNo);
    FLATTENIZE(archive, _locKey2);
    FLATTENIZE(archive, _loc2ZoneTblItemNo);
    FLATTENIZE(archive, _globalInd);
    FLATTENIZE(archive, _oneMatrix);
    FLATTENIZE(archive, _programCode);
    FLATTENIZE(archive, _programText);
    FLATTENIZE(archive, _svcFeesFeatureTblItemNo);
    FLATTENIZE(archive, _taxTextTblItemNo);
    FLATTENIZE(archive, _segCount);
    FLATTENIZE(archive, _segments);
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  Indicator _source;
  SequenceNumberLong _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _validityInd; // 'Y' | 'N'
  Indicator _publicPrivateInd; // ' ' | 'P'
  DateTime _effDate;
  DateTime _discDate;
  int _tvlFirstYear;
  int _tvlFirstMonth;
  int _tvlFirstDay;
  int _tvlLastYear;
  int _tvlLastMonth;
  int _tvlLastDay;
  PaxTypeCode _psgType;
  long _svcFeesAccountCodeTblItemNo;
  long _svcFeesSecurityTblItemNo;
  Indicator _directionality; // '3'|'4'| ' '
  LocKey _locKey1;
  Zone _loc1ZoneTblItemNo;
  LocKey _locKey2;
  Zone _loc2ZoneTblItemNo;
  GlobalDirection _globalInd;
  Indicator _oneMatrix; //'X'|' ':fare component can be combined with other brands,X - can be
                        //combined
  std::string _programCode;
  std::string _programText;
  long _svcFeesFeatureTblItemNo;
  long _taxTextTblItemNo;
  int _segCount;
  BrandedFareSegments _segments;
};
} // tse

