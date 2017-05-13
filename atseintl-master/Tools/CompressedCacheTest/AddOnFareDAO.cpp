//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "AddOnFareDAO.h"
#include "SITAAddonFareInfo.h"
#include "CompressionTestCommon.h"

namespace tse
{
const bool _poolObjects(true);

AddOnFareDAO& AddOnFareDAO::instance()
{
    return *_instance;
}

std::vector<AddonFareInfo*>* AddOnFareDAO::create(AddOnFareKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<AddonFareInfo*>* ptr = new std::vector<AddonFareInfo*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    AddonFareInfo *obj(0);
    if (0 == i % 5)
    {
      obj = new SITAAddonFareInfo;
    }
    else
    {
      obj = new AddonFareInfo;
    }
    obj->dummyData();
    (*ptr)[i] = obj;
  }
  return ptr;
}

void AddOnFareDAO::destroy(AddOnFareKey key, std::vector<AddonFareInfo*>* recs)
{
  if (!destroyPooledVector(recs))
  {
    std::vector<AddonFareInfo*>::const_iterator i;
    for (i = recs->begin(); i != recs->end(); i++) delete *i;
    delete recs;
  }
}

sfc::CompressedData*
AddOnFareDAO::compress(const std::vector<AddonFareInfo*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<AddonFareInfo*>*
AddOnFareDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<AddonFareInfo>(compressed);
}

std::string AddOnFareDAO::_name("AddOnFare");
std::string AddOnFareDAO::_cacheClass("Fares");
AddOnFareDAO* AddOnFareDAO::_instance = 0;

} // namespace tse
