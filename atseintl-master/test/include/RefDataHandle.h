#pragma once

#include "DBAccess/DataHandle.h"

#include <vector>

namespace tse
{
class RefDataHandle : public DataHandle
{

public:
  template <typename T>
  void get(T*& t)
  {
    DataHandle::get(t);
  }
};
} // tse namespace
