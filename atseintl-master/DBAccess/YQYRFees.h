// ----------------------------------------------------------------------------
// 2004, Sabre Inc.  All rights reserved.  This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use,
// reproduction, or transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system or publication,
// is strictly prohibited
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class YQYRFees
{
public:
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  Indicator& subCode() { return _subCode; }
  const Indicator& subCode() const { return _subCode; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& modDate() { return _modDate; }
  const DateTime& modDate() const { return _modDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& firstTktDate() { return _firstTktDate; }
  const DateTime& firstTktDate() const { return _firstTktDate; }

  DateTime& lastTktDate() { return _lastTktDate; }
  const DateTime& lastTktDate() const { return _lastTktDate; }

  Indicator& returnToOrigin() { return _returnToOrigin; }
  const Indicator& returnToOrigin() const { return _returnToOrigin; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  LocType& posLocType() { return _posLocType; }
  const LocType& posLocType() const { return _posLocType; }

  LocCode& posLoc() { return _posLoc; }
  const LocCode& posLoc() const { return _posLoc; }

  Indicator& posLocaleType() { return _posLocaleType; }
  const Indicator& posLocaleType() const { return _posLocaleType; }

  AgencyCode& posAgencyPCC() { return _posAgencyPCC; }
  const AgencyCode& posAgencyPCC() const { return _posAgencyPCC; }

  AgencyIATA& posIataTvlAgencyNo() { return _posIataTvlAgencyNo; }
  const AgencyIATA& posIataTvlAgencyNo() const { return _posIataTvlAgencyNo; }

  LocType& potLocType() { return _potLocType; }
  const LocType& potLocType() const { return _potLocType; }

  LocCode& potLoc() { return _potLoc; }
  const LocCode& potLoc() const { return _potLoc; }

  Indicator& journeyLoc1Ind() { return _journeyLoc1Ind; }
  const Indicator& journeyLoc1Ind() const { return _journeyLoc1Ind; }

  LocTypeCode& journeyLocType1() { return _journeyLocType1; }
  const LocTypeCode& journeyLocType1() const { return _journeyLocType1; }

  LocCode& journeyLoc1() { return _journeyLoc1; }
  const LocCode& journeyLoc1() const { return _journeyLoc1; }

  LocTypeCode& journeyLocType2() { return _journeyLocType2; }
  const LocTypeCode& journeyLocType2() const { return _journeyLocType2; }

  LocCode& journeyLoc2() { return _journeyLoc2; }
  const LocCode& journeyLoc2() const { return _journeyLoc2; }

  LocTypeCode& viaLocType() { return _viaLocType; }
  const LocTypeCode& viaLocType() const { return _viaLocType; }

  LocCode& viaLoc() { return _viaLoc; }
  const LocCode& viaLoc() const { return _viaLoc; }

  LocTypeCode& whollyWithinLocType() { return _whollyWithinLocType; }
  const LocTypeCode& whollyWithinLocType() const { return _whollyWithinLocType; }

  LocCode& whollyWithinLoc() { return _whollyWithinLoc; }
  const LocCode& whollyWithinLoc() const { return _whollyWithinLoc; }

  Indicator& sectorPortionOfTvlInd() { return _sectorPortionOfTvlInd; }
  const Indicator& sectorPortionOfTvlInd() const { return _sectorPortionOfTvlInd; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  LocTypeCode& sectorPortionLocType1() { return _sectorPortionLocType1; }
  const LocTypeCode& sectorPortionLocType1() const { return _sectorPortionLocType1; }

  LocCode& sectorPortionLoc1() { return _sectorPortionLoc1; }
  const LocCode& sectorPortionLoc1() const { return _sectorPortionLoc1; }

  LocTypeCode& sectorPortionLocType2() { return _sectorPortionLocType2; }
  const LocTypeCode& sectorPortionLocType2() const { return _sectorPortionLocType2; }

  LocCode& sectorPortionLoc2() { return _sectorPortionLoc2; }
  const LocCode& sectorPortionLoc2() const { return _sectorPortionLoc2; }

  LocTypeCode& sectorPortionViaLocType() { return _sectorPortionViaLocType; }
  const LocTypeCode& sectorPortionViaLocType() const { return _sectorPortionViaLocType; }

  LocCode& sectorPortionViaLoc() { return _sectorPortionViaLoc; }
  const LocCode& sectorPortionViaLoc() const { return _sectorPortionViaLoc; }

  Indicator& stopConnectInd() { return _stopConnectInd; }
  const Indicator& stopConnectInd() const { return _stopConnectInd; }

  int& stopConnectTime() { return _stopConnectTime; }
  const int& stopConnectTime() const { return _stopConnectTime; }

  Indicator& stopConnectUnit() { return _stopConnectUnit; }
  const Indicator& stopConnectUnit() const { return _stopConnectUnit; }

  Indicator& connectExemptInd() { return _connectExemptInd; }
  const Indicator& connectExemptInd() const { return _connectExemptInd; }

  Indicator& intlDomInd() { return _intlDomInd; }
  const Indicator& intlDomInd() const { return _intlDomInd; }

  BookingCode& bookingCode1() { return _bookingCode1; }
  const BookingCode& bookingCode1() const { return _bookingCode1; }

  BookingCode& bookingCode2() { return _bookingCode2; }
  const BookingCode& bookingCode2() const { return _bookingCode2; }

  BookingCode& bookingCode3() { return _bookingCode3; }
  const BookingCode& bookingCode3() const { return _bookingCode3; }

  EquipmentType& equipCode() { return _equipCode; }
  const EquipmentType& equipCode() const { return _equipCode; }

  std::string& fareBasis() { return _fareBasis; }
  const std::string& fareBasis() const { return _fareBasis; }

  bool& hasSlashInFBC() { return _hasSlashInFBC; }
  bool hasSlashInFBC() const { return _hasSlashInFBC; }

  int& batchNo() { return _batchNo; }
  const int& batchNo() const { return _batchNo; }

  std::string& batchCi() { return _batchCi; }
  const std::string& batchCi() const { return _batchCi; }

  MoneyAmount& feeAmount() { return _feeAmount; }
  const MoneyAmount& feeAmount() const { return _feeAmount; }

  CurrencyCode& cur() { return _cur; }
  const CurrencyCode& cur() const { return _cur; }

  CurrencyNoDec& noDec() { return _noDec; }
  const CurrencyNoDec& noDec() const { return _noDec; }

  Percent& percent() { return _percent; }
  const Percent& percent() const { return _percent; }

  int& percentNoDec() { return _percentNoDec; }
  const int& percentNoDec() const { return _percentNoDec; }

  Indicator& taxIncludedInd() { return _taxIncludedInd; }
  const Indicator& taxIncludedInd() const { return _taxIncludedInd; }

  Indicator& feeApplInd() { return _feeApplInd; }
  const Indicator& feeApplInd() const { return _feeApplInd; }

  int& taxCarrierApplTblItemNo() { return _taxCarrierApplTblItemNo; }
  const int& taxCarrierApplTblItemNo() const { return _taxCarrierApplTblItemNo; }

  int& taxCarrierFltTblItemNo() { return _taxCarrierFltTblItemNo; }
  const int& taxCarrierFltTblItemNo() const { return _taxCarrierFltTblItemNo; }

  int& taxTextTblItemNo() { return _taxTextTblItemNo; }
  const int& taxTextTblItemNo() const { return _taxTextTblItemNo; }

  bool operator==(const YQYRFees& rhs) const
  {
    return &rhs == this
               ? true
               : ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
                  (_taxCode == rhs._taxCode) && (_subCode == rhs._subCode) &&
                  (_seqNo == rhs._seqNo) && (_modDate == rhs._modDate) &&
                  (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
                  (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
                  (_firstTktDate == rhs._firstTktDate) && (_lastTktDate == rhs._lastTktDate) &&
                  (_returnToOrigin == rhs._returnToOrigin) && (_psgType == rhs._psgType) &&
                  (_posLocType == rhs._posLocType) && (_posLoc == rhs._posLoc) &&
                  (_posLocaleType == rhs._posLocaleType) && (_posAgencyPCC == rhs._posAgencyPCC) &&
                  (_posIataTvlAgencyNo == rhs._posIataTvlAgencyNo) &&
                  (_potLocType == rhs._potLocType) && (_potLoc == rhs._potLoc) &&
                  (_journeyLoc1Ind == rhs._journeyLoc1Ind) &&
                  (_journeyLocType1 == rhs._journeyLocType1) &&
                  (_journeyLoc1 == rhs._journeyLoc1) &&
                  (_journeyLocType2 == rhs._journeyLocType2) &&
                  (_journeyLoc2 == rhs._journeyLoc2) && (_viaLocType == rhs._viaLocType) &&
                  (_viaLoc == rhs._viaLoc) && (_whollyWithinLocType == rhs._whollyWithinLocType) &&
                  (_whollyWithinLoc == rhs._whollyWithinLoc) &&
                  (_sectorPortionOfTvlInd == rhs._sectorPortionOfTvlInd) &&
                  (_directionality == rhs._directionality) &&
                  (_sectorPortionLocType1 == rhs._sectorPortionLocType1) &&
                  (_sectorPortionLoc1 == rhs._sectorPortionLoc1) &&
                  (_sectorPortionLocType2 == rhs._sectorPortionLocType2) &&
                  (_sectorPortionLoc2 == rhs._sectorPortionLoc2) &&
                  (_sectorPortionViaLocType == rhs._sectorPortionViaLocType) &&
                  (_sectorPortionViaLoc == rhs._sectorPortionViaLoc) &&
                  (_stopConnectInd == rhs._stopConnectInd) &&
                  (_stopConnectTime == rhs._stopConnectTime) &&
                  (_stopConnectUnit == rhs._stopConnectUnit) &&
                  (_connectExemptInd == rhs._connectExemptInd) &&
                  (_intlDomInd == rhs._intlDomInd) && (_bookingCode1 == rhs._bookingCode1) &&
                  (_bookingCode2 == rhs._bookingCode2) && (_bookingCode3 == rhs._bookingCode3) &&
                  (_equipCode == rhs._equipCode) && (_fareBasis == rhs._fareBasis) &&
                  (_hasSlashInFBC == rhs._hasSlashInFBC) && (_batchNo == rhs._batchNo) &&
                  (_batchCi == rhs._batchCi) && (_feeAmount == rhs._feeAmount) &&
                  (_cur == rhs._cur) && (_noDec == rhs._noDec) && (_percent == rhs._percent) &&
                  (_percentNoDec == rhs._percentNoDec) &&
                  (_taxIncludedInd == rhs._taxIncludedInd) && (_feeApplInd == rhs._feeApplInd) &&
                  (_taxCarrierApplTblItemNo == rhs._taxCarrierApplTblItemNo) &&
                  (_taxCarrierFltTblItemNo == rhs._taxCarrierFltTblItemNo) &&
                  (_taxTextTblItemNo == rhs._taxTextTblItemNo));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(YQYRFees& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._taxCode = "HIJ";
    obj._subCode = 'K';
    obj._seqNo = 1;
    obj._modDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._firstTktDate = time(nullptr);
    obj._lastTktDate = time(nullptr);
    obj._returnToOrigin = 'L';
    obj._psgType = "MNO";
    obj._posLocType = MARKET;
    obj._posLoc = "aaaaaaaa";
    obj._posLocaleType = 'P';
    obj._posAgencyPCC = "QRST";
    obj._posIataTvlAgencyNo = "UVWXYZab";
    obj._potLocType = NATION;
    obj._potLoc = "bbbbbbbb";
    obj._journeyLoc1Ind = 'c';
    obj._journeyLocType1 = 'd';
    obj._journeyLoc1 = "cccccccc";
    obj._journeyLocType2 = 'e';
    obj._journeyLoc2 = "dddddddd";
    obj._viaLocType = 'f';
    obj._viaLoc = "eeeeeeee";
    obj._whollyWithinLocType = 'g';
    obj._whollyWithinLoc = "ffffffff";
    obj._sectorPortionOfTvlInd = 'h';
    obj._directionality = 'i';
    obj._sectorPortionLocType1 = 'j';
    obj._sectorPortionLoc1 = "gggggggg";
    obj._sectorPortionLocType2 = 'k';
    obj._sectorPortionLoc2 = "hhhhhhhh";
    obj._sectorPortionViaLocType = 'l';
    obj._sectorPortionViaLoc = "iiiiiiii";
    obj._stopConnectInd = 'm';
    obj._stopConnectTime = 2;
    obj._stopConnectUnit = 'n';
    obj._connectExemptInd = 'o';
    obj._intlDomInd = 'p';
    obj._bookingCode1 = "qr";
    obj._bookingCode2 = "st";
    obj._bookingCode3 = "uv";
    obj._equipCode = "wxy";
    obj._fareBasis = "jjjjjjjj";
    obj._hasSlashInFBC = false;
    obj._batchNo = 3;
    obj._batchCi = "kkkkkkkk";
    obj._feeAmount = 4.44;
    obj._cur = "z01";
    obj._noDec = 5;
    obj._percent = 6.666;
    obj._percentNoDec = 7;
    obj._taxIncludedInd = '2';
    obj._feeApplInd = '3';
    obj._taxCarrierApplTblItemNo = 10;
    obj._taxCarrierFltTblItemNo = 8;
    obj._taxTextTblItemNo = 9;
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  TaxCode _taxCode;
  Indicator _subCode = ' ';
  int _seqNo = 0;
  DateTime _modDate;
  DateTime _expireDate;
  // Not in Schema    Indicator _inhibit;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _firstTktDate;
  DateTime _lastTktDate;
  Indicator _returnToOrigin = ' ';
  PaxTypeCode _psgType;
  LocType _posLocType = LocType::UNKNOWN_LOC;
  LocCode _posLoc;
  Indicator _posLocaleType = ' ';
  AgencyCode _posAgencyPCC;
  AgencyIATA _posIataTvlAgencyNo;
  LocType _potLocType = LocType::UNKNOWN_LOC;
  LocCode _potLoc;
  Indicator _journeyLoc1Ind = ' ';
  LocTypeCode _journeyLocType1 = ' ';
  LocCode _journeyLoc1;
  LocTypeCode _journeyLocType2 = ' ';
  LocCode _journeyLoc2;
  LocTypeCode _viaLocType = ' ';
  LocCode _viaLoc;
  LocTypeCode _whollyWithinLocType = ' ';
  LocCode _whollyWithinLoc;
  Indicator _sectorPortionOfTvlInd = ' ';
  Indicator _directionality = ' ';
  LocTypeCode _sectorPortionLocType1 = ' ';
  LocCode _sectorPortionLoc1;
  LocTypeCode _sectorPortionLocType2 = ' ';
  LocCode _sectorPortionLoc2;
  LocTypeCode _sectorPortionViaLocType = ' ';
  LocCode _sectorPortionViaLoc;
  Indicator _stopConnectInd = ' ';
  int _stopConnectTime = 0;
  Indicator _stopConnectUnit = ' ';
  Indicator _connectExemptInd = ' ';
  Indicator _intlDomInd = ' ';
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  BookingCode _bookingCode3;
  EquipmentType _equipCode;
  std::string _fareBasis;
  bool _hasSlashInFBC = false;
  int _batchNo = 0;
  std::string _batchCi;
  MoneyAmount _feeAmount = 0;
  CurrencyCode _cur;
  CurrencyNoDec _noDec = 0;
  Percent _percent = 0;
  int _percentNoDec = 0;
  Indicator _taxIncludedInd = ' ';
  Indicator _feeApplInd = ' ';
  int _taxCarrierApplTblItemNo = 0;
  int _taxCarrierFltTblItemNo = 0;
  int _taxTextTblItemNo = 0;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_carrier & ptr->_taxCode & ptr->_subCode & ptr->_seqNo &
           ptr->_modDate & ptr->_expireDate & ptr->_createDate & ptr->_effDate & ptr->_discDate &
           ptr->_firstTktDate & ptr->_lastTktDate & ptr->_returnToOrigin & ptr->_psgType &
           ptr->_posLocType & ptr->_posLoc & ptr->_posLocaleType & ptr->_posAgencyPCC &
           ptr->_posIataTvlAgencyNo & ptr->_potLocType & ptr->_potLoc & ptr->_journeyLoc1Ind &
           ptr->_journeyLocType1 & ptr->_journeyLoc1 & ptr->_journeyLocType2 & ptr->_journeyLoc2 &
           ptr->_viaLocType & ptr->_viaLoc & ptr->_whollyWithinLocType & ptr->_whollyWithinLoc &
           ptr->_sectorPortionOfTvlInd & ptr->_directionality & ptr->_sectorPortionLocType1 &
           ptr->_sectorPortionLoc1 & ptr->_sectorPortionLocType2 & ptr->_sectorPortionLoc2 &
           ptr->_sectorPortionViaLocType & ptr->_sectorPortionViaLoc & ptr->_stopConnectInd &
           ptr->_stopConnectTime & ptr->_stopConnectUnit & ptr->_connectExemptInd &
           ptr->_intlDomInd & ptr->_bookingCode1 & ptr->_bookingCode2 & ptr->_bookingCode3 &
           ptr->_equipCode & ptr->_fareBasis & ptr->_hasSlashInFBC & ptr->_batchNo & ptr->_batchCi &
           ptr->_feeAmount & ptr->_cur & ptr->_noDec & ptr->_percent & ptr->_percentNoDec &
           ptr->_taxIncludedInd & ptr->_feeApplInd & ptr->_taxCarrierApplTblItemNo &
           ptr->_taxCarrierFltTblItemNo & ptr->_taxTextTblItemNo;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _taxCode);
    FLATTENIZE(archive, _subCode);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _modDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _firstTktDate);
    FLATTENIZE(archive, _lastTktDate);
    FLATTENIZE(archive, _returnToOrigin);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _posLocType);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _posLocaleType);
    FLATTENIZE(archive, _posAgencyPCC);
    FLATTENIZE(archive, _posIataTvlAgencyNo);
    FLATTENIZE(archive, _potLocType);
    FLATTENIZE(archive, _potLoc);
    FLATTENIZE(archive, _journeyLoc1Ind);
    FLATTENIZE(archive, _journeyLocType1);
    FLATTENIZE(archive, _journeyLoc1);
    FLATTENIZE(archive, _journeyLocType2);
    FLATTENIZE(archive, _journeyLoc2);
    FLATTENIZE(archive, _viaLocType);
    FLATTENIZE(archive, _viaLoc);
    FLATTENIZE(archive, _whollyWithinLocType);
    FLATTENIZE(archive, _whollyWithinLoc);
    FLATTENIZE(archive, _sectorPortionOfTvlInd);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _sectorPortionLocType1);
    FLATTENIZE(archive, _sectorPortionLoc1);
    FLATTENIZE(archive, _sectorPortionLocType2);
    FLATTENIZE(archive, _sectorPortionLoc2);
    FLATTENIZE(archive, _sectorPortionViaLocType);
    FLATTENIZE(archive, _sectorPortionViaLoc);
    FLATTENIZE(archive, _stopConnectInd);
    FLATTENIZE(archive, _stopConnectTime);
    FLATTENIZE(archive, _stopConnectUnit);
    FLATTENIZE(archive, _connectExemptInd);
    FLATTENIZE(archive, _intlDomInd);
    FLATTENIZE(archive, _bookingCode1);
    FLATTENIZE(archive, _bookingCode2);
    FLATTENIZE(archive, _bookingCode3);
    FLATTENIZE(archive, _equipCode);
    FLATTENIZE(archive, _fareBasis);
    FLATTENIZE(archive, _hasSlashInFBC);
    FLATTENIZE(archive, _batchNo);
    FLATTENIZE(archive, _batchCi);
    FLATTENIZE(archive, _feeAmount);
    FLATTENIZE(archive, _cur);
    FLATTENIZE(archive, _noDec);
    FLATTENIZE(archive, _percent);
    FLATTENIZE(archive, _percentNoDec);
    FLATTENIZE(archive, _taxIncludedInd);
    FLATTENIZE(archive, _feeApplInd);
    FLATTENIZE(archive, _taxCarrierApplTblItemNo);
    FLATTENIZE(archive, _taxCarrierFltTblItemNo);
    FLATTENIZE(archive, _taxTextTblItemNo);
  }
};
} // class YQYRFees
