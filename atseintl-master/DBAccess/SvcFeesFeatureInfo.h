#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class SvcFeesFeatureInfo // T166
{
public:
  SvcFeesFeatureInfo()
    : _itemNo(-1),
      _seqNo(-1),
      _validityInd('Y'),
      _fltTktMerchInd(' '),
      _segmentAppl1(' '),
      _segmentAppl2(' '),
      _segmentAppl3(' '),
      _segmentAppl4(' '),
      _segmentAppl5(' '),
      _segmentAppl6(' '),
      _segmentAppl7(' '),
      _segmentAppl8(' '),
      _segmentAppl9(' '),
      _segmentAppl10(' ')
  {
  }
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }
  long long& itemNo() { return _itemNo; }
  long long itemNo() const { return _itemNo; }
  SequenceNumber& seqNo() { return _seqNo; }
  SequenceNumber seqNo() const { return _seqNo; }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }
  Indicator& validityInd() { return _validityInd; }
  Indicator validityInd() const { return _validityInd; }
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }
  ServiceTypeCode& serviceTypeCode() { return _serviceTypeCode; }
  const ServiceTypeCode& serviceTypeCode() const { return _serviceTypeCode; }
  ServiceSubTypeCode& serviceSubTypeCode() { return _serviceSubTypeCode; }
  const ServiceSubTypeCode& serviceSubTypeCode() const { return _serviceSubTypeCode; }
  Indicator& fltTktMerchInd() { return _fltTktMerchInd; }
  Indicator fltTktMerchInd() const { return _fltTktMerchInd; }
  Indicator& segmentAppl1() { return _segmentAppl1; }
  Indicator segmentAppl1() const { return _segmentAppl1; }
  Indicator& segmentAppl2() { return _segmentAppl2; }
  Indicator segmentAppl2() const { return _segmentAppl2; }
  Indicator& segmentAppl3() { return _segmentAppl3; }
  Indicator segmentAppl3() const { return _segmentAppl3; }
  Indicator& segmentAppl4() { return _segmentAppl4; }
  Indicator segmentAppl4() const { return _segmentAppl4; }
  Indicator& segmentAppl5() { return _segmentAppl5; }
  Indicator segmentAppl5() const { return _segmentAppl5; }
  Indicator& segmentAppl6() { return _segmentAppl6; }
  Indicator segmentAppl6() const { return _segmentAppl6; }
  Indicator& segmentAppl7() { return _segmentAppl7; }
  Indicator segmentAppl7() const { return _segmentAppl7; }
  Indicator& segmentAppl8() { return _segmentAppl8; }
  Indicator segmentAppl8() const { return _segmentAppl8; }
  Indicator& segmentAppl9() { return _segmentAppl9; }
  Indicator segmentAppl9() const { return _segmentAppl9; }
  Indicator& segmentAppl10() { return _segmentAppl10; }
  Indicator segmentAppl10() const { return _segmentAppl10; }

  bool operator==(const SvcFeesFeatureInfo& rhs) const
  {
    return _vendor == rhs._vendor && _itemNo == rhs._itemNo && _seqNo == rhs._seqNo &&
           _createDate == rhs._createDate && _expireDate == rhs._expireDate &&
           _validityInd == rhs._validityInd && _carrier == rhs._carrier &&
           _serviceTypeCode == rhs._serviceTypeCode &&
           _serviceSubTypeCode == rhs._serviceSubTypeCode &&
           _fltTktMerchInd == rhs._fltTktMerchInd && _segmentAppl1 == rhs._segmentAppl1 &&
           _segmentAppl2 == rhs._segmentAppl2 && _segmentAppl3 == rhs._segmentAppl3 &&
           _segmentAppl4 == rhs._segmentAppl4 && _segmentAppl5 == rhs._segmentAppl5 &&
           _segmentAppl6 == rhs._segmentAppl6 && _segmentAppl7 == rhs._segmentAppl7 &&
           _segmentAppl8 == rhs._segmentAppl8 && _segmentAppl9 == rhs._segmentAppl9 &&
           _segmentAppl10 == rhs._segmentAppl10;
  }

  static void dummyData(SvcFeesFeatureInfo& obj)
  {
    obj._vendor = "ATP";
    obj._itemNo = 977;
    obj._seqNo = 22222;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'Y';
    obj._carrier = "AA";
    obj._serviceTypeCode = "AB";
    obj._serviceSubTypeCode = "CDE";
    obj._fltTktMerchInd = FLIGHT_RELATED_SERVICE;
    obj._segmentAppl1 = 'G';
    obj._segmentAppl2 = 'H';
    obj._segmentAppl3 = 'I';
    obj._segmentAppl4 = 'J';
    obj._segmentAppl5 = 'K';
    obj._segmentAppl6 = 'L';
    obj._segmentAppl7 = 'M';
    obj._segmentAppl8 = 'N';
    obj._segmentAppl9 = 'O';
    obj._segmentAppl10 = 'p';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _serviceTypeCode);
    FLATTENIZE(archive, _serviceSubTypeCode);
    FLATTENIZE(archive, _fltTktMerchInd);
    FLATTENIZE(archive, _segmentAppl1);
    FLATTENIZE(archive, _segmentAppl2);
    FLATTENIZE(archive, _segmentAppl3);
    FLATTENIZE(archive, _segmentAppl4);
    FLATTENIZE(archive, _segmentAppl5);
    FLATTENIZE(archive, _segmentAppl6);
    FLATTENIZE(archive, _segmentAppl7);
    FLATTENIZE(archive, _segmentAppl8);
    FLATTENIZE(archive, _segmentAppl9);
    FLATTENIZE(archive, _segmentAppl10);
  }

private:
  VendorCode _vendor;
  long long _itemNo;
  SequenceNumber _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _validityInd; // 'Y' | 'N'
  CarrierCode _carrier;
  ServiceTypeCode _serviceTypeCode;
  ServiceSubTypeCode _serviceSubTypeCode;
  Indicator _fltTktMerchInd;
  Indicator _segmentAppl1;
  Indicator _segmentAppl2;
  Indicator _segmentAppl3;
  Indicator _segmentAppl4;
  Indicator _segmentAppl5;
  Indicator _segmentAppl6;
  Indicator _segmentAppl7;
  Indicator _segmentAppl8;
  Indicator _segmentAppl9;
  Indicator _segmentAppl10;
};
} // tse

