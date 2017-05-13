//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class TaxCarrierFlightInfo
{
public:
  TaxCarrierFlightInfo() : _itemNo(0), _segCnt(0) {}

  ~TaxCarrierFlightInfo()
  {
    std::vector<CarrierFlightSeg*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    {
      delete *SegIt;
    }
    _segs.clear();
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  std::vector<CarrierFlightSeg*>& segs() { return _segs; }
  const std::vector<CarrierFlightSeg*>& segs() const { return _segs; }

  bool operator==(const TaxCarrierFlightInfo& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_segCnt == rhs._segCnt) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(TaxCarrierFlightInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._segCnt = 2;

    obj._segs.push_back(new CarrierFlightSeg);
    obj._segs.push_back(new CarrierFlightSeg);

    CarrierFlightSeg::dummyData(*obj._segs[0]);
    CarrierFlightSeg::dummyData(*obj._segs[1]);
  }

protected:
  VendorCode _vendor;
  int _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  int _segCnt;
  std::vector<CarrierFlightSeg*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _segs);
  }

protected:
private:
  TaxCarrierFlightInfo(const TaxCarrierFlightInfo&);
  TaxCarrierFlightInfo& operator=(const TaxCarrierFlightInfo&);
};
}
