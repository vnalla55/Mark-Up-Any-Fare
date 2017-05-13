#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TSEDateInterval.h"

namespace tse
{

class FareFocusRuleInfo
{
 public:

  FareFocusRuleInfo()
    : _fareFocusRuleId(0)
    , _fareFocusRuleFamilyId(0)
    , _statusCode(' ')
    , _processStateCode(' ')
    , _securityItemNo(0)
    , _ruleTariff(0)
    , _fareClassItemNo(0)
    , _bookingCodeItemNo(0)
    , _owrt(' ')
    , _displayType(' ')
    , _directionality('F')
    , _publicPrivateIndicator(' ')
    , _inhibitCD(' ')
    , _effectiveNowInd (' ')
    , _deactivateNowInd (' ')
    , _globalDir(GlobalDirection::NO_DIR)
    , _psgTypeItemNo(0)
    , _accountCdItemNo(0)
    , _routingItemNo(0)
    , _posDayTimeApplItemNo(0)
    , _travelDayTimeApplItemNo(0)
    , _locationPairExcludeItemNo(0)
    , _fareClassExcludeItemNo(0)
    , _fareTypeExcludeItemNo(0)
    , _displayTypeExcludeItemNo(0)
    , _psgTypeExcludeItemNo(0)
    , _accountCdExcludeItemNo(0)
    , _routingExcludeItemNo(0)  
 {
  }

  uint64_t& fareFocusRuleId() { return _fareFocusRuleId; }
  uint64_t fareFocusRuleId() const { return _fareFocusRuleId; }

  uint64_t& fareFocusRuleFamilyId() { return _fareFocusRuleFamilyId; }
  uint64_t fareFocusRuleFamilyId() const { return _fareFocusRuleFamilyId; }

  Indicator&  statusCode() { return _statusCode; }
  Indicator statusCode() const { return _statusCode; }

  Indicator& processStateCode() { return _processStateCode; }
  Indicator processStateCode() const { return _processStateCode; }

  PseudoCityCode& sourcePCC() { return _sourcePCC; }
  const PseudoCityCode& sourcePCC() const { return _sourcePCC; }

  uint64_t& securityItemNo() { return _securityItemNo; }
  uint64_t securityItemNo() const { return _securityItemNo; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  const CarrierCode& carrier() const { return _carrier; }
  CarrierCode& carrier() { return _carrier; }

  RuleNumber& ruleCode() { return _ruleCode; }
  const RuleNumber& ruleCode() const { return _ruleCode; }

  TariffNumber ruleTariff() const { return _ruleTariff; }
  TariffNumber& ruleTariff() { return _ruleTariff; }

  uint64_t& fareClassItemNo() { return _fareClassItemNo; }
  uint64_t fareClassItemNo() const { return _fareClassItemNo; }

  FareTypeAbbrevC& fareType() { return _fareType; }
  const FareTypeAbbrevC& fareType() const { return _fareType; }

  uint64_t& bookingCodeItemNo() { return _bookingCodeItemNo; }
  uint64_t bookingCodeItemNo() const { return _bookingCodeItemNo; }

  Indicator& owrt() { return _owrt; }
  Indicator owrt() const { return _owrt; }

  Indicator& displayType() { return _displayType; }
  Indicator displayType() const { return _displayType; }

  const LocKey& loc1() const { return _loc1; }
  LocKey& loc1() { return _loc1; }

  const LocKey& loc2() const { return _loc2; }
  LocKey& loc2() { return _loc2; }

  Indicator& directionality() { return _directionality; }
  Indicator directionality() const { return _directionality; }

  Indicator& publicPrivateIndicator() { return _publicPrivateIndicator; }
  Indicator publicPrivateIndicator() const { return _publicPrivateIndicator; }

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate() { return _effInterval.createDate(); }
  DateTime createDate() const { return _effInterval.createDate(); }

  DateTime& effDate() { return _effInterval.effDate(); }
  DateTime effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate() { return _effInterval.expireDate(); }
  DateTime expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate() { return _effInterval.discDate(); }
  DateTime discDate() const { return _effInterval.discDate(); }

  DateTime& lastModDate() { return _lastModDate; }
  DateTime lastModDate() const { return _lastModDate; }

  DateTime& activationDateTime() { return _activationDateTime; }
  DateTime activationDateTime() const { return _activationDateTime; }

  Indicator& inhibitCD() { return _inhibitCD; }
  Indicator inhibitCD() const { return _inhibitCD; }
   
  Indicator& effectiveNowInd() { return _effectiveNowInd; }
  Indicator effectiveNowInd() const { return _effectiveNowInd; }

  Indicator& deactivateNowInd() { return _deactivateNowInd; }
  Indicator deactivateNowInd() const { return _deactivateNowInd; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  uint64_t& psgTypeItemNo() { return _psgTypeItemNo; }
  uint64_t psgTypeItemNo() const { return _psgTypeItemNo; }

  uint64_t& accountCdItemNo() { return _accountCdItemNo; }
  uint64_t accountCdItemNo() const { return _accountCdItemNo; }
  
  uint64_t& routingItemNo() { return _routingItemNo; }
  uint64_t routingItemNo() const { return _routingItemNo; }
  
  FareRetailerCode& retailerCode() { return _retailerCode; }
  const FareRetailerCode& retailerCode() const { return _retailerCode; }

  uint64_t& posDayTimeApplItemNo() { return _posDayTimeApplItemNo; }
  uint64_t posDayTimeApplItemNo() const { return _posDayTimeApplItemNo; }
  
  uint64_t& travelDayTimeApplItemNo() { return _travelDayTimeApplItemNo; }
  uint64_t travelDayTimeApplItemNo() const { return _travelDayTimeApplItemNo; }

  uint64_t& locationPairExcludeItemNo() { return _locationPairExcludeItemNo; }
  uint64_t locationPairExcludeItemNo() const { return _locationPairExcludeItemNo; }

  uint64_t& fareClassExcludeItemNo() { return _fareClassExcludeItemNo; }
  uint64_t fareClassExcludeItemNo() const { return _fareClassExcludeItemNo; }

  uint64_t& fareTypeExcludeItemNo() { return _fareTypeExcludeItemNo; }
  uint64_t fareTypeExcludeItemNo() const { return _fareTypeExcludeItemNo; }

  uint64_t& displayTypeExcludeItemNo() { return _displayTypeExcludeItemNo; }
  uint64_t displayTypeExcludeItemNo() const { return _displayTypeExcludeItemNo; }
  
  uint64_t& psgTypeExcludeItemNo() { return _psgTypeExcludeItemNo; }
  uint64_t psgTypeExcludeItemNo() const { return _psgTypeExcludeItemNo; }


  uint64_t& accountCdExcludeItemNo() { return _accountCdExcludeItemNo; }
  uint64_t accountCdExcludeItemNo() const { return _accountCdExcludeItemNo; }

  uint64_t& routingExcludeItemNo() { return _routingExcludeItemNo; }
  uint64_t routingExcludeItemNo() const { return _routingExcludeItemNo; }

  
  bool operator==(const FareFocusRuleInfo& rhs) const
  {
    return _fareFocusRuleId == rhs._fareFocusRuleId
           && _fareFocusRuleFamilyId == rhs._fareFocusRuleFamilyId
           && _statusCode == rhs._statusCode
           && _processStateCode == rhs._processStateCode
           && _sourcePCC == rhs._sourcePCC
           && _securityItemNo == rhs._securityItemNo
           && _vendor == rhs._vendor
           && _carrier == rhs._carrier
           && _ruleCode == rhs._ruleCode
           && _ruleTariff == rhs._ruleTariff
           && _fareClassItemNo == rhs._fareClassItemNo
           && _fareType == rhs._fareType
           && _bookingCodeItemNo == rhs._bookingCodeItemNo
           && _owrt == rhs._owrt
           && _displayType == rhs._displayType
           && _loc1 == rhs._loc1
           && _loc2 == rhs._loc2
           && _directionality == rhs._directionality
           && _publicPrivateIndicator == rhs._publicPrivateIndicator
	   && _effInterval == rhs._effInterval
           && _lastModDate == rhs._lastModDate
           && _activationDateTime == rhs._activationDateTime
           && _inhibitCD == rhs._inhibitCD
           && _effectiveNowInd == rhs._effectiveNowInd
	   && _deactivateNowInd == rhs._deactivateNowInd
	   && _globalDir == rhs._globalDir
           && _psgTypeItemNo == rhs._psgTypeItemNo
           && _accountCdItemNo == rhs._accountCdItemNo
	   && _routingItemNo == rhs._routingItemNo
           && _retailerCode == rhs._retailerCode
           && _posDayTimeApplItemNo == rhs._posDayTimeApplItemNo
	   && _travelDayTimeApplItemNo == rhs._travelDayTimeApplItemNo
	   && _locationPairExcludeItemNo == rhs._locationPairExcludeItemNo
	   && _fareClassExcludeItemNo == rhs._fareClassExcludeItemNo
           && _fareTypeExcludeItemNo == rhs._fareTypeExcludeItemNo
	   && _displayTypeExcludeItemNo == rhs._displayTypeExcludeItemNo
           && _psgTypeExcludeItemNo == rhs._psgTypeExcludeItemNo
           && _accountCdExcludeItemNo == rhs._accountCdExcludeItemNo
           && _routingExcludeItemNo == rhs._routingExcludeItemNo; 
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(FareFocusRuleInfo& obj)
  {
    obj._fareFocusRuleId = 11111;
    obj._fareFocusRuleFamilyId = 22222;	
    obj._statusCode = 'A';
    obj._processStateCode = 'P';
    obj._sourcePCC = "ABCDE";
    obj._securityItemNo = 33333;
    obj._vendor = "ATP";
    obj._carrier = "AA";
    obj._ruleCode = "0123";
    obj._ruleTariff = 772;
    obj._fareClassItemNo = 44444;
    obj._fareType = "ABC";
    obj._bookingCodeItemNo = 55555;
    obj._owrt = '1';
    obj._displayType = 'L';
    obj._loc1.loc() = "DFW";
    obj._loc1.locType() = 'C';
    obj._loc2.loc() = "NYC";
    obj._loc2.locType() = 'C';
    obj._directionality = 'B';
    obj._publicPrivateIndicator = 'Y';
    TSEDateInterval::dummyData(obj._effInterval);
    obj._lastModDate = std::time(nullptr);
    obj._activationDateTime = std::time(nullptr);
    obj._inhibitCD = 'N';
    obj._effectiveNowInd = 'Y';
    obj._deactivateNowInd = 'Y';
    obj._globalDir = GlobalDirection::AT;
    obj._travelDayTimeApplItemNo = 333;
    obj._locationPairExcludeItemNo = 444;
    obj._psgTypeItemNo = 88888;
    obj._accountCdItemNo = 99999;
    obj._routingItemNo = 222222;
    obj._retailerCode = 'A';
    obj._posDayTimeApplItemNo = 111112;
    obj._fareClassExcludeItemNo = 777;
    obj._fareTypeExcludeItemNo = 888;
    obj._displayTypeExcludeItemNo = 1222;
    obj._psgTypeExcludeItemNo = 1444;
    obj._accountCdExcludeItemNo = 1555;
    obj._routingExcludeItemNo = 1666;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareFocusRuleId);
    FLATTENIZE(archive, _fareFocusRuleFamilyId);
    FLATTENIZE(archive, _statusCode);
    FLATTENIZE(archive, _processStateCode);
    FLATTENIZE(archive, _sourcePCC);
    FLATTENIZE(archive, _securityItemNo);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _ruleCode);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _fareClassItemNo);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _bookingCodeItemNo);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _displayType);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _publicPrivateIndicator);
    FLATTENIZE(archive, _effInterval);
    FLATTENIZE(archive, _lastModDate);
    FLATTENIZE(archive, _activationDateTime);
    FLATTENIZE(archive, _inhibitCD);
    FLATTENIZE(archive, _effectiveNowInd);
    FLATTENIZE(archive, _deactivateNowInd);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _psgTypeItemNo);
    FLATTENIZE(archive, _accountCdItemNo);
    FLATTENIZE(archive, _routingItemNo);
    FLATTENIZE(archive, _retailerCode);
    FLATTENIZE(archive, _posDayTimeApplItemNo);
    FLATTENIZE(archive, _travelDayTimeApplItemNo);
    FLATTENIZE(archive, _locationPairExcludeItemNo);
    FLATTENIZE(archive, _fareClassExcludeItemNo);
    FLATTENIZE(archive, _fareTypeExcludeItemNo);
    FLATTENIZE(archive, _displayTypeExcludeItemNo);
    FLATTENIZE(archive, _psgTypeExcludeItemNo);
    FLATTENIZE(archive, _accountCdExcludeItemNo);
    FLATTENIZE(archive, _routingExcludeItemNo);
  }

 private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_fareFocusRuleId
           & ptr->_fareFocusRuleFamilyId
           & ptr->_statusCode
           & ptr->_processStateCode
           & ptr->_sourcePCC
           & ptr->_securityItemNo
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_ruleCode
           & ptr->_ruleTariff
           & ptr->_fareClassItemNo
           & ptr->_fareType
           & ptr->_bookingCodeItemNo
           & ptr->_owrt
           & ptr->_displayType
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_directionality
           & ptr->_publicPrivateIndicator
           & ptr->_effInterval
           & ptr->_lastModDate
           & ptr->_activationDateTime
	   & ptr->_inhibitCD
           & ptr->_effectiveNowInd
	   & ptr->_deactivateNowInd
	   & ptr->_globalDir
	   & ptr->_psgTypeItemNo
	   & ptr->_accountCdItemNo
           & ptr->_routingItemNo
           & ptr->_retailerCode
	   & ptr->_posDayTimeApplItemNo
	   & ptr->_travelDayTimeApplItemNo
           & ptr->_locationPairExcludeItemNo
	   & ptr->_fareClassExcludeItemNo
	   & ptr->_fareTypeExcludeItemNo
	   & ptr->_displayTypeExcludeItemNo
	   & ptr->_psgTypeExcludeItemNo
           & ptr->_accountCdExcludeItemNo
           & ptr->_routingExcludeItemNo;
  }
  uint64_t _fareFocusRuleId;
  uint64_t _fareFocusRuleFamilyId;
  Indicator _statusCode;
  Indicator _processStateCode;
  PseudoCityCode _sourcePCC;
  uint64_t _securityItemNo;
  VendorCode _vendor;
  CarrierCode _carrier;
  RuleNumber _ruleCode;
  TariffNumber _ruleTariff;
  uint64_t _fareClassItemNo;
  FareTypeAbbrevC _fareType;
  uint64_t _bookingCodeItemNo;
  Indicator _owrt;
  Indicator _displayType;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _directionality;
  Indicator _publicPrivateIndicator;
  TSEDateInterval _effInterval;
  DateTime _lastModDate;
  DateTime _activationDateTime;
  Indicator _inhibitCD;
  Indicator _effectiveNowInd;
  Indicator _deactivateNowInd;
  GlobalDirection _globalDir;
  uint64_t _psgTypeItemNo;
  uint64_t _accountCdItemNo;
  uint64_t _routingItemNo;
  FareRetailerCode _retailerCode;
  uint64_t _posDayTimeApplItemNo;
  uint64_t _travelDayTimeApplItemNo;
  uint64_t _locationPairExcludeItemNo;
  uint64_t _fareClassExcludeItemNo;
  uint64_t _fareTypeExcludeItemNo;
  uint64_t _displayTypeExcludeItemNo;
  uint64_t _psgTypeExcludeItemNo;
  uint64_t _accountCdExcludeItemNo;
  uint64_t _routingExcludeItemNo;
};

}// tse

