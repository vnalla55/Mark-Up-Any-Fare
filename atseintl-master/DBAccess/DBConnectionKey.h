//----------------------------------------------------------------------------
//
// © 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------
#ifndef DBCONNECTIONKEY_H
#define DBCONNECTIONKEY_H

#include <string>

namespace tse
{

class DBConnectionKey
{
public:
  std::string _key;
  std::string _pool;

  DBConnectionKey(const std::string& a_Pool) : _key(), _pool(a_Pool) {}

  DBConnectionKey(const std::string& a_Pool, const std::string& a_Key) : _key(a_Key), _pool(a_Pool)
  {
  }

  DBConnectionKey(const DBConnectionKey& a_Another) : _key(a_Another._key), _pool(a_Another._pool)
  {
  }

  // Assignment operations

  DBConnectionKey& operator=(const DBConnectionKey& value)
  {
    _key = value._key;
    _pool = value._pool;
    return (*this);
  }

  DBConnectionKey& operator=(const std::string& str)
  {
    _key = str;
    return (*this);
  }

  // Comparison operations
  //
  bool operator==(const DBConnectionKey& cs) const { return (_key == cs._key); }

  bool operator!=(const DBConnectionKey& cs) const { return (_key != cs._key); }

  bool operator>(const DBConnectionKey& cs) const { return (_key > cs._key); }

  bool operator<(const DBConnectionKey& cs) const { return (_key < cs._key); }

  bool operator>=(const DBConnectionKey& cs) const { return (_key >= cs._key); }

  bool operator<=(const DBConnectionKey& cs) const { return (_key <= cs._key); }
};
}

#endif
