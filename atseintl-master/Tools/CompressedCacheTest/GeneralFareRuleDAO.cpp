//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "GeneralFareRuleDAO.h"
#include "GeneralFareRuleInfo.h"
#include "CompressionTestCommon.h"

namespace tse
{
const bool _poolObjects(true);

namespace
{
void init(std::vector<GeneralFareRuleInfo*> &vect)
{
  typedef std::vector<GeneralFareRuleInfo*> ::const_iterator It;
  for (It it = vect.begin(); it != vect.end(); ++it)
  {
    (*it)->init();
  }
}
} // namespace
GeneralFareRuleDAO& GeneralFareRuleDAO::instance()
{
  if (_instance == 0)
  {
  }
  return *_instance;
}

std::vector<GeneralFareRuleInfo*> *GeneralFareRuleDAO::create(GeneralFareRuleKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    GeneralFareRuleInfo *obj(new GeneralFareRuleInfo);
    obj->dummyData();
    (*ret)[i] = obj;
  }
  init(*ret);
  return ret;
}

void GeneralFareRuleDAO::destroy(GeneralFareRuleKey key,
                                 std::vector<GeneralFareRuleInfo*>* recs)
{
  if (!destroyPooledVectorCRI(recs))
  {
    destroyContainer(recs);
  }
}

sfc::CompressedData* GeneralFareRuleDAO::compress(const std::vector<GeneralFareRuleInfo*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<GeneralFareRuleInfo*>* GeneralFareRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<GeneralFareRuleInfo>(compressed);
}

std::string GeneralFareRuleDAO::_name("GeneralFareRule");
std::string GeneralFareRuleDAO::_cacheClass("Rules");
GeneralFareRuleDAO* GeneralFareRuleDAO::_instance = 0;
} // namespace tse
