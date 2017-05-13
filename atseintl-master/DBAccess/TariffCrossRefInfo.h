//-------------------------------------------------------------------
//
//  File:           TariffCrossRefInfo.h
//  Created:        2/3/2004
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TSEDateInterval.h"

#include <map>

namespace tse
{

class TariffCrossRefInfo
{

public:
  TariffCrossRefInfo()
    : _fareTariff(0),
      _ruleTariff(0),
      _governingTariff(0),
      _routingTariff1(0),
      _routingTariff2(0),
      _addonTariff1(0),
      _addonTariff2(0),
      _tariffCat(0),
      _crossRefType(DOMESTIC),
      _globalDirection(GlobalDirection::ZZ),
      _zoneType(0)
  {
  }

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate() { return _effInterval.createDate(); }
  const DateTime& createDate() const { return _effInterval.createDate(); }

  DateTime& effDate() { return _effInterval.effDate(); }
  const DateTime& effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate() { return _effInterval.expireDate(); }
  const DateTime& expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate() { return _effInterval.discDate(); }
  const DateTime& discDate() const { return _effInterval.discDate(); }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  RecordScope& crossRefType() { return _crossRefType; }
  const RecordScope& crossRefType() const { return _crossRefType; }

  GlobalDirection& globalDirection() { return _globalDirection; }
  const GlobalDirection& globalDirection() const { return _globalDirection; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber fareTariff() const { return _fareTariff; }
  TariffCode& fareTariffCode() { return _fareTariffCode; }
  const TariffCode& fareTariffCode() const { return _fareTariffCode; }

  TariffCategory& tariffCat() { return _tariffCat; }
  const TariffCategory& tariffCat() const { return _tariffCat; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber ruleTariff() const { return _ruleTariff; }
  TariffCode& ruleTariffCode() { return _ruleTariffCode; }
  const TariffCode& ruleTariffCode() const { return _ruleTariffCode; }

  TariffNumber& governingTariff() { return _governingTariff; }
  const TariffNumber& governingTariff() const { return _governingTariff; }
  TariffCode& governingTariffCode() { return _governingTariffCode; }
  const TariffCode& governingTariffCode() const { return _governingTariffCode; }

  TariffNumber& routingTariff1() { return _routingTariff1; }
  const TariffNumber& routingTariff1() const { return _routingTariff1; }
  TariffCode& routingTariff1Code() { return _routingTariff1Code; }
  const TariffCode& routingTariff1Code() const { return _routingTariff1Code; }

  TariffNumber& routingTariff2() { return _routingTariff2; }
  const TariffNumber& routingTariff2() const { return _routingTariff2; }
  TariffCode& routingTariff2Code() { return _routingTariff2Code; }
  const TariffCode& routingTariff2Code() const { return _routingTariff2Code; }

  TariffNumber& addonTariff1() { return _addonTariff1; }
  const TariffNumber addonTariff1() const { return _addonTariff1; }
  TariffCode& addonTariff1Code() { return _addonTariff1Code; }
  const TariffCode& addonTariff1Code() const { return _addonTariff1Code; }

  TariffNumber& addonTariff2() { return _addonTariff2; }
  const TariffNumber addonTariff2() const { return _addonTariff2; }
  TariffCode& addonTariff2Code() { return _addonTariff2Code; }
  const TariffCode& addonTariff2Code() const { return _addonTariff2Code; }

  Zone& zoneNo() { return _zoneNo; }
  const Zone& zoneNo() const { return _zoneNo; }

  VendorCode& zoneVendor() { return _zoneVendor; }
  const VendorCode& zoneVendor() const { return _zoneVendor; }

  const Indicator zoneType() const
  {
    return _zoneType;
  };
  Indicator& zoneType()
  {
    return _zoneType;
  };

  bool operator==(const TariffCrossRefInfo& rhs) const
  {
    return ((_effInterval == rhs._effInterval) && (_vendor == rhs._vendor) &&
            (_carrier == rhs._carrier) && (_crossRefType == rhs._crossRefType) &&
            (_globalDirection == rhs._globalDirection) && (_fareTariff == rhs._fareTariff) &&
            (_fareTariffCode == rhs._fareTariffCode) && (_tariffCat == rhs._tariffCat) &&
            (_ruleTariff == rhs._ruleTariff) && (_ruleTariffCode == rhs._ruleTariffCode) &&
            (_governingTariff == rhs._governingTariff) &&
            (_governingTariffCode == rhs._governingTariffCode) &&
            (_routingTariff1 == rhs._routingTariff1) &&
            (_routingTariff1Code == rhs._routingTariff1Code) &&
            (_routingTariff2 == rhs._routingTariff2) &&
            (_routingTariff2Code == rhs._routingTariff2Code) &&
            (_addonTariff1 == rhs._addonTariff1) && (_addonTariff1Code == rhs._addonTariff1Code) &&
            (_addonTariff2 == rhs._addonTariff2) && (_addonTariff2Code == rhs._addonTariff2Code) &&
            (_zoneNo == rhs._zoneNo) && (_zoneVendor == rhs._zoneVendor) &&
            (_zoneType == rhs._zoneType));
  }

  static void dummyData(TariffCrossRefInfo& obj)
  {
    TSEDateInterval::dummyData(obj._effInterval);

    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._crossRefType = DOMESTIC;
    obj._globalDirection = GlobalDirection::US;
    obj._fareTariff = 1;
    obj._fareTariffCode = "HIJKLMN";
    obj._tariffCat = 2;
    obj._ruleTariff = 3;
    obj._ruleTariffCode = "OPQ";
    obj._governingTariff = 4;
    obj._governingTariffCode = "RST";
    obj._routingTariff1 = 5;
    obj._routingTariff1Code = "UVW";
    obj._routingTariff2 = 6;
    obj._routingTariff2Code = "XYZ";
    obj._addonTariff1 = 7;
    obj._addonTariff1Code = "abc";
    obj._addonTariff2 = 8;
    obj._addonTariff2Code = "def";
    obj._zoneNo = "ghijklm";
    obj._zoneVendor = "nopq";
    obj._zoneType = 'r';
  }

public:
  TSEDateInterval _effInterval; // interval where record is valid
  TariffNumber _fareTariff; // fare tariff number
  TariffNumber _ruleTariff; // rule tariff number
  TariffNumber _governingTariff; // governing tariff number
  TariffNumber _routingTariff1; // routing tariff #1 number
  TariffNumber _routingTariff2; // routing tariff #2 number
  TariffNumber _addonTariff1; // addon tariff #1 number
  TariffNumber _addonTariff2; // addon tariff #2 number
  TariffCategory _tariffCat;

  VendorCode _vendor; // fare/rules vendor
  VendorCode _zoneVendor;
  CarrierCode _carrier; // carrier code

  TariffCode _fareTariffCode; // fare tariff code
  TariffCode _ruleTariffCode; // rule tariff code
  TariffCode _governingTariffCode; // governing tariff code
  TariffCode _routingTariff1Code; // routing tariff #1 code
  TariffCode _routingTariff2Code; // routing tariff #2 code
  TariffCode _addonTariff1Code; // addon tariff #1 code
  TariffCode _addonTariff2Code; // addon tariff #2 code
  Zone _zoneNo;

  RecordScope _crossRefType; // record scope
  GlobalDirection _globalDirection;
  Indicator _zoneType;

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _effInterval);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _crossRefType);
    FLATTENIZE(archive, _globalDirection);
    FLATTENIZE(archive, _fareTariff);
    FLATTENIZE(archive, _fareTariffCode);
    FLATTENIZE(archive, _tariffCat);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _ruleTariffCode);
    FLATTENIZE(archive, _governingTariff);
    FLATTENIZE(archive, _governingTariffCode);
    FLATTENIZE(archive, _routingTariff1);
    FLATTENIZE(archive, _routingTariff1Code);
    FLATTENIZE(archive, _routingTariff2);
    FLATTENIZE(archive, _routingTariff2Code);
    FLATTENIZE(archive, _addonTariff1);
    FLATTENIZE(archive, _addonTariff1Code);
    FLATTENIZE(archive, _addonTariff2);
    FLATTENIZE(archive, _addonTariff2Code);
    FLATTENIZE(archive, _zoneNo);
    FLATTENIZE(archive, _zoneVendor);
    FLATTENIZE(archive, _zoneType);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_effInterval
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_crossRefType
           & ptr->_globalDirection
           & ptr->_fareTariff
           & ptr->_fareTariffCode
           & ptr->_tariffCat
           & ptr->_ruleTariff
           & ptr->_ruleTariffCode
           & ptr->_governingTariff
           & ptr->_governingTariffCode
           & ptr->_routingTariff1
           & ptr->_routingTariff1Code
           & ptr->_routingTariff2
           & ptr->_routingTariff2Code
           & ptr->_addonTariff1
           & ptr->_addonTariff1Code
           & ptr->_addonTariff2
           & ptr->_addonTariff2Code
           & ptr->_zoneNo
           & ptr->_zoneVendor
           & ptr->_zoneType;
  }

}; // end class TariffCrossRefInfo

class TariffCrossRefInfoContainer
{
  typedef std::map<TariffNumber, std::vector<TariffCrossRefInfo*> > TariffMap;

public:
  TariffCrossRefInfoContainer() {}
  ~TariffCrossRefInfoContainer()
  {
    std::vector<TariffCrossRefInfo*>::iterator i;
    for (i = infoVector_.begin(); i != infoVector_.end(); i++)
      delete *i;
  }

  void initializeTariffMaps()
  {
    std::vector<TariffCrossRefInfo*>::iterator itr = infoVector_.begin();
    std::vector<TariffCrossRefInfo*>::iterator itrend = infoVector_.end();
    for (; itr != itrend; ++itr)
    {
      TariffCrossRefInfo* tcr = *itr;
      if (tcr->fareTariff() >= 0)
        fareTariffMap_[tcr->fareTariff()].push_back(tcr);
      if (tcr->ruleTariff() >= 0)
        ruleTariffMap_[tcr->ruleTariff()].push_back(tcr);
      if (tcr->governingTariff() >= 0)
        govRuleTariffMap_[tcr->governingTariff()].push_back(tcr);
      if (tcr->addonTariff1() >= 0)
        addonTariffMap_[tcr->addonTariff1()].push_back(tcr);
      if (tcr->addonTariff2() >= 0)
        addonTariffMap_[tcr->addonTariff2()].push_back(tcr);
    }
  }

  std::vector<TariffCrossRefInfo*>& tariffInfo() { return infoVector_; }

  std::vector<TariffCrossRefInfo*>* fareTariffs(TariffNumber tariff)
  {
    return getTariffs(fareTariffMap_, tariff);
  }

  std::vector<TariffCrossRefInfo*>* ruleTariffs(TariffNumber tariff)
  {
    return getTariffs(ruleTariffMap_, tariff);
  }

  std::vector<TariffCrossRefInfo*>* govRuleTariffs(TariffNumber tariff)
  {
    return getTariffs(govRuleTariffMap_, tariff);
  }

  std::vector<TariffCrossRefInfo*>* addonTariffs(TariffNumber tariff)
  {
    return getTariffs(addonTariffMap_, tariff);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, infoVector_);
    if (archive.action() == Flattenizable::UNFLATTEN)
    {
      initializeTariffMaps();
    }
  }

  bool operator==(const TariffCrossRefInfoContainer& rhs) const
  {
    if (infoVector_.size() != rhs.infoVector_.size())
      return false;

    size_t size = infoVector_.size();
    for (size_t i = 0; i < size; ++i)
    {
      TariffCrossRefInfo* linfo = infoVector_[i];
      TariffCrossRefInfo* rinfo = rhs.infoVector_[i];
      if (linfo == nullptr || rinfo == nullptr)
        return false;
      if (!(*linfo == *rinfo))
        return false;
    }
    return true;
  }

  static void dummyData(TariffCrossRefInfoContainer& obj)
  {
    TariffCrossRefInfo* info = new TariffCrossRefInfo;
    TariffCrossRefInfo::dummyData(*info);
    obj.infoVector_.push_back(info);
  }

protected:
  std::vector<TariffCrossRefInfo*>* getTariffs(TariffMap& tariffMap, TariffNumber tariff)
  {
    std::vector<TariffCrossRefInfo*>* tariffs = nullptr;
    TariffMap::iterator it = tariffMap.find(tariff);
    if (it != tariffMap.end())
    {
      // std::vector<TariffCrossRefInfo*>& tariffsref = it->second;
      // tariffs = &tariffsref;
      tariffs = &it->second;
    }
    return tariffs;
  }

private:
  std::vector<TariffCrossRefInfo*> infoVector_;
  TariffMap fareTariffMap_;
  TariffMap ruleTariffMap_;
  TariffMap govRuleTariffMap_;
  TariffMap addonTariffMap_;
};
} // namespace tse
