// ---------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/SmallBitSet.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <stdlib.h>

namespace tse
{

class NegFareRestExtSeq
{

public:
  NegFareRestExtSeq() : _itemNo(0), _seqNo(0), _suppressNvbNva(' ') {}
  virtual ~NegFareRestExtSeq() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  int itemNo() const { return _itemNo; }

  int& seqNo() { return _seqNo; }
  int seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  LocCode& cityFrom() { return _cityFrom; }
  const LocCode& cityFrom() const { return _cityFrom; }

  LocCode& cityTo() { return _cityTo; }
  const LocCode& cityTo() const { return _cityTo; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  LocCode& viaCity1() { return _viaCity1; }
  const LocCode& viaCity1() const { return _viaCity1; }

  LocCode& viaCity2() { return _viaCity2; }
  const LocCode& viaCity2() const { return _viaCity2; }

  LocCode& viaCity3() { return _viaCity3; }
  const LocCode& viaCity3() const { return _viaCity3; }

  LocCode& viaCity4() { return _viaCity4; }
  const LocCode& viaCity4() const { return _viaCity4; }

  FareClassCode& publishedFareBasis() { return _publishedFareBasis; }
  const FareClassCode& publishedFareBasis() const { return _publishedFareBasis; }

  FareClassCode& uniqueFareBasis() { return _uniqueFareBasis; }
  const FareClassCode& uniqueFareBasis() const { return _uniqueFareBasis; }

  Indicator& suppressNvbNva() { return _suppressNvbNva; }
  const Indicator& suppressNvbNva() const { return _suppressNvbNva; }

  virtual bool operator==(const NegFareRestExtSeq& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_cityFrom == rhs._cityFrom) && (_cityTo == rhs._cityTo) &&
            (_carrier == rhs._carrier) && (_viaCity1 == rhs._viaCity1) &&
            (_viaCity2 == rhs._viaCity2) && (_viaCity3 == rhs._viaCity3) &&
            (_viaCity4 == rhs._viaCity4) && (_publishedFareBasis == rhs._publishedFareBasis) &&
            (_uniqueFareBasis == rhs._uniqueFareBasis) && (_suppressNvbNva == rhs._suppressNvbNva));
  }

  static void dummyData(NegFareRestExtSeq& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._cityFrom = "LON";
    obj._cityTo = "DFW";
    obj._carrier = "AA";
    obj._viaCity1 = "PAR";
    obj._viaCity2 = "CHI";
    obj._viaCity3 = "WAS";
    obj._viaCity4 = "MIA";
    obj._publishedFareBasis = "ABC";
    obj._uniqueFareBasis = "XYZ";
    obj._suppressNvbNva = 'A';
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
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  LocCode _cityFrom;
  LocCode _cityTo;
  CarrierCode _carrier;
  LocCode _viaCity1;
  LocCode _viaCity2;
  LocCode _viaCity3;
  LocCode _viaCity4;
  FareClassCode _publishedFareBasis;
  FareClassCode _uniqueFareBasis;
  Indicator _suppressNvbNva;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _cityFrom);
    FLATTENIZE(archive, _cityTo);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _viaCity1);
    FLATTENIZE(archive, _viaCity2);
    FLATTENIZE(archive, _viaCity3);
    FLATTENIZE(archive, _viaCity4);
    FLATTENIZE(archive, _publishedFareBasis);
    FLATTENIZE(archive, _uniqueFareBasis);
    FLATTENIZE(archive, _suppressNvbNva);
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
           & ptr->_seqNo
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_cityFrom
           & ptr->_cityTo
           & ptr->_carrier
           & ptr->_viaCity1
           & ptr->_viaCity2
           & ptr->_viaCity3
           & ptr->_viaCity4
           & ptr->_publishedFareBasis
           & ptr->_uniqueFareBasis
           & ptr->_suppressNvbNva;
  }

  NegFareRestExtSeq(const NegFareRestExtSeq&);
  NegFareRestExtSeq& operator=(const NegFareRestExtSeq&);
};

} // end tse namespace

