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

#include "DataAccessObject.h"
#include "CreateResult.h"

namespace sfc
{
  class CompressedData;

  template <typename Key, typename Type> class KeyedFactory
  {
  public:
    KeyedFactory(tse::DataAccessObject<Key, Type>& dao)
      : _dao(dao)
    {
    }
    virtual Type* create(Key key)
    {
      return _dao.create(key);
    }

    virtual tse::CreateResult<Type> create(const Key& key,
                                           int)
    {
      return _dao.create(key, 0);
    }

    virtual void destroy(Key key,
                         Type* vect)
    {
      _dao.destroy(key, vect);
    }

    virtual CompressedData* compress(const Type* data) const
    {
      return _dao.compress(data);
    }

    virtual Type* uncompress(const CompressedData& compressed) const
    {
      return _dao.uncompress(compressed);
    }
  private:
    tse::DataAccessObject<Key, Type>& _dao;
  };
}// sfc
