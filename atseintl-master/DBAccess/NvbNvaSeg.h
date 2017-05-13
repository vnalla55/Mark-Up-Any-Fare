//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{

class NvbNvaSeg
{

public:
  // constructor
  NvbNvaSeg() : _sequenceNumber(0), _fareBasis(), _nvb(' '), _nva(' ') {}

  // accessors
  uint64_t& sequenceNumber() { return _sequenceNumber; }
  const uint64_t& sequenceNumber() const { return _sequenceNumber; }

  FareBasisCode& fareBasis() { return _fareBasis; }
  const FareBasisCode& fareBasis() const { return _fareBasis; }

  Indicator& nvb() { return _nvb; }
  const Indicator& nvb() const { return _nvb; }

  Indicator& nva() { return _nva; }
  const Indicator& nva() const { return _nva; }

  // other public methods
  bool operator==(const NvbNvaSeg& second) const
  {
    return (_sequenceNumber == second._sequenceNumber) && (_nvb == second._nvb) &&
           (_nva == second._nva) && (_fareBasis == second._fareBasis);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    // NVBNVASEGMENT table fields
    FLATTENIZE(archive, _sequenceNumber);
    FLATTENIZE(archive, _nvb);
    FLATTENIZE(archive, _nva);
    FLATTENIZE(archive, _fareBasis);
  }

  static void dummyData(NvbNvaSeg& obj)
  {
    // NVBNVASEGMENT table fields
    obj._sequenceNumber = 65432;
    obj._nvb = 'a';
    obj._nva = 'b';
    obj._fareBasis = "KLMNOPQR";
  }

private:
  // NVBNVASEGMENT table fields
  uint64_t _sequenceNumber;
  FareBasisCode _fareBasis;
  Indicator _nvb;
  Indicator _nva;

  // serialization method
  friend class SerializationTestBase;
}; // class NvbNvaSeg

} // namespace tse

