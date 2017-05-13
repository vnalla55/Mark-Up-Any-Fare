//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include <functional>
#include <algorithm>

#include "ContractPreferenceDAO.h"
#include "ContractPreference.h"
#include "CompressionTestCommon.h"

namespace tse
{
const bool _poolObjects(true);

ContractPreferenceDAO&
ContractPreferenceDAO::instance()
{
    return *_instance;
}

std::vector<ContractPreference*>*
ContractPreferenceDAO::create(ContractPreferenceKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<ContractPreference*>* ret = new std::vector<ContractPreference*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    ContractPreference *obj(new ContractPreference);
    obj->dummyData();
    (*ret)[i] = obj;
  }

    return ret;
}

void
ContractPreferenceDAO::destroy(ContractPreferenceKey key, std::vector<ContractPreference*> * recs)
{
  if (!destroyPooledVector(recs))
  {
	  destroyContainer(recs);
  }
}

std::string ContractPreferenceDAO::_name("ContractPreference");
std::string ContractPreferenceDAO::_cacheClass("Common");
ContractPreferenceDAO* ContractPreferenceDAO::_instance = 0;

sfc::CompressedData*
ContractPreferenceDAO::compress(const std::vector<ContractPreference*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<ContractPreference*>*
ContractPreferenceDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<ContractPreference>(compressed);
}

} // namespace tse
