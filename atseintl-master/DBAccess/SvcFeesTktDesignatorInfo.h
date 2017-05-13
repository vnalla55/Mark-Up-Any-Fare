//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//   ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class SvcFeesTktDesignatorInfo
{
public:
  SvcFeesTktDesignatorInfo() : _itemNo(0), _seqNo(0) {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  TktDesignator& tktDesignator() { return _tktDesignator; }
  const TktDesignator& tktDesignator() const { return _tktDesignator; }

  bool operator==(const SvcFeesTktDesignatorInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_tktDesignator == rhs._tktDesignator));
  }

  static void dummyData(SvcFeesTktDesignatorInfo& obj)
  {
    SvcFeesTktDesignatorInfo::dummyData(obj);

    obj._vendor = "ATP";
    obj._itemNo = 1;
    obj._seqNo = 100;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._tktDesignator = "ID75";
  }

private:
  VendorCode _vendor;
  int _itemNo;
  int _seqNo;
  DateTime _expireDate;
  DateTime _createDate;
  TktDesignator _tktDesignator;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _tktDesignator);
  }

};
}
