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

#include "CompressedData.h"
#include "RemoteCacheHeader.h"

namespace tse
{

template<typename T>
struct CreateResult
{
  CreateResult()
    : _status(RemoteCache::COMPRESSED_VALUE)
    , _ptr(0)
  {
  }

  CreateResult(RemoteCache::StatusType status,
               sfc::CompressedDataPtr compressed)
    : _status(status)
    , _ptr(0)
    , _compressed(compressed)
  {
  }

  operator bool () const
  {
    return (RemoteCache::COMPRESSED_VALUE == _status
            || RemoteCache::UNCOMPRESSED_VALUE == _status)
           && _ptr != 0;
  }

  RemoteCache::StatusType _status;
  T* _ptr;
  sfc::CompressedDataPtr _compressed;
};

}// tse
