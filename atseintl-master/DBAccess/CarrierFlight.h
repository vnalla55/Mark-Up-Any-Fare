//----------------------------------------------------------------------------
//     2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class CarrierFlight
{
public:
  CarrierFlight() : _itemNo(0), _segCnt(0), _inhibit(' ') {}

  ~CarrierFlight()
  {
    std::vector<CarrierFlightSeg*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    { // Nuke 'em!
      delete *SegIt;
    }
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

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<CarrierFlightSeg*>& segs() { return _segs; }
  const std::vector<CarrierFlightSeg*>& segs() const { return _segs; }

  bool operator==(const CarrierFlight& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_segCnt == rhs._segCnt) && (_inhibit == rhs._inhibit) &&
            (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(CarrierFlight& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._segCnt = 2;
    obj._inhibit = 'E';

    CarrierFlightSeg* cfs1 = new CarrierFlightSeg;
    CarrierFlightSeg* cfs2 = new CarrierFlightSeg;

    CarrierFlightSeg::dummyData(*cfs1);
    CarrierFlightSeg::dummyData(*cfs2);

    obj._segs.push_back(cfs1);
    obj._segs.push_back(cfs2);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  VendorCode _vendor;
  int _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  int _segCnt;
  Indicator _inhibit;
  std::vector<CarrierFlightSeg*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _segs);
  }

protected:
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_itemNo
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_segCnt
           & ptr->_inhibit
           & ptr->_segs;
  }

  CarrierFlight(const CarrierFlight&);
  CarrierFlight& operator=(const CarrierFlight&);
};

}// tse

