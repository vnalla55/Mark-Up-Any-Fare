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

#include "Global.h"
#include "raw_ptr.h"
#include "KeyedFactory.h" // sfc
#include "TrxMalloc.h"
#include "CacheDeleter.h"
#include "RemoteCacheHeader.h"
#include "CreateResult.h"

#include <memory>
#include <vector>

#define _USERAWPOINTERS

namespace sfc
{
  struct CompressedCacheStats
  {
    CompressedCacheStats ()
      : _totalSize(0)
      , _totalCapacity(0)
      , _uncompressedSize(0)
      , _uncompressedCapacity(0)
      , _compressedSize(0)
      , _averageCompressedBytes(0)
      , _numberEmpty(0)
      , _memoryEstimate(0)
      , _averageRatio(0)
      , _threshold(0)
    {
    }
    size_t _totalSize;
    size_t _totalCapacity;
    size_t _uncompressedSize;
    size_t _uncompressedCapacity;
    size_t _compressedSize;
    size_t _averageCompressedBytes;
    size_t _numberEmpty;
    size_t _memoryEstimate;
    double _averageRatio;
    size_t _threshold;
    std::string _errors;
  };

  template<typename Key, typename Type, typename FactoryType>
  class Cache
  {
  protected:

    FactoryType& _factory;
    size_t _accumulatorSize;
    typedef std::vector<Type *> TrashBin;
    TrashBin _accumulator;
    typename tse::CacheDeleter<Key, Type> _cacheDeleter;
    size_t _totalCapacity;
    size_t _threshold;
  public:

    typedef Key key_type;
    typedef Type value_type;
#ifdef _USERAWPOINTERS
    typedef RawPtr<Type> pointer_type;
#else
    typedef std::shared_ptr<Type> pointer_type;
#endif// _USERAWPOINTERS
    typedef typename std::shared_ptr<std::vector<Key>> key_vector_type;

    Cache( FactoryType & factory
         , const std::string & type
         , const std::string & name
         , size_t version
         )
      : _factory   ( factory    )
      , _accumulatorSize(100)
      , _cacheDeleter(factory)
      , _totalCapacity(0)
      , _threshold(0)
      , _type      ( type       )
      , _name      ( name       )
      , _isLoading ( false      )
      , _version   ( version    )
    {
    }

    virtual ~Cache()
    {
    }

    virtual size_t size() = 0;

    virtual void clear() = 0;

    virtual pointer_type get( const Key & key ) = 0 ;

    virtual pointer_type getIfResident(const Key& key) = 0;

    virtual CompressedDataPtr getCompressed(const Key& key,
                                            tse::RemoteCache::StatusType& status)
    {
      return CompressedDataPtr();
    }

    virtual void put( const Key & key, Type * object, bool updateLDC ) = 0 ;

    virtual void invalidate( const Key & key ) = 0 ;

    virtual std::shared_ptr<std::vector<Key>> keys() = 0;

    virtual void emptyTrash () = 0;

    // Not all caches are required to suppoirt "consolidation", so
    //      provide default behavior for "consolitation" functions.

    // Function to fetch how many objects would be moved by a consolidation
    virtual size_t consolidationSize()
    {
      // No consolidation to be done for this cache, so no records to consolidate
      return 0;
    }

    // Function to perform a consolidation
    virtual size_t consolidate(size_t)
    {
      // No consolidation to be done for this cache

      return 0;  // Number of objects consolidated
    }

    // Helpers

    virtual void actionQueueClear()
    {
    }

    const std::string & getName() const
    {
      return _name ;
    }

    const std::string & getType() const
    {
      return _type ;
    }

    void setName( const std::string & name )
    {
      _name = name ;
    }

    bool isLoading() const
    {
        return _isLoading ;
    }

    void setLoading( bool val = true )
    {
        _isLoading = val ;
    }

    void queueDiskPut (const Key &,
                       bool)
    {
    }

    void queueDiskInvalidate (const Key &key,
                              bool distCacheOp,
                              bool ldcOp)
    {
    }

    void queueDiskClear()
    {
    }

    void setAccumulatorSize (int accumulatorSize)
    {
      _accumulatorSize = accumulatorSize;
    }

    void moveToAccumulator (Type *ptr)
    {
      MallocContextDisabler disableCustomAllocator;
      _accumulator.push_back(ptr);
      if (_accumulator.size() >= _accumulatorSize)
      {
        _cacheDeleter.moveToTrashBin(_accumulator);
      }
    }

    virtual void getCompressionStats (CompressedCacheStats &)
    {
    }

    size_t getTotalCapacity () const
    {
      return _totalCapacity;
    }

    void setTotalCapacity (size_t totalCapacity)
    {
      _totalCapacity = totalCapacity;
    }

    size_t getThreshold () const
    {
      return _threshold;
    }

    void setThreshold (size_t threshold)
    {
      _threshold = threshold;
    }

    virtual size_t tableVersion() const
    {
      return _version;
    }

  private:

    std::string _type ;
    std::string _name ;
    bool        _isLoading ;

    const size_t _version;
 
    Cache() ;
    Cache( const Cache & rhs ) ;
    Cache & operator=( const Cache & rhs ) ;

  };

} // namespace sfc


