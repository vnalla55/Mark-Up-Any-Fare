//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "FareClassAppDAO.h"
#include "FareClassAppInfo.h"
#include "CompressionTestCommon.h"

namespace tse
{

const bool _poolObjects(true);

std::vector<const FareClassAppInfo*>* FareClassAppDAO::create(FareClassAppKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    FareClassAppInfo* obj(new FareClassAppInfo);
    obj->dummyData();
    (*ret)[i] = obj;
  }

    return ret;
}

void FareClassAppDAO::destroy(FareClassAppKey key, std::vector<const FareClassAppInfo*>* recs)
{
  if (!destroyPooledVector(recs))
  {
    std::vector<const FareClassAppInfo*>::iterator i;
    FareClassAppSegInfoList::const_iterator j;
    for (i = recs->begin(); i != recs->end(); ++i)
    {
        const FareClassAppSegInfoList& segs = (*i)->_segs;
        for (j = segs.begin() ; j != segs.end() ; ++j)
            delete *j;
        delete *i;
    }
    delete recs;
  }
}

sfc::CompressedData*
FareClassAppDAO::compress(const std::vector<const FareClassAppInfo*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<const FareClassAppInfo*>*
FareClassAppDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<const FareClassAppInfo>(compressed);
}

std::string FareClassAppDAO::_name("FareClassApp");
std::string FareClassAppDAO::_cacheClass("Fares");
FareClassAppDAO* FareClassAppDAO::_instance = 0;
} // namespace tse
