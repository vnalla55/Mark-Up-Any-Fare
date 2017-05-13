//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDisplayPrefDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDisplayPref.h"
#include "DBAccess/Queries/QueryGetFareDisplayPref.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDisplayPrefDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareDisplayPrefDAO"));

FareDisplayPrefDAO&
FareDisplayPrefDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDisplayPref*>&
getFareDisplayPrefData(const Indicator& userApplType,
                       const UserApplCode& userAppl,
                       const Indicator& pseudoCityType,
                       const PseudoCityCode& pseudoCity,
                       const TJRGroup& tjrGroup,
                       DeleteList& deleteList)
{
  FareDisplayPrefDAO& dao = FareDisplayPrefDAO::instance();
  return dao.get(deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
}

const std::vector<FareDisplayPref*>&
FareDisplayPrefDAO::get(DeleteList& del,
                        const Indicator& userApplType,
                        const UserApplCode& userAppl,
                        const Indicator& pseudoCityType,
                        const PseudoCityCode& pseudoCity,
                        const TJRGroup& tjrGroup)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDisplayPrefKey key(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDisplayPrefKey
FareDisplayPrefDAO::createKey(FareDisplayPref* info)
{
  return FareDisplayPrefKey(info->userApplType(),
                            info->userAppl(),
                            info->pseudoCityType(),
                            info->pseudoCity(),
                            info->ssgGroupNo());
}

std::vector<FareDisplayPref*>*
FareDisplayPrefDAO::create(FareDisplayPrefKey key)
{
  std::vector<FareDisplayPref*>* ret = new std::vector<FareDisplayPref*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDisplayPref dp(dbAdapter->getAdapter());
    dp.findFareDisplayPref(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDisplayPrefDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDisplayPrefDAO::destroy(FareDisplayPrefKey key, std::vector<FareDisplayPref*>* recs)
{
  destroyContainer(recs);
}

void
FareDisplayPrefDAO::load()
{
  StartupLoader<QueryGetAllFareDisplayPref, FareDisplayPref, FareDisplayPrefDAO>();
}

std::string
FareDisplayPrefDAO::_name("FareDisplayPref");
std::string
FareDisplayPrefDAO::_cacheClass("FareDisplay");

DAOHelper<FareDisplayPrefDAO>
FareDisplayPrefDAO::_helper(_name);

FareDisplayPrefDAO* FareDisplayPrefDAO::_instance = nullptr;

} // namespace tse
