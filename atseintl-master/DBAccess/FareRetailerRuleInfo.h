#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TSEDateInterval.h"

namespace tse
{

class FareRetailerRuleInfo
{
public:
  uint64_t& fareRetailerRuleId() { return _fareRetailerRuleId; }
  uint64_t fareRetailerRuleId() const { return _fareRetailerRuleId; }

  PseudoCityCode& sourcePseudoCity() { return _sourcePseudoCity; }
  const PseudoCityCode& sourcePseudoCity() const { return _sourcePseudoCity; }

  PseudoCityCode& ownerPseudoCity() { return _ownerPseudoCity; }
  const PseudoCityCode& ownerPseudoCity() const { return _ownerPseudoCity; }

  Indicator& applicationInd() { return _applicationInd; }
  Indicator applicationInd() const { return _applicationInd; }

  uint64_t& ruleSeqNo() { return _ruleSeqNo; }
  uint64_t ruleSeqNo() const { return _ruleSeqNo; }

  Indicator& statusCd() { return _statusCd; }
  Indicator statusCd() const { return _statusCd; }

  Indicator& updateableRuleInd() { return _updateableRuleInd; }
  Indicator updateableRuleInd() const { return _updateableRuleInd; }

  uint64_t& securityItemNo() { return _securityItemNo; }
  uint64_t securityItemNo() const { return _securityItemNo; }

  uint64_t& resultingFareAttrItemNo() { return _resultingFareAttrItemNo; }
  uint64_t resultingFareAttrItemNo() const { return _resultingFareAttrItemNo; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  uint64_t& carrierItemNo() { return _carrierItemNo; }
  uint64_t carrierItemNo() const { return _carrierItemNo; }

  uint64_t& ruleCdItemNo() { return _ruleCdItemNo; }
  uint64_t ruleCdItemNo() const { return _ruleCdItemNo; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  TariffNumber ruleTariff() const { return _ruleTariff; }

  uint64_t& fareClassItemNo() { return _fareClassItemNo; }
  uint64_t fareClassItemNo() const { return _fareClassItemNo; }

  FareTypeAbbrevC& fareType() { return _fareType; }
  const FareTypeAbbrevC& fareType() const { return _fareType; }

  uint64_t& bookingCdItemNo() { return _bookingCdItemNo; }
  uint64_t bookingCdItemNo() const { return _bookingCdItemNo; }

  Indicator& owrt() { return _owrt; }
  Indicator owrt() const { return _owrt; }

  Indicator& displayCatType() { return _displayCatType; }
  Indicator displayCatType() const { return _displayCatType; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& directionality() { return _directionality; }
  Indicator directionality() const { return _directionality; }

  Indicator& publicPrivateInd() { return _publicPrivateInd; }
  Indicator publicPrivateInd() const { return _publicPrivateInd; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  uint64_t& psgTypeItemNo() { return _psgTypeItemNo; }
  uint64_t psgTypeItemNo() const { return _psgTypeItemNo; }

  uint64_t& accountCdItemNo() { return _accountCdItemNo; }
  uint64_t accountCdItemNo() const { return _accountCdItemNo; }

  uint64_t& posDayTimeApplItemNo() { return _posDayTimeApplItemNo; }
  uint64_t posDayTimeApplItemNo() const { return _posDayTimeApplItemNo; }

  uint64_t& routingItemNo() { return _routingItemNo; }
  uint64_t routingItemNo() const { return _routingItemNo; }

  uint64_t& tktDesignatorItemNo() { return _tktDesignatorItemNo; }
  uint64_t tktDesignatorItemNo() const { return _tktDesignatorItemNo; }

  std::string& actionCd() { return _actionCd; }
  const std::string& actionCd() const { return _actionCd; }

  Indicator& inhibitCd() { return _inhibitCd; }
  Indicator inhibitCd() const { return _inhibitCd; }

  uint64_t& fareRetailerCalcItemNo() { return _fareRetailerCalcItemNo; }
  uint64_t fareRetailerCalcItemNo() const { return _fareRetailerCalcItemNo; }

  Indicator& effectiveNowInd() { return _effectiveNowInd; }
  Indicator effectiveNowInd() const { return _effectiveNowInd; }

  Indicator& deactivateNowInd() { return _deactivateNowInd; }
  Indicator deactivateNowInd() const { return _deactivateNowInd; }

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

  uint64_t& travelDayTimeApplItemNo() { return _travelDayTimeApplItemNo; }
  uint64_t travelDayTimeApplItemNo() const { return _travelDayTimeApplItemNo; }

  uint64_t& locationPairExcludeItemNo() { return _locationPairExcludeItemNo; }
  uint64_t locationPairExcludeItemNo() const { return _locationPairExcludeItemNo; }

  uint64_t& ruleCdExcludeItemNo() { return _ruleCdExcludeItemNo; }
  uint64_t ruleCdExcludeItemNo() const { return _ruleCdExcludeItemNo; }

  uint64_t& ruleTariffExcludeItemNo() { return _ruleTariffExcludeItemNo; }
  uint64_t ruleTariffExcludeItemNo() const { return _ruleTariffExcludeItemNo; }

  uint64_t& fareClassExcludeItemNo() { return _fareClassExcludeItemNo; }
  uint64_t fareClassExcludeItemNo() const { return _fareClassExcludeItemNo; }

  uint64_t& fareTypeExcludeItemNo() { return _fareTypeExcludeItemNo; }
  uint64_t fareTypeExcludeItemNo() const { return _fareTypeExcludeItemNo; }

  uint64_t& bookingCdExcludeItemNo() { return _bookingCdExcludeItemNo; }
  uint64_t bookingCdExcludeItemNo() const { return _bookingCdExcludeItemNo; }

  uint64_t& owrtExcludeItemNo() { return _owrtExcludeItemNo; }
  uint64_t owrtExcludeItemNo() const { return _owrtExcludeItemNo; }

  uint64_t& displayCatTypeExcludeItemNo() { return _displayCatTypeExcludeItemNo; }
  uint64_t displayCatTypeExcludeItemNo() const { return _displayCatTypeExcludeItemNo; }

  uint64_t& globalDirExcludeItemNo() { return _globalDirExcludeItemNo; }
  uint64_t globalDirExcludeItemNo() const { return _globalDirExcludeItemNo; }

  uint64_t& psgTypeExcludeItemNo() { return _psgTypeExcludeItemNo; }
  uint64_t psgTypeExcludeItemNo() const { return _psgTypeExcludeItemNo; }

  uint64_t& accountCdExcludeItemNo() { return _accountCdExcludeItemNo; }
  uint64_t accountCdExcludeItemNo() const { return _accountCdExcludeItemNo; }

  uint64_t& routingExcludeItemNo() { return _routingExcludeItemNo; }
  uint64_t routingExcludeItemNo() const { return _routingExcludeItemNo; }

  uint64_t& tktDesignatorExcludeItemNo() { return _tktDesignatorExcludeItemNo; }
  uint64_t tktDesignatorExcludeItemNo() const { return _tktDesignatorExcludeItemNo; }

  FareRetailerCode& fareRetailerCode() { return _fareRetailerCode; }
  const FareRetailerCode& fareRetailerCode() const { return _fareRetailerCode; }

  Indicator& journeyTypeCd() { return _journeyTypeCd; }
  Indicator journeyTypeCd() const { return _journeyTypeCd; }

  bool operator==(const FareRetailerRuleInfo& rhs) const
  {
    return  _fareRetailerRuleId == rhs._fareRetailerRuleId
           && _sourcePseudoCity == rhs._sourcePseudoCity
           && _ownerPseudoCity == rhs._ownerPseudoCity
           && _applicationInd == rhs._applicationInd
           && _ruleSeqNo == rhs._ruleSeqNo
           && _statusCd == rhs._statusCd
           && _updateableRuleInd == rhs._updateableRuleInd
           && _securityItemNo == rhs._securityItemNo
           && _resultingFareAttrItemNo == rhs._resultingFareAttrItemNo
           && _vendor == rhs._vendor
           && _carrierItemNo == rhs._carrierItemNo
           && _ruleCdItemNo == rhs._ruleCdItemNo
           && _ruleTariff == rhs._ruleTariff
           && _fareClassItemNo == rhs._fareClassItemNo
           && _fareType == rhs._fareType
           && _bookingCdItemNo == rhs._bookingCdItemNo
           && _owrt == rhs._owrt
           && _displayCatType == rhs._displayCatType
           && _loc1 == rhs._loc1
           && _loc2 == rhs._loc2
           && _directionality == rhs._directionality
           && _publicPrivateInd == rhs._publicPrivateInd
           && _globalDir == rhs._globalDir
           && _psgTypeItemNo == rhs._psgTypeItemNo
           && _accountCdItemNo == rhs._accountCdItemNo
           && _posDayTimeApplItemNo == rhs._posDayTimeApplItemNo
           && _routingItemNo == rhs._routingItemNo
           && _tktDesignatorItemNo == rhs._tktDesignatorItemNo
           && _actionCd == rhs._actionCd
           && _inhibitCd == rhs._inhibitCd
           && _fareRetailerCalcItemNo == rhs._fareRetailerCalcItemNo
           && _effectiveNowInd == rhs._effectiveNowInd
           && _deactivateNowInd == rhs._deactivateNowInd
           && _effInterval == rhs._effInterval
           && _lastModDate == rhs._lastModDate
           && _activationDateTime == rhs._activationDateTime
           && _travelDayTimeApplItemNo == rhs._travelDayTimeApplItemNo
           && _locationPairExcludeItemNo == rhs._locationPairExcludeItemNo
           && _ruleCdExcludeItemNo == rhs._ruleCdExcludeItemNo
           && _ruleTariffExcludeItemNo == rhs._ruleTariffExcludeItemNo
           && _fareClassExcludeItemNo == rhs._fareClassExcludeItemNo
           && _fareTypeExcludeItemNo == rhs._fareTypeExcludeItemNo
           && _bookingCdExcludeItemNo == rhs._bookingCdExcludeItemNo
           && _owrtExcludeItemNo == rhs._owrtExcludeItemNo
           && _displayCatTypeExcludeItemNo == rhs._displayCatTypeExcludeItemNo
           && _globalDirExcludeItemNo == rhs._globalDirExcludeItemNo
           && _psgTypeExcludeItemNo == rhs._psgTypeExcludeItemNo
           && _accountCdExcludeItemNo == rhs._accountCdExcludeItemNo
           && _routingExcludeItemNo == rhs._routingExcludeItemNo
           && _tktDesignatorExcludeItemNo == rhs._tktDesignatorExcludeItemNo
           && _fareRetailerCode == rhs._fareRetailerCode
           && _journeyTypeCd == rhs._journeyTypeCd;
  }

  static void dummyData(FareRetailerRuleInfo& obj)
  {
    obj._fareRetailerRuleId = 11111;
    obj._sourcePseudoCity = "5KAD";
    obj._ownerPseudoCity = "B4T0";
    obj._applicationInd = 'N';
    obj._ruleSeqNo = 22222;
    obj._statusCd = 'A';
    obj._updateableRuleInd = 'N';
    obj._securityItemNo = 33333;
    obj._resultingFareAttrItemNo = 44444;
    obj._vendor = "ATP";
    obj._carrierItemNo = 55555;
    obj._ruleCdItemNo = 66666;
    obj._ruleTariff = 854;
    obj._fareClassItemNo = 66622;
    obj._fareType = "ZEX";
    obj._bookingCdItemNo = 77777;
    obj._owrt = '1';
    obj._displayCatType = 'C';
    obj._loc1.loc() = "DFW";
    obj._loc1.locType() = 'C';
    obj._loc2.loc() = "LON";
    obj._loc2.locType() = 'C';
    obj._directionality = 'F';
    obj._publicPrivateInd = '1';
    obj._globalDir = GlobalDirection::AT;
    obj._psgTypeItemNo = 88888;
    obj._accountCdItemNo = 99999;
    obj._posDayTimeApplItemNo = 111112;
    obj._routingItemNo        = 222222;
    obj._tktDesignatorItemNo  = 333332;
    obj._actionCd = "XP";
    obj._inhibitCd = 'N';
    obj._fareRetailerCalcItemNo = 444442;
    obj._effectiveNowInd = 'Y';
    obj._deactivateNowInd = 'Y';
    TSEDateInterval::dummyData(obj._effInterval);
    obj._lastModDate = std::time(nullptr);
    obj._activationDateTime = std::time(nullptr);
    obj._travelDayTimeApplItemNo = 333;
    obj._locationPairExcludeItemNo = 444;
    obj._ruleCdExcludeItemNo = 555;
    obj._ruleTariffExcludeItemNo = 666;
    obj._fareClassExcludeItemNo = 777;
    obj._fareTypeExcludeItemNo = 888;
    obj._bookingCdExcludeItemNo = 999;
    obj._owrtExcludeItemNo = 1000;
    obj._displayCatTypeExcludeItemNo = 1222;
    obj._globalDirExcludeItemNo = 1333;
    obj._psgTypeExcludeItemNo = 1444;
    obj._accountCdExcludeItemNo = 1555;
    obj._routingExcludeItemNo = 1666;
    obj._tktDesignatorExcludeItemNo = 1777;
    obj._journeyTypeCd = 'A';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareRetailerRuleId);
    FLATTENIZE(archive, _sourcePseudoCity);
    FLATTENIZE(archive, _ownerPseudoCity);
    FLATTENIZE(archive, _applicationInd);
    FLATTENIZE(archive, _ruleSeqNo);
    FLATTENIZE(archive, _statusCd);
    FLATTENIZE(archive, _updateableRuleInd);
    FLATTENIZE(archive, _securityItemNo);
    FLATTENIZE(archive, _resultingFareAttrItemNo);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrierItemNo);
    FLATTENIZE(archive, _ruleCdItemNo);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _fareClassItemNo);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _bookingCdItemNo);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _displayCatType);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _publicPrivateInd);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _psgTypeItemNo);
    FLATTENIZE(archive, _accountCdItemNo);
    FLATTENIZE(archive, _posDayTimeApplItemNo);
    FLATTENIZE(archive, _routingItemNo);
    FLATTENIZE(archive, _tktDesignatorItemNo);
    FLATTENIZE(archive, _actionCd);
    FLATTENIZE(archive, _inhibitCd);
    FLATTENIZE(archive, _fareRetailerCalcItemNo);
    FLATTENIZE(archive, _effectiveNowInd);
    FLATTENIZE(archive, _deactivateNowInd);
    FLATTENIZE(archive, _effInterval);
    FLATTENIZE(archive, _lastModDate);
    FLATTENIZE(archive, _activationDateTime);
    FLATTENIZE(archive, _travelDayTimeApplItemNo);
    FLATTENIZE(archive, _locationPairExcludeItemNo);
    FLATTENIZE(archive, _ruleCdExcludeItemNo);
    FLATTENIZE(archive, _ruleTariffExcludeItemNo);
    FLATTENIZE(archive, _fareClassExcludeItemNo);
    FLATTENIZE(archive, _fareTypeExcludeItemNo);
    FLATTENIZE(archive, _bookingCdExcludeItemNo);
    FLATTENIZE(archive, _owrtExcludeItemNo);
    FLATTENIZE(archive, _displayCatTypeExcludeItemNo);
    FLATTENIZE(archive, _globalDirExcludeItemNo);
    FLATTENIZE(archive, _psgTypeExcludeItemNo);
    FLATTENIZE(archive, _accountCdExcludeItemNo);
    FLATTENIZE(archive, _routingExcludeItemNo);
    FLATTENIZE(archive, _tktDesignatorExcludeItemNo);
    FLATTENIZE(archive, _fareRetailerCode);
    FLATTENIZE(archive, _journeyTypeCd);
  }

private:
  uint64_t _fareRetailerRuleId = 0;
  PseudoCityCode _sourcePseudoCity;
  PseudoCityCode _ownerPseudoCity;
  Indicator _applicationInd = ' ';
  uint64_t _ruleSeqNo = 0;
  Indicator _statusCd = ' ';
  Indicator _updateableRuleInd = ' ';
  uint64_t _securityItemNo = 0;
  uint64_t _resultingFareAttrItemNo = 0;
  VendorCode _vendor;
  uint64_t _carrierItemNo = 0;
  uint64_t _ruleCdItemNo = 0;
  TariffNumber _ruleTariff = 0;
  uint64_t _fareClassItemNo = 0;
  FareTypeAbbrevC _fareType;
  uint64_t _bookingCdItemNo = 0;
  Indicator _owrt = ' ';
  Indicator _displayCatType = ' ';
  LocKey _loc1;
  LocKey _loc2;
  Indicator _directionality = 'F';
  Indicator _publicPrivateInd = ' ';
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  uint64_t _psgTypeItemNo = 0;
  uint64_t _accountCdItemNo = 0;
  uint64_t _posDayTimeApplItemNo = 0;
  uint64_t _routingItemNo = 0;
  uint64_t _tktDesignatorItemNo = 0;
  std::string  _actionCd;
  Indicator _inhibitCd = ' ';
  uint64_t _fareRetailerCalcItemNo = 0;
  Indicator _effectiveNowInd = ' ';
  Indicator _deactivateNowInd = ' ';
  TSEDateInterval _effInterval;
  DateTime _lastModDate;
  DateTime _activationDateTime;
  uint64_t _travelDayTimeApplItemNo = 0;
  uint64_t _locationPairExcludeItemNo = 0;
  uint64_t _ruleCdExcludeItemNo = 0;
  uint64_t _ruleTariffExcludeItemNo = 0;
  uint64_t _fareClassExcludeItemNo = 0;
  uint64_t _fareTypeExcludeItemNo = 0;
  uint64_t _bookingCdExcludeItemNo = 0;
  uint64_t _owrtExcludeItemNo = 0;
  uint64_t _displayCatTypeExcludeItemNo = 0;
  uint64_t _globalDirExcludeItemNo = 0;
  uint64_t _psgTypeExcludeItemNo = 0;
  uint64_t _accountCdExcludeItemNo = 0;
  uint64_t _routingExcludeItemNo = 0;
  uint64_t _tktDesignatorExcludeItemNo = 0;
  FareRetailerCode _fareRetailerCode;
  Indicator _journeyTypeCd = ' ';
};
}// tse
