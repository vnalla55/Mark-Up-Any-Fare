//----------------------------------------------------------------------------
//     © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------
#ifndef VENDORCROSSREF_H
#define VENDORCROSSREF_H

#include "DBAccess/Flattenizable.h"

namespace tse
{
class VendorCrossRef
{
private:
  VendorCode _vendor;
  Indicator _vendorType;
  VendorCode _tariffCrossRefVendor;
  VendorCode _ruleCategoryVendor;
  DateTime _createDate;

public:
  VendorCrossRef() : _vendorType(' ') {}

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _vendorType);
    FLATTENIZE(archive, _tariffCrossRefVendor);
    FLATTENIZE(archive, _ruleCategoryVendor);
    FLATTENIZE(archive, _createDate);
  }

  bool operator==(const VendorCrossRef& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_vendorType == rhs._vendorType) &&
            (_tariffCrossRefVendor == rhs._tariffCrossRefVendor) &&
            (_ruleCategoryVendor == rhs._ruleCategoryVendor) && (_createDate == rhs._createDate));
  }

  static void dummyData(VendorCrossRef& obj)
  {
    obj.vendor() = "ABCD";
    obj.vendorType() = 'D';
    obj.tariffCrossRefVendor() = "EFGH";
    obj.ruleCategoryVendor() = "IJKL";
    obj.createDate() = time(nullptr);
  }

  VendorCode& vendor() { return _vendor; }
  Indicator& vendorType() { return _vendorType; }
  VendorCode& tariffCrossRefVendor() { return _tariffCrossRefVendor; }
  VendorCode& ruleCategoryVendor() { return _ruleCategoryVendor; }
  DateTime& createDate() { return _createDate; }

  const VendorCode& vendor() const { return _vendor; }
  const Indicator& vendorType() const { return _vendorType; }
  const VendorCode& tariffCrossRefVendor() const { return _tariffCrossRefVendor; }
  const VendorCode& ruleCategoryVendor() const { return _ruleCategoryVendor; }
  const DateTime& createDate() const { return _createDate; }
};
}
#endif
