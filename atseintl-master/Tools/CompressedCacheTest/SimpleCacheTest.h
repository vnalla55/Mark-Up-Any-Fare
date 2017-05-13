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

#include "SimpleCacheRP.h"

namespace sfc
{
  template<typename Key, typename Type>
  class SimpleCacheTest : public SimpleCache<Key, Type>
  {
    typedef typename Cache<Key, Type>::pointer_type _pointer_type;
  public:
    SimpleCacheTest (KeyedFactory<Key, Type> &factory,
                     const std::string &name,
                     size_t capacity,
                     size_t version)
      : SimpleCache<Key, Type>(factory, name, capacity, version)
    {
    }

    virtual ~SimpleCacheTest ()
    {
      this->clear();
      delete this->_uninitializedCacheEntry.get();
    }

    virtual _pointer_type getIfResident (const Key &key)
    {
      _pointer_type ptr(SimpleCache<Key, Type>::getIfResident(key));
      if (!ptr)
      {
	      //std::cout << __FUNCTION__ << ":!ptr" << std::endl;
      }
      else if (this->_uninitializedCacheEntry == ptr)
      {
 	      std::cout << __FUNCTION__ << ":!!!!!!!!!!!!!!!!!!_uninitializedCacheEntry == ptr!!!!!!!!!!!!!!!!!!" << std::endl;
      }
      return ptr;
    }

    virtual _pointer_type get (const Key &key)
    {
      _pointer_type ptr(SimpleCache<Key, Type>::get(key));
      if (!ptr)
      {
        std::cout << __FUNCTION__ << ":!ptr" << std::endl;
      }
      else if (this->_uninitializedCacheEntry == ptr)
      {
 	      std::cout << __FUNCTION__ << ":_uninitializedCacheEntry == ptr" << std::endl;
      }
      return ptr;
    }
  };// class SimpleCacheTest
}// namespace sfc
