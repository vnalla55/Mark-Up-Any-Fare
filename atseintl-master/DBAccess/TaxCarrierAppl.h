// ----------------------------------------------------------------------------
// 2004, Sabre Inc.  All rights reserved.  This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use,
// reproduction, or transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system or publication,
// is strictly prohibited
// ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Flattenizable.h"
#include "DBAccess/TaxCarrierApplSeg.h"

namespace tse
{

class TaxCarrierAppl
{
public:
  TaxCarrierAppl() = default;
  TaxCarrierAppl(const TaxCarrierAppl&) = delete;
  TaxCarrierAppl& operator=(const TaxCarrierAppl&) = delete;

  ~TaxCarrierAppl()
  {
    for (const auto taxCarrierApplSeg : _segs)
      delete taxCarrierApplSeg;
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  std::vector<TaxCarrierApplSeg*>& segs() { return _segs; }
  const std::vector<TaxCarrierApplSeg*>& segs() const { return _segs; }

  bool operator==(const TaxCarrierAppl& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }
  static void dummyData(TaxCarrierAppl& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);

    TaxCarrierApplSeg* tcas1 = new TaxCarrierApplSeg;
    TaxCarrierApplSeg* tcas2 = new TaxCarrierApplSeg;

    TaxCarrierApplSeg::dummyData(*tcas1);
    TaxCarrierApplSeg::dummyData(*tcas2);

    obj._segs.push_back(tcas1);
    obj._segs.push_back(tcas2);
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
  int _itemNo = 0;
  DateTime _expireDate;
  DateTime _createDate;
  std::vector<TaxCarrierApplSeg*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _segs);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_itemNo
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_segs;
  }
};
}
