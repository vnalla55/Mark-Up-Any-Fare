//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "AddOnCombFareClassDAO.h"
//#include "DBAccess/DAOHelper.h"
//#include "DBAccess/DBAdapterPool.h"
//#include "DBAccess/DeleteList.h"
#include "AddonCombFareClassInfo.h"
//#include "DBAccess/DAOInterface.h"
//#include "DBAccess/Queries/QueryGetCombFareClass.h"
#include "CompressionTestCommon.h"

namespace tse
{
const bool _poolObjects(true);

// Historical Stuff //////////////////////////////////////////////////////////////////////////////////////////////////////////////
//log4cxx::LoggerPtr AddOnCombFareClassHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnCombFareClassHistoricalDAO"));

AddOnCombFareClassHistoricalDAO& AddOnCombFareClassHistoricalDAO::instance()
{
    if (_instance == 0)
    {
        //_helper.init();
    }
    return *_instance;
}

std::vector<AddonCombFareClassInfo*>* AddOnCombFareClassHistoricalDAO::create(AddOnCombFareClassHistoricalKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<AddonCombFareClassInfo*> *ret = new std::vector<AddonCombFareClassInfo*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    AddonCombFareClassInfo *obj(new AddonCombFareClassInfo);
    obj->dummyData();
    (*ret)[i] = obj;
  }

  return ret;
}

void AddOnCombFareClassHistoricalDAO::destroy(AddOnCombFareClassHistoricalKey key, std::vector<AddonCombFareClassInfo*>* recs)
{
  if (!destroyPooledVector(recs))
  {
    std::vector<AddonCombFareClassInfo*>::const_iterator currIter(recs->begin());
    std::vector<AddonCombFareClassInfo*>::const_iterator endIter(recs->end());
    for (; currIter != endIter; ++currIter)
    {
      delete *currIter;
    }
    delete recs;
  }
}

sfc::CompressedData*
AddOnCombFareClassHistoricalDAO::compress(const std::vector<AddonCombFareClassInfo*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<AddonCombFareClassInfo*>*
AddOnCombFareClassHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<AddonCombFareClassInfo>(compressed);
}

std::string AddOnCombFareClassHistoricalDAO::_name("AddOnCombFareClassHistorical");
std::string AddOnCombFareClassHistoricalDAO::_cacheClass("Fares");
//DAOHelper<AddOnCombFareClassHistoricalDAO> AddOnCombFareClassHistoricalDAO::_helper(_name);
AddOnCombFareClassHistoricalDAO* AddOnCombFareClassHistoricalDAO::_instance = 0;

} // namespace tse
