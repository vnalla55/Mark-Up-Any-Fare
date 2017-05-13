//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDisplayPrefSegDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDisplayPrefSeg.h"
#include "DBAccess/Queries/QueryGetFareDisplayPrefSeg.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDisplayPrefSegDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDisplayPrefSegDAO"));

FareDisplayPrefSegDAO&
FareDisplayPrefSegDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDisplayPrefSeg*>&
getFareDisplayPrefSegData(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const TJRGroup& tjrGroup,
                          DeleteList& deleteList)
{
  FareDisplayPrefSegDAO& dao = FareDisplayPrefSegDAO::instance();
  return dao.get(deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
}

const std::vector<FareDisplayPrefSeg*>&
FareDisplayPrefSegDAO::get(DeleteList& del,
                           const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pseudoCityType,
                           const PseudoCityCode& pseudoCity,
                           const TJRGroup& tjrGroup)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDisplayPrefSegKey key(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDisplayPrefSegKey
FareDisplayPrefSegDAO::createKey(FareDisplayPrefSeg* info)
{
  return FareDisplayPrefSegKey(info->userApplType(),
                               info->userAppl(),
                               info->pseudoCityType(),
                               info->pseudoCity(),
                               info->ssgGroupNo());
}

std::vector<FareDisplayPrefSeg*>*
FareDisplayPrefSegDAO::create(FareDisplayPrefSegKey key)
{
  std::vector<FareDisplayPrefSeg*>* ret = new std::vector<FareDisplayPrefSeg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDisplayPrefSeg dps(dbAdapter->getAdapter());
    dps.findFareDisplayPrefSeg(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDisplayPrefSegDAO::create");
    throw;
  }

  return ret;
}

void
FareDisplayPrefSegDAO::destroy(FareDisplayPrefSegKey key, std::vector<FareDisplayPrefSeg*>* recs)
{
  destroyContainer(recs);
}

void
FareDisplayPrefSegDAO::load()
{
  StartupLoader<QueryGetAllFareDisplayPrefSeg, FareDisplayPrefSeg, FareDisplayPrefSegDAO>();
}

std::string
FareDisplayPrefSegDAO::_name("FareDisplayPrefSeg");
std::string
FareDisplayPrefSegDAO::_cacheClass("FareDisplay");

DAOHelper<FareDisplayPrefSegDAO>
FareDisplayPrefSegDAO::_helper(_name);

FareDisplayPrefSegDAO* FareDisplayPrefSegDAO::_instance = nullptr;

} // namespace tse
