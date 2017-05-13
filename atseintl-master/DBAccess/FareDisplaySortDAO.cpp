//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDisplaySortDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDisplaySort.h"
#include "DBAccess/Queries/QueryGetFareDisplaySort.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDisplaySortDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareDisplaySortDAO"));

FareDisplaySortDAO&
FareDisplaySortDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDisplaySort*>&
getFareDisplaySortData(const Indicator& userApplType,
                       const UserApplCode& userAppl,
                       const Indicator& pseudoCityType,
                       const PseudoCityCode& pseudoCity,
                       const TJRGroup& tjrGroup,
                       const DateTime& travelDate,
                       DeleteList& deleteList)
{
  FareDisplaySortDAO& dao = FareDisplaySortDAO::instance();
  return dao.get(deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
}

const std::vector<FareDisplaySort*>&
FareDisplaySortDAO::get(DeleteList& del,
                        const Indicator& userApplType,
                        const UserApplCode& userAppl,
                        const Indicator& pseudoCityType,
                        const PseudoCityCode& pseudoCity,
                        const TJRGroup& tjrGroup)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDisplaySortKey key(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareDisplaySort*>* ret = new std::vector<FareDisplaySort*>;
  DAOCache::value_type::iterator i = ptr->begin();

  while (ptr->end() !=
         (i = find_if(i, ptr->end(), IsEffective<FareDisplaySort>(DateTime::localTime()))))
  {
    ret->push_back(*i++);
  }

  del.adopt(ret);
  return *ret;
}

FareDisplaySortKey
FareDisplaySortDAO::createKey(FareDisplaySort* info)
{
  return FareDisplaySortKey(info->userApplType(),
                            info->userAppl(),
                            info->pseudoCityType(),
                            info->pseudoCity(),
                            info->ssgGroupNo());
}

std::vector<FareDisplaySort*>*
FareDisplaySortDAO::create(FareDisplaySortKey key)
{
  std::vector<FareDisplaySort*>* ret = new std::vector<FareDisplaySort*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDisplaySort ds(dbAdapter->getAdapter());
    ds.findFareDisplaySort(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDisplaySortDAO::create");
    throw;
  }

  return ret;
}

void
FareDisplaySortDAO::destroy(FareDisplaySortKey key, std::vector<FareDisplaySort*>* recs)
{
  destroyContainer(recs);
}

void
FareDisplaySortDAO::load()
{
  StartupLoader<QueryGetAllFareDisplaySort, FareDisplaySort, FareDisplaySortDAO>();
}

std::string
FareDisplaySortDAO::_name("FareDisplaySort");
std::string
FareDisplaySortDAO::_cacheClass("FareDisplay");

DAOHelper<FareDisplaySortDAO>
FareDisplaySortDAO::_helper(_name);

FareDisplaySortDAO* FareDisplaySortDAO::_instance = nullptr;

} // namespace tse
