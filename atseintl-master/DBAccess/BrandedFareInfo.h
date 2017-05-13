#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{
class BrandedFareSeg;
typedef std::vector<BrandedFareSeg*> BrandedFareSegments;

class BrandedFareInfo // S8
{
public:
  BrandedFareInfo()
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
      _segCount(0)
  {
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
  LocKey& locKey2() { return _locKey2; }
  const LocKey& locKey2() const { return _locKey2; }
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
  int& segCount() { return _segCount; }
  int segCount() const { return _segCount; }
  BrandedFareSegments& segments() { return _segments; }
  const BrandedFareSegments& segments() const { return _segments; }

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
  Indicator _directionality; // '3'|'4'|' '
  LocKey _locKey1;
  LocKey _locKey2;
  GlobalDirection _globalInd;
  Indicator _oneMatrix; //'X'|' ':fare component can be combined with other brands,X - can be
                        //combined
  std::string _programCode;
  std::string _programText;
  long _svcFeesFeatureTblItemNo;
  int _segCount;
  BrandedFareSegments _segments;
};
} // tse

