//-------------------------------------------------------------------
//
//  File:       CombinabilityRuleInfo.h
//  Authors:    Devapriya SenGupta
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{
class CombinabilityRuleInfo final
{
public:
  typedef CombinabilityRuleItemInfo item_info_type;

  CombinabilityRuleInfo()
    : _expireDate(_cri.createDate()), _effDate(_cri.createDate()), _discDate(_cri.createDate())
  {
  }

  CombinabilityRuleInfo(const CombinabilityRuleInfo&) = delete;
  CombinabilityRuleInfo& operator=(const CombinabilityRuleInfo&) = delete;

  virtual ~CombinabilityRuleInfo() = default;

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  uint32_t& segCnt() { return _segCnt; }
  const uint32_t& segCnt() const { return _segCnt; }

  uint32_t& jointCarrierTblItemNo() { return _jointCarrierTblItemNo; }
  const uint32_t& jointCarrierTblItemNo() const { return _jointCarrierTblItemNo; }

  uint32_t& samepointstblItemNo() { return _samepointstblItemNo; }
  const uint32_t& samepointstblItemNo() const { return _samepointstblItemNo; }

  TariffNumber& dojGeneralRuleTariff() { return _dojGeneralRuleTariff; }
  const TariffNumber& dojGeneralRuleTariff() const { return _dojGeneralRuleTariff; }

  TariffNumber& ct2GeneralRuleTariff() { return _ct2GeneralRuleTariff; }
  const TariffNumber& ct2GeneralRuleTariff() const { return _ct2GeneralRuleTariff; }

  TariffNumber& ct2plusGeneralRuleTariff() { return _ct2plusGeneralRuleTariff; }
  const TariffNumber& ct2plusGeneralRuleTariff() const { return _ct2plusGeneralRuleTariff; }

  TariffNumber& eoeGeneralRuleTariff() { return _eoeGeneralRuleTariff; }
  const TariffNumber& eoeGeneralRuleTariff() const { return _eoeGeneralRuleTariff; }

  TariffNumber& arbGeneralRuleTariff() { return _arbGeneralRuleTariff; }
  const TariffNumber& arbGeneralRuleTariff() const { return _arbGeneralRuleTariff; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  Indicator& routingAppl() { return _routingAppl; }
  const Indicator& routingAppl() const { return _routingAppl; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  Footnote& footNote1() { return _footNote1; }
  const Footnote& footNote1() const { return _footNote1; }

  Footnote& footNote2() { return _footNote2; }
  const Footnote& footNote2() const { return _footNote2; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  Indicator& seasonType() { return _seasonType; }
  const Indicator& seasonType() const { return _seasonType; }

  Indicator& dowType() { return _dowType; }
  const Indicator& dowType() const { return _dowType; }

  std::string& batchci() { return _batchci; }
  const std::string& batchci() const { return _batchci; }

  Indicator& sojInd() { return _sojInd; }
  const Indicator& sojInd() const { return _sojInd; }

  Indicator& sojorigIndestInd() { return _sojorigIndestInd; }
  const Indicator& sojorigIndestInd() const { return _sojorigIndestInd; }

  Indicator& dojInd() { return _dojInd; }
  const Indicator& dojInd() const { return _dojInd; }

  Indicator& dojCarrierRestInd() { return _dojCarrierRestInd; }
  const Indicator& dojCarrierRestInd() const { return _dojCarrierRestInd; }

  Indicator& dojTariffRuleRestInd() { return _dojTariffRuleRestInd; }
  const Indicator& dojTariffRuleRestInd() const { return _dojTariffRuleRestInd; }

  Indicator& dojFareClassTypeRestInd() { return _dojFareClassTypeRestInd; }
  const Indicator& dojFareClassTypeRestInd() const { return _dojFareClassTypeRestInd; }

  RuleNumber& dojGeneralRule() { return _dojGeneralRule; }
  const RuleNumber& dojGeneralRule() const { return _dojGeneralRule; }

  Indicator& dojGeneralRuleAppl() { return _dojGeneralRuleAppl; }
  const Indicator& dojGeneralRuleAppl() const { return _dojGeneralRuleAppl; }

  Indicator& ct2Ind() { return _ct2Ind; }
  const Indicator& ct2Ind() const { return _ct2Ind; }

  Indicator& ct2CarrierRestInd() { return _ct2CarrierRestInd; }
  const Indicator& ct2CarrierRestInd() const { return _ct2CarrierRestInd; }

  Indicator& ct2TariffRuleRestInd() { return _ct2TariffRuleRestInd; }
  const Indicator& ct2TariffRuleRestInd() const { return _ct2TariffRuleRestInd; }

  Indicator& ct2FareClassTypeRestInd() { return _ct2FareClassTypeRestInd; }
  const Indicator& ct2FareClassTypeRestInd() const { return _ct2FareClassTypeRestInd; }

  RuleNumber& ct2GeneralRule() { return _ct2GeneralRule; }
  const RuleNumber& ct2GeneralRule() const { return _ct2GeneralRule; }

  Indicator& ct2GeneralRuleAppl() { return _ct2GeneralRuleAppl; }
  const Indicator& ct2GeneralRuleAppl() const { return _ct2GeneralRuleAppl; }

  Indicator& ct2plusInd() { return _ct2plusInd; }
  const Indicator& ct2plusInd() const { return _ct2plusInd; }

  Indicator& ct2plusCarrierRestInd() { return _ct2plusCarrierRestInd; }
  const Indicator& ct2plusCarrierRestInd() const { return _ct2plusCarrierRestInd; }

  Indicator& ct2plusTariffRuleRestInd() { return _ct2plusTariffRuleRestInd; }
  const Indicator& ct2plusTariffRuleRestInd() const { return _ct2plusTariffRuleRestInd; }

  Indicator& ct2plusFareClassTypeRestInd() { return _ct2plusFareClassTypeRestInd; }
  const Indicator& ct2plusFareClassTypeRestInd() const { return _ct2plusFareClassTypeRestInd; }

  RuleNumber& ct2plusGeneralRule() { return _ct2plusGeneralRule; }
  const RuleNumber& ct2plusGeneralRule() const { return _ct2plusGeneralRule; }

  Indicator& ct2plusGeneralRuleAppl() { return _ct2plusGeneralRuleAppl; }
  const Indicator& ct2plusGeneralRuleAppl() const { return _ct2plusGeneralRuleAppl; }

  Indicator& eoeInd() { return _eoeInd; }
  const Indicator& eoeInd() const { return _eoeInd; }

  Indicator& eoeCarrierRestInd() { return _eoeCarrierRestInd; }
  const Indicator& eoeCarrierRestInd() const { return _eoeCarrierRestInd; }

  Indicator& eoeTariffRuleRestInd() { return _eoeTariffRuleRestInd; }
  const Indicator& eoeTariffRuleRestInd() const { return _eoeTariffRuleRestInd; }

  Indicator& eoeFareClassTypeRestInd() { return _eoeFareClassTypeRestInd; }
  const Indicator& eoeFareClassTypeRestInd() const { return _eoeFareClassTypeRestInd; }

  RuleNumber& eoeGeneralRule() { return _eoeGeneralRule; }
  const RuleNumber& eoeGeneralRule() const { return _eoeGeneralRule; }

  Indicator& eoeGeneralRuleAppl() { return _eoeGeneralRuleAppl; }
  const Indicator& eoeGeneralRuleAppl() const { return _eoeGeneralRuleAppl; }

  Indicator& arbInd() { return _arbInd; }
  const Indicator& arbInd() const { return _arbInd; }

  Indicator& arbCarrierRestInd() { return _arbCarrierRestInd; }
  const Indicator& arbCarrierRestInd() const { return _arbCarrierRestInd; }

  Indicator& arbTariffRuleRestInd() { return _arbTariffRuleRestInd; }
  const Indicator& arbTariffRuleRestInd() const { return _arbTariffRuleRestInd; }

  Indicator& arbFareClassTypeRestInd() { return _arbFareClassTypeRestInd; }
  const Indicator& arbFareClassTypeRestInd() const { return _arbFareClassTypeRestInd; }

  RuleNumber& arbGeneralRule() { return _arbGeneralRule; }
  const RuleNumber& arbGeneralRule() const { return _arbGeneralRule; }

  Indicator& arbGeneralRuleAppl() { return _arbGeneralRuleAppl; }
  const Indicator& arbGeneralRuleAppl() const { return _arbGeneralRuleAppl; }

  Indicator& versioninheritedInd() { return _versioninheritedInd; }
  const Indicator& versioninheritedInd() const { return _versioninheritedInd; }

  Indicator& versionDisplayInd() { return _versionDisplayInd; }
  const Indicator& versionDisplayInd() const { return _versionDisplayInd; }

  LocKey& locKey1() { return _locKey1; }
  const LocKey& locKey1() const { return _locKey1; }

  LocKey& locKey2() { return _locKey2; }
  const LocKey& locKey2() const { return _locKey2; }

  // 101
  Indicator& dojSameCarrierInd() { return _dojSameCarrierInd; }
  const Indicator& dojSameCarrierInd() const { return _dojSameCarrierInd; }

  Indicator& dojSameRuleTariffInd() { return _dojSameRuleTariffInd; }
  const Indicator& dojSameRuleTariffInd() const { return _dojSameRuleTariffInd; }

  Indicator& dojSameFareInd() { return _dojSameFareInd; }
  const Indicator& dojSameFareInd() const { return _dojSameFareInd; }

  // 102
  Indicator& ct2SameCarrierInd() { return _ct2SameCarrierInd; }
  const Indicator& ct2SameCarrierInd() const { return _ct2SameCarrierInd; }

  Indicator& ct2SameRuleTariffInd() { return _ct2SameRuleTariffInd; }
  const Indicator& ct2SameRuleTariffInd() const { return _ct2SameRuleTariffInd; }

  Indicator& ct2SameFareInd() { return _ct2SameFareInd; }
  const Indicator& ct2SameFareInd() const { return _ct2SameFareInd; }

  // 103
  Indicator& ct2plusSameCarrierInd() { return _ct2plusSameCarrierInd; }
  const Indicator& ct2plusSameCarrierInd() const { return _ct2plusSameCarrierInd; }

  Indicator& ct2plusSameRuleTariffInd() { return _ct2plusSameRuleTariffInd; }
  const Indicator& ct2plusSameRuleTariffInd() const { return _ct2plusSameRuleTariffInd; }

  Indicator& ct2plusSameFareInd() { return _ct2plusSameFareInd; }
  const Indicator& ct2plusSameFareInd() const { return _ct2plusSameFareInd; }

  // 104
  Indicator& eoeSameCarrierInd() { return _eoeSameCarrierInd; }
  const Indicator& eoeSameCarrierInd() const { return _eoeSameCarrierInd; }

  Indicator& eoeSameRuleTariffInd() { return _eoeSameRuleTariffInd; }
  const Indicator& eoeSameRuleTariffInd() const { return _eoeSameRuleTariffInd; }

  Indicator& eoeSameFareInd() { return _eoeSameFareInd; }
  const Indicator& eoeSameFareInd() const { return _eoeSameFareInd; }

  // from CategoryRuleInfoT

  uint16_t& categoryNumber() { return _cri.categoryNumber(); }
  const uint16_t& categoryNumber() const { return _cri.categoryNumber(); }

  SequenceNumberLong& sequenceNumber() { return _cri.sequenceNumber(); }
  SequenceNumberLong sequenceNumber() const { return _cri.sequenceNumber(); }

  VendorCode& vendorCode() { return _cri.vendorCode(); }
  const VendorCode& vendorCode() const { return _cri.vendorCode(); }

  TariffNumber& tariffNumber() { return _cri.tariffNumber(); }
  const TariffNumber& tariffNumber() const { return _cri.tariffNumber(); }

  CarrierCode& carrierCode(){ return _cri.carrierCode(); }
  const CarrierCode& carrierCode() const { return _cri.carrierCode(); }

  DateTime& createDate() { return _cri.createDate(); }
  const DateTime& createDate() const { return _cri.createDate(); }

  LocKey& loc1() { return _cri.loc1(); }
  const LocKey& loc1() const { return _cri.loc1(); }

  LocKey& loc2() { return _cri.loc2(); }
  const LocKey& loc2() const { return _cri.loc2(); }

  RuleNumber& ruleNumber() { return _cri.ruleNumber(); }
  const RuleNumber& ruleNumber() const { return _cri.ruleNumber(); }

  const std::vector<CombinabilityRuleItemInfoSet*>& categoryRuleItemInfoSet() const
  {
    return _cri.categoryRuleItemInfoSet();
  }

  void addItemInfoSetNosync(CombinabilityRuleItemInfoSet* s) { _cri.addItemInfoSetNosync(s); }

  void sync_with_cache() { _cri.sync_with_cache(); }

  Indicator& applInd() { return _cri.applInd(); }
  const Indicator& applInd() const { return _cri.applInd(); }

  bool hasCat(unsigned catNumber) const { return _cri.hasCat(catNumber); }

  bool hasCatStopovers() const { return _cri.hasCatStopovers(); }

  bool hasCatTransfers() const { return _cri.hasCatTransfers(); }

  // end from CategoryRuleInfoT

  bool operator==(const CombinabilityRuleInfo& rhs) const
  {
    return (_cri == rhs._cri) && (_expireDate == rhs._expireDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_segCnt == rhs._segCnt) && (_jointCarrierTblItemNo == rhs._jointCarrierTblItemNo) &&
            (_samepointstblItemNo == rhs._samepointstblItemNo) &&
            (_dojGeneralRuleTariff == rhs._dojGeneralRuleTariff) &&
            (_ct2GeneralRuleTariff == rhs._ct2GeneralRuleTariff) &&
            (_ct2plusGeneralRuleTariff == rhs._ct2plusGeneralRuleTariff) &&
            (_eoeGeneralRuleTariff == rhs._eoeGeneralRuleTariff) &&
            (_arbGeneralRuleTariff == rhs._arbGeneralRuleTariff) &&
            (_validityInd == rhs._validityInd) && (_inhibit == rhs._inhibit) &&
            (_locKey1 == rhs._locKey1) && (_locKey2 == rhs._locKey2) &&
            (_fareClass == rhs._fareClass) && (_owrt == rhs._owrt) &&
            (_routingAppl == rhs._routingAppl) && (_routing == rhs._routing) &&
            (_footNote1 == rhs._footNote1) && (_footNote2 == rhs._footNote2) &&
            (_fareType == rhs._fareType) && (_seasonType == rhs._seasonType) &&
            (_dowType == rhs._dowType) && (_batchci == rhs._batchci) && (_sojInd == rhs._sojInd) &&
            (_sojorigIndestInd == rhs._sojorigIndestInd) && (_dojInd == rhs._dojInd) &&
            (_dojCarrierRestInd == rhs._dojCarrierRestInd) &&
            (_dojTariffRuleRestInd == rhs._dojTariffRuleRestInd) &&
            (_dojFareClassTypeRestInd == rhs._dojFareClassTypeRestInd) &&
            (_dojGeneralRule == rhs._dojGeneralRule) &&
            (_dojGeneralRuleAppl == rhs._dojGeneralRuleAppl) && (_ct2Ind == rhs._ct2Ind) &&
            (_ct2CarrierRestInd == rhs._ct2CarrierRestInd) &&
            (_ct2TariffRuleRestInd == rhs._ct2TariffRuleRestInd) &&
            (_ct2FareClassTypeRestInd == rhs._ct2FareClassTypeRestInd) &&
            (_ct2GeneralRule == rhs._ct2GeneralRule) &&
            (_ct2GeneralRuleAppl == rhs._ct2GeneralRuleAppl) && (_ct2plusInd == rhs._ct2plusInd) &&
            (_ct2plusCarrierRestInd == rhs._ct2plusCarrierRestInd) &&
            (_ct2plusTariffRuleRestInd == rhs._ct2plusTariffRuleRestInd) &&
            (_ct2plusFareClassTypeRestInd == rhs._ct2plusFareClassTypeRestInd) &&
            (_ct2plusGeneralRule == rhs._ct2plusGeneralRule) &&
            (_ct2plusGeneralRuleAppl == rhs._ct2plusGeneralRuleAppl) && (_eoeInd == rhs._eoeInd) &&
            (_eoeCarrierRestInd == rhs._eoeCarrierRestInd) &&
            (_eoeTariffRuleRestInd == rhs._eoeTariffRuleRestInd) &&
            (_eoeFareClassTypeRestInd == rhs._eoeFareClassTypeRestInd) &&
            (_eoeGeneralRule == rhs._eoeGeneralRule) &&
            (_eoeGeneralRuleAppl == rhs._eoeGeneralRuleAppl) && (_arbInd == rhs._arbInd) &&
            (_arbCarrierRestInd == rhs._arbCarrierRestInd) &&
            (_arbTariffRuleRestInd == rhs._arbTariffRuleRestInd) &&
            (_arbFareClassTypeRestInd == rhs._arbFareClassTypeRestInd) &&
            (_arbGeneralRule == rhs._arbGeneralRule) &&
            (_arbGeneralRuleAppl == rhs._arbGeneralRuleAppl) &&
            (_versioninheritedInd == rhs._versioninheritedInd) &&
            (_versionDisplayInd == rhs._versionDisplayInd) &&
            (_dojSameCarrierInd == rhs._dojSameCarrierInd) &&
            (_dojSameRuleTariffInd == rhs._dojSameRuleTariffInd) &&
            (_dojSameFareInd == rhs._dojSameFareInd) &&
            (_ct2SameCarrierInd == rhs._ct2SameCarrierInd) &&
            (_ct2SameRuleTariffInd == rhs._ct2SameRuleTariffInd) &&
            (_ct2SameFareInd == rhs._ct2SameFareInd) &&
            (_ct2plusSameCarrierInd == rhs._ct2plusSameCarrierInd) &&
            (_ct2plusSameRuleTariffInd == rhs._ct2plusSameRuleTariffInd) &&
            (_ct2plusSameFareInd == rhs._ct2plusSameFareInd) &&
            (_eoeSameCarrierInd == rhs._eoeSameCarrierInd) &&
            (_eoeSameRuleTariffInd == rhs._eoeSameRuleTariffInd) &&
            (_eoeSameFareInd == rhs._eoeSameFareInd);
  }

  static void dummyData(CombinabilityRuleInfo& obj)
  {
    CategoryRuleInfoT<CombinabilityRuleItemInfo>::dummyData(obj._cri);

    obj._expireDate = DateTime(2010, 04, 01, 13, 45, 30, 10);
    obj._effDate = DateTime(2010, 04, 01, 13, 45, 30, 10);
    obj._discDate = DateTime(2010, 04, 01, 13, 45, 30, 10);
    obj._segCnt = 1;
    obj._jointCarrierTblItemNo = 2;
    obj._samepointstblItemNo = 3;
    obj._dojGeneralRuleTariff = 4;
    obj._ct2GeneralRuleTariff = 5;
    obj._ct2plusGeneralRuleTariff = 6;
    obj._eoeGeneralRuleTariff = 7;
    obj._arbGeneralRuleTariff = 8;
    obj._validityInd = 'A';
    obj._inhibit = 'B';

    LocKey::dummyData(obj._locKey1);
    LocKey::dummyData(obj._locKey2);

    obj._fareClass = "aaaaaaaa";
    obj._owrt = 'C';
    obj._routingAppl = 'D';
    obj._routing = "EFGH";
    obj._footNote1 = "IJ";
    obj._footNote2 = "KL";
    obj._fareType = "MNOPQRST";
    obj._seasonType = 'U';
    obj._dowType = 'V';
    obj._batchci = "bbbbbbbb";
    obj._sojInd = 'W';
    obj._sojorigIndestInd = 'X';
    obj._dojInd = 'Y';
    obj._dojCarrierRestInd = 'Z';
    obj._dojTariffRuleRestInd = 'a';
    obj._dojFareClassTypeRestInd = 'b';
    obj._dojGeneralRule = "cdef";
    obj._dojGeneralRuleAppl = 'g';
    obj._ct2Ind = 'h';
    obj._ct2CarrierRestInd = 'i';
    obj._ct2TariffRuleRestInd = 'j';
    obj._ct2FareClassTypeRestInd = 'k';
    obj._ct2GeneralRule = "lmno";
    obj._ct2GeneralRuleAppl = 'p';
    obj._ct2plusInd = 'q';
    obj._ct2plusCarrierRestInd = 'r';
    obj._ct2plusTariffRuleRestInd = 's';
    obj._ct2plusFareClassTypeRestInd = 't';
    obj._ct2plusGeneralRule = "uvwx";
    obj._ct2plusGeneralRuleAppl = 'y';
    obj._eoeInd = 'z';
    obj._eoeCarrierRestInd = '1';
    obj._eoeTariffRuleRestInd = '2';
    obj._eoeFareClassTypeRestInd = '3';
    obj._eoeGeneralRule = "4567";
    obj._eoeGeneralRuleAppl = '8';
    obj._arbInd = '9';
    obj._arbCarrierRestInd = '0';
    obj._arbTariffRuleRestInd = 'A';
    obj._arbFareClassTypeRestInd = 'B';
    obj._arbGeneralRule = "CDEF";
    obj._arbGeneralRuleAppl = 'G';
    obj._versioninheritedInd = 'H';
    obj._versionDisplayInd = 'I';
    obj._dojSameCarrierInd = 'J';
    obj._dojSameRuleTariffInd = 'K';
    obj._dojSameFareInd = 'L';
    obj._ct2SameCarrierInd = 'M';
    obj._ct2SameRuleTariffInd = 'N';
    obj._ct2SameFareInd = 'O';
    obj._ct2plusSameCarrierInd = 'P';
    obj._ct2plusSameRuleTariffInd = 'Q';
    obj._ct2plusSameFareInd = 'R';
    obj._eoeSameCarrierInd = 'S';
    obj._eoeSameRuleTariffInd = 'T';
    obj._eoeSameFareInd = 'U';
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

private:
  CategoryRuleInfoT<CombinabilityRuleItemInfo> _cri;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  uint32_t _segCnt = 0;
  uint32_t _jointCarrierTblItemNo = 0;
  uint32_t _samepointstblItemNo = 0;
  TariffNumber _dojGeneralRuleTariff = 0;
  TariffNumber _ct2GeneralRuleTariff = 0;
  TariffNumber _ct2plusGeneralRuleTariff = 0;
  TariffNumber _eoeGeneralRuleTariff = 0;
  TariffNumber _arbGeneralRuleTariff = 0;
  Indicator _validityInd = ' ';
  Indicator _inhibit = ' ';
  LocKey _locKey1;
  LocKey _locKey2;
  FareClassCode _fareClass;
  Indicator _owrt = ' ';
  Indicator _routingAppl = ' ';
  RoutingNumber _routing;
  Footnote _footNote1;
  Footnote _footNote2;
  FareType _fareType;
  Indicator _seasonType = ' ';
  Indicator _dowType = ' ';
  std::string _batchci;
  Indicator _sojInd = ' ';
  Indicator _sojorigIndestInd = ' ';
  Indicator _dojInd = ' ';
  Indicator _dojCarrierRestInd = ' ';
  Indicator _dojTariffRuleRestInd = ' ';
  Indicator _dojFareClassTypeRestInd = ' ';
  RuleNumber _dojGeneralRule;
  Indicator _dojGeneralRuleAppl = ' ';
  Indicator _ct2Ind = ' ';
  Indicator _ct2CarrierRestInd = ' ';
  Indicator _ct2TariffRuleRestInd = ' ';
  Indicator _ct2FareClassTypeRestInd = ' ';
  RuleNumber _ct2GeneralRule;
  Indicator _ct2GeneralRuleAppl = ' ';
  Indicator _ct2plusInd = ' ';
  Indicator _ct2plusCarrierRestInd = ' ';
  Indicator _ct2plusTariffRuleRestInd = ' ';
  Indicator _ct2plusFareClassTypeRestInd = ' ';
  RuleNumber _ct2plusGeneralRule;
  Indicator _ct2plusGeneralRuleAppl = ' ';
  Indicator _eoeInd = ' ';
  Indicator _eoeCarrierRestInd = ' ';
  Indicator _eoeTariffRuleRestInd = ' ';
  Indicator _eoeFareClassTypeRestInd = ' ';
  RuleNumber _eoeGeneralRule;
  Indicator _eoeGeneralRuleAppl = ' ';
  Indicator _arbInd = ' ';
  Indicator _arbCarrierRestInd = ' ';
  Indicator _arbTariffRuleRestInd = ' ';
  Indicator _arbFareClassTypeRestInd = ' ';
  RuleNumber _arbGeneralRule;
  Indicator _arbGeneralRuleAppl = ' ';
  Indicator _versioninheritedInd = ' ';
  Indicator _versionDisplayInd = ' ';

  // 101
  Indicator _dojSameCarrierInd = ' '; // 106
  Indicator _dojSameRuleTariffInd = ' '; // 107
  Indicator _dojSameFareInd = ' '; // 108

  // 102
  Indicator _ct2SameCarrierInd = ' ';
  Indicator _ct2SameRuleTariffInd = ' ';
  Indicator _ct2SameFareInd = ' ';

  // 103
  Indicator _ct2plusSameCarrierInd = ' ';
  Indicator _ct2plusSameRuleTariffInd = ' ';
  Indicator _ct2plusSameFareInd = ' ';

  // 104
  Indicator _eoeSameCarrierInd = ' ';
  Indicator _eoeSameRuleTariffInd = ' ';
  Indicator _eoeSameFareInd = ' ';

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _cri);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _jointCarrierTblItemNo);
    FLATTENIZE(archive, _samepointstblItemNo);
    FLATTENIZE(archive, _dojGeneralRuleTariff);
    FLATTENIZE(archive, _ct2GeneralRuleTariff);
    FLATTENIZE(archive, _ct2plusGeneralRuleTariff);
    FLATTENIZE(archive, _eoeGeneralRuleTariff);
    FLATTENIZE(archive, _arbGeneralRuleTariff);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _locKey1);
    FLATTENIZE(archive, _locKey2);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _routingAppl);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _footNote1);
    FLATTENIZE(archive, _footNote2);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _seasonType);
    FLATTENIZE(archive, _dowType);
    FLATTENIZE(archive, _batchci);
    FLATTENIZE(archive, _sojInd);
    FLATTENIZE(archive, _sojorigIndestInd);
    FLATTENIZE(archive, _dojInd);
    FLATTENIZE(archive, _dojCarrierRestInd);
    FLATTENIZE(archive, _dojTariffRuleRestInd);
    FLATTENIZE(archive, _dojFareClassTypeRestInd);
    FLATTENIZE(archive, _dojGeneralRule);
    FLATTENIZE(archive, _dojGeneralRuleAppl);
    FLATTENIZE(archive, _ct2Ind);
    FLATTENIZE(archive, _ct2CarrierRestInd);
    FLATTENIZE(archive, _ct2TariffRuleRestInd);
    FLATTENIZE(archive, _ct2FareClassTypeRestInd);
    FLATTENIZE(archive, _ct2GeneralRule);
    FLATTENIZE(archive, _ct2GeneralRuleAppl);
    FLATTENIZE(archive, _ct2plusInd);
    FLATTENIZE(archive, _ct2plusCarrierRestInd);
    FLATTENIZE(archive, _ct2plusTariffRuleRestInd);
    FLATTENIZE(archive, _ct2plusFareClassTypeRestInd);
    FLATTENIZE(archive, _ct2plusGeneralRule);
    FLATTENIZE(archive, _ct2plusGeneralRuleAppl);
    FLATTENIZE(archive, _eoeInd);
    FLATTENIZE(archive, _eoeCarrierRestInd);
    FLATTENIZE(archive, _eoeTariffRuleRestInd);
    FLATTENIZE(archive, _eoeFareClassTypeRestInd);
    FLATTENIZE(archive, _eoeGeneralRule);
    FLATTENIZE(archive, _eoeGeneralRuleAppl);
    FLATTENIZE(archive, _arbInd);
    FLATTENIZE(archive, _arbCarrierRestInd);
    FLATTENIZE(archive, _arbTariffRuleRestInd);
    FLATTENIZE(archive, _arbFareClassTypeRestInd);
    FLATTENIZE(archive, _arbGeneralRule);
    FLATTENIZE(archive, _arbGeneralRuleAppl);
    FLATTENIZE(archive, _versioninheritedInd);
    FLATTENIZE(archive, _versionDisplayInd);
    FLATTENIZE(archive, _dojSameCarrierInd);
    FLATTENIZE(archive, _dojSameRuleTariffInd);
    FLATTENIZE(archive, _dojSameFareInd);
    FLATTENIZE(archive, _ct2SameCarrierInd);
    FLATTENIZE(archive, _ct2SameRuleTariffInd);
    FLATTENIZE(archive, _ct2SameFareInd);
    FLATTENIZE(archive, _ct2plusSameCarrierInd);
    FLATTENIZE(archive, _ct2plusSameRuleTariffInd);
    FLATTENIZE(archive, _ct2plusSameFareInd);
    FLATTENIZE(archive, _eoeSameCarrierInd);
    FLATTENIZE(archive, _eoeSameRuleTariffInd);
    FLATTENIZE(archive, _eoeSameFareInd);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    CategoryRuleInfoT<CombinabilityRuleItemInfo>::convert(buffer, &(ptr->_cri));
    return buffer & ptr->_expireDate & ptr->_effDate & ptr->_discDate & ptr->_segCnt &
           ptr->_jointCarrierTblItemNo & ptr->_samepointstblItemNo & ptr->_dojGeneralRuleTariff &
           ptr->_ct2GeneralRuleTariff & ptr->_ct2plusGeneralRuleTariff &
           ptr->_eoeGeneralRuleTariff & ptr->_arbGeneralRuleTariff & ptr->_validityInd &
           ptr->_inhibit & ptr->_locKey1 & ptr->_locKey2 & ptr->_fareClass & ptr->_owrt &
           ptr->_routingAppl & ptr->_routing & ptr->_footNote1 & ptr->_footNote2 & ptr->_fareType &
           ptr->_seasonType & ptr->_dowType & ptr->_batchci & ptr->_sojInd &
           ptr->_sojorigIndestInd & ptr->_dojInd & ptr->_dojCarrierRestInd &
           ptr->_dojTariffRuleRestInd & ptr->_dojFareClassTypeRestInd & ptr->_dojGeneralRule &
           ptr->_dojGeneralRuleAppl & ptr->_ct2Ind & ptr->_ct2CarrierRestInd &
           ptr->_ct2TariffRuleRestInd & ptr->_ct2FareClassTypeRestInd & ptr->_ct2GeneralRule &
           ptr->_ct2GeneralRuleAppl & ptr->_ct2plusInd & ptr->_ct2plusCarrierRestInd &
           ptr->_ct2plusTariffRuleRestInd & ptr->_ct2plusFareClassTypeRestInd &
           ptr->_ct2plusGeneralRule & ptr->_ct2plusGeneralRuleAppl & ptr->_eoeInd &
           ptr->_eoeCarrierRestInd & ptr->_eoeTariffRuleRestInd & ptr->_eoeFareClassTypeRestInd &
           ptr->_eoeGeneralRule & ptr->_eoeGeneralRuleAppl & ptr->_arbInd &
           ptr->_arbCarrierRestInd & ptr->_arbTariffRuleRestInd & ptr->_arbFareClassTypeRestInd &
           ptr->_arbGeneralRule & ptr->_arbGeneralRuleAppl & ptr->_versioninheritedInd &
           ptr->_versionDisplayInd & ptr->_dojSameCarrierInd & ptr->_dojSameRuleTariffInd &
           ptr->_dojSameFareInd & ptr->_ct2SameCarrierInd & ptr->_ct2SameRuleTariffInd &
           ptr->_ct2SameFareInd & ptr->_ct2plusSameCarrierInd & ptr->_ct2plusSameRuleTariffInd &
           ptr->_ct2plusSameFareInd & ptr->_eoeSameCarrierInd & ptr->_eoeSameRuleTariffInd &
           ptr->_eoeSameFareInd;
  }
};
}
