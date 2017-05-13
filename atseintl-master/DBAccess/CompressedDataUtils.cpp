#include "DBAccess/CompressedDataUtils.h"

#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "DBAccess/SITAFareInfo.h"

namespace tse
{


void
RBuffer::read(std::vector<AddonFareInfo*>& vect)
{
  boost::uint32_t size(0);
  read(size);
  vect.resize(size);
  for (boost::uint32_t i = 0; i < size; ++i)
  {
    boost::uint8_t type(eAddonFareInfo);
    read(type);
    AddonFareInfo* entry(AddonFareInfoFactory::create(static_cast<eAddonFareInfoType>(type)));
    if (entry != nullptr)
    {
      entry->read(*this);
      vect[i] = entry;
    }
  }
}

void
RBuffer::read(std::vector<const FareInfo*>& vect)
{
  boost::uint32_t size(0);
  read(size);
  vect.resize(size);
  for (boost::uint32_t i = 0; i < size; ++i)
  {
    boost::uint8_t type(eFareInfo);
    read(type);
    FareInfo* entry(FareInfoFactory::create(static_cast<eFareInfoType>(type)));
    if (entry != nullptr)
    {
      entry->read(*this);
      vect[i] = entry;
    }
  }
}

void
WBuffer::write(const std::vector<AddonFareInfo*>& vect)
{
  writeContainer(vect);
}

void
WBuffer::write(const std::vector<const FareInfo*>& vect)
{
  writeContainer(vect);
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

sfc::CompressedData*
compressEntry(const AddonFareClassCombMultiMap* entry)
{
  std::vector<AddonCombFareClassInfo*> vect;
  vect.reserve(entry->size());
  for (const auto& elem : *entry)
  {
    for (auto afc: elem.second)
      vect.push_back(afc);
  }
  return compressVector(&vect);
}

#else

sfc::CompressedData*
compressEntry(const AddonFareClassCombMultiMap* entry)
{
  std::vector<AddonCombFareClassInfo*> vect;
  vect.reserve(entry->size());
  for (const auto& elem : *entry)
  {
    vect.push_back(elem.second);
  }
  return compressVector(&vect);
}

#endif

sfc::CompressedData*
compressEntry(const BookingCodeExceptionSequenceList* entry)
{
  return compressVector(&entry->getSequences());
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

AddonFareClassCombMultiMap*
uncompressEntry(const sfc::CompressedData& compressed, const AddonFareClassCombMultiMap*)
{
  const std::vector<AddonCombFareClassInfo*>* vect(
      uncompressVectorPtr<AddonCombFareClassInfo>(compressed));
  AddonFareClassCombMultiMap* map(new AddonFareClassCombMultiMap);
  if (vect)
  {
    for (const auto addon : *vect)
    {
      AddonCombFareClassSpecifiedKey key(addon->fareClass(), addon->owrt());
      AddonFareClassCombMultiMap::mapped_type& pr((*map)[key]);
      pr.push_back(addon);
    }
  }
  delete vect;
  return map;
}

#else

AddonFareClassCombMultiMap*
uncompressEntry(const sfc::CompressedData& compressed, const AddonFareClassCombMultiMap*)
{
  const std::vector<AddonCombFareClassInfo*>* vect(
      uncompressVectorPtr<AddonCombFareClassInfo>(compressed));
  AddonFareClassCombMultiMap* map(new AddonFareClassCombMultiMap);
  if (vect)
  {
    for (const auto addon : *vect)
    {
      AddonCombFareClassInfoKey key(
          addon->addonFareClass(), addon->geoAppl(), addon->owrt(), addon->fareClass());
      AddonFareClassCombMultiMap::value_type pr(key, addon);
      map->insert(boost::move(pr));
    }
  }
  delete vect;
  return map;
}

#endif

BookingCodeExceptionSequenceList*
uncompressEntry(const sfc::CompressedData& compressed, const BookingCodeExceptionSequenceList*)
{
  BookingCodeExceptionSequenceList* ret(new BookingCodeExceptionSequenceList);
  ret->getSequences() = *uncompressVectorPtr<BookingCodeExceptionSequence>(compressed);
  return ret;
}

} // tse
