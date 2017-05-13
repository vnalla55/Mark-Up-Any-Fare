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

#include "CompressedDataUtils.h"
#include <cassert>
#include <snappy.h>
#include "LocKey.h"
#include "CombinabilityRuleItemInfo.h"
#include "OptionalServicesInfo.h"
#include "MarkupControl.h"
#include "FareClassAppInfo.h"
#include "SITAFareInfo.h"
#include "SITAAddonFareInfo.h"
#include "CategoryRuleItemInfoSet.h"
#include "TaxRestrictionPsg.h"
#include "TaxExemptionCarrier.h"
#include "TaxRestrictionTransit.h"

namespace tse
{

namespace
{

const bool _compress2(true);

size_t getUncompressedLength(const CharBuffer& deflated)
{
  size_t inflatedSz(0);
  if (!deflated.empty() && snappy::GetUncompressedLength(&deflated[0], deflated.size(), &inflatedSz))
  {
    return inflatedSz;
  }
  return 0;
}

size_t getUncompressedLength(const std::vector<char>& deflated)
{
  size_t inflatedSz(0);
  if (!deflated.empty() && snappy::GetUncompressedLength(&deflated[0], deflated.size(), &inflatedSz))
  {
    return inflatedSz;
  }
  return 0;
}

sfc::CompressedData* compress1(const WBuffer& os)
{
  size_t inflatedSz(os.size());
  CharBuffer deflated(snappy::MaxCompressedLength(inflatedSz));
  size_t deflatedSz(0);
  snappy::RawCompress(os.buffer(), inflatedSz, &deflated[0], &deflatedSz);
  return new sfc::CompressedData(&deflated[0], deflatedSz);
}

bool uncompress1(const sfc::CompressedData& compressed,
                 CharBuffer& inflated)
{
  size_t inflatedSz(0);
  if (!compressed._deflated.empty() && (inflatedSz = getUncompressedLength(compressed._deflated)) > 0)
  {
    inflated.resize(inflatedSz);
    return snappy::RawUncompress(&compressed._deflated[0], compressed._deflated.size(), &inflated[0]);
  }
  return false;
}

sfc::CompressedData* compress2(const WBuffer& os)
{
  size_t inflatedSz(os.size());
  CharBuffer deflated(snappy::MaxCompressedLength(inflatedSz));
  size_t deflatedSz(0);
  snappy::RawCompress(os.buffer(), inflatedSz, &deflated[0], &deflatedSz);
  CharBuffer deflated2(snappy::MaxCompressedLength(deflatedSz));
  snappy::RawCompress(&deflated[0], deflatedSz, &deflated2[0], &deflatedSz);
  return new sfc::CompressedData(&deflated2[0], deflatedSz);
}

bool uncompress2(const sfc::CompressedData& compressed,
                 CharBuffer& inflated)
{
  size_t deflatedSz(0);
  if (!compressed._deflated.empty() && (deflatedSz = getUncompressedLength(compressed._deflated)) > 0)
  {
    CharBuffer deflated(deflatedSz);
    if (snappy::RawUncompress(&compressed._deflated[0], compressed._deflated.size(), &deflated[0]))
    {
      size_t inflatedSz(0);
      if (!deflated.empty() && (inflatedSz = getUncompressedLength(deflated)) > 0)
      {
        inflated.resize(inflatedSz);
        return snappy::RawUncompress(&deflated[0], deflatedSz, &inflated[0]);
      }
    }
  }
  return false;
}

}

RBuffer::RBuffer(const char* source)
  : _readPointer(source)
  , _poolPointer(0)
{
  if (_readPointer)
  {
    size_t memSize(0);
    read(memSize);
    if (memSize > 0)
    {
      _poolPointer = CacheEntryPool::instance().createEntry(memSize);
    }
  }
}

void RBuffer::read(std::vector<std::string>& vector)
{
  boost::uint32_t size(0);
  read(size);
  vector.reserve(size);
  for (boost::uint32_t i = 0; i < size; ++i)
  {
    std::string item;
    read(item);
    vector.push_back(item);
  }
}

void RBuffer::read(LocKey& locKey)
{
  read(locKey.loc());
  read(locKey.locType());
}

void RBuffer::read(TaxRestrictionPsg& field)
{
  field.read(*this);
}

void RBuffer::read(TaxExemptionCarrier& field)
{
  field.read(*this);
}

void RBuffer::read(TaxRestrictionTransit& field)
{
  field.read(*this);
}

void RBuffer::read(std::vector<CategoryRuleItemInfo*>& vect)
{
  boost::uint32_t size(0);
  read(size);
  vect.reserve(size);
  for (boost::uint32_t i = 0; i < size; ++i)
  {
    char type(0);
    read(type);
    CategoryRuleItemInfo* entry(0);
    switch (type)
    {
    case 'B':
      entry = construct(entry);
      break;
    case 'C':
      entry = construct(static_cast<CombinabilityRuleItemInfo*>(entry));
      break;
    default:
      assert(false);
      break;
    }
    if (entry != 0)
    {
      entry->read(*this);
      vect.push_back(entry);
    }
  }
}

void RBuffer::read(std::vector<const FareInfo*>& vect)
{
  boost::uint32_t size(0);
  read(size);
  vect.reserve(size);
  for (boost::uint32_t i = 0; i < size; ++i)
  {
    char type(0);
    read(type);
    FareInfo* entry(0);
    switch (type)
    {
    case 'A':// ATP
      entry = construct(entry);
      break;
    case 'S':// SITA
      entry = construct(static_cast<SITAFareInfo*>(entry));
      break;
    default:
      assert(false);
      break;
    }
    if (entry != 0)
    {
      entry->read(*this);
      vect.push_back(entry);
    }
  }
}

void RBuffer::read(std::vector<AddonFareInfo*>& vect)
{
  boost::uint32_t size(0);
  read(size);
  vect.reserve(size);
  for (boost::uint32_t i = 0; i < size; ++i)
  {
    char type(0);
    read(type);
    AddonFareInfo* entry(0);
    switch (type)
    {
    case 'A':// ATP
      entry = construct(entry);
      break;
    case 'S':// SITA
      entry = construct(static_cast<SITAAddonFareInfo*>(entry));
      break;
    default:
      assert(false);
      break;
    }
    if (entry != 0)
    {
      entry->read(*this);
      vect.push_back(entry);
    }
  }
}

bool RBuffer::uncompress(const sfc::CompressedData& compressed,
                         CharBuffer& inflated)
{
  if (_compress2)
  {
    return uncompress2(compressed, inflated);
  }
  else
  {
    return uncompress1(compressed, inflated);
  }
}

WBuffer::WBuffer(std::ostream* os)
  : _size(0)
  , _os(os)
  , _memSize(0)
{
  if (0 == os)
  {
    _buffer.resize(2048);
  }
}

const char* WBuffer::buffer() const
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

void WBuffer::write(const LocKey& locKey)
{
  if (_os)
  {
    *_os << '{' << locKey.loc() << '|' << locKey.locType() << "}|";
  }
  else
  {
    write(locKey.loc());
    write(locKey.locType());
  }
}

void WBuffer::write(const TaxRestrictionPsg& field)
{
  field.write(*this);
}

void WBuffer::write(const TaxExemptionCarrier& field)
{
  field.write(*this);
}

void WBuffer::write(const TaxRestrictionTransit& field)
{
  field.write(*this);
}

void WBuffer::write(const std::vector<CategoryRuleItemInfo*>& vect)
{
  if (_os)
  {
    *_os << '[';
    for (const auto ptr : vect)
    {
      *_os << '{';
      ptr->write(*this);
      *_os << '}';
    }
    *_os << "]|";
  }
  else
  {
    writeContainer(vect);
  }
}

void WBuffer::write(const std::vector<const FareInfo*>& vect)
{
  writeContainer(vect);
}

void WBuffer::write(const std::vector<AddonFareInfo*>& vect)
{
  writeContainer(vect);
}

sfc::CompressedData* WBuffer::compress() const
{
  if (_compress2)
  {
    return compress2(*this);
  }
  else
  {
    return compress1(*this);
  }
}

size_t getUncompressedSize(const sfc::CompressedData* compressed)
{
  size_t size(0);
  if (compressed)
  {
    if (_compress2)
    {
      size_t deflatedSz(0);
      if (!compressed->_deflated.empty() && (deflatedSz = getUncompressedLength(compressed->_deflated)) > 0)
      {
        CharBuffer deflated(deflatedSz);
        if (snappy::RawUncompress(&compressed->_deflated[0], compressed->_deflated.size(), &deflated[0]))
        {
          size_t inflatedSz(0);
          if (!deflated.empty() && (inflatedSz = getUncompressedLength(deflated)) > 0)
          {
            return getUncompressedLength(deflated);
          }
        }
      }
    }
    else
    {
      return getUncompressedLength(compressed->_deflated);
    }
  }
  return size;
}

template <typename T> void destroyElement(const T* child)
{
  child->~T();
}

void destroyElements(std::vector<CategoryRuleItemInfoSet*>& categoryRuleItemInfoSetVector)
{
  typedef std::vector<CategoryRuleItemInfoSet*> CRIISVector;
  for (const auto ptr : categoryRuleItemInfoSetVector)
  {
    ptr->callDestructors();
    ptr->~CategoryRuleItemInfoSet();
  }
  CRIISVector empty;
  categoryRuleItemInfoSetVector.swap(empty);
}

template <typename T> void destroyElements(std::vector<T*>& children)
{
  std::for_each(children.begin(), children.end(), boost::bind(&destroyElement<T>, _1));
  typedef std::vector<T*> Children;
  Children empty;
  children.swap(empty);
}

bool destroyPooledVector(std::vector<OptionalServicesInfo*>* recs)
{
  typedef std::vector<OptionalServicesInfo*> InfoVector;
  if (recs && CacheEntryPool::instance().entry(recs) != 0)
  {
    for (const auto ptr : *recs)
    {
      destroyElements(ptr->segs());
      ptr->~OptionalServicesInfo();
    }
    recs->~InfoVector();
    CacheEntryPool::instance().removeEntry(recs);
    return true;
  }
  return false;
}

bool destroyPooledVector(std::vector<MarkupControl*>* recs)
{
  typedef std::vector<MarkupControl*> InfoVector;
  if (recs && CacheEntryPool::instance().entry(recs) != 0)
  {
    for (const auto ptr : *recs)
    {
      destroyElements(ptr->calcs());
      ptr->~MarkupControl();
    }
    recs->~InfoVector();
    CacheEntryPool::instance().removeEntry(recs);
    return true;
  }
  return false;
}

bool destroyPooledVector(std::vector<const FareClassAppInfo*>* recs)
{
  if (recs && CacheEntryPool::instance().entry(recs) != 0)
  {
    typedef std::vector<const FareClassAppInfo*> InfoVector;
    for (const auto ptr : *recs)
    {
      destroyElements(const_cast<FareClassAppSegInfoList&>(ptr->_segs));
      ptr->~FareClassAppInfo();
    }
    recs->~InfoVector();
    CacheEntryPool::instance().removeEntry(recs);
    return true;
  }
  return false;
}

}// tse
