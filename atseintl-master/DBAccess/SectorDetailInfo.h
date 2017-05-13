//----------------------------------------------------------------------------
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class SectorDetailInfo
{
  friend inline std::ostream& dumpObject(std::ostream& os, const SectorDetailInfo& obj);
public:
  std::ostream& print(std::ostream& out) const;

  bool operator<(const SectorDetailInfo& rhs) const;

  bool operator==(const SectorDetailInfo& rhs) const;

  static void dummyData(SectorDetailInfo& obj);

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

  Indicator& applyTag() { return _applyTag; }
  const Indicator& applyTag() const { return _applyTag; }

  FareBasisTktDesignator& fareBasisTktDesignator() { return _fareBasisTktDesignator; }
  const FareBasisTktDesignator& fareBasisTktDesignator() const { return _fareBasisTktDesignator; }

  EquipmentType& equipmentCode() { return _equipmentCode; }
  const EquipmentType& equipmentCode() const { return _equipmentCode; }

  Indicator& cabinCode() { return _cabinCode; }
  const Indicator& cabinCode() const { return _cabinCode; }

  Indicator& rbd1() { return _rbd1; }
  const Indicator& rbd1() const { return _rbd1; }

  Indicator& rbd2() { return _rbd2; }
  const Indicator& rbd2() const { return _rbd2; }

  Indicator& rbd3() { return _rbd3; }
  const Indicator& rbd3() const { return _rbd3; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  FareTariffInd& fareTariffInd() { return _fareTariffInd; }
  const FareTariffInd& fareTariffInd() const { return _fareTariffInd; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  FareOwningCxr& fareOwningCxr() { return _fareOwningCxr; }
  const FareOwningCxr& fareOwningCxr() const { return _fareOwningCxr; }

  TktCode& tktCode() { return _tktCode; }
  const TktCode& tktCode() const { return _tktCode; }

  int& versionNbr() { return _versionNbr; }
  const int& versionNbr() const { return _versionNbr; }

private:
  VendorCode _vendor;
  int _itemNo = 0;
  int _seqNo = 0;
  DateTime _createDate;
  DateTime _lockDate;
  DateTime _expireDate;
  DateTime _lastModificationDate;
  Indicator _applyTag = ' ';
  FareBasisTktDesignator _fareBasisTktDesignator;
  EquipmentType _equipmentCode;
  Indicator _cabinCode = ' ';
  Indicator _rbd1 = ' ';
  Indicator _rbd2 = ' ';
  Indicator _rbd3 = ' ';
  FareType _fareType;
  FareTariffInd _fareTariffInd;
  TariffNumber _fareTariff = 0;
  RuleNumber _rule;
  FareOwningCxr _fareOwningCxr;
  TktCode _tktCode;
  int _versionNbr = 0;

};

} // tse
