//----------------------------------------------------------------------------
//
//   File:           PaxType.h
//   Description:    PsgType  data
//   Created:        3/27/2004
//   Authors:        Roger Kelly
//
//
//   (c) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class PaxTypeInfo
{
public:
  PaxTypeInfo() : _childInd(' '), _adultInd(' '), _numberSeatsReq(0), _infantInd(' ') {}

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  Description& description() { return _description; }
  const Description& description() const { return _description; }

  Indicator& childInd() { return _childInd; }
  const Indicator& childInd() const { return _childInd; }

  Indicator& adultInd() { return _adultInd; }
  const Indicator& adultInd() const { return _adultInd; }

  PaxGroupTypeCode& psgGroupType() { return _psgGroupType; }
  const PaxGroupTypeCode& psgGroupType() const { return _psgGroupType; }

  int& numberSeatsReq() { return _numberSeatsReq; }
  const int& numberSeatsReq() const { return _numberSeatsReq; }

  Indicator& infantInd() { return _infantInd; }
  const Indicator& infantInd() const { return _infantInd; }

  bool isAdult() const { return _psgType.isAdult == 1; }
  bool isChild() const { return _psgType.isChild == 1; }
  bool isInfant() const { return _psgType.isInfant == 1; }
  int paxTypeStatus() const { return _psgType.paxTypeStatus; }

  void initPsgType()
  {
    _psgType.isChild = ((_childInd == 'Y' && _infantInd == 'N' && _paxType != INS) ||
                        _paxType == JNN || _paxType == UNN)
                           ? 1
                           : 0;

    _psgType.isInfant = (_infantInd == 'Y' || _paxType == JNF || _paxType == INS) ? 1 : 0;

    _psgType.isAdult =
        ((_paxType != JCB) && (_adultInd != 'Y' || _psgType.isChild == 1 || _psgType.isInfant == 1))
            ? 0
            : 1;

    if (isInfant())
      _psgType.paxTypeStatus = 3;
    else if (isChild())
      _psgType.paxTypeStatus = 2;
    else if (isAdult())
      _psgType.paxTypeStatus = 1;
  }

  bool operator<(const PaxTypeInfo& rhs) const
  {
    if (_paxType != rhs._paxType)
      return (_paxType < rhs._paxType);
    if (_vendor != rhs._vendor)
      return (_vendor < rhs._vendor);
    if (_createDate != rhs._createDate)
      return (_createDate < rhs._createDate);
    if (_description != rhs._description)
      return (_description < rhs._description);
    if (_childInd != rhs._childInd)
      return (_childInd < rhs._childInd);
    if (_adultInd != rhs._adultInd)
      return (_adultInd < rhs._adultInd);
    if (_psgGroupType != rhs._psgGroupType)
      return (_psgGroupType < rhs._psgGroupType);
    if (_numberSeatsReq != rhs._numberSeatsReq)
      return (_numberSeatsReq < rhs._numberSeatsReq);
    if (_infantInd != rhs._infantInd)
      return (_infantInd < rhs._infantInd);
    if (_psgType != rhs._psgType)
      return (_psgType < rhs._psgType);

    return false;
  }

  bool operator==(const PaxTypeInfo& rhs) const
  {
    return ((_paxType == rhs._paxType) && (_vendor == rhs._vendor) &&
            (_createDate == rhs._createDate) && (_description == rhs._description) &&
            (_childInd == rhs._childInd) && (_adultInd == rhs._adultInd) &&
            (_psgGroupType == rhs._psgGroupType) && (_numberSeatsReq == rhs._numberSeatsReq) &&
            (_infantInd == rhs._infantInd) && (_psgType == rhs._psgType));
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const PaxTypeInfo& obj)
  {
    os << "[" << obj._paxType << "|" << obj._vendor << "|" << obj._createDate << "|"
       << obj._description << "|" << obj._childInd << "|" << obj._adultInd << "|"
       << obj._psgGroupType << "|" << obj._numberSeatsReq << "|" << obj._infantInd;

    dumpObject(os, obj._psgType);

    os << "]";

    return os;
  }

  static void dummyData(PaxTypeInfo& obj)
  {
    obj._paxType = "zzz";
    obj._vendor = "xxxx";
    obj._createDate = time(nullptr);
    obj._description = "aaaaaaaa";
    obj._childInd = 'N';
    obj._adultInd = 'Y';
    obj._psgGroupType = "JKL";
    obj._numberSeatsReq = 1;
    obj._infantInd = 'N';
    obj.initPsgType();
  }

protected:
  PaxTypeCode _paxType;
  VendorCode _vendor;
  DateTime _createDate;
  Description _description;
  Indicator _childInd;
  Indicator _adultInd;
  PaxGroupTypeCode _psgGroupType;
  int _numberSeatsReq;
  Indicator _infantInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _childInd);
    FLATTENIZE(archive, _adultInd);
    FLATTENIZE(archive, _psgGroupType);
    FLATTENIZE(archive, _numberSeatsReq);
    FLATTENIZE(archive, _infantInd);

    if (archive.action() == Flattenizable::UNFLATTEN)
    {
      initPsgType();
    }
  }

protected:
private:
  struct PsgTypeStatus
  {
    PsgTypeStatus() : isAdult(0), isChild(0), isInfant(0), paxTypeStatus(0) {}

    unsigned char isAdult : 1;
    unsigned char isChild : 1;
    unsigned char isInfant : 1;
    unsigned char paxTypeStatus : 2;

    bool operator<(const PsgTypeStatus& rhs) const
    {
      if (isAdult != rhs.isAdult)
        return (isAdult < rhs.isAdult);
      if (isChild != rhs.isChild)
        return (isChild < rhs.isChild);
      if (isInfant != rhs.isInfant)
        return (isInfant < rhs.isInfant);
      if (paxTypeStatus != rhs.paxTypeStatus)
        return (paxTypeStatus < rhs.paxTypeStatus);

      return false;
    }

    bool operator==(const PsgTypeStatus& rhs) const
    {
      return ((isAdult == rhs.isAdult) && (isChild == rhs.isChild) && (isInfant == rhs.isInfant) &&
              (paxTypeStatus == rhs.paxTypeStatus));
    }

    bool operator!=(const PsgTypeStatus& rhs) const { return !(*this == rhs); }

    friend inline std::ostream& dumpObject(std::ostream& os, const PsgTypeStatus& obj)
    {
      return os << "[" << obj.isAdult << "|" << obj.isChild << "|" << obj.isInfant << "|"
                << obj.paxTypeStatus << "]";
    }

  private:
  } _psgType;
};
}

