//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include <memory>

namespace tse
{
template <typename T, typename SmartPointerType = std::unique_ptr<T>>
class CopyablePtr : public SmartPointerType
{
public:
  CopyablePtr(CopyablePtr&&) = default;
  CopyablePtr& operator=(CopyablePtr&&) = default;

  // New copy Constructors
  CopyablePtr() = default;
  CopyablePtr(T* ptr) : SmartPointerType(ptr) {}
  CopyablePtr(const CopyablePtr& other)
  {
    if (other != nullptr)
      *this = CopyablePtr(new T(*other));
  }

  // New copy assignment operator
  CopyablePtr& operator=(const CopyablePtr& other)
  {
    if (this == &other)
      return *this;

    if (other != nullptr)
      *this = CopyablePtr(new T(*other));
    else
      this->reset();

    return *this;
  }
};
}
