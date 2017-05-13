//----------------------------------------------------------------------------
//
//  File:           SmallBitSet.h
//  Created:        19/2/2004
//  Authors:        Vadim Nikushin
//
//  Description:    Simple template to manipulate bits in unsegned ints.
//
//  Updates:
//          19/02/04 - VN - file created.
//          13/03/04 - VN - bunch of stuff reoranged and renamed.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{
template <class StorageType, class ArgClass>
class SmallBitSet
{
public:
  SmallBitSet() = default;
  SmallBitSet(ArgClass arg) : _buffer(arg) {}
  SmallBitSet(const SmallBitSet& copy) : _buffer(copy._buffer) {}

  // whole bitset operations

  void initialize(ArgClass arg) { _buffer = arg; }
  void setNull() { _buffer = static_cast<StorageType>(0); }
  bool isNull() const { return _buffer == static_cast<StorageType>(0); }

  // bitwise operations

  bool set(ArgClass arg, bool setBits = true)
  {
    if (setBits)
      _buffer |= static_cast<StorageType>(arg);
    else
      clear(arg);

    return setBits;
  }
  void combine(const SmallBitSet& other, bool setBits = true)
  {
    if (LIKELY(setBits))
      _buffer |= other._buffer;
    else
      _buffer &= ~(other._buffer);
  }
  void clear(ArgClass arg) { _buffer &= static_cast<StorageType>(~arg); }

  bool isSet(ArgClass arg) const { return _buffer & static_cast<StorageType>(arg); }
  bool isAnySet(StorageType arg) const { return _buffer & arg; }
  bool isAllSet(ArgClass arg) const { return (_buffer & static_cast<StorageType>(arg)) == arg; }
  const StorageType& value() const { return _buffer; }

  void flattenize(Flattenizable::Archive& archive) { FLATTENIZE(archive, _buffer); }

private:
  StorageType _buffer = static_cast<StorageType>(0);
};

} // end tse namespace

