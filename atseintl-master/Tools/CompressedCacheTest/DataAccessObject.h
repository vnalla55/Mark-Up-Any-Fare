//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <iostream>
#include "CreateResult.h"

namespace sfc
{
  class CompressedData;
}
namespace tse
{

template <typename Key, typename Type> class DataAccessObject
{
public:
  DataAccessObject ()
  {
  }
  virtual Type *create (Key key) = 0;

  virtual CreateResult<Type> create(Key key,
                                    int)
  {
    CreateResult<Type> result;
    result._ptr = create(key);
    return result;
  }

  virtual void destroy (Key key,
                        Type *vect)
  {
    std::cout << __FUNCTION__ << ":pure virtual call" << std::endl;
  }

  virtual sfc::CompressedData* compress(const Type*) const
  {
    return 0;
  }

  virtual Type* uncompress(const sfc::CompressedData&) const
  {
    return 0;
  }

  template <typename Container> void destroyContainer (Container *container)
  {
    if (container == 0)
    {
      return;
    }
    for (typename Container::iterator i = container->begin(), iEnd = container->end();
         i != iEnd; ++i)
    {
      delete *i;
    }
    delete container;
  }
};
}// tse
