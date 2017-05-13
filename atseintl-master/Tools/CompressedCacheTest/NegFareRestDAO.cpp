//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include <algorithm>
#include <functional>

#include "NegFareRestDAO.h"
#include "NegFareRest.h"
#include "CompressionTestCommon.h"

using namespace sfc;

namespace tse
{

const bool _poolObjects(true);

NegFareRestDAO& NegFareRestDAO::instance()
{
    return *_instance;
}

std::vector<NegFareRest*>*NegFareRestDAO::create(NegFareRestKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<NegFareRest*>* ret = new std::vector<NegFareRest*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    NegFareRest *obj(new NegFareRest);
    obj->dummyData();
    (*ret)[i] = obj;
  }
  return ret;
}

void NegFareRestDAO::destroy(NegFareRestKey key, std::vector<NegFareRest*>* recs)
{
  if (!destroyPooledVector(recs))
  {
    std::vector<NegFareRest*>::iterator i;
    for (i = recs->begin(); i != recs->end(); i++) delete *i;
    delete recs;
  }
}

sfc::CompressedData*
NegFareRestDAO::compress(const std::vector<NegFareRest*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<NegFareRest*>*
NegFareRestDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<NegFareRest>(compressed);
}

std::string NegFareRestDAO::_name("NegFareRest");
std::string NegFareRestDAO::_cacheClass("Rules");
NegFareRestDAO* NegFareRestDAO::_instance = 0;

} // namespace tse
