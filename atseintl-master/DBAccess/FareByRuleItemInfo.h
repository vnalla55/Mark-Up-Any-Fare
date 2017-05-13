//----------------------------------------------------------------------------
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class FareByRuleItemInfo : public RuleItemInfo
{
public:
  virtual bool operator==(const FareByRuleItemInfo& rhs) const
  {
    return (
        (RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_baseTableItemNo == rhs._baseTableItemNo) &&
        (_minMileage == rhs._minMileage) && (_maxMileage == rhs._maxMileage) &&
        (_resultRoutingTariff == rhs._resultRoutingTariff) && (_resultmpm == rhs._resultmpm) &&
        (_bookingCodeTblItemNo == rhs._bookingCodeTblItemNo) &&
        (_specifiedFareAmt1 == rhs._specifiedFareAmt1) &&
        (_specifiedFareAmt2 == rhs._specifiedFareAmt2) && (_minFareAmt1 == rhs._minFareAmt1) &&
        (_maxFareAmt1 == rhs._maxFareAmt1) && (_minFareAmt2 == rhs._minFareAmt2) &&
        (_maxFareAmt2 == rhs._maxFareAmt2) && (_minAge == rhs._minAge) &&
        (_maxAge == rhs._maxAge) && (_minNoPsg == rhs._minNoPsg) && (_maxNoPsg == rhs._maxNoPsg) &&
        (_fltSegCnt == rhs._fltSegCnt) && (_specifiedNoDec1 == rhs._specifiedNoDec1) &&
        (_specifiedNoDec2 == rhs._specifiedNoDec2) && (_noDec1 == rhs._noDec1) &&
        (_noDec2 == rhs._noDec2) && (_ruleTariff == rhs._ruleTariff) &&
        (_percentNoDec == rhs._percentNoDec) && (_percent == rhs._percent) &&
        (_inhibit == rhs._inhibit) && (_unavailtag == rhs._unavailtag) &&
        (_negPsgstatusInd == rhs._negPsgstatusInd) && (_passengerInd == rhs._passengerInd) &&
        (_psgid == rhs._psgid) && (_discountInd == rhs._discountInd) &&
        (_fareInd == rhs._fareInd) && (_resultowrt == rhs._resultowrt) &&
        (_resultseasonType == rhs._resultseasonType) && (_resultdowType == rhs._resultdowType) &&
        (_resultDisplaycatType == rhs._resultDisplaycatType) &&
        (_resultpricingcatType == rhs._resultpricingcatType) &&
        (_tktCodeModifier == rhs._tktCodeModifier) &&
        (_tktDesignatorModifier == rhs._tktDesignatorModifier) && (_ovrdcat1 == rhs._ovrdcat1) &&
        (_ovrdcat2 == rhs._ovrdcat2) && (_ovrdcat3 == rhs._ovrdcat3) &&
        (_ovrdcat4 == rhs._ovrdcat4) && (_ovrdcat5 == rhs._ovrdcat5) &&
        (_ovrdcat6 == rhs._ovrdcat6) && (_ovrdcat7 == rhs._ovrdcat7) &&
        (_ovrdcat8 == rhs._ovrdcat8) && (_ovrdcat9 == rhs._ovrdcat9) &&
        (_ovrdcat10 == rhs._ovrdcat10) && (_ovrdcat11 == rhs._ovrdcat11) &&
        (_ovrdcat12 == rhs._ovrdcat12) && (_ovrdcat13 == rhs._ovrdcat13) &&
        (_ovrdcat14 == rhs._ovrdcat14) && (_ovrdcat15 == rhs._ovrdcat15) &&
        (_ovrdcat16 == rhs._ovrdcat16) && (_ovrdcat17 == rhs._ovrdcat17) &&
        (_ovrdcat18 == rhs._ovrdcat18) && (_ovrdcat19 == rhs._ovrdcat19) &&
        (_ovrdcat20 == rhs._ovrdcat20) && (_ovrdcat21 == rhs._ovrdcat21) &&
        (_ovrdcat22 == rhs._ovrdcat22) && (_ovrdcat23 == rhs._ovrdcat23) &&
        (_ovrdcat24 == rhs._ovrdcat24) && (_ovrdcat26 == rhs._ovrdcat26) &&
        (_ovrdcat27 == rhs._ovrdcat27) && (_ovrdcat28 == rhs._ovrdcat28) &&
        (_ovrdcat29 == rhs._ovrdcat29) && (_ovrdcat30 == rhs._ovrdcat30) &&
        (_ovrdcat31 == rhs._ovrdcat31) && (_ovrdcat32 == rhs._ovrdcat32) &&
        (_ovrdcat33 == rhs._ovrdcat33) && (_ovrdcat34 == rhs._ovrdcat34) &&
        (_ovrdcat35 == rhs._ovrdcat35) && (_ovrdcat36 == rhs._ovrdcat36) &&
        (_ovrdcat37 == rhs._ovrdcat37) && (_ovrdcat38 == rhs._ovrdcat38) &&
        (_ovrdcat39 == rhs._ovrdcat39) && (_ovrdcat40 == rhs._ovrdcat40) &&
        (_ovrdcat41 == rhs._ovrdcat41) && (_ovrdcat42 == rhs._ovrdcat42) &&
        (_ovrdcat43 == rhs._ovrdcat43) && (_ovrdcat44 == rhs._ovrdcat44) &&
        (_ovrdcat45 == rhs._ovrdcat45) && (_ovrdcat46 == rhs._ovrdcat46) &&
        (_ovrdcat47 == rhs._ovrdcat47) && (_ovrdcat48 == rhs._ovrdcat48) &&
        (_ovrdcat49 == rhs._ovrdcat49) && (_ovrdcat50 == rhs._ovrdcat50) &&
        (_highestInd == rhs._highestInd) && (_psgLoc1 == rhs._psgLoc1) &&
        (_whollyWithinLoc == rhs._whollyWithinLoc) && (_tsi == rhs._tsi) && (_loc1 == rhs._loc1) &&
        (_specifiedCur1 == rhs._specifiedCur1) && (_specifiedCur2 == rhs._specifiedCur2) &&
        (_cur1 == rhs._cur1) && (_cur2 == rhs._cur2) && (_carrier == rhs._carrier) &&
        (_baseFareClass == rhs._baseFareClass) && (_baseFareType == rhs._baseFareType) &&
        (_resultglobalDir == rhs._resultglobalDir) && (_resultRouting == rhs._resultRouting) &&
        (_resultFareClass1 == rhs._resultFareClass1) &&
        (_resultFareType1 == rhs._resultFareType1) && (_bookingCode1 == rhs._bookingCode1) &&
        (_bookingCode2 == rhs._bookingCode2) && (_bookingCode3 == rhs._bookingCode3) &&
        (_bookingCode4 == rhs._bookingCode4) && (_bookingCode5 == rhs._bookingCode5) &&
        (_bookingCode6 == rhs._bookingCode6) && (_bookingCode7 == rhs._bookingCode7) &&
        (_bookingCode8 == rhs._bookingCode8) && (_tktCode == rhs._tktCode) &&
        (_tktDesignator == rhs._tktDesignator) && (_paxType == rhs._paxType) &&
        (_sameTariffRule == rhs._sameTariffRule) && (_primeSector == rhs._primeSector) &&
        (_resultRoutingVendor == rhs._resultRoutingVendor));
  }

  static void dummyData(FareByRuleItemInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._baseTableItemNo = 1;
    obj._minMileage = 2;
    obj._maxMileage = 3;
    obj._resultRoutingTariff = 4;
    obj._resultmpm = 5;
    obj._bookingCodeTblItemNo = 6;
    obj._specifiedFareAmt1 = 7.77;
    obj._specifiedFareAmt2 = 8.88;
    obj._minFareAmt1 = 9.99;
    obj._maxFareAmt1 = 10.10;
    obj._minFareAmt2 = 11.11;
    obj._maxFareAmt2 = 12.12;
    obj._minAge = 13;
    obj._maxAge = 14;
    obj._minNoPsg = 15;
    obj._maxNoPsg = 16;
    obj._fltSegCnt = 17;
    obj._specifiedNoDec1 = 18;
    obj._specifiedNoDec2 = 19;
    obj._noDec1 = 20;
    obj._noDec2 = 21;
    obj._ruleTariff = 22;
    obj._percentNoDec = 23;
    obj._percent = 24.242;
    obj._inhibit = 'A';
    obj._unavailtag = 'B';
    obj._negPsgstatusInd = 'C';
    obj._passengerInd = 'D';
    obj._psgid = 'E';
    obj._discountInd = 'F';
    obj._fareInd = 'G';
    obj._resultowrt = 'H';
    obj._resultseasonType = 'I';
    obj._resultdowType = 'J';
    obj._resultDisplaycatType = 'K';
    obj._resultpricingcatType = 'L';
    obj._tktCodeModifier = 'M';
    obj._tktDesignatorModifier = 'N';
    obj._ovrdcat1 = 'O';
    obj._ovrdcat2 = 'P';
    obj._ovrdcat3 = 'Q';
    obj._ovrdcat4 = 'R';
    obj._ovrdcat5 = 'S';
    obj._ovrdcat6 = 'T';
    obj._ovrdcat7 = 'U';
    obj._ovrdcat8 = 'V';
    obj._ovrdcat9 = 'W';
    obj._ovrdcat10 = 'X';
    obj._ovrdcat11 = 'Y';
    obj._ovrdcat12 = 'Z';
    obj._ovrdcat13 = 'a';
    obj._ovrdcat14 = 'b';
    obj._ovrdcat15 = 'c';
    obj._ovrdcat16 = 'd';
    obj._ovrdcat17 = 'e';
    obj._ovrdcat18 = 'f';
    obj._ovrdcat19 = 'g';
    obj._ovrdcat20 = 'h';
    obj._ovrdcat21 = 'i';
    obj._ovrdcat22 = 'j';
    obj._ovrdcat23 = 'k';
    obj._ovrdcat24 = 'l';
    obj._ovrdcat26 = 'm';
    obj._ovrdcat27 = 'n';
    obj._ovrdcat28 = 'o';
    obj._ovrdcat29 = 'p';
    obj._ovrdcat30 = 'q';
    obj._ovrdcat31 = 'r';
    obj._ovrdcat32 = 's';
    obj._ovrdcat33 = 't';
    obj._ovrdcat34 = 'u';
    obj._ovrdcat35 = 'v';
    obj._ovrdcat36 = 'w';
    obj._ovrdcat37 = 'x';
    obj._ovrdcat38 = 'y';
    obj._ovrdcat39 = 'z';
    obj._ovrdcat40 = '1';
    obj._ovrdcat41 = '2';
    obj._ovrdcat42 = '3';
    obj._ovrdcat43 = '4';
    obj._ovrdcat44 = '5';
    obj._ovrdcat45 = '6';
    obj._ovrdcat46 = '7';
    obj._ovrdcat47 = '8';
    obj._ovrdcat48 = '9';
    obj._ovrdcat49 = '0';
    obj._ovrdcat50 = 'A';
    obj._highestInd = 'B';

    LocKey::dummyData(obj._psgLoc1);
    LocKey::dummyData(obj._whollyWithinLoc);

    obj._tsi = 25;

    LocKey::dummyData(obj._loc1);

    obj._specifiedCur1 = "CDE";
    obj._specifiedCur2 = "FGH";
    obj._cur1 = "IJK";
    obj._cur2 = "LMN";
    obj._carrier = "OPQ";
    obj._baseFareClass = "RSTUVWXY";
    obj._baseFareType = "Zabcdefg";
    obj._resultglobalDir = GlobalDirection::US;
    obj._resultRouting = "hijk";
    obj._resultFareClass1 = "lmnopqrs";
    obj._resultFareType1 = "tuvwxyz1";
    obj._bookingCode1 = "23";
    obj._bookingCode2 = "45";
    obj._bookingCode3 = "67";
    obj._bookingCode4 = "89";
    obj._bookingCode5 = "0A";
    obj._bookingCode6 = "BC";
    obj._bookingCode7 = "DE";
    obj._bookingCode8 = "FG";
    obj._tktCode = "HIJKLMNOPQ";
    obj._tktDesignator = "RSTUVWXYZa";
    obj._paxType = "bcd";
    obj._sameTariffRule = 'E';
    obj._primeSector = 'F';
    obj._resultRoutingVendor = "GHIJ";
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  int _baseTableItemNo = 0;
  uint32_t _minMileage = 0;
  uint32_t _maxMileage = 0;
  TariffNumber _resultRoutingTariff = 0;
  int _resultmpm = 0;
  int _bookingCodeTblItemNo = 0;
  MoneyAmount _specifiedFareAmt1 = 0;
  MoneyAmount _specifiedFareAmt2 = 0;
  MoneyAmount _minFareAmt1 = 0;
  MoneyAmount _maxFareAmt1 = 0;
  MoneyAmount _minFareAmt2 = 0;
  MoneyAmount _maxFareAmt2 = 0;
  int _minAge = 0;
  int _maxAge = 0;
  int _minNoPsg = 0;
  int _maxNoPsg = 0;
  uint32_t _fltSegCnt = 0;
  int _specifiedNoDec1 = 0;
  int _specifiedNoDec2 = 0;
  int _noDec1 = 0;
  int _noDec2 = 0;
  TariffNumber _ruleTariff = 0;
  int _percentNoDec = 0;
  Percent _percent = 0;
  Indicator _inhibit = ' ';
  Indicator _unavailtag = ' ';
  Indicator _negPsgstatusInd = ' ';
  Indicator _passengerInd = ' ';
  Indicator _psgid = ' ';
  Indicator _discountInd = ' ';
  Indicator _fareInd = ' ';
  Indicator _resultowrt = ' ';
  Indicator _resultseasonType = ' ';
  Indicator _resultdowType = ' ';
  Indicator _resultDisplaycatType = ' ';
  Indicator _resultpricingcatType = ' ';
  Indicator _tktCodeModifier = ' ';
  Indicator _tktDesignatorModifier = ' ';
  Indicator _ovrdcat1 = ' ';
  Indicator _ovrdcat2 = ' ';
  Indicator _ovrdcat3 = ' ';
  Indicator _ovrdcat4 = ' ';
  Indicator _ovrdcat5 = ' ';
  Indicator _ovrdcat6 = ' ';
  Indicator _ovrdcat7 = ' ';
  Indicator _ovrdcat8 = ' ';
  Indicator _ovrdcat9 = ' ';
  Indicator _ovrdcat10 = ' ';
  Indicator _ovrdcat11 = ' ';
  Indicator _ovrdcat12 = ' ';
  Indicator _ovrdcat13 = ' ';
  Indicator _ovrdcat14 = ' ';
  Indicator _ovrdcat15 = ' ';
  Indicator _ovrdcat16 = ' ';
  Indicator _ovrdcat17 = ' ';
  Indicator _ovrdcat18 = ' ';
  Indicator _ovrdcat19 = ' ';
  Indicator _ovrdcat20 = ' ';
  Indicator _ovrdcat21 = ' ';
  Indicator _ovrdcat22 = ' ';
  Indicator _ovrdcat23 = ' ';
  Indicator _ovrdcat24 = ' ';
  Indicator _ovrdcat26 = ' ';
  Indicator _ovrdcat27 = ' ';
  Indicator _ovrdcat28 = ' ';
  Indicator _ovrdcat29 = ' ';
  Indicator _ovrdcat30 = ' ';
  Indicator _ovrdcat31 = ' ';
  Indicator _ovrdcat32 = ' ';
  Indicator _ovrdcat33 = ' ';
  Indicator _ovrdcat34 = ' ';
  Indicator _ovrdcat35 = ' ';
  Indicator _ovrdcat36 = ' ';
  Indicator _ovrdcat37 = ' ';
  Indicator _ovrdcat38 = ' ';
  Indicator _ovrdcat39 = ' ';
  Indicator _ovrdcat40 = ' ';
  Indicator _ovrdcat41 = ' ';
  Indicator _ovrdcat42 = ' ';
  Indicator _ovrdcat43 = ' ';
  Indicator _ovrdcat44 = ' ';
  Indicator _ovrdcat45 = ' ';
  Indicator _ovrdcat46 = ' ';
  Indicator _ovrdcat47 = ' ';
  Indicator _ovrdcat48 = ' ';
  Indicator _ovrdcat49 = ' ';
  Indicator _ovrdcat50 = ' ';
  Indicator _highestInd = ' ';
  LocKey _psgLoc1;
  LocKey _whollyWithinLoc;
  TSICode _tsi = 0;
  LocKey _loc1;
  CurrencyCode _specifiedCur1;
  CurrencyCode _specifiedCur2;
  CurrencyCode _cur1;
  CurrencyCode _cur2;
  CarrierCode _carrier;
  FareClassCode _baseFareClass;
  FareType _baseFareType;
  GlobalDirection _resultglobalDir = GlobalDirection::NO_DIR;
  RoutingNumber _resultRouting;
  FareClassCode _resultFareClass1;
  FareType _resultFareType1;
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  BookingCode _bookingCode3;
  BookingCode _bookingCode4;
  BookingCode _bookingCode5;
  BookingCode _bookingCode6;
  BookingCode _bookingCode7;
  BookingCode _bookingCode8;
  TktCode _tktCode;
  TktDesignator _tktDesignator;
  PaxTypeCode _paxType;
  Indicator _sameTariffRule = ' ';
  Indicator _primeSector = ' ';
  VendorCode _resultRoutingVendor;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _baseTableItemNo);
    FLATTENIZE(archive, _minMileage);
    FLATTENIZE(archive, _maxMileage);
    FLATTENIZE(archive, _resultRoutingTariff);
    FLATTENIZE(archive, _resultmpm);
    FLATTENIZE(archive, _bookingCodeTblItemNo);
    FLATTENIZE(archive, _specifiedFareAmt1);
    FLATTENIZE(archive, _specifiedFareAmt2);
    FLATTENIZE(archive, _minFareAmt1);
    FLATTENIZE(archive, _maxFareAmt1);
    FLATTENIZE(archive, _minFareAmt2);
    FLATTENIZE(archive, _maxFareAmt2);
    FLATTENIZE(archive, _minAge);
    FLATTENIZE(archive, _maxAge);
    FLATTENIZE(archive, _minNoPsg);
    FLATTENIZE(archive, _maxNoPsg);
    FLATTENIZE(archive, _fltSegCnt);
    FLATTENIZE(archive, _specifiedNoDec1);
    FLATTENIZE(archive, _specifiedNoDec2);
    FLATTENIZE(archive, _noDec1);
    FLATTENIZE(archive, _noDec2);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _percentNoDec);
    FLATTENIZE(archive, _percent);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _negPsgstatusInd);
    FLATTENIZE(archive, _passengerInd);
    FLATTENIZE(archive, _psgid);
    FLATTENIZE(archive, _discountInd);
    FLATTENIZE(archive, _fareInd);
    FLATTENIZE(archive, _resultowrt);
    FLATTENIZE(archive, _resultseasonType);
    FLATTENIZE(archive, _resultdowType);
    FLATTENIZE(archive, _resultDisplaycatType);
    FLATTENIZE(archive, _resultpricingcatType);
    FLATTENIZE(archive, _tktCodeModifier);
    FLATTENIZE(archive, _tktDesignatorModifier);
    FLATTENIZE(archive, _ovrdcat1);
    FLATTENIZE(archive, _ovrdcat2);
    FLATTENIZE(archive, _ovrdcat3);
    FLATTENIZE(archive, _ovrdcat4);
    FLATTENIZE(archive, _ovrdcat5);
    FLATTENIZE(archive, _ovrdcat6);
    FLATTENIZE(archive, _ovrdcat7);
    FLATTENIZE(archive, _ovrdcat8);
    FLATTENIZE(archive, _ovrdcat9);
    FLATTENIZE(archive, _ovrdcat10);
    FLATTENIZE(archive, _ovrdcat11);
    FLATTENIZE(archive, _ovrdcat12);
    FLATTENIZE(archive, _ovrdcat13);
    FLATTENIZE(archive, _ovrdcat14);
    FLATTENIZE(archive, _ovrdcat15);
    FLATTENIZE(archive, _ovrdcat16);
    FLATTENIZE(archive, _ovrdcat17);
    FLATTENIZE(archive, _ovrdcat18);
    FLATTENIZE(archive, _ovrdcat19);
    FLATTENIZE(archive, _ovrdcat20);
    FLATTENIZE(archive, _ovrdcat21);
    FLATTENIZE(archive, _ovrdcat22);
    FLATTENIZE(archive, _ovrdcat23);
    FLATTENIZE(archive, _ovrdcat24);
    FLATTENIZE(archive, _ovrdcat26);
    FLATTENIZE(archive, _ovrdcat27);
    FLATTENIZE(archive, _ovrdcat28);
    FLATTENIZE(archive, _ovrdcat29);
    FLATTENIZE(archive, _ovrdcat30);
    FLATTENIZE(archive, _ovrdcat31);
    FLATTENIZE(archive, _ovrdcat32);
    FLATTENIZE(archive, _ovrdcat33);
    FLATTENIZE(archive, _ovrdcat34);
    FLATTENIZE(archive, _ovrdcat35);
    FLATTENIZE(archive, _ovrdcat36);
    FLATTENIZE(archive, _ovrdcat37);
    FLATTENIZE(archive, _ovrdcat38);
    FLATTENIZE(archive, _ovrdcat39);
    FLATTENIZE(archive, _ovrdcat40);
    FLATTENIZE(archive, _ovrdcat41);
    FLATTENIZE(archive, _ovrdcat42);
    FLATTENIZE(archive, _ovrdcat43);
    FLATTENIZE(archive, _ovrdcat44);
    FLATTENIZE(archive, _ovrdcat45);
    FLATTENIZE(archive, _ovrdcat46);
    FLATTENIZE(archive, _ovrdcat47);
    FLATTENIZE(archive, _ovrdcat48);
    FLATTENIZE(archive, _ovrdcat49);
    FLATTENIZE(archive, _ovrdcat50);
    FLATTENIZE(archive, _highestInd);
    FLATTENIZE(archive, _psgLoc1);
    FLATTENIZE(archive, _whollyWithinLoc);
    FLATTENIZE(archive, _tsi);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _specifiedCur1);
    FLATTENIZE(archive, _specifiedCur2);
    FLATTENIZE(archive, _cur1);
    FLATTENIZE(archive, _cur2);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _baseFareClass);
    FLATTENIZE(archive, _baseFareType);
    FLATTENIZE(archive, _resultglobalDir);
    FLATTENIZE(archive, _resultRouting);
    FLATTENIZE(archive, _resultFareClass1);
    FLATTENIZE(archive, _resultFareType1);
    FLATTENIZE(archive, _bookingCode1);
    FLATTENIZE(archive, _bookingCode2);
    FLATTENIZE(archive, _bookingCode3);
    FLATTENIZE(archive, _bookingCode4);
    FLATTENIZE(archive, _bookingCode5);
    FLATTENIZE(archive, _bookingCode6);
    FLATTENIZE(archive, _bookingCode7);
    FLATTENIZE(archive, _bookingCode8);
    FLATTENIZE(archive, _tktCode);
    FLATTENIZE(archive, _tktDesignator);
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _sameTariffRule);
    FLATTENIZE(archive, _primeSector);
    FLATTENIZE(archive, _resultRoutingVendor);
  }

protected:
public:
  const static char SPECIFIED;
  const static char SPECIFIED_K;
  const static char SPECIFIED_E;
  const static char SPECIFIED_F;

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& baseTableItemNo() { return _baseTableItemNo; }
  const int& baseTableItemNo() const { return _baseTableItemNo; }

  uint32_t& minMileage() { return _minMileage; }
  const uint32_t& minMileage() const { return _minMileage; }

  uint32_t& maxMileage() { return _maxMileage; }
  const uint32_t& maxMileage() const { return _maxMileage; }

  TariffNumber& resultRoutingTariff() { return _resultRoutingTariff; }
  const TariffNumber& resultRoutingTariff() const { return _resultRoutingTariff; }

  int& resultmpm() { return _resultmpm; }
  const int& resultmpm() const { return _resultmpm; }

  int& bookingCodeTblItemNo() { return _bookingCodeTblItemNo; }
  const int& bookingCodeTblItemNo() const { return _bookingCodeTblItemNo; }

  MoneyAmount& specifiedFareAmt1() { return _specifiedFareAmt1; }
  const MoneyAmount& specifiedFareAmt1() const { return _specifiedFareAmt1; }

  MoneyAmount& specifiedFareAmt2() { return _specifiedFareAmt2; }
  const MoneyAmount& specifiedFareAmt2() const { return _specifiedFareAmt2; }

  MoneyAmount& minFareAmt1() { return _minFareAmt1; }
  const MoneyAmount& minFareAmt1() const { return _minFareAmt1; }

  MoneyAmount& maxFareAmt1() { return _maxFareAmt1; }
  const MoneyAmount& maxFareAmt1() const { return _maxFareAmt1; }

  MoneyAmount& minFareAmt2() { return _minFareAmt2; }
  const MoneyAmount& minFareAmt2() const { return _minFareAmt2; }

  MoneyAmount& maxFareAmt2() { return _maxFareAmt2; }
  const MoneyAmount& maxFareAmt2() const { return _maxFareAmt2; }

  int& minAge() { return _minAge; }
  const int& minAge() const { return _minAge; }

  int& maxAge() { return _maxAge; }
  const int& maxAge() const { return _maxAge; }

  int& minNoPsg() { return _minNoPsg; }
  const int& minNoPsg() const { return _minNoPsg; }

  int& maxNoPsg() { return _maxNoPsg; }
  const int& maxNoPsg() const { return _maxNoPsg; }

  uint32_t& fltSegCnt() { return _fltSegCnt; }
  const uint32_t& fltSegCnt() const { return _fltSegCnt; }

  int& specifiedNoDec1() { return _specifiedNoDec1; }
  const int& specifiedNoDec1() const { return _specifiedNoDec1; }

  int& specifiedNoDec2() { return _specifiedNoDec2; }
  const int& specifiedNoDec2() const { return _specifiedNoDec2; }

  int& noDec1() { return _noDec1; }
  const int& noDec1() const { return _noDec1; }

  int& noDec2() { return _noDec2; }
  const int& noDec2() const { return _noDec2; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  int& percentNoDec() { return _percentNoDec; }
  const int& percentNoDec() const { return _percentNoDec; }

  Percent& percent() { return _percent; }
  const Percent& percent() const { return _percent; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  Indicator& negPsgstatusInd() { return _negPsgstatusInd; }
  const Indicator& negPsgstatusInd() const { return _negPsgstatusInd; }

  Indicator& passengerInd() { return _passengerInd; }
  const Indicator& passengerInd() const { return _passengerInd; }

  Indicator& psgid() { return _psgid; }
  const Indicator& psgid() const { return _psgid; }

  Indicator& discountInd() { return _discountInd; }
  const Indicator& discountInd() const { return _discountInd; }

  Indicator& fareInd() { return _fareInd; }
  const Indicator& fareInd() const { return _fareInd; }

  Indicator& resultowrt() { return _resultowrt; }
  const Indicator& resultowrt() const { return _resultowrt; }

  Indicator& resultseasonType() { return _resultseasonType; }
  const Indicator& resultseasonType() const { return _resultseasonType; }

  Indicator& resultdowType() { return _resultdowType; }
  const Indicator& resultdowType() const { return _resultdowType; }

  Indicator& resultDisplaycatType() { return _resultDisplaycatType; }
  const Indicator& resultDisplaycatType() const { return _resultDisplaycatType; }

  Indicator& resultpricingcatType() { return _resultpricingcatType; }
  const Indicator& resultpricingcatType() const { return _resultpricingcatType; }

  Indicator& tktCodeModifier() { return _tktCodeModifier; }
  const Indicator& tktCodeModifier() const { return _tktCodeModifier; }

  Indicator& tktDesignatorModifier() { return _tktDesignatorModifier; }
  const Indicator& tktDesignatorModifier() const { return _tktDesignatorModifier; }

  Indicator& ovrdcat1() { return _ovrdcat1; }
  const Indicator& ovrdcat1() const { return _ovrdcat1; }

  Indicator& ovrdcat2() { return _ovrdcat2; }
  const Indicator& ovrdcat2() const { return _ovrdcat2; }

  Indicator& ovrdcat3() { return _ovrdcat3; }
  const Indicator& ovrdcat3() const { return _ovrdcat3; }

  Indicator& ovrdcat4() { return _ovrdcat4; }
  const Indicator& ovrdcat4() const { return _ovrdcat4; }

  Indicator& ovrdcat5() { return _ovrdcat5; }
  const Indicator& ovrdcat5() const { return _ovrdcat5; }

  Indicator& ovrdcat6() { return _ovrdcat6; }
  const Indicator& ovrdcat6() const { return _ovrdcat6; }

  Indicator& ovrdcat7() { return _ovrdcat7; }
  const Indicator& ovrdcat7() const { return _ovrdcat7; }

  Indicator& ovrdcat8() { return _ovrdcat8; }
  const Indicator& ovrdcat8() const { return _ovrdcat8; }

  Indicator& ovrdcat9() { return _ovrdcat9; }
  const Indicator& ovrdcat9() const { return _ovrdcat9; }

  Indicator& ovrdcat10() { return _ovrdcat10; }
  const Indicator& ovrdcat10() const { return _ovrdcat10; }

  Indicator& ovrdcat11() { return _ovrdcat11; }
  const Indicator& ovrdcat11() const { return _ovrdcat11; }

  Indicator& ovrdcat12() { return _ovrdcat12; }
  const Indicator& ovrdcat12() const { return _ovrdcat12; }

  Indicator& ovrdcat13() { return _ovrdcat13; }
  const Indicator& ovrdcat13() const { return _ovrdcat13; }

  Indicator& ovrdcat14() { return _ovrdcat14; }
  const Indicator& ovrdcat14() const { return _ovrdcat14; }

  Indicator& ovrdcat15() { return _ovrdcat15; }
  const Indicator& ovrdcat15() const { return _ovrdcat15; }

  Indicator& ovrdcat16() { return _ovrdcat16; }
  const Indicator& ovrdcat16() const { return _ovrdcat16; }

  Indicator& ovrdcat17() { return _ovrdcat17; }
  const Indicator& ovrdcat17() const { return _ovrdcat17; }

  Indicator& ovrdcat18() { return _ovrdcat18; }
  const Indicator& ovrdcat18() const { return _ovrdcat18; }

  Indicator& ovrdcat19() { return _ovrdcat19; }
  const Indicator& ovrdcat19() const { return _ovrdcat19; }

  Indicator& ovrdcat20() { return _ovrdcat20; }
  const Indicator& ovrdcat20() const { return _ovrdcat20; }

  Indicator& ovrdcat21() { return _ovrdcat21; }
  const Indicator& ovrdcat21() const { return _ovrdcat21; }

  Indicator& ovrdcat22() { return _ovrdcat22; }
  const Indicator& ovrdcat22() const { return _ovrdcat22; }

  Indicator& ovrdcat23() { return _ovrdcat23; }
  const Indicator& ovrdcat23() const { return _ovrdcat23; }

  Indicator& ovrdcat24() { return _ovrdcat24; }
  const Indicator& ovrdcat24() const { return _ovrdcat24; }

  Indicator& ovrdcat26() { return _ovrdcat26; }
  const Indicator& ovrdcat26() const { return _ovrdcat26; }

  Indicator& ovrdcat27() { return _ovrdcat27; }
  const Indicator& ovrdcat27() const { return _ovrdcat27; }

  Indicator& ovrdcat28() { return _ovrdcat28; }
  const Indicator& ovrdcat28() const { return _ovrdcat28; }

  Indicator& ovrdcat29() { return _ovrdcat29; }
  const Indicator& ovrdcat29() const { return _ovrdcat29; }

  Indicator& ovrdcat30() { return _ovrdcat30; }
  const Indicator& ovrdcat30() const { return _ovrdcat30; }

  Indicator& ovrdcat31() { return _ovrdcat31; }
  const Indicator& ovrdcat31() const { return _ovrdcat31; }

  Indicator& ovrdcat32() { return _ovrdcat32; }
  const Indicator& ovrdcat32() const { return _ovrdcat32; }

  Indicator& ovrdcat33() { return _ovrdcat33; }
  const Indicator& ovrdcat33() const { return _ovrdcat33; }

  Indicator& ovrdcat34() { return _ovrdcat34; }
  const Indicator& ovrdcat34() const { return _ovrdcat34; }

  Indicator& ovrdcat35() { return _ovrdcat35; }
  const Indicator& ovrdcat35() const { return _ovrdcat35; }

  Indicator& ovrdcat36() { return _ovrdcat36; }
  const Indicator& ovrdcat36() const { return _ovrdcat36; }

  Indicator& ovrdcat37() { return _ovrdcat37; }
  const Indicator& ovrdcat37() const { return _ovrdcat37; }

  Indicator& ovrdcat38() { return _ovrdcat38; }
  const Indicator& ovrdcat38() const { return _ovrdcat38; }

  Indicator& ovrdcat39() { return _ovrdcat39; }
  const Indicator& ovrdcat39() const { return _ovrdcat39; }

  Indicator& ovrdcat40() { return _ovrdcat40; }
  const Indicator& ovrdcat40() const { return _ovrdcat40; }

  Indicator& ovrdcat41() { return _ovrdcat41; }
  const Indicator& ovrdcat41() const { return _ovrdcat41; }

  Indicator& ovrdcat42() { return _ovrdcat42; }
  const Indicator& ovrdcat42() const { return _ovrdcat42; }

  Indicator& ovrdcat43() { return _ovrdcat43; }
  const Indicator& ovrdcat43() const { return _ovrdcat43; }

  Indicator& ovrdcat44() { return _ovrdcat44; }
  const Indicator& ovrdcat44() const { return _ovrdcat44; }

  Indicator& ovrdcat45() { return _ovrdcat45; }
  const Indicator& ovrdcat45() const { return _ovrdcat45; }

  Indicator& ovrdcat46() { return _ovrdcat46; }
  const Indicator& ovrdcat46() const { return _ovrdcat46; }

  Indicator& ovrdcat47() { return _ovrdcat47; }
  const Indicator& ovrdcat47() const { return _ovrdcat47; }

  Indicator& ovrdcat48() { return _ovrdcat48; }
  const Indicator& ovrdcat48() const { return _ovrdcat48; }

  Indicator& ovrdcat49() { return _ovrdcat49; }
  const Indicator& ovrdcat49() const { return _ovrdcat49; }

  Indicator& ovrdcat50() { return _ovrdcat50; }
  const Indicator& ovrdcat50() const { return _ovrdcat50; }

  Indicator& highestInd() { return _highestInd; }
  const Indicator& highestInd() const { return _highestInd; }

  LocKey& psgLoc1() { return _psgLoc1; }
  const LocKey& psgLoc1() const { return _psgLoc1; }

  LocKey& whollyWithinLoc() { return _whollyWithinLoc; }
  const LocKey& whollyWithinLoc() const { return _whollyWithinLoc; }

  TSICode& tsi() { return _tsi; }
  const TSICode& tsi() const { return _tsi; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  CurrencyCode& specifiedCur1() { return _specifiedCur1; }
  const CurrencyCode& specifiedCur1() const { return _specifiedCur1; }

  CurrencyCode& specifiedCur2() { return _specifiedCur2; }
  const CurrencyCode& specifiedCur2() const { return _specifiedCur2; }

  CurrencyCode& cur1() { return _cur1; }
  const CurrencyCode& cur1() const { return _cur1; }

  CurrencyCode& cur2() { return _cur2; }
  const CurrencyCode& cur2() const { return _cur2; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  FareClassCode& baseFareClass() { return _baseFareClass; }
  const FareClassCode& baseFareClass() const { return _baseFareClass; }

  FareType& baseFareType() { return _baseFareType; }
  const FareType& baseFareType() const { return _baseFareType; }

  GlobalDirection& resultglobalDir() { return _resultglobalDir; }
  const GlobalDirection& resultglobalDir() const { return _resultglobalDir; }

  RoutingNumber& resultRouting() { return _resultRouting; }
  const RoutingNumber& resultRouting() const { return _resultRouting; }

  FareClassCode& resultFareClass1() { return _resultFareClass1; }
  const FareClassCode& resultFareClass1() const { return _resultFareClass1; }

  FareType& resultFareType1() { return _resultFareType1; }
  const FareType& resultFareType1() const { return _resultFareType1; }

  BookingCode& bookingCode1() { return _bookingCode1; }
  const BookingCode& bookingCode1() const { return _bookingCode1; }

  BookingCode& bookingCode2() { return _bookingCode2; }
  const BookingCode& bookingCode2() const { return _bookingCode2; }

  BookingCode& bookingCode3() { return _bookingCode3; }
  const BookingCode& bookingCode3() const { return _bookingCode3; }

  BookingCode& bookingCode4() { return _bookingCode4; }
  const BookingCode& bookingCode4() const { return _bookingCode4; }

  BookingCode& bookingCode5() { return _bookingCode5; }
  const BookingCode& bookingCode5() const { return _bookingCode5; }

  BookingCode& bookingCode6() { return _bookingCode6; }
  const BookingCode& bookingCode6() const { return _bookingCode6; }

  BookingCode& bookingCode7() { return _bookingCode7; }
  const BookingCode& bookingCode7() const { return _bookingCode7; }

  BookingCode& bookingCode8() { return _bookingCode8; }
  const BookingCode& bookingCode8() const { return _bookingCode8; }

  TktCode& tktCode() { return _tktCode; }
  const TktCode& tktCode() const { return _tktCode; }

  TktDesignator& tktDesignator() { return _tktDesignator; }
  const TktDesignator& tktDesignator() const { return _tktDesignator; }

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  Indicator& sameTariffRule() { return _sameTariffRule; }
  const Indicator& sameTariffRule() const { return _sameTariffRule; }

  Indicator& primeSector() { return _primeSector; }
  const Indicator& primeSector() const { return _primeSector; }

  VendorCode& resultRoutingVendor() { return _resultRoutingVendor; }
  const VendorCode& resultRoutingVendor() const { return _resultRoutingVendor; }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer & ptr->_createDate & ptr->_expireDate & ptr->_baseTableItemNo & ptr->_minMileage &
           ptr->_maxMileage & ptr->_resultRoutingTariff & ptr->_resultmpm &
           ptr->_bookingCodeTblItemNo & ptr->_specifiedFareAmt1 & ptr->_specifiedFareAmt2 &
           ptr->_minFareAmt1 & ptr->_maxFareAmt1 & ptr->_minFareAmt2 & ptr->_maxFareAmt2 &
           ptr->_minAge & ptr->_maxAge & ptr->_minNoPsg & ptr->_maxNoPsg & ptr->_fltSegCnt &
           ptr->_specifiedNoDec1 & ptr->_specifiedNoDec2 & ptr->_noDec1 & ptr->_noDec2 &
           ptr->_ruleTariff & ptr->_percentNoDec & ptr->_percent & ptr->_inhibit &
           ptr->_unavailtag & ptr->_negPsgstatusInd & ptr->_passengerInd & ptr->_psgid &
           ptr->_discountInd & ptr->_fareInd & ptr->_resultowrt & ptr->_resultseasonType &
           ptr->_resultdowType & ptr->_resultDisplaycatType & ptr->_resultpricingcatType &
           ptr->_tktCodeModifier & ptr->_tktDesignatorModifier & ptr->_ovrdcat1 & ptr->_ovrdcat2 &
           ptr->_ovrdcat3 & ptr->_ovrdcat4 & ptr->_ovrdcat5 & ptr->_ovrdcat6 & ptr->_ovrdcat7 &
           ptr->_ovrdcat8 & ptr->_ovrdcat9 & ptr->_ovrdcat10 & ptr->_ovrdcat11 & ptr->_ovrdcat12 &
           ptr->_ovrdcat13 & ptr->_ovrdcat14 & ptr->_ovrdcat15 & ptr->_ovrdcat16 & ptr->_ovrdcat17 &
           ptr->_ovrdcat18 & ptr->_ovrdcat19 & ptr->_ovrdcat20 & ptr->_ovrdcat21 & ptr->_ovrdcat22 &
           ptr->_ovrdcat23 & ptr->_ovrdcat24 & ptr->_ovrdcat26 & ptr->_ovrdcat27 & ptr->_ovrdcat28 &
           ptr->_ovrdcat29 & ptr->_ovrdcat30 & ptr->_ovrdcat31 & ptr->_ovrdcat32 & ptr->_ovrdcat33 &
           ptr->_ovrdcat34 & ptr->_ovrdcat35 & ptr->_ovrdcat36 & ptr->_ovrdcat37 & ptr->_ovrdcat38 &
           ptr->_ovrdcat39 & ptr->_ovrdcat40 & ptr->_ovrdcat41 & ptr->_ovrdcat42 & ptr->_ovrdcat43 &
           ptr->_ovrdcat44 & ptr->_ovrdcat45 & ptr->_ovrdcat46 & ptr->_ovrdcat47 & ptr->_ovrdcat48 &
           ptr->_ovrdcat49 & ptr->_ovrdcat50 & ptr->_highestInd & ptr->_psgLoc1 &
           ptr->_whollyWithinLoc & ptr->_tsi & ptr->_loc1 & ptr->_specifiedCur1 &
           ptr->_specifiedCur2 & ptr->_cur1 & ptr->_cur2 & ptr->_carrier & ptr->_baseFareClass &
           ptr->_baseFareType & ptr->_resultglobalDir & ptr->_resultRouting &
           ptr->_resultFareClass1 & ptr->_resultFareType1 & ptr->_bookingCode1 &
           ptr->_bookingCode2 & ptr->_bookingCode3 & ptr->_bookingCode4 & ptr->_bookingCode5 &
           ptr->_bookingCode6 & ptr->_bookingCode7 & ptr->_bookingCode8 & ptr->_tktCode &
           ptr->_tktDesignator & ptr->_paxType & ptr->_sameTariffRule & ptr->_primeSector &
           ptr->_resultRoutingVendor;
  }
};
}

