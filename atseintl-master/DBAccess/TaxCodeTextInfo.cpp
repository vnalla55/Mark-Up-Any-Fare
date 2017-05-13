//----------------------------------------------------------------------------
//
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#include "DBAccess/TaxCodeTextInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

std::ostream&
dumpObject(std::ostream& os, const TaxCodeTextInfo& obj)
{
  os << "[" << obj._vendor << "|" << obj._itemNo << "|" << obj._seqNo << "|" << obj._createDate
     << "|" << obj._lockDate << "|" << obj._expireDate << "|" << obj._lastModificationDate << "|"
     << obj._taxCodeText << "|" << obj._versionNbr;
  os << "]";

  return os;
}

TaxCodeTextInfo::TaxCodeTextInfo() : _itemNo(0), _seqNo(0), _versionNbr(0) {}

std::ostream&
TaxCodeTextInfo::print(std::ostream& out) const
{
  out << "\nVENDOR:      " << _vendor;
  out << "\nITEMNO:      " << _itemNo;
  out << "\nSEQNO:       " << _seqNo;
  out << "\nCREATEDATE:  " << _createDate;
  out << "\nLOCKDATE:    " << _lockDate;
  out << "\nEXPIREDATE:  " << _expireDate;
  out << "\nLASTMODDATE: " << _lastModificationDate;
  out << "\nTAXCODETEXT: " << _taxCodeText;
  out << "\nVERSIONNBR:  " << _versionNbr;
  return out;
}

bool
TaxCodeTextInfo::
operator<(const TaxCodeTextInfo& rhs) const
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
  if (_taxCodeText != rhs._taxCodeText)
    return (_taxCodeText < rhs._taxCodeText);
  if (_versionNbr != rhs._versionNbr)
    return (_versionNbr < rhs._versionNbr);

  return false;
}

bool
TaxCodeTextInfo::
operator==(const TaxCodeTextInfo& rhs) const
{
  return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
          (_createDate == rhs._createDate) && (_lockDate == rhs._lockDate) &&
          (_expireDate == rhs._expireDate) &&
          (_lastModificationDate == rhs._lastModificationDate) &&
          (_taxCodeText == rhs._taxCodeText) && (_versionNbr == rhs._versionNbr));
}

void
TaxCodeTextInfo::dummyData(TaxCodeTextInfo& obj)
{
  obj._vendor = "xxxx";
  obj._itemNo = 1;
  obj._seqNo = 1;
  obj._createDate = time(nullptr);
  obj._lockDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._lastModificationDate = time(nullptr);
  obj._taxCodeText = "aaaaaaaaaaaa";
  obj._versionNbr = 1;
}

void
TaxCodeTextInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _itemNo);
  FLATTENIZE(archive, _seqNo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _lockDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _lastModificationDate);
  FLATTENIZE(archive, _taxCodeText);
  FLATTENIZE(archive, _versionNbr);
}

} // tse
