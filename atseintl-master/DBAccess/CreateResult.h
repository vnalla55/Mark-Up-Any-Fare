#pragma once

#include "DBAccess/CompressedData.h"
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

namespace tse
{

template <typename T>
struct CreateResult
{
  CreateResult() : _status(RemoteCache::RC_NONE), _ptr(nullptr) {}

  CreateResult(RemoteCache::StatusType status, sfc::CompressedDataPtr compressed)
    : _status(status), _ptr(nullptr), _compressed(compressed)
  {
  }

  operator bool() const
  {
    return (RemoteCache::RC_COMPRESSED_VALUE == _status
            || RemoteCache::RC_UNCOMPRESSED_VALUE == _status)
           && _ptr != nullptr;
  }

  RemoteCache::StatusType _status;
  T* _ptr;
  sfc::CompressedDataPtr _compressed;
};

} // tse

