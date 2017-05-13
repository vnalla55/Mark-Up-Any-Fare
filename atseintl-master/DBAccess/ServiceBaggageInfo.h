//----------------------------------------------------------------------------
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class ServiceBaggageInfo
{
  friend inline std::ostream& dumpObject(std::ostream& os, const ServiceBaggageInfo& obj);
public:
  ServiceBaggageInfo();

  std::ostream& print(std::ostream& out) const;

  bool operator<(const ServiceBaggageInfo& rhs) const;

  bool operator==(const ServiceBaggageInfo& rhs) const;

  static void dummyData(ServiceBaggageInfo& obj);

  void flattenize(Flattenizable::Archive& archive);

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lastModificationDate() { return _lastModificationDate; }
  const DateTime& lastModificationDate() const { return _lastModificationDate; }

  SvcType& svcType() { return _svcType; }
  const SvcType& svcType() const { return _svcType; }

  FeeOwnerCxr& feeOwnerCxr() { return _feeOwnerCxr; }
  const FeeOwnerCxr& feeOwnerCxr() const { return _feeOwnerCxr; }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  TaxTypeSubCode& taxTypeSubCode() { return _taxTypeSubCode; }
  const TaxTypeSubCode& taxTypeSubCode() const { return _taxTypeSubCode; }

  AttrSubGroup& attrSubGroup() { return _attrSubGroup; }
  const AttrSubGroup& attrSubGroup() const { return _attrSubGroup; }

  AttrGroup& attrGroup() { return _attrGroup; }
  const AttrGroup& attrGroup() const { return _attrGroup; }

  Indicator& applyTag() { return _applyTag; }
  const Indicator& applyTag() const { return _applyTag; }

  int& versionNbr() { return _versionNbr; }
  const int& versionNbr() const { return _versionNbr; }

private:
  VendorCode _vendor;
  int _itemNo;
  int _seqNo;
  DateTime _createDate;
  DateTime _lockDate;
  DateTime _expireDate;
  DateTime _lastModificationDate;
  SvcType _svcType;
  FeeOwnerCxr _feeOwnerCxr;
  TaxCode _taxCode;
  TaxTypeSubCode _taxTypeSubCode;
  AttrSubGroup _attrSubGroup;
  AttrGroup _attrGroup;
  Indicator _applyTag;
  int _versionNbr;

};

} // tse

