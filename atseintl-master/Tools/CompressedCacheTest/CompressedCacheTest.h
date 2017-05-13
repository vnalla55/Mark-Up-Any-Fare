//  Copyright Sabre 2016
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include "BaseTrx.h"
#include "CompressedCache.h"

namespace sfc
{
  template<typename Key, typename Type>
  class CompressedCacheTest : public CompressedCache<Key, Type>
  {
    typedef typename Cache<Key, Type>::pointer_type _pointer_type;
  public:
    CompressedCacheTest (KeyedFactory<Key, Type> &factory,
                         const std::string &name,
                         size_t capacity,
                         size_t version)
      : CompressedCache<Key, Type>(factory, name, capacity, version)
    {
    }

    virtual ~CompressedCacheTest ()
    {
      {
        tse::BaseTrx trx;
        this->clear();
        delete this->_uninitializedCacheEntry.get();
      }
      for (const auto ptr : this->_accumulator)
      {
        static Key dummy;
        this->_factory.destroy(dummy, ptr);
      }
      this->_accumulator.clear();
    }

    virtual _pointer_type getIfResident (const Key &key)
    {
      _pointer_type ptr(CompressedCache<Key, Type>::getIfResident(key));
      if (!ptr)
      {
	//std::cout << __FUNCTION__ << ":!ptr" << std::endl;
      }
      else if (this->_uninitializedCacheEntry == ptr)
      {
 	std::cout << __FUNCTION__ << ":! _uninitializedCacheEntry == ptr !" << std::endl;
      }
      return ptr;
    }

    virtual _pointer_type get (const Key &key)
    {
      _pointer_type ptr(CompressedCache<Key, Type>::get(key));
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
  };// class CompressedCacheTest
}// namespace sfc
