// ----------------------------------------------------------------------------
//  � 2006, Sabre Inc.  All rights reserved.  This software/documentation
//    is the confidential and proprietary product of Sabre Inc. Any
//    unauthorized use, reproduction, or transfer of this
//    software/documentation, in any medium, or incorporation of this
//    software/documentation into any system or publication, is strictly
//    prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class SurfaceTransfersInfo
{
public:
  SurfaceTransfersInfo()
    : _vendor(""),
      _itemNo(0),
      _seqNo(0),
      _createDate(0),
      _expireDate(0),
      _restriction(' '),
      _originDest(' '),
      _fareBrkEmbeddedLoc(),
      _surfaceLoc(),
      _inhibit(' ')
  {
  }

  ~SurfaceTransfersInfo() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  uint32_t& itemNo() { return _itemNo; }
  const uint32_t& itemNo() const { return _itemNo; }

  uint32_t& seqNo() { return _seqNo; }
  const uint32_t& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& restriction() { return _restriction; }
  const Indicator& restriction() const { return _restriction; }

  Indicator& originDest() { return _originDest; }
  const Indicator& originDest() const { return _originDest; }

  LocKey& fareBrkEmbeddedLoc() { return _fareBrkEmbeddedLoc; }
  const LocKey& fareBrkEmbeddedLoc() const { return _fareBrkEmbeddedLoc; }

  LocKey& surfaceLoc() { return _surfaceLoc; }
  const LocKey& surfaceLoc() const { return _surfaceLoc; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  bool operator==(const SurfaceTransfersInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_restriction == rhs._restriction) && (_originDest == rhs._originDest) &&
            (_fareBrkEmbeddedLoc == rhs._fareBrkEmbeddedLoc) && (_surfaceLoc == rhs._surfaceLoc) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(SurfaceTransfersInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._restriction = 'E';
    obj._originDest = 'F';

    LocKey::dummyData(obj._fareBrkEmbeddedLoc);
    LocKey::dummyData(obj._surfaceLoc);

    obj._inhibit = 'G';
  }

private:
  VendorCode _vendor;
  uint32_t _itemNo;
  uint32_t _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _restriction;
  Indicator _originDest;
  LocKey _fareBrkEmbeddedLoc;
  LocKey _surfaceLoc;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _restriction);
    FLATTENIZE(archive, _originDest);
    FLATTENIZE(archive, _fareBrkEmbeddedLoc);
    FLATTENIZE(archive, _surfaceLoc);
    FLATTENIZE(archive, _inhibit);
  }

};
}
