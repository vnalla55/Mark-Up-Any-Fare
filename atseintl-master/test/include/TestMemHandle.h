//----------------------------------------------------------------------------
//  Copyright Sabre 2009
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#ifndef TEST_MEM_HANDLE_H
#define TEST_MEM_HANDLE_H

#include "DBAccess/DeleteList.h"

#include <utility>

namespace tse
{

class TestMemHandle
{
public:
  TestMemHandle(size_t deleteListSize = 1) : _deleteList(deleteListSize) {}

  void import(TestMemHandle& another);
  void clear();
  const DeleteList& deleteList() const;

  template <typename T>
  size_t get(T*& t)
  {
    t = create<T>();
    return 0;
  }

  template<typename T, typename... A>
  T* create(A&&... args)
  {
    return insert(new T(std::forward<A>(args)...));
  }

  template <typename T>
  T* insert(T* t)
  {
    _deleteList.adopt(t);
    return t;
  }

  template <typename T>
  T* operator()(T* t)
  {
    return insert(t);
  }

protected:
  DeleteList _deleteList;
};
} // namespace tse

#endif // TEST_MEM_HANDLE_H
