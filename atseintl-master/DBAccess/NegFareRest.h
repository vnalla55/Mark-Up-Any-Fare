//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class NegFareRest : public RuleItemInfo
{
public:
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& negFareSecurityTblItemNo() { return _negFareSecurityTblItemNo; }
  const int& negFareSecurityTblItemNo() const { return _negFareSecurityTblItemNo; }

  int& negFareCalcTblItemNo() { return _negFareCalcTblItemNo; }
  const int& negFareCalcTblItemNo() const { return _negFareCalcTblItemNo; }

  MoneyAmount& fareAmt1() { return _fareAmt1; }
  const MoneyAmount& fareAmt1() const { return _fareAmt1; }

  MoneyAmount& fareAmt2() { return _fareAmt2; }
  const MoneyAmount& fareAmt2() const { return _fareAmt2; }

  MoneyAmount& commAmt1() { return _commAmt1; }
  const MoneyAmount& commAmt1() const { return _commAmt1; }

  MoneyAmount& commAmt2() { return _commAmt2; }
  const MoneyAmount& commAmt2() const { return _commAmt2; }

  Percent& commPercent() { return _commPercent; }
  const Percent& commPercent() const { return _commPercent; }

  int& commPercentNoDec() { return _commPercentNoDec; }
  const int& commPercentNoDec() const { return _commPercentNoDec; }

  int& noDec1() { return _noDec1; }
  const int& noDec1() const { return _noDec1; }

  int& noDec2() { return _noDec2; }
  const int& noDec2() const { return _noDec2; }

  int& noSegs() { return _noSegs; }
  const int& noSegs() const { return _noSegs; }

  int& noDec11() { return _noDec11; }
  const int& noDec11() const { return _noDec11; }

  TariffNumber& ruleTariff1() { return _ruleTariff1; }
  const TariffNumber& ruleTariff1() const { return _ruleTariff1; }

  int& noDec21() { return _noDec21; }
  const int& noDec21() const { return _noDec21; }

  TariffNumber& ruleTariff2() { return _ruleTariff2; }
  const TariffNumber& ruleTariff2() const { return _ruleTariff2; }

  Indicator& netRemitMethod() { return _netRemitMethod; }
  const Indicator& netRemitMethod() const { return _netRemitMethod; }

  Indicator& netGrossInd() { return _netGrossInd; }
  const Indicator& netGrossInd() const { return _netGrossInd; }

  Indicator& upgradeInd() { return _upgradeInd; }
  const Indicator& upgradeInd() const { return _upgradeInd; }

  Indicator& bagTypeInd() { return _bagTypeInd; }
  const Indicator& bagTypeInd() const { return _bagTypeInd; }

  Indicator& validationInd() { return _validationInd; }
  const Indicator& validationInd() const { return _validationInd; }

  Indicator& tktAppl() { return _tktAppl; }
  const Indicator& tktAppl() const { return _tktAppl; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  CurrencyCode& cur1() { return _cur1; }
  const CurrencyCode& cur1() const { return _cur1; }

  CurrencyCode& cur2() { return _cur2; }
  const CurrencyCode& cur2() const { return _cur2; }

  std::string& bagNo() { return _bagNo; }
  const std::string& bagNo() const { return _bagNo; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& couponInd1() { return _couponInd1; }
  const Indicator& couponInd1() const { return _couponInd1; }

  Indicator& tourBoxCodeType1() { return _tourBoxCodeType1; }
  const Indicator& tourBoxCodeType1() const { return _tourBoxCodeType1; }

  Indicator& tktFareDataInd1() { return _tktFareDataInd1; }
  const Indicator& tktFareDataInd1() const { return _tktFareDataInd1; }

  Indicator& owrt1() { return _owrt1; }
  const Indicator& owrt1() const { return _owrt1; }

  Indicator& seasonType1() { return _seasonType1; }
  const Indicator& seasonType1() const { return _seasonType1; }

  Indicator& dowType1() { return _dowType1; }
  const Indicator& dowType1() const { return _dowType1; }

  std::string& tourBoxCode1() { return _tourBoxCode1; }
  const std::string& tourBoxCode1() const { return _tourBoxCode1; }

  TktDesignator& tktDesignator1() { return _tktDesignator1; }
  const TktDesignator& tktDesignator1() const { return _tktDesignator1; }

  GlobalDirection& globalDir1() { return _globalDir1; }
  const GlobalDirection& globalDir1() const { return _globalDir1; }

  CarrierCode& carrier11() { return _carrier11; }
  const CarrierCode& carrier11() const { return _carrier11; }

  RuleNumber& rule1() { return _rule1; }
  const RuleNumber& rule1() const { return _rule1; }

  FareClassCode& fareClass1() { return _fareClass1; }
  const FareClassCode& fareClass1() const { return _fareClass1; }

  FareType& fareType1() { return _fareType1; }
  const FareType& fareType1() const { return _fareType1; }

  LocCode& betwCity1() { return _betwCity1; }
  const LocCode& betwCity1() const { return _betwCity1; }

  LocCode& andCity1() { return _andCity1; }
  const LocCode& andCity1() const { return _andCity1; }

  std::string& fareBoxText1() { return _fareBoxText1; }
  const std::string& fareBoxText1() const { return _fareBoxText1; }

  CurrencyCode& cur11() { return _cur11; }
  const CurrencyCode& cur11() const { return _cur11; }

  Indicator& couponInd2() { return _couponInd2; }
  const Indicator& couponInd2() const { return _couponInd2; }

  Indicator& tourBoxCodeType2() { return _tourBoxCodeType2; }
  const Indicator& tourBoxCodeType2() const { return _tourBoxCodeType2; }

  Indicator& tktFareDataInd2() { return _tktFareDataInd2; }
  const Indicator& tktFareDataInd2() const { return _tktFareDataInd2; }

  Indicator& owrt2() { return _owrt2; }
  const Indicator& owrt2() const { return _owrt2; }

  Indicator& seasonType2() { return _seasonType2; }
  const Indicator& seasonType2() const { return _seasonType2; }

  Indicator& dowType2() { return _dowType2; }
  const Indicator& dowType2() const { return _dowType2; }

  std::string& tourBoxCode2() { return _tourBoxCode2; }
  const std::string& tourBoxCode2() const { return _tourBoxCode2; }

  TktDesignator& tktDesignator2() { return _tktDesignator2; }
  const TktDesignator& tktDesignator2() const { return _tktDesignator2; }

  GlobalDirection& globalDir2() { return _globalDir2; }
  const GlobalDirection& globalDir2() const { return _globalDir2; }

  CarrierCode& carrier21() { return _carrier21; }
  const CarrierCode& carrier21() const { return _carrier21; }

  RuleNumber& rule2() { return _rule2; }
  const RuleNumber& rule2() const { return _rule2; }

  FareClassCode& fareClass2() { return _fareClass2; }
  const FareClassCode& fareClass2() const { return _fareClass2; }

  FareType& fareType2() { return _fareType2; }
  const FareType& fareType2() const { return _fareType2; }

  LocCode& betwCity2() { return _betwCity2; }
  const LocCode& betwCity2() const { return _betwCity2; }

  LocCode& andCity2() { return _andCity2; }
  const LocCode& andCity2() const { return _andCity2; }

  std::string& fareBoxText2() { return _fareBoxText2; }
  const std::string& fareBoxText2() const { return _fareBoxText2; }

  CurrencyCode& cur21() { return _cur21; }
  const CurrencyCode& cur21() const { return _cur21; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const NegFareRest& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
            (_negFareSecurityTblItemNo == rhs._negFareSecurityTblItemNo) &&
            (_negFareCalcTblItemNo == rhs._negFareCalcTblItemNo) && (_fareAmt1 == rhs._fareAmt1) &&
            (_fareAmt2 == rhs._fareAmt2) && (_commAmt1 == rhs._commAmt1) &&
            (_commAmt2 == rhs._commAmt2) && (_commPercent == rhs._commPercent) &&
            (_commPercentNoDec == rhs._commPercentNoDec) && (_noDec1 == rhs._noDec1) &&
            (_noDec2 == rhs._noDec2) && (_noSegs == rhs._noSegs) && (_noDec11 == rhs._noDec11) &&
            (_ruleTariff1 == rhs._ruleTariff1) && (_noDec21 == rhs._noDec21) &&
            (_ruleTariff2 == rhs._ruleTariff2) && (_netRemitMethod == rhs._netRemitMethod) &&
            (_netGrossInd == rhs._netGrossInd) && (_upgradeInd == rhs._upgradeInd) &&
            (_bagTypeInd == rhs._bagTypeInd) && (_validationInd == rhs._validationInd) &&
            (_tktAppl == rhs._tktAppl) && (_psgType == rhs._psgType) && (_cur1 == rhs._cur1) &&
            (_cur2 == rhs._cur2) && (_bagNo == rhs._bagNo) && (_carrier == rhs._carrier) &&
            (_couponInd1 == rhs._couponInd1) && (_tourBoxCodeType1 == rhs._tourBoxCodeType1) &&
            (_tktFareDataInd1 == rhs._tktFareDataInd1) && (_owrt1 == rhs._owrt1) &&
            (_seasonType1 == rhs._seasonType1) && (_dowType1 == rhs._dowType1) &&
            (_tourBoxCode1 == rhs._tourBoxCode1) && (_tktDesignator1 == rhs._tktDesignator1) &&
            (_globalDir1 == rhs._globalDir1) && (_carrier11 == rhs._carrier11) &&
            (_rule1 == rhs._rule1) && (_fareClass1 == rhs._fareClass1) &&
            (_fareType1 == rhs._fareType1) && (_betwCity1 == rhs._betwCity1) &&
            (_andCity1 == rhs._andCity1) && (_fareBoxText1 == rhs._fareBoxText1) &&
            (_cur11 == rhs._cur11) && (_couponInd2 == rhs._couponInd2) &&
            (_tourBoxCodeType2 == rhs._tourBoxCodeType2) &&
            (_tktFareDataInd2 == rhs._tktFareDataInd2) && (_owrt2 == rhs._owrt2) &&
            (_seasonType2 == rhs._seasonType2) && (_dowType2 == rhs._dowType2) &&
            (_tourBoxCode2 == rhs._tourBoxCode2) && (_tktDesignator2 == rhs._tktDesignator2) &&
            (_globalDir2 == rhs._globalDir2) && (_carrier21 == rhs._carrier21) &&
            (_rule2 == rhs._rule2) && (_fareClass2 == rhs._fareClass2) &&
            (_fareType2 == rhs._fareType2) && (_betwCity2 == rhs._betwCity2) &&
            (_andCity2 == rhs._andCity2) && (_fareBoxText2 == rhs._fareBoxText2) &&
            (_cur21 == rhs._cur21) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(NegFareRest& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._negFareSecurityTblItemNo = 1;
    obj._negFareCalcTblItemNo = 2;
    obj._fareAmt1 = 3.33;
    obj._fareAmt2 = 4.44;
    obj._commAmt1 = 5.55;
    obj._commAmt2 = 6.66;
    obj._commPercent = 7.777;
    obj._commPercentNoDec = 8;
    obj._noDec1 = 9;
    obj._noDec2 = 10;
    obj._noSegs = 11;
    obj._noDec11 = 12;
    obj._ruleTariff1 = 13;
    obj._noDec21 = 14;
    obj._ruleTariff2 = 15;
    obj._netRemitMethod = 'B';
    obj._netGrossInd = 'C';
    obj._upgradeInd = 'D';
    obj._bagTypeInd = 'E';
    obj._validationInd = 'F';
    obj._tktAppl = 'G';
    obj._psgType = "HIJ";
    obj._cur1 = "KLM";
    obj._cur2 = "NOP";
    obj._bagNo = "aaaaaaaa";
    obj._carrier = "QRS";
    obj._couponInd1 = 'T';
    obj._tourBoxCodeType1 = 'U';
    obj._tktFareDataInd1 = 'V';
    obj._owrt1 = 'W';
    obj._seasonType1 = 'X';
    obj._dowType1 = 'Y';
    obj._tourBoxCode1 = "bbbbbbbb";
    obj._tktDesignator1 = "abcdefghij";
    obj._globalDir1 = GlobalDirection::US;
    obj._carrier11 = "klm";
    obj._rule1 = "nopq";
    obj._fareClass1 = "rstuvwxy";
    obj._fareType1 = "z1234567";
    obj._betwCity1 = "890ABCDE";
    obj._andCity1 = "FGHIJKLM";
    obj._fareBoxText1 = "cccccccc";
    obj._cur11 = "NOP";
    obj._couponInd2 = 'R';
    obj._tourBoxCodeType2 = 'S';
    obj._tktFareDataInd2 = 'T';
    obj._owrt2 = 'U';
    obj._seasonType2 = 'V';
    obj._dowType2 = 'W';
    obj._tourBoxCode2 = "dddddddd";
    obj._tktDesignator2 = "XYZabcdefg";
    obj._globalDir2 = GlobalDirection::US;
    obj._carrier21 = "hij";
    obj._rule2 = "klmn";
    obj._fareClass2 = "opqrstuv";
    obj._fareType2 = "wxyz1234";
    obj._betwCity2 = "567890AB";
    obj._andCity2 = "CDEFGHIJ";
    obj._fareBoxText2 = "eeeeeeee";
    obj._cur21 = "KLM";
    obj._inhibit = 'N';
  }
  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailTag = ' ';
  int _negFareSecurityTblItemNo = 0;
  int _negFareCalcTblItemNo = 0;
  MoneyAmount _fareAmt1 = 0;
  MoneyAmount _fareAmt2 = 0;
  MoneyAmount _commAmt1 = 0;
  MoneyAmount _commAmt2 = 0;
  Percent _commPercent = 0;
  int _commPercentNoDec = 0;
  int _noDec1 = 0;
  int _noDec2 = 0;
  int _noSegs = 0;
  int _noDec11 = 0;
  TariffNumber _ruleTariff1 = 0;
  int _noDec21 = 0;
  TariffNumber _ruleTariff2 = 0;
  Indicator _netRemitMethod = ' ';
  Indicator _netGrossInd = ' ';
  Indicator _upgradeInd = ' ';
  Indicator _bagTypeInd = ' ';
  Indicator _validationInd = ' ';
  Indicator _tktAppl = ' ';
  PaxTypeCode _psgType;
  CurrencyCode _cur1;
  CurrencyCode _cur2;
  std::string _bagNo;
  CarrierCode _carrier;
  Indicator _couponInd1 = ' ';
  Indicator _tourBoxCodeType1 = ' ';
  Indicator _tktFareDataInd1 = ' ';
  Indicator _owrt1 = ' ';
  Indicator _seasonType1 = ' ';
  Indicator _dowType1 = ' ';
  std::string _tourBoxCode1;
  TktDesignator _tktDesignator1;
  GlobalDirection _globalDir1 = GlobalDirection::NO_DIR;
  CarrierCode _carrier11;
  RuleNumber _rule1;
  FareClassCode _fareClass1;
  FareType _fareType1;
  LocCode _betwCity1;
  LocCode _andCity1;
  std::string _fareBoxText1;
  CurrencyCode _cur11;
  Indicator _couponInd2 = ' ';
  Indicator _tourBoxCodeType2 = ' ';
  Indicator _tktFareDataInd2 = ' ';
  Indicator _owrt2 = ' ';
  Indicator _seasonType2 = ' ';
  Indicator _dowType2 = ' ';
  std::string _tourBoxCode2;
  TktDesignator _tktDesignator2;
  GlobalDirection _globalDir2 = GlobalDirection::NO_DIR;
  CarrierCode _carrier21;
  RuleNumber _rule2;
  FareClassCode _fareClass2;
  FareType _fareType2;
  LocCode _betwCity2;
  LocCode _andCity2;
  std::string _fareBoxText2;
  CurrencyCode _cur21;
  Indicator _inhibit = ' ';

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _negFareSecurityTblItemNo);
    FLATTENIZE(archive, _negFareCalcTblItemNo);
    FLATTENIZE(archive, _fareAmt1);
    FLATTENIZE(archive, _fareAmt2);
    FLATTENIZE(archive, _commAmt1);
    FLATTENIZE(archive, _commAmt2);
    FLATTENIZE(archive, _commPercent);
    FLATTENIZE(archive, _commPercentNoDec);
    FLATTENIZE(archive, _noDec1);
    FLATTENIZE(archive, _noDec2);
    FLATTENIZE(archive, _noSegs);
    FLATTENIZE(archive, _noDec11);
    FLATTENIZE(archive, _ruleTariff1);
    FLATTENIZE(archive, _noDec21);
    FLATTENIZE(archive, _ruleTariff2);
    FLATTENIZE(archive, _netRemitMethod);
    FLATTENIZE(archive, _netGrossInd);
    FLATTENIZE(archive, _upgradeInd);
    FLATTENIZE(archive, _bagTypeInd);
    FLATTENIZE(archive, _validationInd);
    FLATTENIZE(archive, _tktAppl);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _cur1);
    FLATTENIZE(archive, _cur2);
    FLATTENIZE(archive, _bagNo);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _couponInd1);
    FLATTENIZE(archive, _tourBoxCodeType1);
    FLATTENIZE(archive, _tktFareDataInd1);
    FLATTENIZE(archive, _owrt1);
    FLATTENIZE(archive, _seasonType1);
    FLATTENIZE(archive, _dowType1);
    FLATTENIZE(archive, _tourBoxCode1);
    FLATTENIZE(archive, _tktDesignator1);
    FLATTENIZE(archive, _globalDir1);
    FLATTENIZE(archive, _carrier11);
    FLATTENIZE(archive, _rule1);
    FLATTENIZE(archive, _fareClass1);
    FLATTENIZE(archive, _fareType1);
    FLATTENIZE(archive, _betwCity1);
    FLATTENIZE(archive, _andCity1);
    FLATTENIZE(archive, _fareBoxText1);
    FLATTENIZE(archive, _cur11);
    FLATTENIZE(archive, _couponInd2);
    FLATTENIZE(archive, _tourBoxCodeType2);
    FLATTENIZE(archive, _tktFareDataInd2);
    FLATTENIZE(archive, _owrt2);
    FLATTENIZE(archive, _seasonType2);
    FLATTENIZE(archive, _dowType2);
    FLATTENIZE(archive, _tourBoxCode2);
    FLATTENIZE(archive, _tktDesignator2);
    FLATTENIZE(archive, _globalDir2);
    FLATTENIZE(archive, _carrier21);
    FLATTENIZE(archive, _rule2);
    FLATTENIZE(archive, _fareClass2);
    FLATTENIZE(archive, _fareType2);
    FLATTENIZE(archive, _betwCity2);
    FLATTENIZE(archive, _andCity2);
    FLATTENIZE(archive, _fareBoxText2);
    FLATTENIZE(archive, _cur21);
    FLATTENIZE(archive, _inhibit);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer & ptr->_createDate & ptr->_expireDate & ptr->_unavailTag &
           ptr->_negFareSecurityTblItemNo & ptr->_negFareCalcTblItemNo & ptr->_fareAmt1 &
           ptr->_fareAmt2 & ptr->_commAmt1 & ptr->_commAmt2 & ptr->_commPercent &
           ptr->_commPercentNoDec & ptr->_noDec1 & ptr->_noDec2 & ptr->_noSegs & ptr->_noDec11 &
           ptr->_ruleTariff1 & ptr->_noDec21 & ptr->_ruleTariff2 & ptr->_netRemitMethod &
           ptr->_netGrossInd & ptr->_upgradeInd & ptr->_bagTypeInd & ptr->_validationInd &
           ptr->_tktAppl & ptr->_psgType & ptr->_cur1 & ptr->_cur2 & ptr->_bagNo & ptr->_carrier &
           ptr->_couponInd1 & ptr->_tourBoxCodeType1 & ptr->_tktFareDataInd1 & ptr->_owrt1 &
           ptr->_seasonType1 & ptr->_dowType1 & ptr->_tourBoxCode1 & ptr->_tktDesignator1 &
           ptr->_globalDir1 & ptr->_carrier11 & ptr->_rule1 & ptr->_fareClass1 & ptr->_fareType1 &
           ptr->_betwCity1 & ptr->_andCity1 & ptr->_fareBoxText1 & ptr->_cur11 & ptr->_couponInd2 &
           ptr->_tourBoxCodeType2 & ptr->_tktFareDataInd2 & ptr->_owrt2 & ptr->_seasonType2 &
           ptr->_dowType2 & ptr->_tourBoxCode2 & ptr->_tktDesignator2 & ptr->_globalDir2 &
           ptr->_carrier21 & ptr->_rule2 & ptr->_fareClass2 & ptr->_fareType2 & ptr->_betwCity2 &
           ptr->_andCity2 & ptr->_fareBoxText2 & ptr->_cur21 & ptr->_inhibit;
  }
};
}
