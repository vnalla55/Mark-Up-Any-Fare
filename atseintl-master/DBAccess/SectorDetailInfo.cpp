//----------------------------------------------------------------------------
//
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#include "DBAccess/SectorDetailInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{
std::ostream&
dumpObject(std::ostream& os, const SectorDetailInfo& obj)
{
  os << "[" << obj._vendor << "|" << obj._itemNo << "|" << obj._seqNo << "|" << obj._createDate
     << "|" << obj._lockDate << "|" << obj._expireDate << "|" << obj._lastModificationDate << "|"
     << obj._applyTag << "|" << obj._fareBasisTktDesignator << "|" << obj._equipmentCode << "|"
     << obj._cabinCode << "|" << obj._rbd1 << "|" << obj._rbd2 << "|" << obj._rbd3 << "|"
     << obj._fareType << "|" << obj._fareTariffInd << "|" << obj._fareTariff << "|" << obj._rule
     << "|" << obj._fareOwningCxr << "|" << obj._tktCode << "|" << obj._versionNbr;
  os << "]";

  return os;
}

std::ostream&
SectorDetailInfo::print(std::ostream& out) const
{
  out << "\nVENDOR:                 " << _vendor;
  out << "\nITEMNO:                 " << _itemNo;
  out << "\nSEQNO:                  " << _seqNo;
  out << "\nCREATEDATE:             " << _createDate;
  out << "\nLOCKDATE:               " << _lockDate;
  out << "\nEXPIREDATE:             " << _expireDate;
  out << "\nLASTMODDATE:            " << _lastModificationDate;
  out << "\nAPPLTAG:                " << _applyTag;
  out << "\nFAREBASISTKTDESIGNATOR: " << _fareBasisTktDesignator;
  out << "\nEQUIPMENTCODE:          " << _equipmentCode;
  out << "\nCABINCODE:              " << _cabinCode;
  out << "\nRBD1:                   " << _rbd1;
  out << "\nRBD2:                   " << _rbd2;
  out << "\nRBD3:                   " << _rbd3;
  out << "\nFARETYPE:               " << _fareType;
  out << "\nFARETARIFFIND:          " << _fareTariffInd;
  out << "\nFARETARIFF:             " << _fareTariff;
  out << "\nRULE:                   " << _rule;
  out << "\nFAREOWNINGCXR:          " << _fareOwningCxr;
  out << "\nTKTCODE:                " << _tktCode;
  out << "\nVERSIONNBR:             " << _versionNbr;
  out << "\n";
  return out;
}

bool
SectorDetailInfo::
operator<(const SectorDetailInfo& rhs) const
{
  if (_vendor != rhs._vendor)
    return (_vendor < rhs._vendor);
  if (_itemNo != rhs._itemNo)
    return (_itemNo < rhs._itemNo);
  if (_seqNo != rhs._seqNo)
    return (_seqNo < rhs._seqNo);
  if (_createDate != rhs._createDate)
    return (_createDate < rhs._createDate);
  if (_lockDate != rhs._lockDate)
    return (_lockDate < rhs._lockDate);
  if (_expireDate != rhs._expireDate)
    return (_expireDate < rhs._expireDate);
  if (_lastModificationDate != rhs._lastModificationDate)
    return (_lastModificationDate < rhs._lastModificationDate);
  if (_applyTag != rhs._applyTag)
    return (_applyTag < rhs._applyTag);
  if (_fareBasisTktDesignator != rhs._fareBasisTktDesignator)
    return (_fareBasisTktDesignator < rhs._fareBasisTktDesignator);
  if (_equipmentCode != rhs._equipmentCode)
    return (_equipmentCode < rhs._equipmentCode);
  if (_cabinCode != rhs._cabinCode)
    return (_cabinCode < rhs._cabinCode);
  if (_rbd1 != rhs._rbd1)
    return (_rbd1 < rhs._rbd1);
  if (_rbd2 != rhs._rbd2)
    return (_rbd2 < rhs._rbd2);
  if (_rbd3 != rhs._rbd3)
    return (_rbd3 < rhs._rbd3);
  if (_fareType != rhs._fareType)
    return (_fareType < rhs._fareType);
  if (_fareTariffInd != rhs._fareTariffInd)
    return (_fareTariffInd < rhs._fareTariffInd);
  if (_fareTariff != rhs._fareTariff)
    return (_fareTariff < rhs._fareTariff);
  if (_rule != rhs._rule)
    return (_rule < rhs._rule);
  if (_fareOwningCxr != rhs._fareOwningCxr)
    return (_fareOwningCxr < rhs._fareOwningCxr);
  if (_tktCode != rhs._tktCode)
    return (_tktCode < rhs._tktCode);
  if (_versionNbr != rhs._versionNbr)
    return (_versionNbr < rhs._versionNbr);

  return false;
}

bool
SectorDetailInfo::
operator==(const SectorDetailInfo& rhs) const
{
  return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
          (_createDate == rhs._createDate) && (_lockDate == rhs._lockDate) &&
          (_expireDate == rhs._expireDate) &&
          (_lastModificationDate == rhs._lastModificationDate) && (_applyTag == rhs._applyTag) &&
          (_fareBasisTktDesignator == rhs._fareBasisTktDesignator) &&
          (_equipmentCode == rhs._equipmentCode) && (_cabinCode == rhs._cabinCode) &&
          (_rbd1 == rhs._rbd1) && (_rbd2 == rhs._rbd2) && (_rbd3 == rhs._rbd3) &&
          (_fareType == rhs._fareType) && (_fareTariffInd == rhs._fareTariffInd) &&
          (_fareTariff == rhs._fareTariff) && (_rule == rhs._rule) &&
          (_fareOwningCxr == rhs._fareOwningCxr) && (_tktCode == rhs._tktCode) &&
          (_versionNbr == rhs._versionNbr));
}

void
SectorDetailInfo::dummyData(SectorDetailInfo& obj)
{
  obj._vendor = "xxxx";
  obj._itemNo = 1;
  obj._seqNo = 1;
  obj._createDate = time(nullptr);
  obj._lockDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._lastModificationDate = time(nullptr);
  obj._applyTag = 'a';
  obj._fareBasisTktDesignator = "aaaaaaaaaa";
  obj._equipmentCode = "aa";
  obj._cabinCode = 'a';
  obj._rbd1 = 'a';
  obj._rbd2 = 'a';
  obj._rbd3 = 'a';
  obj._fareType = "aaa";
  obj._fareTariffInd = "xx";
  obj._fareTariff = 1;
  obj._rule = "xxx";
  obj._fareOwningCxr = "xx";
  obj._tktCode = "xx";
  obj._versionNbr = 1;
}

void
SectorDetailInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _itemNo);
  FLATTENIZE(archive, _seqNo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _lockDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _lastModificationDate);
  FLATTENIZE(archive, _applyTag);
  FLATTENIZE(archive, _fareBasisTktDesignator);
  FLATTENIZE(archive, _equipmentCode);
  FLATTENIZE(archive, _cabinCode);
  FLATTENIZE(archive, _rbd1);
  FLATTENIZE(archive, _rbd2);
  FLATTENIZE(archive, _rbd3);
  FLATTENIZE(archive, _fareType);
  FLATTENIZE(archive, _fareTariffInd);
  FLATTENIZE(archive, _fareTariff);
  FLATTENIZE(archive, _rule);
  FLATTENIZE(archive, _fareOwningCxr);
  FLATTENIZE(archive, _tktCode);
  FLATTENIZE(archive, _versionNbr);
}

} // tse
