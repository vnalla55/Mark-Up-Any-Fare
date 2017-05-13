//----------------------------------------------------------------------------
//       Copyright 2004, Sabre Inc.  All rights reserved.  This software/documentation is the
//confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/HashKey.h"

#include <map>

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

#include <unordered_map>

namespace tse
{

typedef HashKey<VendorCode, TariffNumber, CarrierCode> AddOnCombFareClassKey;

class WBuffer;
class RBuffer;

class AddonCombFareClassInfo
{
public:
  AddonCombFareClassInfo() : _addonFareClass(' '), _geoAppl(' '), _owrt(' ') {}

  ~AddonCombFareClassInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  const DateTime& effDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  const DateTime& discDate() const { return _infiniteDiscontinue; }

  char& addonFareClass() { return _addonFareClass; }
  const char addonFareClass() const { return _addonFareClass; }

  Indicator& geoAppl() { return _geoAppl; }
  const Indicator& geoAppl() const { return _geoAppl; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  FareClassCodeC& fareClass() { return _fareClass; }
  const FareClassCodeC& fareClass() const { return _fareClass; }

  bool operator<(const AddonCombFareClassInfo& rhs) const;

  bool operator==(const AddonCombFareClassInfo& rhs) const;

  friend std::ostream& dumpObject(std::ostream& os, const AddonCombFareClassInfo& obj);

  static void dummyData(AddonCombFareClassInfo& obj);
  static void dummyData2(AddonCombFareClassInfo& obj);
  static void dummyData(AddOnCombFareClassKey& key, AddonCombFareClassInfo& obj);

  void flattenize(Flattenizable::Archive& archive);

  WBuffer& write(WBuffer& os) const;
  RBuffer& read(RBuffer& is);

protected:
  DateTime _createDate;
  DateTime _expireDate;
  FareClassCodeC _fareClass;
  char _addonFareClass;
  Indicator _geoAppl;
  Indicator _owrt;

  static DateTime _infiniteDiscontinue;

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_createDate & ptr->_expireDate & ptr->_fareClass & 
           ptr->_addonFareClass & ptr->_geoAppl & ptr->_owrt;
  }
};

struct SortACFCKeyLess
{
  bool operator()(const AddonCombFareClassInfo* first, const AddonCombFareClassInfo* second) const;
};

class AddonCombFareClassSpecifiedKey
{
public:
  AddonCombFareClassSpecifiedKey(): _owrt(' ') { }

  AddonCombFareClassSpecifiedKey(const FareClassCodeC& specifiedFareClass,
                                 const Indicator owrt)
  : _specifiedFareClass(specifiedFareClass), _owrt(owrt)
  { }

  AddonCombFareClassSpecifiedKey(const AddonCombFareClassSpecifiedKey& rhs)
  : _specifiedFareClass(rhs._specifiedFareClass),
    _owrt(rhs._owrt)
  { }

  AddonCombFareClassSpecifiedKey& operator=(const AddonCombFareClassSpecifiedKey& rhs);

  friend bool operator<(const AddonCombFareClassSpecifiedKey& x, const AddonCombFareClassSpecifiedKey& y);

  bool operator==(const AddonCombFareClassSpecifiedKey& rhs) const
  {
    return _owrt == rhs._owrt && _specifiedFareClass == rhs._specifiedFareClass;
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const AddonCombFareClassSpecifiedKey& obj)
  {
    return os << "|" << obj._specifiedFareClass << "|" << obj._owrt << "]";
  }

  void flattenize(Flattenizable::Archive& archive);

  const FareClassCodeC& specifiedFareClass() const { return _specifiedFareClass; }
  Indicator owrt() const { return _owrt; }

  static void dummyData(AddonCombFareClassSpecifiedKey&);
  static void dummyData2(AddonCombFareClassSpecifiedKey&);

protected:
  FareClassCodeC _specifiedFareClass;
  Indicator _owrt;
};

class AddonCombFareClassInfoKey1
{
public:
  // construction/destruction
  // ========================

  AddonCombFareClassInfoKey1() : _fareTariff(0) {}

  AddonCombFareClassInfoKey1(const VendorCode& vendor,
                             const TariffNumber& fareTariff,
                             const CarrierCode& carrier)
    : _vendor(vendor), _fareTariff(fareTariff), _carrier(carrier)
  {
  }

  AddonCombFareClassInfoKey1(const AddonCombFareClassInfoKey1& rhs)
    : _vendor(rhs._vendor), _fareTariff(rhs._fareTariff), _carrier(rhs._carrier) {};

  const VendorCode& getVendor() const { return _vendor; }
  TariffNumber getFareTariff() const { return _fareTariff; }
  const CarrierCode& getCarrier() const { return _carrier; }

  AddonCombFareClassInfoKey1& operator=(const AddonCombFareClassInfoKey1& rhs);

  bool operator==(const AddonCombFareClassInfoKey1& rhs) const
  {
    return _fareTariff == rhs._fareTariff && _vendor == rhs._vendor && _carrier == rhs._carrier;
  }

  void flattenize(Flattenizable::Archive& archive);

protected:
  VendorCode _vendor;
  TariffNumber _fareTariff;
  CarrierCode _carrier;

};

bool operator<(const AddonCombFareClassInfoKey1& x, const AddonCombFareClassInfoKey1& y);

std::ostream& dumpObject(std::ostream& os, const AddonCombFareClassInfoKey1& obj);

struct addonCombFareClassSpecHashEqual
{
  bool
  operator()(const AddonCombFareClassSpecifiedKey& first, const AddonCombFareClassSpecifiedKey& second) const
  {
    return first == second;
  }
};

struct P05SpecHash
{
  size_t operator()(const AddonCombFareClassSpecifiedKey& key) const;

private:
  static constexpr int RADIX = 37;
  static constexpr int DIV = 393241;

  static int hashAlphanum(int x)
  {
    if (isupper(x))
      x -= 'A';
    else
      x -= '0' - 'Z' + 'A' - 1;
    return x;
  }
};

typedef std::unordered_map<AddonCombFareClassSpecifiedKey,
                           std::vector<AddonCombFareClassInfo*>,
                           P05SpecHash,
                           addonCombFareClassSpecHashEqual> ADDONSPECHASHMAP;

class AddonFareClassCombMultiMap : public ADDONSPECHASHMAP
{
public:
  virtual ~AddonFareClassCombMultiMap() {}

  //friend std::ostream& dumpObject(std::ostream& os, const AddonFareClassCombMultiMap& obj);

  virtual bool operator==(const AddonFareClassCombMultiMap& rhs) const;

  void flattenize(Flattenizable::Archive& archive);

  static void dummyData(AddonFareClassCombMultiMap& obj);

  typedef std::vector<AddonCombFareClassInfo*> INFOVEC;
  typedef std::map<AddonCombFareClassSpecifiedKey, INFOVEC, std::less<AddonCombFareClassSpecifiedKey> >
  INFOMAP;
};

typedef AddonFareClassCombMultiMap::const_iterator AddonFareClassCombMultiMapIterator;

// yet another map of maps

typedef std::map<AddonCombFareClassInfoKey1, AddonFareClassCombMultiMap*> AddonFareClassVCCombMap;

typedef AddonFareClassVCCombMap::const_iterator AddonFareClassCombMultiMapMapIterator;

} // tse

#else

#include "DBAccess/TSEDateInterval.h"

#include <tr1/unordered_map>

namespace tse
{

class WBuffer;
class RBuffer;

class AddonCombFareClassInfo
{
public:
  AddonCombFareClassInfo() : _fareTariff(1), _geoAppl(' '), _owrt(' ') {}

  ~AddonCombFareClassInfo() {}

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effectiveInterval() const { return _effInterval; }

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

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  FareClassCodeC& addonFareClass() { return _addonFareClass; }
  const FareClassCodeC& addonFareClass() const { return _addonFareClass; }

  Indicator& geoAppl() { return _geoAppl; }
  const Indicator& geoAppl() const { return _geoAppl; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  FareClassCodeC& fareClass() { return _fareClass; }
  const FareClassCodeC& fareClass() const { return _fareClass; }

  bool operator<(const AddonCombFareClassInfo& rhs) const;

  bool operator==(const AddonCombFareClassInfo& rhs) const;

  friend std::ostream& dumpObject(std::ostream& os, const AddonCombFareClassInfo& obj);

  static void dummyData(AddonCombFareClassInfo& obj);

  static void dummyData2(AddonCombFareClassInfo& obj);

  void flattenize(Flattenizable::Archive& archive);

  WBuffer& write(WBuffer& os) const;
  RBuffer& read(RBuffer& is);

protected:
  TSEDateInterval _effInterval;
  VendorCode _vendor;
  TariffNumber _fareTariff;
  CarrierCode _carrier;
  FareClassCodeC _addonFareClass;
  Indicator _geoAppl;
  Indicator _owrt;
  FareClassCodeC _fareClass;

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_effInterval & ptr->_vendor & ptr->_fareTariff & ptr->_carrier &
           ptr->_addonFareClass & ptr->_geoAppl & ptr->_owrt & ptr->_fareClass;
  }
};

struct SortACFCKeyLess
{
  bool operator()(const AddonCombFareClassInfo* first, const AddonCombFareClassInfo* second) const;
};

class AddonCombFareClassInfoKey
{
public:
  // construction/destruction
  // ========================

  AddonCombFareClassInfoKey() : _geoAppl(' '), _owrt(' ') {}

  AddonCombFareClassInfoKey(const FareClassCodeC& addonFareClass,
                            const Indicator geoAppl,
                            const Indicator owrt,
                            const FareClassCodeC& fareClass)
    : _addonFareClass(addonFareClass), _geoAppl(geoAppl), _owrt(owrt), _fareClass(fareClass)
  {
  }

  AddonCombFareClassInfoKey(const AddonCombFareClassInfoKey& rhs)
    : _addonFareClass(rhs._addonFareClass),
      _geoAppl(rhs._geoAppl),
      _owrt(rhs._owrt),
      _fareClass(rhs._fareClass)
  {
  }

  AddonCombFareClassInfoKey& operator=(const AddonCombFareClassInfoKey& rhs);

  friend bool operator<(const AddonCombFareClassInfoKey& x, const AddonCombFareClassInfoKey& y);

  bool operator==(const AddonCombFareClassInfoKey& rhs) const
  {
    return _geoAppl == rhs._geoAppl && _owrt == rhs._owrt &&
           _addonFareClass == rhs._addonFareClass && _fareClass == rhs._fareClass;
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const AddonCombFareClassInfoKey& obj)
  {
    return os << "|" << obj._addonFareClass << "|" << obj._geoAppl << "|" << obj._owrt << "|"
              << obj._fareClass << "]";
  }

  static void dummyData(AddonCombFareClassInfoKey& obj);

  static void dummyData2(AddonCombFareClassInfoKey& obj);

  void flattenize(Flattenizable::Archive& archive);

  const FareClassCodeC& addonFareClass() const { return _addonFareClass; }
  Indicator geoAppl() const { return _geoAppl; }
  Indicator owrt() const { return _owrt; }

  const FareClassCodeC& fareClass() const { return _fareClass; }

protected:
  FareClassCodeC _addonFareClass;
  Indicator _geoAppl;
  Indicator _owrt;
  FareClassCodeC _fareClass;

};

class AddonCombFareClassInfoKey1
{
public:
  // construction/destruction
  // ========================

  AddonCombFareClassInfoKey1() : _fareTariff(0) {}

  AddonCombFareClassInfoKey1(const VendorCode& vendor,
                             const TariffNumber& fareTariff,
                             const CarrierCode& carrier)
    : _vendor(vendor), _fareTariff(fareTariff), _carrier(carrier)
  {
  }

  AddonCombFareClassInfoKey1(const AddonCombFareClassInfoKey1& rhs)
    : _vendor(rhs._vendor), _fareTariff(rhs._fareTariff), _carrier(rhs._carrier) {};

  AddonCombFareClassInfoKey1& operator=(const AddonCombFareClassInfoKey1& rhs);

  friend bool operator<(const AddonCombFareClassInfoKey1& x, const AddonCombFareClassInfoKey1& y);

  bool operator==(const AddonCombFareClassInfoKey1& rhs) const
  {
    return _fareTariff == rhs._fareTariff && _vendor == rhs._vendor && _carrier == rhs._carrier;
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const AddonCombFareClassInfoKey1& obj)
  {
    return os << "[" << obj._vendor << "|" << obj._fareTariff << "|" << obj._carrier << "]";
  }

  void flattenize(Flattenizable::Archive& archive);

protected:
  VendorCode _vendor;
  TariffNumber _fareTariff;
  CarrierCode _carrier;

};

struct addonCombFareClassHashEqual
{
  bool
  operator()(const AddonCombFareClassInfoKey& first, const AddonCombFareClassInfoKey& second) const
  {
    return first == second;
  }

};

struct P05Hash
{
  size_t operator()(const AddonCombFareClassInfoKey& key) const;

private:
  static constexpr int RADIX = 37;
  static constexpr int DIV = 393241;

  static int hashAlphanum(int x)
  {
    if (isupper(x))
      x -= 'A';
    else
      x -= '0' - 'Z' + 'A' - 1;
    return x;
  }
};

typedef std::tr1::unordered_multimap<const AddonCombFareClassInfoKey,
                                     AddonCombFareClassInfo*,
                                     P05Hash,
                                     addonCombFareClassHashEqual> ADDONHASHMULTI;

class AddonFareClassCombMultiMap : public ADDONHASHMULTI
{
public:
  virtual ~AddonFareClassCombMultiMap() {}

  friend std::ostream& dumpObject(std::ostream& os, const AddonFareClassCombMultiMap& obj);

  virtual bool operator==(const AddonFareClassCombMultiMap& rhs) const;

  void flattenize(Flattenizable::Archive& archive);

  static void dummyData(AddonFareClassCombMultiMap& obj);

  typedef std::vector<AddonCombFareClassInfo*> INFOVEC;
  typedef std::map<AddonCombFareClassInfoKey, INFOVEC, std::less<AddonCombFareClassInfoKey> >
  INFOMAP;

};

typedef AddonFareClassCombMultiMap::const_iterator AddonFareClassCombMultiMapIterator;

// yet another map of maps

typedef std::map<AddonCombFareClassInfoKey1, AddonFareClassCombMultiMap*> AddonFareClassVCCombMap;

typedef AddonFareClassVCCombMap::const_iterator AddonFareClassCombMultiMapMapIterator;

} // tse

#endif

