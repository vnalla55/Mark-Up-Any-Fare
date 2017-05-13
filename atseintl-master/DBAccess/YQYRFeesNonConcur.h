// ----------------------------------------------------------------------------
// � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the
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

class YQYRFeesNonConcur
{
public:
  YQYRFeesNonConcur() : _selfAppl(' '), _taxCarrierApplTblItemNo(0) {}

  ~YQYRFeesNonConcur() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& selfAppl() { return _selfAppl; }
  const Indicator& selfAppl() const { return _selfAppl; }

  int& taxCarrierApplTblItemNo() { return _taxCarrierApplTblItemNo; }
  const int& taxCarrierApplTblItemNo() const { return _taxCarrierApplTblItemNo; }

  bool operator==(const YQYRFeesNonConcur& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_selfAppl == rhs._selfAppl) &&
            (_taxCarrierApplTblItemNo == rhs._taxCarrierApplTblItemNo));

    return eq;
  }
  static void dummyData(YQYRFeesNonConcur& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._selfAppl = 'H';
    obj._taxCarrierApplTblItemNo = 9;
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  // Not in Schema    Indicator   _inhibit;
  Indicator _selfAppl;
  int _taxCarrierApplTblItemNo;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _selfAppl);
    FLATTENIZE(archive, _taxCarrierApplTblItemNo);
  }
};
}
