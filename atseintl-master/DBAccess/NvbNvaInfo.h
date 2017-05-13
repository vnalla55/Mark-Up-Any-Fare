//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/Flattenizable.h"
#include "DBAccess/NvbNvaSeg.h"

namespace tse
{

class NvbNvaInfo
{
public:
  // constructor and destructor
  NvbNvaInfo()
    : _nvbNvaId(0),
      _vendor(),
      _carrier(),
      _ruleTariff(0),
      _rule(),
      _createDate(0),
      _expireDate(0),
      _segs()
  {
  }

  ~NvbNvaInfo()
  {
    std::vector<NvbNvaSeg*>::iterator segIt;
    for (segIt = _segs.begin(); segIt != _segs.end(); segIt++)
    {
      delete *segIt;
    }
  }

  // accessors
  uint64_t& nvbNvaId() { return _nvbNvaId; }
  const uint64_t& nvbNvaId() const { return _nvbNvaId; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<NvbNvaSeg*>& segs()
  {
    return _segs;
  };
  const std::vector<NvbNvaSeg*>& segs() const
  {
    return _segs;
  };

  // other public methods
  bool operator==(const NvbNvaInfo& second) const
  {
    bool equal = (_nvbNvaId == second._nvbNvaId) && (_vendor == second._vendor) &&
                 (_carrier == second._carrier) && (_ruleTariff == second._ruleTariff) &&
                 (_rule == second._rule) && (_createDate == second._createDate) &&
                 (_expireDate == second._expireDate) && (_segs.size() == second._segs.size());

    for (size_t i = 0; equal && (i < _segs.size()); ++i)
    {
      equal = (*_segs[i] == *second._segs[i]);
    }

    return equal;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    // NVBNVA table fields
    FLATTENIZE(archive, _nvbNvaId);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);

    // segment container
    FLATTENIZE(archive, _segs);
  }

  static void dummyData(NvbNvaInfo& obj)
  {
    // NVBNVA table fields
    obj._nvbNvaId = 54321;
    obj._vendor = "ABCD";
    obj._carrier = "EF";
    obj._ruleTariff = 14;
    obj._rule = "GHIJ";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);

    // create dummy segment data
    obj._segs.push_back(new NvbNvaSeg);
    obj._segs.push_back(new NvbNvaSeg);
    NvbNvaSeg::dummyData(*obj._segs[0]);
    NvbNvaSeg::dummyData(*obj._segs[1]);
  }

private:
  // NVBNVA table fields
  uint64_t _nvbNvaId;
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _ruleTariff;
  RuleNumber _rule;
  DateTime _createDate;
  DateTime _expireDate;

  // segment container
  std::vector<NvbNvaSeg*> _segs;

  // serialization method
  friend class SerializationTestBase;
}; // class NvbNvaInfo

} // namespace tse

