#pragma once

#include "Common/Assert.h"
#include "Common/Code.h"
#include "Common/DateTime.h"
#include "Common/Utils/ShadowPtr.h"
#include "Common/Utils/ShadowVector.h"
#include "DBAccess/CompressedData.h"
#include "DBAccess/CompressedDataImpl.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TSEDateInterval.h"

#include <boost/bind.hpp>
#include <boost/container/string.hpp>
#include <boost/type_traits.hpp>

#include <cstring>
#include <sstream>
#include <type_traits>

namespace tse
{
class AdditionalInfoContainer;

template<class T>
struct cdu_pod_traits: std::is_trivially_copyable<T>{};

template<> struct cdu_pod_traits<Code<2ul>[8]>: std::true_type{};

template<>
struct cdu_pod_traits<std::pair<Code<4ul>, Code<2ul>>>: std::true_type{};


class RBuffer : boost::noncopyable
{
public:
  explicit RBuffer(const std::vector<char>& source) : _pointer(source.empty() ? nullptr : &source[0]) {}
  void read(char& ch) { ch = *_pointer++; }
  void read(char* buffer, size_t sz)
  {
    memcpy(buffer, _pointer, sz);
    _pointer += sz;
  }
  void read(std::string& str)
  {
    size_t length(strlen(_pointer));
    str.assign(_pointer, length);
    _pointer += length + 1;
  }

  void read(boost::container::string& str)
  {
    size_t length(strlen(_pointer));
    str.assign(_pointer, length);
    _pointer += length + 1;
  }

  void read(BoostString& str)
  {
    size_t length(strlen(_pointer));
    str.assign(_pointer, length);
    _pointer += length + 1;
  }

  void read(DateTime& dt)
  {
    boost::int64_t rep(0);
    read(rep);
    dt.setIntRep(rep);
  }
  template <size_t n>
  void read(Code<n>& str)
  {
    char* dst(&str[0]);
    while ((*dst++ = *_pointer++) != 0)
      ;
  }
  void read(TSEDateInterval& di)
  {
    boost::int64_t rep[4] = {};
    read(reinterpret_cast<char*>(rep), sizeof(boost::int64_t) * 4);
    di.createDate().setIntRep(rep[0]);
    di.effDate().setIntRep(rep[1]);
    di.expireDate().setIntRep(rep[2]);
    di.discDate().setIntRep(rep[3]);
  }

  template <size_t n>
  void read(std::vector<Code<n> >& vector)
  {
    boost::uint32_t size(0);
    read(size);
    vector.reserve(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      Code<n> item;
      read(item);
      vector.emplace_back(item);
    }
  }

  template <size_t n, size_t m>
  void read(Code<n> arr[m])
  {
    for (size_t i = 0; i < m; ++i)
    {
      read(arr[i]);
    }
  }

  void read(std::vector<class AddonFareInfo*>& vect);
  void read(std::vector<const class FareInfo*>& vect);

  template <typename T>
  void read(std::vector<T*>& vect)
  {
    boost::uint32_t size(0);
    read(size);
    vect.resize(size);
    typedef typename boost::remove_const<T>::type NONCONST;
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      NONCONST* entry(new NONCONST);
      entry->read(*this);
      vect[i] = entry;
    }
  }

  template <class T, class CacheAccessor>
  void read(ShadowVector<T, CacheAccessor>& v)
  {
    boost::uint32_t size(0);
    read(size);
    v.mutableStorage().resize(size);
    typedef typename boost::remove_const<T>::type NONCONST;
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      NONCONST* entry(new NONCONST);
      read(*entry);
      v.mutableStorage()[i] = entry;
    }
    v.sync_with_cache();
  }

  template <class T, class CacheAccessor>
  void read(ShadowPtr<T, CacheAccessor>& p)
  {
    typedef typename boost::remove_const<T>::type NONCONST;
    std::unique_ptr<NONCONST> entry(new NONCONST);
    read(*entry);
    p.reset(entry.release());
    p.sync_with_cache();
  }

  void read(nil_t) {}

  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F,
            typename G,
            typename H,
            typename I,
            typename J>
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

  void read(AdditionalInfoContainer*) {}
  void read(LocKey& locKey)
  {
    read(locKey.loc());
    read(locKey.locType());
  }

  template <typename T>
  void read(T& field)
  {
    static_assert(cdu_pod_traits<T>::value,
        "Non trivially-copyable class interpreted as binary array");
    read(reinterpret_cast<char*>(&field), sizeof(T));
  }
  void read(std::vector<std::string>& vector)
  {
    boost::uint32_t size(0);
    read(size);
    vector.reserve(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      std::string item;
      read(item);
      vector.emplace_back(item);
    }
  }
  template <typename T>
  void read(std::vector<T>& vector)
  {
    boost::uint32_t size(0);
    read(size);
    vector.reserve(size);
    for (boost::uint32_t sz = 0; sz < size; ++sz)
    {
      vector.push_back(T());
      read(vector.back());
    }
  }

  template <typename T>
  void read(std::set<T>& set)
  {
    boost::uint32_t size(0);
    read(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      T entry;
      read(entry);
      set.emplace(entry);
    }
  }
  template <size_t n>
  void read(std::set<Code<n> >& set)
  {
    boost::uint32_t size(0);
    read(size);
    for (boost::uint32_t i = 0; i < size; ++i)
    {
      Code<n> entry;
      read(entry);
      set.emplace(entry);
    }
  }
  template <typename T>
  RBuffer& operator&(T& value)
  {
    read(value);
    return *this;
  }

private:
  const char* _pointer;
};

class WBuffer : boost::noncopyable
{
public:
  WBuffer() : _pointer(0) { _buffer.resize(1024); }
  size_t size() const { return _pointer; }
  void write(char ch)
  {
    size_t reqSize(_pointer + 1);
    if (reqSize > _buffer.size())
    {
      _buffer.resize(2 * reqSize);
    }
    _buffer[_pointer] = ch;
    ++_pointer;
  }
  void write(const char* source, size_t sz)
  {
    if (sz > 0)
    {
      size_t reqSize(_pointer + sz);
      if (reqSize > _buffer.size())
      {
        _buffer.resize(2 * reqSize);
      }
      memcpy(&_buffer[0] + _pointer, source, sz);
      _pointer += sz;
    }
  }
  const char* buffer() const
  {
    if (_buffer.empty())
    {
      static const char* const emptyString("");
      return emptyString;
    }
    else
    {
      return &_buffer[0];
    }
  }
  template <size_t n>
  void write(const Code<n>& str)
  {
    writeString(str.c_str(), str.length());
  }
  void write(const std::string& str) { writeString(str.c_str(), str.length()); }

  void write(const boost::container::string& str) { writeString(str.c_str(), str.length()); }

  void write(const BoostString& str)
  {
    writeString(str.c_str(), str.length());
  }

  void write(const DateTime& dt) { write(dt.getIntRep()); }
  template <size_t n>
  void write(const std::vector<Code<n> >& vect)
  {
    size_t sz(vect.size());
    write(static_cast<boost::uint32_t>(sz));
    for (size_t i = 0; i < sz; ++i)
    {
      write(vect[i]);
    }
  }

  template <size_t n, size_t m>
  void write(const Code<n> arr[m])
  {
    for (size_t i = 0; i < m; ++i)
    {
      write(arr[i]);
    }
  }

  void write(const std::vector<class AddonFareInfo*>& vect);
  void write(const std::vector<const class FareInfo*>& vect);

  template <typename T>
  void write(const T& field)
  {
    static_assert(cdu_pod_traits<T>::value,
            "Non trivially-copyable class interpreted as binary array");
    write(reinterpret_cast<const char*>(&field), sizeof(T));
  }
  void write(const LocKey& locKey)
  {
    write(locKey.loc());
    write(locKey.locType());
  }
  void write(AdditionalInfoContainer* const) {}
  void write(const TSEDateInterval& di)
  {
    boost::int64_t arr[4] = { di.createDate().getIntRep(), di.effDate().getIntRep(),
                              di.expireDate().getIntRep(), di.discDate().getIntRep() };
    write(reinterpret_cast<const char*>(arr), sizeof(boost::int64_t) * 4);
  }

  void write(nil_t) {}

  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F,
            typename G,
            typename H,
            typename I,
            typename J>
  void write(const HashKey<A, B, C, D, E, F, G, H, I, J>& key)
  {
    write(key.initialized);
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

  template <typename T>
  void write(const std::vector<T*>& vect)
  {
    write(static_cast<boost::uint32_t>(vect.size()));
    for (const auto elem : vect)
    {
      elem->write(*this);
    }
  }

  template <class T, class CacheAccessor>
  void write(const ShadowVector<T, CacheAccessor>& v)
  {
    write(static_cast<boost::uint32_t>(v.storage().size()));
    for (const auto* elem: v.storage())
    {
      write(*elem);
    }
  }

  template <class T, class CacheAccessor>
  void write(const ShadowPtr<T, CacheAccessor>& p)
  {
    TSE_ASSERT(p.get() != nullptr);
    write(*p);
  }

  template <typename T>
  void writeContainer(const std::vector<T*>& vect)
  {
    write(static_cast<boost::uint32_t>(vect.size()));
    std::for_each(vect.begin(),
                  vect.end(),
                  boost::bind(&T::write, _1, boost::ref(*this), static_cast<size_t*>(nullptr)));
  }

  template <typename T>
  void write(const std::vector<T>& vector)
  {
    write(static_cast<boost::uint32_t>(vector.size()));
    for (const auto& elem : vector)
    {
      write(elem);
    }
  }
  template <typename T>
  void write(const std::set<T>& set)
  {
    write(static_cast<boost::uint32_t>(set.size()));
    for (const auto& elem : set)
    {
      write(elem);
    }
  }
  template <typename T>
  WBuffer& operator&(const T& value)
  {
    write(value);
    return *this;
  }

private:
  void writeString(const char* str, size_t sz)
  {
    size_t reqSize(_pointer + sz + 1);
    if (reqSize > _buffer.size())
    {
      _buffer.resize(2 * reqSize);
    }
    if (sz > 0)
    {
      memcpy(&_buffer[0] + _pointer, str, sz);
    }
    _pointer += sz;
    _buffer[_pointer] = 0;
    ++_pointer;
  }
  std::vector<char> _buffer;
  size_t _pointer;
};

template <typename T>
bool
equalPtr(const T* first, const T* second)
{
  return *first == *second;
}

template <typename T> bool equalPtrContainer(const T& first,
                                             const T& second,
                                             std::string& msg)
{
  return first == second;
}

template <typename T> bool equalPtrContainer(const std::vector<T*>& first,
                                             const std::vector<T*>& second,
                                             std::string& msg)
{
  std::ostringstream os;
  if (first.size() != second.size())
  {
    os << "first.size()=" << first.size() << " second.size()=" << second.size();
    msg = os.str();
    return false;
  }
  return std::equal(first.begin(), first.end(), second.begin(), equalPtr<T>);
}

template <typename T>
bool
equalPtrHashMap(const T& first, const T& second)
{
  if (first.size() != second.size())
  {
    return false;
  }
  for (typename T::const_iterator it1(first.begin()), itend1(first.end()), itend2(second.end());
       it1 != itend1;
       ++it1)
  {
    typename T::const_iterator it2(second.find(it1->first));
    if (it2 == itend2)
    {
      return false;
    }
    if (!(*it2->second == *it1->second))
    {
      return false;
    }
  }
  return true;
}

template <typename T>
sfc::CompressedData*
compressVector(const T* vect)
{
  if (vect && !vect->empty())
  {
    WBuffer os;
    os.write(*vect);
    return CompressedDataImpl::compress(os.buffer(), os.size());
  }
  return nullptr;
}

template <typename T>
std::vector<T*>*
uncompressVectorPtr(const sfc::CompressedData& compressed)
{
  if (!compressed._deflated.empty() && compressed._inflatedSz > 0)
  {
    std::vector<char> inflated;
    const std::vector<char>* result(CompressedDataImpl::uncompress(compressed, inflated));
    if (result)
    {
      RBuffer is(*result);
      std::vector<T*>* vect(new std::vector<T*>);
      is.read(*vect);
      return vect;
    }
  }
  return nullptr;
}

template <typename T>
struct is_vector_ptr
{
  static constexpr bool value = false;
};

template <class T>
struct is_vector_ptr<std::vector<T*> >
{
  static bool const value = true;
};

sfc::CompressedData*
compressEntry(const class AddonFareClassCombMultiMap* entry);

sfc::CompressedData*
compressEntry(const class BookingCodeExceptionSequenceList* entry);

AddonFareClassCombMultiMap*
uncompressEntry(const sfc::CompressedData& compressed,
                const class AddonFareClassCombMultiMap* entry);

BookingCodeExceptionSequenceList*
uncompressEntry(const sfc::CompressedData& compressed,
                const class BookingCodeExceptionSequenceList* entry);

template <typename T>
sfc::CompressedData*
compressEntry(const T* entry)
{
  if (is_vector_ptr<T>::value)
  {
    return compressVector(entry);
  }
  assert(false);
  return nullptr;
}

template <typename T>
std::vector<T*>*
uncompressVectorPtr(const sfc::CompressedData& compressed, const std::vector<T*>*)
{
  return uncompressVectorPtr<T>(compressed);
}

template <typename T>
T*
uncompressEntry(const sfc::CompressedData& compressed, const T* entry)
{
  if (is_vector_ptr<T>::value)
  {
    return uncompressVectorPtr(compressed, entry);
  }
  assert(false);
  return 0;
}

} // tse
