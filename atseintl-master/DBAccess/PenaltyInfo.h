//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class PenaltyInfo : public RuleItemInfo
{
public:
  PenaltyInfo()
    : _unavailTag(' '),
      _geoTblItemNo(0),
      _penaltyNoDec1(0),
      _penaltyAmt1(0),
      _penaltyNoDec2(0),
      _penaltyAmt2(0),
      _penaltyPercentNoDec(0),
      _penaltyPercent(0),
      _volAppl(' '),
      _involAppl(' '),
      _cancelRefundAppl(' '),
      _noRefundInd(' '),
      _penaltyCancel(' '),
      _penaltyFail(' '),
      _penaltyReissue(' '),
      _penaltyExchange(' '),
      _penaltyNoReissue(' '),
      _penaltyRefund(' '),
      _penaltyPta(' '),
      _penaltyAppl(' '),
      _penaltyPortion(' '),
      _penaltyHlInd(' '),
      _waiverPsgDeath(' '),
      _waiverPsgIll(' '),
      _waiverPsgFamDeath(' '),
      _waiverPsgFamIll(' '),
      _waiverSchedChg(' '),
      _waiverTktUpgrade(' '),
      _inhibit(' ')
  {
  }

  virtual ~PenaltyInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  int& penaltyNoDec1() { return _penaltyNoDec1; }
  const int& penaltyNoDec1() const { return _penaltyNoDec1; }

  MoneyAmount& penaltyAmt1() { return _penaltyAmt1; }
  const MoneyAmount& penaltyAmt1() const { return _penaltyAmt1; }

  int& penaltyNoDec2() { return _penaltyNoDec2; }
  const int& penaltyNoDec2() const { return _penaltyNoDec2; }

  MoneyAmount& penaltyAmt2() { return _penaltyAmt2; }
  const MoneyAmount& penaltyAmt2() const { return _penaltyAmt2; }

  int& penaltyPercentNoDec() { return _penaltyPercentNoDec; }
  const int& penaltyPercentNoDec() const { return _penaltyPercentNoDec; }

  Percent& penaltyPercent() { return _penaltyPercent; }
  const Percent& penaltyPercent() const { return _penaltyPercent; }

  Indicator& volAppl() { return _volAppl; }
  const Indicator& volAppl() const { return _volAppl; }

  Indicator& involAppl() { return _involAppl; }
  const Indicator& involAppl() const { return _involAppl; }

  Indicator& cancelRefundAppl() { return _cancelRefundAppl; }
  const Indicator& cancelRefundAppl() const { return _cancelRefundAppl; }

  Indicator& noRefundInd() { return _noRefundInd; }
  const Indicator& noRefundInd() const { return _noRefundInd; }

  Indicator& penaltyCancel() { return _penaltyCancel; }
  const Indicator& penaltyCancel() const { return _penaltyCancel; }

  Indicator& penaltyFail() { return _penaltyFail; }
  const Indicator& penaltyFail() const { return _penaltyFail; }

  Indicator& penaltyReissue() { return _penaltyReissue; }
  const Indicator& penaltyReissue() const { return _penaltyReissue; }

  Indicator& penaltyExchange() { return _penaltyExchange; }
  const Indicator& penaltyExchange() const { return _penaltyExchange; }

  Indicator& penaltyNoReissue() { return _penaltyNoReissue; }
  const Indicator& penaltyNoReissue() const { return _penaltyNoReissue; }

  Indicator& penaltyRefund() { return _penaltyRefund; }
  const Indicator& penaltyRefund() const { return _penaltyRefund; }

  Indicator& penaltyPta() { return _penaltyPta; }
  const Indicator& penaltyPta() const { return _penaltyPta; }

  Indicator& penaltyAppl() { return _penaltyAppl; }
  const Indicator& penaltyAppl() const { return _penaltyAppl; }

  Indicator& penaltyPortion() { return _penaltyPortion; }
  const Indicator& penaltyPortion() const { return _penaltyPortion; }

  CurrencyCode& penaltyCur1() { return _penaltyCur1; }
  const CurrencyCode& penaltyCur1() const { return _penaltyCur1; }

  CurrencyCode& penaltyCur2() { return _penaltyCur2; }
  const CurrencyCode& penaltyCur2() const { return _penaltyCur2; }

  Indicator& penaltyHlInd() { return _penaltyHlInd; }
  const Indicator& penaltyHlInd() const { return _penaltyHlInd; }

  Indicator& waiverPsgDeath() { return _waiverPsgDeath; }
  const Indicator& waiverPsgDeath() const { return _waiverPsgDeath; }

  Indicator& waiverPsgIll() { return _waiverPsgIll; }
  const Indicator& waiverPsgIll() const { return _waiverPsgIll; }

  Indicator& waiverPsgFamDeath() { return _waiverPsgFamDeath; }
  const Indicator& waiverPsgFamDeath() const { return _waiverPsgFamDeath; }

  Indicator& waiverPsgFamIll() { return _waiverPsgFamIll; }
  const Indicator& waiverPsgFamIll() const { return _waiverPsgFamIll; }

  Indicator& waiverSchedChg() { return _waiverSchedChg; }
  const Indicator& waiverSchedChg() const { return _waiverSchedChg; }

  Indicator& waiverTktUpgrade() { return _waiverTktUpgrade; }
  const Indicator& waiverTktUpgrade() const { return _waiverTktUpgrade; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const PenaltyInfo& rhs) const
  {
    return (
        (RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
        (_geoTblItemNo == rhs._geoTblItemNo) && (_penaltyNoDec1 == rhs._penaltyNoDec1) &&
        (_penaltyAmt1 == rhs._penaltyAmt1) && (_penaltyNoDec2 == rhs._penaltyNoDec2) &&
        (_penaltyAmt2 == rhs._penaltyAmt2) && (_penaltyPercentNoDec == rhs._penaltyPercentNoDec) &&
        (_penaltyPercent == rhs._penaltyPercent) && (_volAppl == rhs._volAppl) &&
        (_involAppl == rhs._involAppl) && (_cancelRefundAppl == rhs._cancelRefundAppl) &&
        (_noRefundInd == rhs._noRefundInd) && (_penaltyCancel == rhs._penaltyCancel) &&
        (_penaltyFail == rhs._penaltyFail) && (_penaltyReissue == rhs._penaltyReissue) &&
        (_penaltyExchange == rhs._penaltyExchange) &&
        (_penaltyNoReissue == rhs._penaltyNoReissue) && (_penaltyRefund == rhs._penaltyRefund) &&
        (_penaltyPta == rhs._penaltyPta) && (_penaltyAppl == rhs._penaltyAppl) &&
        (_penaltyPortion == rhs._penaltyPortion) && (_penaltyCur1 == rhs._penaltyCur1) &&
        (_penaltyCur2 == rhs._penaltyCur2) && (_penaltyHlInd == rhs._penaltyHlInd) &&
        (_waiverPsgDeath == rhs._waiverPsgDeath) && (_waiverPsgIll == rhs._waiverPsgIll) &&
        (_waiverPsgFamDeath == rhs._waiverPsgFamDeath) &&
        (_waiverPsgFamIll == rhs._waiverPsgFamIll) && (_waiverSchedChg == rhs._waiverSchedChg) &&
        (_waiverTktUpgrade == rhs._waiverTktUpgrade) && (_inhibit == rhs._inhibit));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  virtual RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(PenaltyInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._geoTblItemNo = 1;
    obj._penaltyNoDec1 = 2;
    obj._penaltyAmt1 = 3.33;
    obj._penaltyNoDec2 = 4;
    obj._penaltyAmt2 = 5.55;
    obj._penaltyPercentNoDec = 6;
    obj._penaltyPercent = 7.777;
    obj._volAppl = 'B';
    obj._involAppl = 'C';
    obj._cancelRefundAppl = 'D';
    obj._noRefundInd = 'E';
    obj._penaltyCancel = 'F';
    obj._penaltyFail = 'G';
    obj._penaltyReissue = 'H';
    obj._penaltyExchange = 'I';
    obj._penaltyNoReissue = 'J';
    obj._penaltyRefund = 'K';
    obj._penaltyPta = 'L';
    obj._penaltyAppl = 'M';
    obj._penaltyPortion = 'N';
    obj._penaltyCur1 = "OPQ";
    obj._penaltyCur2 = "RST";
    obj._penaltyHlInd = 'U';
    obj._waiverPsgDeath = 'V';
    obj._waiverPsgIll = 'W';
    obj._waiverPsgFamDeath = 'X';
    obj._waiverPsgFamIll = 'Y';
    obj._waiverSchedChg = 'Z';
    obj._waiverTktUpgrade = 'a';
    obj._inhibit = 'b';
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailTag;
  int _geoTblItemNo;
  int _penaltyNoDec1;
  MoneyAmount _penaltyAmt1;
  int _penaltyNoDec2;
  MoneyAmount _penaltyAmt2;
  int _penaltyPercentNoDec;
  Percent _penaltyPercent;
  Indicator _volAppl;
  Indicator _involAppl;
  Indicator _cancelRefundAppl;
  Indicator _noRefundInd;
  Indicator _penaltyCancel;
  Indicator _penaltyFail;
  Indicator _penaltyReissue;
  Indicator _penaltyExchange;
  Indicator _penaltyNoReissue;
  Indicator _penaltyRefund;
  Indicator _penaltyPta;
  Indicator _penaltyAppl;
  Indicator _penaltyPortion;
  CurrencyCode _penaltyCur1;
  CurrencyCode _penaltyCur2;
  Indicator _penaltyHlInd;
  Indicator _waiverPsgDeath;
  Indicator _waiverPsgIll;
  Indicator _waiverPsgFamDeath;
  Indicator _waiverPsgFamIll;
  Indicator _waiverSchedChg;
  Indicator _waiverTktUpgrade;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _geoTblItemNo);
    FLATTENIZE(archive, _penaltyNoDec1);
    FLATTENIZE(archive, _penaltyAmt1);
    FLATTENIZE(archive, _penaltyNoDec2);
    FLATTENIZE(archive, _penaltyAmt2);
    FLATTENIZE(archive, _penaltyPercentNoDec);
    FLATTENIZE(archive, _penaltyPercent);
    FLATTENIZE(archive, _volAppl);
    FLATTENIZE(archive, _involAppl);
    FLATTENIZE(archive, _cancelRefundAppl);
    FLATTENIZE(archive, _noRefundInd);
    FLATTENIZE(archive, _penaltyCancel);
    FLATTENIZE(archive, _penaltyFail);
    FLATTENIZE(archive, _penaltyReissue);
    FLATTENIZE(archive, _penaltyExchange);
    FLATTENIZE(archive, _penaltyNoReissue);
    FLATTENIZE(archive, _penaltyRefund);
    FLATTENIZE(archive, _penaltyPta);
    FLATTENIZE(archive, _penaltyAppl);
    FLATTENIZE(archive, _penaltyPortion);
    FLATTENIZE(archive, _penaltyCur1);
    FLATTENIZE(archive, _penaltyCur2);
    FLATTENIZE(archive, _penaltyHlInd);
    FLATTENIZE(archive, _waiverPsgDeath);
    FLATTENIZE(archive, _waiverPsgIll);
    FLATTENIZE(archive, _waiverPsgFamDeath);
    FLATTENIZE(archive, _waiverPsgFamIll);
    FLATTENIZE(archive, _waiverSchedChg);
    FLATTENIZE(archive, _waiverTktUpgrade);
    FLATTENIZE(archive, _inhibit);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer & ptr->_createDate & ptr->_expireDate & ptr->_unavailTag & ptr->_geoTblItemNo &
           ptr->_penaltyNoDec1 & ptr->_penaltyAmt1 & ptr->_penaltyNoDec2 & ptr->_penaltyAmt2 &
           ptr->_penaltyPercentNoDec & ptr->_penaltyPercent & ptr->_volAppl & ptr->_involAppl &
           ptr->_cancelRefundAppl & ptr->_noRefundInd & ptr->_penaltyCancel & ptr->_penaltyFail &
           ptr->_penaltyReissue & ptr->_penaltyExchange & ptr->_penaltyNoReissue &
           ptr->_penaltyRefund & ptr->_penaltyPta & ptr->_penaltyAppl & ptr->_penaltyPortion &
           ptr->_penaltyCur1 & ptr->_penaltyCur2 & ptr->_penaltyHlInd & ptr->_waiverPsgDeath &
           ptr->_waiverPsgIll & ptr->_waiverPsgFamDeath & ptr->_waiverPsgFamIll &
           ptr->_waiverSchedChg & ptr->_waiverTktUpgrade & ptr->_inhibit;
  }
};
}
