//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "CombinabilityRuleDAO.h"
#include "CombinabilityRuleInfo.h"
#include "CompressionTestCommon.h"

namespace tse
{
const bool _poolObjects(true);

CombinabilityRuleDAO& CombinabilityRuleDAO::instance()
{
    return *_instance;
}

std::vector<CombinabilityRuleInfo*>* CombinabilityRuleDAO::create(CombinabilityRuleKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<CombinabilityRuleInfo*>* ret = new std::vector<CombinabilityRuleInfo*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    CombinabilityRuleInfo* obj(new CombinabilityRuleInfo);
    obj->dummyData();
    (*ret)[i] = obj;
  }
  return ret;
}

void CombinabilityRuleDAO::destroy(CombinabilityRuleKey key, std::vector<CombinabilityRuleInfo*>* recs)
{
  if (!destroyPooledVectorCRI(recs))
  {
    std::vector<CombinabilityRuleInfo*>::const_iterator i;
    for (i = recs->begin(); i != recs->end(); i++) delete *i;
    delete recs;
  }
}

sfc::CompressedData*
CombinabilityRuleDAO::compress(const std::vector<CombinabilityRuleInfo*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<CombinabilityRuleInfo*>*
CombinabilityRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<CombinabilityRuleInfo>(compressed);
}


std::string CombinabilityRuleDAO::_name("CombinabilityRule");
std::string CombinabilityRuleDAO::_cacheClass("Rules");
CombinabilityRuleDAO* CombinabilityRuleDAO::_instance = 0;

} // namespace tse
