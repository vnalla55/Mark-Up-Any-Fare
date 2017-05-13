//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef COMMISSIONCAP_H
#define COMMISSIONCAP_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CommissionCap
{
public:
  CommissionCap()
    : _seqNo(0),
      _amtNoDec(0),
      _amt(0),
      _minAmtNoDec(0),
      _minAmt(0),
      _maxAmtNoDec(0),
      _maxAmt(0),
      _agencyType(' '),
      _amtType(' '),
      _minAmtType(' '),
      _maxAmtType(' '),
      _canada(' '),
      _domestic(' '),
      _international(' '),
      _transBorder(' '),
      _owRt(' '),
      _valCarrierTvl(' '),
      _inhibit(' ')
  {
  }
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  CurrencyCode& cur() { return _cur; }
  const CurrencyCode& cur() const { return _cur; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& amtNoDec() { return _amtNoDec; }
  const int& amtNoDec() const { return _amtNoDec; }

  MoneyAmount& amt() { return _amt; }
  const MoneyAmount& amt() const { return _amt; }

  int& minAmtNoDec() { return _minAmtNoDec; }
  const int& minAmtNoDec() const { return _minAmtNoDec; }

  MoneyAmount& minAmt() { return _minAmt; }
  const MoneyAmount& minAmt() const { return _minAmt; }

  int& maxAmtNoDec() { return _maxAmtNoDec; }
  const int& maxAmtNoDec() const { return _maxAmtNoDec; }

  MoneyAmount& maxAmt() { return _maxAmt; }
  const MoneyAmount& maxAmt() const { return _maxAmt; }

  Indicator& agencyType() { return _agencyType; }
  const Indicator& agencyType() const { return _agencyType; }

  Indicator& amtType() { return _amtType; }
  const Indicator& amtType() const { return _amtType; }

  Indicator& minAmtType() { return _minAmtType; }
  const Indicator& minAmtType() const { return _minAmtType; }

  Indicator& maxAmtType() { return _maxAmtType; }
  const Indicator& maxAmtType() const { return _maxAmtType; }

  Indicator& canada() { return _canada; }
  const Indicator& canada() const { return _canada; }

  Indicator& domestic() { return _domestic; }
  const Indicator& domestic() const { return _domestic; }

  Indicator& international() { return _international; }
  const Indicator& international() const { return _international; }

  Indicator& transBorder() { return _transBorder; }
  const Indicator& transBorder() const { return _transBorder; }

  Indicator& owRt() { return _owRt; }
  const Indicator& owRt() const { return _owRt; }

  Indicator& valCarrierTvl() { return _valCarrierTvl; }
  const Indicator& valCarrierTvl() const { return _valCarrierTvl; }

  NationCode& tktIssueNation() { return _tktIssueNation; }
  const NationCode& tktIssueNation() const { return _tktIssueNation; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  bool operator==(const CommissionCap& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_cur == rhs._cur) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_amtNoDec == rhs._amtNoDec) && (_amt == rhs._amt) &&
            (_minAmtNoDec == rhs._minAmtNoDec) && (_minAmt == rhs._minAmt) &&
            (_maxAmtNoDec == rhs._maxAmtNoDec) && (_maxAmt == rhs._maxAmt) &&
            (_agencyType == rhs._agencyType) && (_amtType == rhs._amtType) &&
            (_minAmtType == rhs._minAmtType) && (_maxAmtType == rhs._maxAmtType) &&
            (_canada == rhs._canada) && (_domestic == rhs._domestic) &&
            (_international == rhs._international) && (_transBorder == rhs._transBorder) &&
            (_owRt == rhs._owRt) && (_valCarrierTvl == rhs._valCarrierTvl) &&
            (_tktIssueNation == rhs._tktIssueNation) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(CommissionCap& obj)
  {
    obj._carrier = "ABC";
    obj._cur = "DEF";
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._amtNoDec = 2;
    obj._amt = 3.33;
    obj._minAmtNoDec = 4;
    obj._minAmt = 5.55;
    obj._maxAmtNoDec = 6;
    obj._maxAmt = 7.77;
    obj._agencyType = 'G';
    obj._amtType = 'H';
    obj._minAmtType = 'I';
    obj._maxAmtType = 'J';
    obj._canada = 'K';
    obj._domestic = 'L';
    obj._international = 'M';
    obj._transBorder = 'N';
    obj._owRt = 'O';
    obj._valCarrierTvl = 'P';
    obj._tktIssueNation = "QRST";
    obj._inhibit = 'U';
  }

private:
  CarrierCode _carrier;
  CurrencyCode _cur;
  int _seqNo;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  int _amtNoDec;
  MoneyAmount _amt;
  int _minAmtNoDec;
  MoneyAmount _minAmt;
  int _maxAmtNoDec;
  MoneyAmount _maxAmt;
  Indicator _agencyType;
  Indicator _amtType;
  Indicator _minAmtType;
  Indicator _maxAmtType;
  Indicator _canada;
  Indicator _domestic;
  Indicator _international;
  Indicator _transBorder;
  Indicator _owRt;
  Indicator _valCarrierTvl;
  NationCode _tktIssueNation;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _cur);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _amtNoDec);
    FLATTENIZE(archive, _amt);
    FLATTENIZE(archive, _minAmtNoDec);
    FLATTENIZE(archive, _minAmt);
    FLATTENIZE(archive, _maxAmtNoDec);
    FLATTENIZE(archive, _maxAmt);
    FLATTENIZE(archive, _agencyType);
    FLATTENIZE(archive, _amtType);
    FLATTENIZE(archive, _minAmtType);
    FLATTENIZE(archive, _maxAmtType);
    FLATTENIZE(archive, _canada);
    FLATTENIZE(archive, _domestic);
    FLATTENIZE(archive, _international);
    FLATTENIZE(archive, _transBorder);
    FLATTENIZE(archive, _owRt);
    FLATTENIZE(archive, _valCarrierTvl);
    FLATTENIZE(archive, _tktIssueNation);
    FLATTENIZE(archive, _inhibit);
  }

};
}
#endif
