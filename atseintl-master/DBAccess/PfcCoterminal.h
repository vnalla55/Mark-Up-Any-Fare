//----------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef PFCCOTERMINAL_H
#define PFCCOTERMINAL_H

#include "DBAccess/Flattenizable.h"

namespace tse
{

class PfcCoterminal
{
public:
  PfcCoterminal() : _orderNo(0) {}
  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  LocCode& cotermLoc() { return _cotermLoc; }
  const LocCode& cotermLoc() const { return _cotermLoc; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  bool operator==(const PfcCoterminal& rhs) const
  {
    return ((_orderNo == rhs._orderNo) && (_cotermLoc == rhs._cotermLoc) &&
            (_vendor == rhs._vendor));
  }

  static void dummyData(PfcCoterminal& obj)
  {
    obj._orderNo = 1;
    obj._cotermLoc = "aaaaaaaa";
    obj._vendor = "ABCD";
  }

private:
  int _orderNo;
  LocCode _cotermLoc;
  VendorCode _vendor;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _cotermLoc);
    FLATTENIZE(archive, _vendor);
  }

};
}

#endif
