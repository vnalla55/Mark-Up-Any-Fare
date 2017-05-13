//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class Mileage
{
public:
  bool operator==(const Mileage& rhs) const
  {
    return ((_orig == rhs._orig) && (_dest == rhs._dest) && (_mileageType == rhs._mileageType) &&
            (_globaldir == rhs._globaldir) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_expireDate == rhs._expireDate) &&
            (_discDate == rhs._discDate) && (_mileage == rhs._mileage) && (_vendor == rhs._vendor));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(Mileage& obj)
  {
    obj._orig = "aaaaaaaa";
    obj._dest = "bbbbbbbb";
    obj._mileageType = 'A';
    obj._globaldir = GlobalDirection::US;
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._mileage = 1;
    obj._vendor = "BCDE";
  }

private:
  LocCode _orig;
  LocCode _dest;
  Indicator _mileageType = ' ';
  GlobalDirection _globaldir = GlobalDirection::NO_DIR;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _expireDate;
  DateTime _discDate;
  uint32_t _mileage = 0;
  VendorCode _vendor;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_orig & ptr->_dest & ptr->_mileageType & ptr->_globaldir &
           ptr->_createDate & ptr->_effDate & ptr->_expireDate & ptr->_discDate & ptr->_mileage &
           ptr->_vendor;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orig);
    FLATTENIZE(archive, _dest);
    FLATTENIZE(archive, _mileageType);
    FLATTENIZE(archive, _globaldir);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _mileage);
    FLATTENIZE(archive, _vendor);
  }

public:
  LocCode& orig() { return _orig; }
  const LocCode& orig() const { return _orig; }

  LocCode& dest() { return _dest; }
  const LocCode& dest() const { return _dest; }

  Indicator& mileageType() { return _mileageType; }
  const Indicator& mileageType() const { return _mileageType; }

  GlobalDirection& globaldir() { return _globaldir; }
  const GlobalDirection& globaldir() const { return _globaldir; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  uint32_t& mileage() { return _mileage; }
  const uint32_t& mileage() const { return _mileage; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }
};
}
