//----------------------------------------------------------------------------
//
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#include "DBAccess/ServiceBaggageInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

std::ostream&
dumpObject(std::ostream& os, const ServiceBaggageInfo& obj)
{
  os << "[" << obj._vendor << "|" << obj._itemNo << "|" << obj._seqNo << "|" << obj._createDate
     << "|" << obj._lockDate << "|" << obj._expireDate << "|" << obj._lastModificationDate << "|"
     << obj._svcType << "|" << obj._feeOwnerCxr << "|" << obj._taxCode << "|" << obj._taxTypeSubCode
     << "|" << obj._attrSubGroup << "|" << obj._attrGroup << "|" << obj._applyTag << "|"
     << obj._versionNbr;
  os << "]";

  return os;
}

ServiceBaggageInfo::ServiceBaggageInfo()
  : _itemNo(0), _seqNo(0), _svcType(' '), _applyTag(' '), _versionNbr(0)
{
}

std::ostream&
ServiceBaggageInfo::print(std::ostream& out) const
{
  out << "\nVENDOR:         " << _vendor;
  out << "\nITEMNO:         " << _itemNo;
  out << "\nSEQNO:          " << _seqNo;
  out << "\nCREATEDATE:     " << _createDate;
  out << "\nLOCKDATE:       " << _lockDate;
  out << "\nEXPIREDATE:     " << _expireDate;
  out << "\nLASTMODDATE:    " << _lastModificationDate;
  out << "\nSVCTYPE:        " << _svcType;
  out << "\nFEEOWNERCXR:    " << _feeOwnerCxr;
  out << "\nTAXCODE:        " << _taxCode;
  out << "\nTAXTYPESUBCODE: " << _taxTypeSubCode;
  out << "\nATTRSUBGROUP:   " << _attrSubGroup;
  out << "\nATTRGROUP:      " << _attrGroup;
  out << "\nAPPLTAG:        " << _applyTag;
  out << "\nVERSIONNBR:     " << _versionNbr;
  out << "\n";
  return out;
}

bool
ServiceBaggageInfo::
operator<(const ServiceBaggageInfo& rhs) const
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
  if (_svcType != rhs._svcType)
    return (_svcType < rhs._svcType);
  if (_feeOwnerCxr != rhs._feeOwnerCxr)
    return (_feeOwnerCxr < rhs._feeOwnerCxr);
  if (_taxCode != rhs._taxCode)
    return (_taxCode < rhs._taxCode);
  if (_taxTypeSubCode != rhs._taxTypeSubCode)
    return (_taxTypeSubCode < rhs._taxTypeSubCode);
  if (_attrSubGroup != rhs._attrSubGroup)
    return (_attrSubGroup < rhs._attrSubGroup);
  if (_attrGroup != rhs._attrGroup)
    return (_attrGroup < rhs._attrGroup);
  if (_applyTag != rhs._applyTag)
    return (_applyTag < rhs._applyTag);
  if (_versionNbr != rhs._versionNbr)
    return (_versionNbr < rhs._versionNbr);

  return false;
}

bool
ServiceBaggageInfo::
operator==(const ServiceBaggageInfo& rhs) const
{
  return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
          (_createDate == rhs._createDate) && (_lockDate == rhs._lockDate) &&
          (_expireDate == rhs._expireDate) &&
          (_lastModificationDate == rhs._lastModificationDate) && (_svcType == rhs._svcType) &&
          (_feeOwnerCxr == rhs._feeOwnerCxr) && (_taxCode == rhs._taxCode) &&
          (_taxTypeSubCode == rhs._taxTypeSubCode) && (_attrSubGroup == rhs._attrSubGroup) &&
          (_attrGroup == rhs._attrGroup) && (_applyTag == rhs._applyTag) &&
          (_versionNbr == rhs._versionNbr));
}

void
ServiceBaggageInfo::dummyData(ServiceBaggageInfo& obj)
{
  obj._vendor = "xxxx";
  obj._itemNo = 1;
  obj._seqNo = 1;
  obj._createDate = time(nullptr);
  obj._lockDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._lastModificationDate = time(nullptr);
  obj._svcType = 'a';
  obj._feeOwnerCxr = "xx";
  obj._taxCode = "xx";
  obj._taxTypeSubCode = "xxx";
  obj._attrSubGroup = "xx";
  obj._attrGroup = "xx";
  obj._applyTag = 'a';
  obj._versionNbr = 1;
}

void
ServiceBaggageInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _itemNo);
  FLATTENIZE(archive, _seqNo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _lockDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _lastModificationDate);
  FLATTENIZE(archive, _svcType);
  FLATTENIZE(archive, _feeOwnerCxr);
  FLATTENIZE(archive, _taxCode);
  FLATTENIZE(archive, _taxTypeSubCode);
  FLATTENIZE(archive, _attrSubGroup);
  FLATTENIZE(archive, _attrGroup);
  FLATTENIZE(archive, _applyTag);
  FLATTENIZE(archive, _versionNbr);
}

} // tse
