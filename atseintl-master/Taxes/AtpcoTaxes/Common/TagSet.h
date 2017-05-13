// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once
#include "DataModel/Common/SafeEnums.h"

namespace tax
{

class TagSetBase
{

public:
  friend bool
  overlap(TagSetBase const& lhs, TagSetBase const& rhs)
  { return lhs._flags & rhs._flags; }

  bool hasBit(int flag) const { return _flags & (1U << flag); }
  bool isEmpty() const { return _flags == 0U; }
  operator bool () const { return !isEmpty(); }
  bool operator!() const { return isEmpty(); }

protected:
  explicit TagSetBase(unsigned flags) : _flags(flags) {}
  unsigned _flags;
};

template <typename EnumMapper>
class TagSet : public TagSetBase
{
public:
  static TagSet none() { return TagSet(0U); }
  static TagSet all() { return TagSet(EnumMapper::all()); }
  bool hasTag(typename EnumMapper::enum_type flag) const { return _flags & EnumMapper::toBit(flag); }
  void setTag(typename EnumMapper::enum_type flag) { _flags |= EnumMapper::toBit(flag); }

private:
  explicit TagSet(unsigned flags) : TagSetBase(flags) {}
};

} // namespace tax

