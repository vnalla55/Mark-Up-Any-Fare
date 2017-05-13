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

#include <cstring>
#include <boost/container/string.hpp>
#include <boost/type_traits.hpp>
#include "CharBuffer.h"
#include "CacheEntryPool.h"
#include "Code.h"
#include "HashKey.h"
#include "TSEDateInterval.h"
#include "CompressedData.h"
#include "Cache.h"

namespace tse
{

const int m(8);

class RBuffer : boost::noncopyable
{
public:
  explicit RBuffer(const char* source);

  template <typename T> T* construct(T* object)
  {
    if (_poolPointer)
    {
      object = new (_poolPointer) T;
      _poolPointer += sizeof(T);
    }
    else
    {
      object = new T;
    }
    return object;
  }

  template <size_t n> void read(Code<n>& str)
  {
    char* dst(&str[0]);
    while ((*dst++ = *_readPointer++) != 0);
  }

  void read(std::string& str)
  {
    size_t length(strlen(_readPointer));
    str.assign(_readPointer, length);
    _readPointer += length + 1;
  }

  void read(boost::container::string& str)
  {
    size_t length(strlen(_readPointer));
    str.assign(_readPointer, length);
    _readPointer += length + 1;
  }

  void read(DateTime& dt)
  {
    const boost::int64_t* rep(reinterpret_cast<const boost::int64_t*>(_readPointer));
    dt.setIntRep(*rep);
    _readPointer += sizeof(boost::int64_t);
  }

  void read(TSEDateInterval& di)
  {
    const boost::int64_t* rep(reinterpret_cast<const boost::int64_t*>(_readPointer));
    di.createDate().setIntRep(*rep++);
    di.effDate().setIntRep(*rep++);
    di.expireDate().setIntRep(*rep++);
    di.discDate().setIntRep(*rep);
    _readPointer += sizeof(boost::int64_t) * 4;
  }

  template <size_t n> void read(std::vector<Code<n>>& vector)
  {
    boost::uint32_t size(0);
    read(size);
    vector.reserve(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      Code<n> item;
      read(item);
      vector.push_back(item);
    }
  }

  template <size_t n> void read(Code<n> arr[m])
  {
    for (int i = 0; i < m; ++i)
    {
      arr[i] = _readPointer;
      _readPointer += n;
    }
  }

  void read(std::vector<class CategoryRuleItemInfo*>& vect);
  void read(std::vector<const class FareInfo*>& vect);
  void read(std::vector<class AddonFareInfo*>& vect);
  void read(class TaxRestrictionPsg& field);
  void read(class TaxExemptionCarrier& field);
  void read(class TaxRestrictionTransit& field);


  template <typename T> void read(std::vector<T*>& vect)
  {
    typedef typename boost::remove_const<T>::type NONCONST;
    boost::uint32_t size(0);
    read(size);
    vect.reserve(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      NONCONST* entry(0);
      entry = construct(entry);
      entry->read(*this);
      vect.push_back(entry);
    }
  }

  void read(nil_t)
  {
  }

  template<typename A, typename B, typename C, typename D,
           typename E, typename F, typename G, typename H,
           typename I, typename J>
  void read(HashKey<A, B, C, D, E, F, G, H, I, J>& key)
  {
    read(key.initialized);
    read(key._a);
    read(key._b);
    read(key._c);
    read(key._d);
    read(key._e);
    read(key._f);
    read(key._g);
    read(key._h);
    read(key._i);
    read(key._j);
  }

  void read(class AdditionalInfoContainer*)
  {
  }

  void read(class LocKey& locKey);

  void read(char& ch)
  {
    ch = *_readPointer++;
  }

  template <typename T> void read(T& field)
  {
    field = *reinterpret_cast<const T*>(_readPointer);
    _readPointer += sizeof(T);
  }

  void read(std::vector<std::string>& vector);

  template <typename T> void read(std::vector<T>& vector)
  {
    boost::uint32_t size(0);
    read(size);
    vector.reserve(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      T entry;
      read(entry);
      vector.push_back(entry);
    }
  }

  template <typename T> void read(std::set<T>& set)
  {
    boost::uint32_t size(0);
    read(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      T entry;
      read(entry);
      set.insert(entry);
    }
  }

  template <size_t n> void read(std::set<Code<n>>& set)
  {
    boost::uint32_t size(0);
    read(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      Code<n> entry;
      read(entry);
      set.insert(entry);
    }
  }

  template <typename T> RBuffer& operator & (T& value)
  {
    read(value);
    return *this;
  }

  static bool uncompress(const sfc::CompressedData& compressed,
                         CharBuffer& inflated);
private:
  const char* _readPointer;
  char* _poolPointer;
};

class WBuffer : boost::noncopyable
{
public:
  WBuffer(std::ostream* os = 0);

  size_t size() const
  {
    return _size;
  }

  sfc::CompressedData* compress() const;

  const char* buffer() const;

  template <size_t n> void write(const Code<n>& str)
  {
    writeString(str.c_str(), str.length());
  }

  void write(const std::string& str)
  {
    writeString(str.c_str(), str.length());
  }

  void write(const boost::container::string& str)
  {
    writeString(str.c_str(), str.length());
  }

  void write(const DateTime& dt)
  {
    if (_os)
    {
      *_os << boost::posix_time::to_iso_extended_string(dt) << '|';
    }
    else
    {
      write(dt.getIntRep());
    }
  }

  template <size_t n> void write(const std::vector<Code<n>>& vect)
  {
    if (_os)
    {
      *_os << '[';
      size_t sz(vect.size());
      for (size_t i = 0; i < sz; ++i)
      {
        write(vect[i]);
      }
      *_os << "]|";
    }
    else
    {
      size_t sz(vect.size());
      write(static_cast<boost::uint32_t>(sz));
      for (size_t i = 0; i < sz; ++i)
      {
        write(vect[i]);
      }
    }
  }

  template <size_t n> void write(const Code<n> arr[m])
  {
    if (_os)
    {
      *_os << '[';
      for (int i = 0; i < m; ++i)
      {
        *_os << arr[i] << '|';
      }
      *_os << "]|";
    }
    else
    {
      for (int i = 0; i < m; ++i)
      {
        write(arr[i].c_str(), n);
      }
    }
  }

  void write(const class LocKey& locKey);

  void write(class AdditionalInfoContainer* const)
  {
  }

  void write(const TSEDateInterval& di)
  {
    if (_os)
    {
      *_os << '{'
           << boost::posix_time::to_iso_extended_string(di.createDate()) << '|'
           << boost::posix_time::to_iso_extended_string(di.effDate()) << '|'
           << boost::posix_time::to_iso_extended_string(di.expireDate()) << '|'
           << boost::posix_time::to_iso_extended_string(di.discDate())
           << "}|";
    }
    else
    {
      boost::int64_t arr[4] = { di.createDate().getIntRep(),
                                di.effDate().getIntRep(),
                                di.expireDate().getIntRep(),
                                di.discDate().getIntRep() };
      write(reinterpret_cast<const char*>(arr), sizeof(boost::int64_t) * 4);
    }
  }

  void write(nil_t)
  {
  }

  template<typename A, typename B, typename C, typename D,
           typename E, typename F, typename G, typename H,
           typename I, typename J>
  void write(const HashKey<A, B, C, D, E, F, G, H, I, J>& key)
  {
    if (0 == _os)
    {
      write(key.initialized);
    }
    write(key._a);
    write(key._b);
    write(key._c);
    write(key._d);
    write(key._e);
    write(key._f);
    write(key._g);
    write(key._h);
    write(key._i);
    write(key._j);
  }

  void write(const std::vector<class CategoryRuleItemInfo*>& vect);
  void write(const std::vector<const class FareInfo*>& vect);
  void write(const std::vector<class AddonFareInfo*>& vect);

  template <typename T> void write(const std::vector<T*>& vect)
  {
    if (_os)
    {
      *_os << '[';
    }
    else
    {
      write(static_cast<boost::uint32_t>(vect.size()));
    }
    for (const auto ptr : vect)
    {
      ptr->write(*this);
      _memSize += sizeof(T);
    }
    if (_os)
    {
      *_os << "]|";
    }
  }
  
  template <typename T> void writeContainer(const std::vector<T*>& vect)
  {
    write(static_cast<boost::uint32_t>(vect.size()));
    std::for_each(vect.begin(), vect.end(), boost::bind(&T::write, _1, boost::ref(*this), &_memSize));
  }

  template <typename T> void write(const std::vector<T>& vector)
  {
    if (_os)
    {
      *_os << '[';
    }
    else
    {
      write(static_cast<boost::uint32_t>(vector.size()));
    }
    for (const auto& elem : vector)
    {
      write(elem);
    }
    if (_os)
    {
      *_os << "]|";
    }
  }

  template <typename T> void write(const std::set<T>& set)
  {
    if (_os)
    {
      *_os << '[';
    }
    else
    {
      write(static_cast<boost::uint32_t>(set.size()));
    }
    for (const auto& elem : set)
    {
      write(elem);
    }
    if (_os)
    {
      *_os << "]|";
    }
  }

  void write(const class TaxRestrictionPsg& field);
  void write(const class TaxExemptionCarrier& field);
  void write(const class TaxRestrictionTransit& field);

  template <typename T> void write(const T& field)
  {
    if (_os)
    {
      *_os << field << '|';
    }
    else
    {
      write(reinterpret_cast<const char*>(&field), sizeof(T));
    }
  }

  template <typename T> WBuffer& operator & (const T& value)
  {
    write(value);
    return *this;
  }

  template <typename Key, typename T> void printCache(sfc::Cache<Key, std::vector<T*>>& cache)
  {
    if (_os != 0)
    {
      std::shared_ptr<std::vector<Key>> keys(cache.keys());
      for (const auto& key : *keys)
      {
        const std::vector<T*>* result(cache.getIfResident(key).get());
        *_os << "(";
        write(key);
        *_os << ")\n[";
        for (const auto ptr : *result)
        {
          *_os << "{";
          ptr->write(*this);
          *_os << "}|\n";
        }
        *_os << "]\n";
      }
    }
  }

  void writeMemorySise()
  {
    std::memcpy(&_buffer[0], reinterpret_cast<const char*>(&_memSize), sizeof(_memSize));
  }

  void incrementMemorySize(size_t incr)
  {
    _memSize += incr;
  }
private:
  void write(const char* source,
             size_t sz)
  {
    if (sz > 0)
    {
      size_t reqSize(_size + sz);
      if (reqSize > _buffer.size())
      {
        _buffer.resize(2 * reqSize);
      }
      memcpy(&_buffer[0] + _size, source, sz);
      _size += sz;
    }
  }

  void writeString(const char* str,
                   size_t sz)
  {
    if (_os)
    {
      *_os << str << '|';
    }
    else
    {
      size_t reqSize(_size + sz + 1);
      if (reqSize > _buffer.size())
      {
        _buffer.resize(2 * reqSize);
      }
      std::memcpy(&_buffer[0] + _size, str, sz + 1);
      _size += sz + 1;
    }
  }

  CharBuffer _buffer;
  size_t _size;
  std::ostream* _os;
  size_t _memSize;
};

size_t getUncompressedSize(const sfc::CompressedData* compressed);

template <typename T> bool equalPtr(const T* first,
                                    const T* second)
{
  return *first == *second;
}

template <typename T> bool equalPtrVector(const std::vector<T*>& first,
                                          const std::vector<T*>& second)
{
  return std::equal(first.begin(), first.end(), second.begin(), equalPtr<T>);
}

template <typename T> sfc::CompressedData* compressVector(const std::vector<T*>* vect,
                                                          bool poolObjects = false)
{
  if (vect && !vect->empty())
  {
    WBuffer os(0);
    size_t dummy(0);
    os.write(dummy);
    os.write(*vect);
    if (poolObjects)
    {
      os.incrementMemorySize(sizeof(std::vector<T*>));
      os.writeMemorySise();
    }
    return os.compress();
  }
  return 0;
}

template <typename T> std::vector<T*>* uncompressEntry(const sfc::CompressedData& compressed)
{
  CharBuffer inflated;
  if (RBuffer::uncompress(compressed, inflated))
  {
    RBuffer is(inflated.empty() ? 0 : &inflated[0]);
    std::vector<T*>* vect(0);
    vect = is.construct(vect);
    is.read(*vect);
    return vect;
  }
  return 0;
}

template <typename T, typename U = T> struct uncompressVector
{
  typename std::vector<T*>* operator () (const sfc::CompressedData& compressed) const
  {
    return uncompressEntry<T>(compressed);
  }
};

bool destroyPooledVector(std::vector<class OptionalServicesInfo*>* recs);
bool destroyPooledVector(std::vector<class MarkupControl*>* recs);
bool destroyPooledVector(std::vector<const class FareClassAppInfo*>* recs);

template <typename T> bool destroyPooledVector(std::vector<T*>* recs)
{
  if (recs && CacheEntryPool::instance().entry(recs) != 0)
  {
    typedef typename std::vector<T*> Vector;
    for (const auto ptr : *recs)
    {
      ptr->~T();
    }
    recs->~Vector();
    CacheEntryPool::instance().removeEntry(recs);
    return true;
  }
  return false;
}

void destroyElements(std::vector<class CategoryRuleItemInfoSet*>& categoryRuleItemInfoSetVector);

template <typename T> bool destroyPooledVectorCRI(std::vector<T*>* recs)
{
  if (CacheEntryPool::instance().entry(recs) != 0)
  {
    typedef std::vector<T*> Vector;
    for (const auto ptr : *recs)
    {
      destroyElements(ptr->categoryRuleItemInfoSet());
      ptr->~T();
    }
    recs->~Vector();
    CacheEntryPool::instance().removeEntry(recs);
    return true;
  }
  return false;
}

}// tse
