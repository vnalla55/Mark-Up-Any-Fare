//----------------------------------------------------------------------------
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class PaxTypeCodeInfo
{
  friend inline std::ostream& dumpObject(std::ostream& os, const PaxTypeCodeInfo& obj);
public:
  PaxTypeCodeInfo();

  std::ostream& print(std::ostream& out) const;

  static void dummyData(PaxTypeCodeInfo& obj);

  void flattenize(Flattenizable::Archive& archive);

  bool operator<(const PaxTypeCodeInfo& rhs) const;

  bool operator==(const PaxTypeCodeInfo& rhs) const;

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

  Indicator& applyTag() { return _applyTag; }
  const Indicator& applyTag() const { return _applyTag; }

  PaxTypeCode& psgrType() { return _psgrType; }
  const PaxTypeCode& psgrType() const { return _psgrType; }

  int& paxMinAge() { return _paxMinAge; }
  const int& paxMinAge() const { return _paxMinAge; }

  int& paxMaxAge() { return _paxMaxAge; }
  const int& paxMaxAge() const { return _paxMaxAge; }

  Indicator& paxStatus() { return _paxStatus; }
  const Indicator& paxStatus() const { return _paxStatus; }

  LocKey& loc() { return _loc; }
  const LocKey& loc() const { return _loc; }

  Indicator& ptcMatchIndicator() { return _ptcMatchIndicator; }
  const Indicator& ptcMatchIndicator() const { return _ptcMatchIndicator; }

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
  Indicator _applyTag;
  PaxTypeCode _psgrType;
  int _paxMinAge;
  int _paxMaxAge;
  Indicator _paxStatus;
  LocKey _loc;
  Indicator _ptcMatchIndicator;
  int _versionNbr;

};

} // tse

