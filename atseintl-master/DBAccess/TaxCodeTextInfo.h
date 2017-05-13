//----------------------------------------------------------------------------
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class TaxCodeTextInfo
{
  friend inline std::ostream& dumpObject(std::ostream& os, const TaxCodeTextInfo& obj);
public:
  TaxCodeTextInfo();

  std::ostream& print(std::ostream& out) const;

  static void dummyData(TaxCodeTextInfo& obj);

  void flattenize(Flattenizable::Archive& archive);

  bool operator<(const TaxCodeTextInfo& rhs) const;

  bool operator==(const TaxCodeTextInfo& rhs) const;

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

  std::string& taxCodeText() { return _taxCodeText; }
  const std::string& taxCodeText() const { return _taxCodeText; }

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
  std::string _taxCodeText;
  int _versionNbr;

};

} // tse

