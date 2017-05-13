//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "FareDAO.h"
#include <algorithm>
#include <functional>

#include "SITAFareInfo.h"
#include "CompressionTestCommon.h"

const long _delay(0);

using namespace sfc;

namespace tse
{
const bool _poolObjects(true);

std::vector<const FareInfo*>*
FareDAO::create(FareInfoKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<const FareInfo*>* ptr = new std::vector<const FareInfo*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    FareInfo *obj(0);
    if (0 == i % 5)
    {
      obj = new SITAFareInfo;
    }
    else
    {
      obj = new FareInfo;
    }
    obj->dummyData();
    (*ptr)[i] = obj;
  }
  return ptr;
}

void
FareDAO::destroy(FareInfoKey key, std::vector<const FareInfo*>* recs)
{
	if(recs == NULL) {
		return;
	}
  if (!destroyPooledVector(recs))
  {
    std::vector<const FareInfo*>::const_iterator it(recs->begin());
    std::vector<const FareInfo*>::const_iterator itend(recs->end());
    for ( ; it != itend; ++it) delete *it;

    delete recs;
  }
}

sfc::CompressedData*
FareDAO::compress(const std::vector<const FareInfo*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<const FareInfo*>*
FareDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<const FareInfo>(compressed);
}

} // namespace tse
