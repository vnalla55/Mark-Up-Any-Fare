// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include <memory>

namespace tax
{

// this handle does or does not owe the resource, based on how it is constructed
template <typename T>
class Handle
{
  std::unique_ptr<T> _owingPtr;
  T& _ref;

public:
  explicit Handle(T& ref)                 // if this ctor used, we do not owe the resource
    : _owingPtr(), _ref(ref) {}

  explicit Handle(std::unique_ptr<T> ptr) // if this ctor used, we do owe the resource
    : _owingPtr(std::move(ptr)), _ref(*_owingPtr) {}

  const T& get() const { return _ref; }
  T& get() { return _ref; }

  const T& operator()() const { return _ref; }
  T& operator()() { return _ref; }

};

} // namespace tax

