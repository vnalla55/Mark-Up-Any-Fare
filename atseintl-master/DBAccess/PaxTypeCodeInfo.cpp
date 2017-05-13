//----------------------------------------------------------------------------
//
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#include "DBAccess/PaxTypeCodeInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

std::ostream&
dumpObject(std::ostream& os, const PaxTypeCodeInfo& obj)
{
  os << "[" << obj._vendor << "|" << obj._itemNo << "|" << obj._seqNo << "|" << obj._createDate
     << "|" << obj._lockDate << "|" << obj._expireDate << "|" << obj._lastModificationDate << "|"
     << obj._applyTag << "|" << obj._psgrType << "|" << obj._paxMinAge << "|" << obj._paxMaxAge
     << "|" << obj._paxStatus << "|" << obj._loc << "|" << obj._ptcMatchIndicator << "|"
     << obj._versionNbr;
  os << "]";

  return os;
}

PaxTypeCodeInfo::PaxTypeCodeInfo()
  : _itemNo(0),
    _seqNo(0),
    _applyTag(' '),
    _paxMinAge(0),
    _paxMaxAge(0),
    _paxStatus(' '),
    _ptcMatchIndicator(' '),
    _versionNbr(0)
{
}

std::ostream&
PaxTypeCodeInfo::print(std::ostream& out) const
{
  out << "\nVENDOR:      " << _vendor;
  out << "\nITEMNO:      " << _itemNo;
  out << "\nSEQNO:       " << _seqNo;
  out << "\nCREATEDATE:  " << _createDate;
  out << "\nLOCKDATE:    " << _lockDate;
  out << "\nEXPIREDATE:  " << _expireDate;
  out << "\nLASTMODDATE: " << _lastModificationDate;
  out << "\nAPPLTAG:     " << _applyTag;
  out << "\nPSGRTYPE:    " << _psgrType;
  out << "\nPSGRMINAGE:  " << _paxMinAge;
  out << "\nPSGRMAXAGE:  " << _paxMaxAge;
  out << "\nPSGRSTATUS:  " << _paxStatus;
  out << "\nLOCTYPE:     " << _loc.locType();
  out << "\nLOC:         " << _loc.loc();
  out << "\nPTCMATCHIND: " << _ptcMatchIndicator;
  out << "\nVERSIONNBR:  " << _versionNbr;
  return out;
}

bool
PaxTypeCodeInfo::
operator<(const PaxTypeCodeInfo& rhs) const
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
  if (_psgrType != rhs._psgrType)
    return (_psgrType < rhs._psgrType);
  if (_paxMinAge != rhs._paxMinAge)
    return (_paxMinAge < rhs._paxMinAge);
  if (_paxMaxAge != rhs._paxMaxAge)
    return (_paxMaxAge < rhs._paxMaxAge);
  if (_paxStatus != rhs._paxStatus)
    return (_paxStatus < rhs._paxStatus);
  if (_loc != rhs._loc)
    return (_loc < rhs._loc);
  if (_ptcMatchIndicator != rhs._ptcMatchIndicator)
    return (_ptcMatchIndicator < rhs._ptcMatchIndicator);
  if (_versionNbr != rhs._versionNbr)
    return (_versionNbr < rhs._versionNbr);

  return false;
}

bool
PaxTypeCodeInfo::
operator==(const PaxTypeCodeInfo& rhs) const
{
  return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
          (_createDate == rhs._createDate) && (_lockDate == rhs._lockDate) &&
          (_expireDate == rhs._expireDate) &&
          (_lastModificationDate == rhs._lastModificationDate) && (_applyTag == rhs._applyTag) &&
          (_psgrType == rhs._psgrType) && (_paxMinAge == rhs._paxMinAge) &&
          (_paxMaxAge == rhs._paxMaxAge) && (_paxStatus == rhs._paxStatus) && (_loc == rhs._loc) &&
          (_ptcMatchIndicator == rhs._ptcMatchIndicator) && (_versionNbr == rhs._versionNbr));
}

void
PaxTypeCodeInfo::dummyData(PaxTypeCodeInfo& obj)
{
  obj._vendor = "xxxx";
  obj._itemNo = 1;
  obj._seqNo = 1;
  obj._createDate = time(nullptr);
  obj._lockDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._lastModificationDate = time(nullptr);
  obj._applyTag = 'a';
  obj._psgrType = "aaa";
  obj._paxMinAge = 1;
  obj._paxMaxAge = 1;
  obj._paxStatus = 'a';
  LocKey::dummyData(obj._loc);
  obj._ptcMatchIndicator = 'a';
  obj._versionNbr = 1;
}

void
PaxTypeCodeInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _itemNo);
  FLATTENIZE(archive, _seqNo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _lockDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _lastModificationDate);
  FLATTENIZE(archive, _applyTag);
  FLATTENIZE(archive, _psgrType);
  FLATTENIZE(archive, _paxMinAge);
  FLATTENIZE(archive, _paxMaxAge);
  FLATTENIZE(archive, _paxStatus);
  FLATTENIZE(archive, _loc);
  FLATTENIZE(archive, _ptcMatchIndicator);
  FLATTENIZE(archive, _versionNbr);
}

} // tse
