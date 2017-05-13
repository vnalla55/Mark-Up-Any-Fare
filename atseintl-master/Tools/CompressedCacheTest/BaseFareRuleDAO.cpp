//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include <algorithm>
#include <functional>

#include "BaseFareRuleDAO.h"
#include "BaseFareRule.h"
#include "CompressionTestCommon.h"

using namespace sfc;

namespace tse
{
const bool _poolObjects(true);

BaseFareRuleDAO& BaseFareRuleDAO::instance()
{
    if (_instance == 0)
    {
    }
    return *_instance;
}

std::vector<const BaseFareRule*>* BaseFareRuleDAO::create(BaseFareRuleKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
    std::vector<const BaseFareRule*>* ret = new std::vector<const BaseFareRule*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    BaseFareRule *obj(new BaseFareRule);
    obj->dummyData();
    (*ret)[i] = obj;
  }
    return ret;
}

void BaseFareRuleDAO::destroy(BaseFareRuleKey key, std::vector<const BaseFareRule*>* recs)
{
  if (!destroyPooledVector(recs))
  {
    std::vector<const BaseFareRule*>::iterator i;
    for (i = recs->begin(); i != recs->end(); i++) delete *i;
    delete recs;
  }
}

void BaseFareRuleDAO::load()
{
}

sfc::CompressedData*
BaseFareRuleDAO::compress (const std::vector<const BaseFareRule*> *vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<const BaseFareRule*>*
BaseFareRuleDAO::uncompress (const sfc::CompressedData& compressed) const
{
  return uncompressEntry<const BaseFareRule>(compressed);
}

std::string BaseFareRuleDAO::_name("BaseFareRule");
std::string BaseFareRuleDAO::_cacheClass("Rules");
BaseFareRuleDAO* BaseFareRuleDAO::_instance = 0;

} // namespace tse
