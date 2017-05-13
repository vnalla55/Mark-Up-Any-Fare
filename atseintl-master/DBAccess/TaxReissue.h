//----------------------------------------------------------------------------
//
//   2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class TaxReissue
{
public:
  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  TaxType& taxType() { return _taxType; }
  const TaxType& taxType() const { return _taxType; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  MoneyAmount& taxAmt() { return _taxAmt; }
  const MoneyAmount& taxAmt() const { return _taxAmt; }

  int& memoNo() { return _memoNo; }
  const int& memoNo() const { return _memoNo; }

  int& newSeqNo() { return _newSeqNo; }
  const int& newSeqNo() const { return _newSeqNo; }

  CurrencyNoDec& currencyNoDec() { return _currencyNoDec; }
  const CurrencyNoDec& currencyNoDec() const { return _currencyNoDec; }

  CurrencyCode& currencyCode() { return _currencyCode; }
  const CurrencyCode& currencyCode() const { return _currencyCode; }

  Indicator& reissueExclLocInd() { return _reissueExclLocInd; }
  const Indicator& reissueExclLocInd() const { return _reissueExclLocInd; }

  LocType& reissueLocType() { return _reissueLocType; }
  const LocType& reissueLocType() const { return _reissueLocType; }

  LocCode& reissueLoc() { return _reissueLoc; }
  const LocCode& reissueLoc() const { return _reissueLoc; }

  Indicator& refundInd() { return _refundInd; }
  const Indicator& refundInd() const { return _refundInd; }

  Indicator& tktlCxrExclInd() { return _tktlCxrExclInd; }
  const Indicator& tktlCxrExclInd() const { return _tktlCxrExclInd; }

  std::vector<CarrierCode>& validationCxr() { return _validationCxr; }
  const std::vector<CarrierCode>& validationCxr() const { return _validationCxr; }

  Indicator& cat33onlyInd() { return _cat33onlyInd; }
  const Indicator& cat33onlyInd() const { return _cat33onlyInd; }

  bool operator==(const TaxReissue& rhs) const
  {
    return ((_taxCode == rhs._taxCode) && (_taxType == rhs._taxType) &&
            (_versionDate == rhs._versionDate) &&
            (_createDate == rhs._createDate) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_lockDate == rhs._lockDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_taxAmt == rhs._taxAmt) && (_memoNo == rhs._memoNo) && (_newSeqNo == rhs._newSeqNo) &&
            (_currencyNoDec == rhs._currencyNoDec) && (_currencyCode == rhs._currencyCode) &&
            (_reissueExclLocInd == rhs._reissueExclLocInd) &&
            (_reissueLocType == rhs._reissueLocType) && (_reissueLoc == rhs._reissueLoc) &&
            (_refundInd == rhs._refundInd) && (_tktlCxrExclInd == rhs._tktlCxrExclInd) &&
            (_validationCxr == rhs._validationCxr) &&
            (_cat33onlyInd == rhs._cat33onlyInd));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxReissue& obj)
  {
    obj._taxCode = "ABC";
    obj._taxType = "001";
    obj._versionDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._taxAmt = 2.22;
    obj._memoNo = 3;
    obj._newSeqNo = 4;
    obj._currencyNoDec = 5;
    obj._currencyCode = "DEF";
    obj._reissueExclLocInd = 'G';
    obj._reissueLocType = MARKET;
    obj._reissueLoc = "HIJKL";
    obj._refundInd = 'M';
    obj._tktlCxrExclInd = 'N';
    obj._cat33onlyInd = 'N';

    obj._validationCxr.push_back("OPQ");
    obj._validationCxr.push_back("RST");
  }

private:
  TaxCode _taxCode;
  TaxType _taxType;
  DateTime _versionDate;
  DateTime _createDate;
  int _seqNo = 0;
  DateTime _expireDate;
  DateTime _lockDate;
  DateTime _effDate;
  DateTime _discDate;
  MoneyAmount _taxAmt = 0;
  int _memoNo = 0;
  int _newSeqNo = 0;
  CurrencyNoDec _currencyNoDec = 0;
  CurrencyCode _currencyCode;
  Indicator _reissueExclLocInd = ' ';
  LocType _reissueLocType = LocType::UNKNOWN_LOC;
  LocCode _reissueLoc;
  Indicator _refundInd = ' ';
  Indicator _tktlCxrExclInd = ' ';
  std::vector<CarrierCode> _validationCxr;
  Indicator _cat33onlyInd;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_taxCode & ptr->_taxType & ptr->_versionDate & ptr->_createDate &
           ptr->_seqNo & ptr->_expireDate & ptr->_lockDate & ptr->_effDate & ptr->_discDate &
           ptr->_taxAmt & ptr->_memoNo & ptr->_newSeqNo & ptr->_currencyNoDec & ptr->_currencyCode &
           ptr->_reissueExclLocInd & ptr->_reissueLocType & ptr->_reissueLoc & ptr->_refundInd &
           ptr->_tktlCxrExclInd & ptr->_validationCxr & ptr->_cat33onlyInd;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _taxCode);
    FLATTENIZE(archive, _taxType);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _taxAmt);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _newSeqNo);
    FLATTENIZE(archive, _currencyNoDec);
    FLATTENIZE(archive, _currencyCode);
    FLATTENIZE(archive, _reissueExclLocInd);
    FLATTENIZE(archive, _reissueLocType);
    FLATTENIZE(archive, _reissueLoc);
    FLATTENIZE(archive, _refundInd);
    FLATTENIZE(archive, _tktlCxrExclInd);
    FLATTENIZE(archive, _validationCxr);
    FLATTENIZE(archive, _cat33onlyInd);
  }
};
}
