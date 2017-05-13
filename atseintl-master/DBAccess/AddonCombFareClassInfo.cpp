//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

#include "DBAccess/AddonCombFareClassInfo.h"

#include "Common/ObjectComparison.h"
#include "DBAccess/CompressedDataUtils.h"

#include <boost/functional/hash.hpp>

namespace tse
{

DateTime AddonCombFareClassInfo::_infiniteDiscontinue = DateTime(max_date_time);

bool
AddonCombFareClassInfo::
operator<(const AddonCombFareClassInfo& rhs) const
{
  if (_createDate != rhs._createDate)
    return _createDate < rhs._createDate;
  if (_expireDate != rhs._expireDate)
    return _expireDate < rhs._expireDate;
  if (_fareClass != rhs._fareClass)
    return (_fareClass < rhs._fareClass);
  if (_addonFareClass != rhs._addonFareClass)
    return (_addonFareClass < rhs._addonFareClass);
  if (_geoAppl != rhs._geoAppl)
    return (_geoAppl < rhs._geoAppl);
  if (_owrt != rhs._owrt)
    return (_owrt < rhs._owrt);

  return false;
}

bool
AddonCombFareClassInfo::
operator==(const AddonCombFareClassInfo& rhs) const
{
  return ((_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
          (_addonFareClass == rhs._addonFareClass) && (_geoAppl == rhs._geoAppl) &&
          (_owrt == rhs._owrt) && (_fareClass == rhs._fareClass));
}

void
AddonCombFareClassInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _fareClass);
  FLATTENIZE(archive, _addonFareClass);
  FLATTENIZE(archive, _geoAppl);
  FLATTENIZE(archive, _owrt);
}

void
AddonCombFareClassInfo::dummyData(AddonCombFareClassInfo& obj)
{
  obj._createDate = time(nullptr);
  obj._expireDate = time(nullptr);

  obj._addonFareClass = 'H';
  obj._geoAppl = 'P';
  obj._owrt = 'Q';
  obj._fareClass = "RSTUVWXY";
}

void
AddonCombFareClassInfo::dummyData2(AddonCombFareClassInfo& obj)
{
  obj._createDate = time(nullptr);
  obj._expireDate = time(nullptr);

  obj._addonFareClass = 'h';
  obj._geoAppl = 'p';
  obj._owrt = 'q';
  obj._fareClass = "rstuvwxy";
}

void
AddonCombFareClassInfo::dummyData(AddOnCombFareClassKey& key, AddonCombFareClassInfo& obj)
{
  obj._createDate = time(nullptr);
  obj._expireDate = time(nullptr);

  key = AddOnCombFareClassKey("ABCD", 1, "EFG");
  obj._addonFareClass = 'H';
  obj._geoAppl = 'P';
  obj._owrt = 'Q';
  obj._fareClass = "RSTUVWXY";
}

WBuffer&
AddonCombFareClassInfo::write(WBuffer& os) const
{
  return convert(os, this);
}

RBuffer&
AddonCombFareClassInfo::read(RBuffer& is)
{
  return convert(is, this);
}

std::ostream&
dumpObject(std::ostream& os, const AddonCombFareClassInfo& obj)
{
  os << "[" << obj._createDate << "|" << obj._expireDate << "|"
     << obj._addonFareClass << "|" << obj._geoAppl << "|" << obj._owrt << "|" << obj._fareClass
     << "]";
  return os;
}

bool
SortACFCKeyLess::
operator()(const AddonCombFareClassInfo* first, const AddonCombFareClassInfo* second) const
{
  if (first->addonFareClass() < second->addonFareClass())
  {
    return true;
  }
  if (first->addonFareClass() > second->addonFareClass())
  {
    return false;
  }
  if (first->geoAppl() < second->geoAppl())
  {
    return true;
  }
  if (first->geoAppl() > second->geoAppl())
  {
    return false;
  }
  if (first->owrt() < second->owrt())
  {
    return true;
  }
  if (first->owrt() > second->owrt())
  {
    return false;
  }
  return first->fareClass() < second->fareClass();
}

AddonCombFareClassSpecifiedKey&
AddonCombFareClassSpecifiedKey::
operator=(const AddonCombFareClassSpecifiedKey& rhs)
{
  if (this != &rhs)
  {
    _specifiedFareClass = rhs._specifiedFareClass;
    _owrt = rhs._owrt;
  }
  return *this;
}

bool operator<(const AddonCombFareClassSpecifiedKey& x, const AddonCombFareClassSpecifiedKey& y)
{
  if (x._owrt < y._owrt)
    return true;
  if (x._owrt > y._owrt)
    return false;

  return x._specifiedFareClass < y._specifiedFareClass;
}

void
AddonCombFareClassSpecifiedKey::dummyData(AddonCombFareClassSpecifiedKey& obj)
{
  obj._specifiedFareClass = "aaaaaaaa";
  obj._owrt = '1';
}

void
AddonCombFareClassSpecifiedKey::dummyData2(AddonCombFareClassSpecifiedKey& obj)
{
  obj._specifiedFareClass = "bbbbbbbb";
  obj._owrt = '2';
}

void
AddonCombFareClassSpecifiedKey::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _specifiedFareClass);
  FLATTENIZE(archive, _owrt);
}

AddonCombFareClassInfoKey1&
AddonCombFareClassInfoKey1::
operator=(const AddonCombFareClassInfoKey1& rhs)
{
  if (this != &rhs)
  {
    _vendor = rhs._vendor;
    _fareTariff = rhs._fareTariff;
    _carrier = rhs._carrier;
  }

  return *this;
}

bool operator<(const AddonCombFareClassInfoKey1& x, const AddonCombFareClassInfoKey1& y)
{
  if (x.getFareTariff() < y.getFareTariff())
    return true;
  if (x.getFareTariff() > y.getFareTariff())
    return false;

  if (x.getVendor() < y.getVendor())
    return true;
  if (x.getVendor() > y.getVendor())
    return false;

  return x.getCarrier() < y.getCarrier();
}

std::ostream& dumpObject(std::ostream& os, const AddonCombFareClassInfoKey1& obj)
{
  return os << "[" << obj.getVendor() << "|" << obj.getFareTariff() << "|" << obj.getCarrier() << "]";
}

void
AddonCombFareClassInfoKey1::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _fareTariff);
  FLATTENIZE(archive, _carrier);
}

size_t
P05SpecHash::operator()(const AddonCombFareClassSpecifiedKey& key) const
{
  std::size_t hash(boost::hash_value(key.owrt()));
  boost::hash_range(hash, key.specifiedFareClass().begin(),
                    key.specifiedFareClass().end());
  return hash;
}

void
AddonFareClassCombMultiMap::dummyData(AddonFareClassCombMultiMap& obj)
{
  AddonCombFareClassSpecifiedKey key1;
  AddonCombFareClassSpecifiedKey key2;

  AddonCombFareClassSpecifiedKey::dummyData(key1);
  AddonCombFareClassSpecifiedKey::dummyData2(key2);

  AddonCombFareClassInfo* acfci1 = new AddonCombFareClassInfo;
  AddonCombFareClassInfo* acfci2 = new AddonCombFareClassInfo;

  AddonCombFareClassInfo::dummyData(*acfci1);
  AddonCombFareClassInfo::dummyData2(*acfci2);

  obj[key1].push_back(acfci1);
  obj[key2].push_back(acfci2);
}

void
AddonFareClassCombMultiMap::flattenize(Flattenizable::Archive& archive)
{
  ADDONSPECHASHMAP* pBaseClass = static_cast<ADDONSPECHASHMAP*>(this);
  FLATTENIZE(archive, *pBaseClass);
}

bool
AddonFareClassCombMultiMap::
operator==(const AddonFareClassCombMultiMap& rhs) const
{
  const ADDONSPECHASHMAP& obj1(static_cast<const ADDONSPECHASHMAP&>(*this));
  const ADDONSPECHASHMAP& obj2(static_cast<const ADDONSPECHASHMAP&>(rhs));
  return objectsIdentical(obj1, obj2);
}

} // tse

#else

#include "DBAccess/AddonCombFareClassInfo.h"

#include "Common/ObjectComparison.h"
#include "DBAccess/CompressedDataUtils.h"

#include <boost/functional/hash.hpp>

namespace tse
{

bool
AddonCombFareClassInfo::
operator<(const AddonCombFareClassInfo& rhs) const
{
  if (_effInterval != rhs._effInterval)
    return (_effInterval < rhs._effInterval);
  if (_vendor != rhs._vendor)
    return (_vendor < rhs._vendor);
  if (_fareTariff != rhs._fareTariff)
    return (_fareTariff < rhs._fareTariff);
  if (_carrier != rhs._carrier)
    return (_carrier < rhs._carrier);
  if (_addonFareClass != rhs._addonFareClass)
    return (_addonFareClass < rhs._addonFareClass);
  if (_geoAppl != rhs._geoAppl)
    return (_geoAppl < rhs._geoAppl);
  if (_owrt != rhs._owrt)
    return (_owrt < rhs._owrt);
  if (_fareClass != rhs._fareClass)
    return (_fareClass < rhs._fareClass);

  return false;
}

bool
AddonCombFareClassInfo::
operator==(const AddonCombFareClassInfo& rhs) const
{
  return ((_effInterval == rhs._effInterval) && (_vendor == rhs._vendor) &&
          (_fareTariff == rhs._fareTariff) && (_carrier == rhs._carrier) &&
          (_addonFareClass == rhs._addonFareClass) && (_geoAppl == rhs._geoAppl) &&
          (_owrt == rhs._owrt) && (_fareClass == rhs._fareClass));
}

void
AddonCombFareClassInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _effInterval);
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _fareTariff);
  FLATTENIZE(archive, _carrier);
  FLATTENIZE(archive, _addonFareClass);
  FLATTENIZE(archive, _geoAppl);
  FLATTENIZE(archive, _owrt);
  FLATTENIZE(archive, _fareClass);
}

void
AddonCombFareClassInfo::dummyData(AddonCombFareClassInfo& obj)
{
  TSEDateInterval::dummyData(obj._effInterval);

  obj._vendor = "ABCD";
  obj._fareTariff = 1;
  obj._carrier = "EFG";
  obj._addonFareClass = "HIJKLMNO";
  obj._geoAppl = 'P';
  obj._owrt = 'Q';
  obj._fareClass = "RSTUVWXY";
}

void
AddonCombFareClassInfo::dummyData2(AddonCombFareClassInfo& obj)
{
  TSEDateInterval::dummyData(obj._effInterval);

  obj._vendor = "abcd";
  obj._fareTariff = 2;
  obj._carrier = "efg";
  obj._addonFareClass = "hijklmno";
  obj._geoAppl = 'p';
  obj._owrt = 'q';
  obj._fareClass = "rstuvwxy";
}

WBuffer&
AddonCombFareClassInfo::write(WBuffer& os) const
{
  return convert(os, this);
}

RBuffer&
AddonCombFareClassInfo::read(RBuffer& is)
{
  return convert(is, this);
}

std::ostream&
dumpObject(std::ostream& os, const AddonCombFareClassInfo& obj)
{
  os << "[";
  dumpObject(os, obj._effInterval);
  os << "|" << obj._vendor << "|" << obj._fareTariff << "|" << obj._carrier << "|"
     << obj._addonFareClass << "|" << obj._geoAppl << "|" << obj._owrt << "|" << obj._fareClass
     << "]";
  return os;
}

bool
SortACFCKeyLess::
operator()(const AddonCombFareClassInfo* first, const AddonCombFareClassInfo* second) const
{
  if (first->geoAppl() < second->geoAppl())
  {
    return true;
  }
  if (first->geoAppl() > second->geoAppl())
  {
    return false;
  }
  if (first->owrt() < second->owrt())
  {
    return true;
  }
  if (first->owrt() > second->owrt())
  {
    return false;
  }
  if (first->addonFareClass() < second->addonFareClass())
  {
    return true;
  }
  if (first->addonFareClass() > second->addonFareClass())
  {
    return false;
  }
  return first->fareClass() < second->fareClass();
}

AddonCombFareClassInfoKey&
AddonCombFareClassInfoKey::
operator=(const AddonCombFareClassInfoKey& rhs)
{
  if (this != &rhs)
  {
    _addonFareClass = rhs._addonFareClass;
    _geoAppl = rhs._geoAppl;
    _owrt = rhs._owrt;
    _fareClass = rhs._fareClass;
  }

  return *this;
}

bool operator<(const AddonCombFareClassInfoKey& x, const AddonCombFareClassInfoKey& y)
{
  if (x._geoAppl < y._geoAppl)
    return true;
  if (x._geoAppl > y._geoAppl)
    return false;

  if (x._owrt < y._owrt)
    return true;
  if (x._owrt > y._owrt)
    return false;

  if (x._addonFareClass < y._addonFareClass)
    return true;
  if (x._addonFareClass > y._addonFareClass)
    return false;

  return x._fareClass < y._fareClass;
}

void
AddonCombFareClassInfoKey::dummyData(AddonCombFareClassInfoKey& obj)
{
  obj._addonFareClass = "aaaaaaaa";
  obj._geoAppl = 'A';
  obj._owrt = '1';
  obj._fareClass = "bbbbbbbb";
}

void
AddonCombFareClassInfoKey::dummyData2(AddonCombFareClassInfoKey& obj)
{
  obj._addonFareClass = "cccccccc";
  obj._geoAppl = 'B';
  obj._owrt = '2';
  obj._fareClass = "dddddddd";
}

void
AddonCombFareClassInfoKey::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _addonFareClass);
  FLATTENIZE(archive, _geoAppl);
  FLATTENIZE(archive, _owrt);
  FLATTENIZE(archive, _fareClass);
}

AddonCombFareClassInfoKey1&
AddonCombFareClassInfoKey1::
operator=(const AddonCombFareClassInfoKey1& rhs)
{
  if (this != &rhs)
  {
    _vendor = rhs._vendor;
    _fareTariff = rhs._fareTariff;
    _carrier = rhs._carrier;
  }

  return *this;
}

bool operator<(const AddonCombFareClassInfoKey1& x, const AddonCombFareClassInfoKey1& y)
{
  if (x._fareTariff < y._fareTariff)
    return true;
  if (x._fareTariff > y._fareTariff)
    return false;

  if (x._vendor < y._vendor)
    return true;
  if (x._vendor > y._vendor)
    return false;

  return x._carrier < y._carrier;
}

void
AddonCombFareClassInfoKey1::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _fareTariff);
  FLATTENIZE(archive, _carrier);
}

size_t
P05Hash::
operator()(const AddonCombFareClassInfoKey& key) const
{
  std::size_t hash(boost::hash_value(*key.addonFareClass().c_str()));
  boost::hash_combine(hash, key.geoAppl());
  boost::hash_combine(hash, key.owrt());
  boost::hash_range(hash, key.fareClass().begin(), key.fareClass().end());
  return hash;
}

void
AddonFareClassCombMultiMap::dummyData(AddonFareClassCombMultiMap& obj)
{
  AddonCombFareClassInfoKey key1;
  AddonCombFareClassInfoKey key2;

  AddonCombFareClassInfoKey::dummyData(key1);
  AddonCombFareClassInfoKey::dummyData2(key2);

  AddonCombFareClassInfo* acfci1 = new AddonCombFareClassInfo;
  AddonCombFareClassInfo* acfci2 = new AddonCombFareClassInfo;

  AddonCombFareClassInfo::dummyData(*acfci1);
  AddonCombFareClassInfo::dummyData2(*acfci2);

  obj.insert(AddonFareClassCombMultiMap::value_type(key1, acfci1));
  obj.insert(AddonFareClassCombMultiMap::value_type(key2, acfci2));
}

void
AddonFareClassCombMultiMap::flattenize(Flattenizable::Archive& archive)
{
  ADDONHASHMULTI* pBaseClass = static_cast<ADDONHASHMULTI*>(this);
  FLATTENIZE(archive, *pBaseClass);
}

bool
AddonFareClassCombMultiMap::
operator==(const AddonFareClassCombMultiMap& rhs) const
{
  const ADDONHASHMULTI& obj1(static_cast<const ADDONHASHMULTI&>(*this));
  const ADDONHASHMULTI& obj2(static_cast<const ADDONHASHMULTI&>(rhs));
  return objectsIdentical(obj1, obj2);
}

std::ostream&
dumpObject(std::ostream& os, const tse::AddonFareClassCombMultiMap& obj)
{
  const ADDONHASHMULTI& refBaseClass(static_cast<const ADDONHASHMULTI&>(obj));
  dumpObject(os, refBaseClass);
  return os;
}

} // tse

#endif

