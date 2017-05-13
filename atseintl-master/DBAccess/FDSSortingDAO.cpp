//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FDSSortingDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FDSSorting.h"
#include "DBAccess/Queries/QueryGetFDSSorting.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FDSSortingDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDSSortingDAO"));

FDSSortingDAO&
FDSSortingDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FDSSorting*>&
getFDSSortingData(const Indicator& userApplType,
                  const UserApplCode& userAppl,
                  const Indicator& pseudoCityType,
                  const PseudoCityCode& pseudoCity,
                  const TJRGroup& tjrGroup,
                  const Indicator& fareDisplayType,
                  const Indicator& domIntlAppl,
                  const uint64_t& seqno,
                  DeleteList& deleteList)
{
  FDSSortingDAO& dao = FDSSortingDAO::instance();
  return dao.get(deleteList,
                 userApplType,
                 userAppl,
                 pseudoCityType,
                 pseudoCity,
                 tjrGroup,
                 fareDisplayType,
                 domIntlAppl,
                 seqno);
}

const std::vector<FDSSorting*>&
FDSSortingDAO::get(DeleteList& del,
                   const Indicator& userApplType,
                   const UserApplCode& userAppl,
                   const Indicator& pseudoCityType,
                   const PseudoCityCode& pseudoCity,
                   const TJRGroup& tjrGroup,
                   const Indicator& fareDisplayType,
                   const Indicator& domIntlAppl,
                   const uint64_t& seqno)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDSSortingKey key(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FDSSorting*>* ret = new std::vector<FDSSorting*>;
  del.adopt(ret);
  DAOCache::value_type::iterator i = ptr->begin();
  for (; i != ptr->end(); ++i)
  {
    if ((*i)->fareDisplayType() == fareDisplayType && (*i)->domIntlAppl() == domIntlAppl &&
        (*i)->seqno() == seqno)
    {
      ret->push_back(*i);
    }
  }

  return *ret;
}

FDSSortingKey
FDSSortingDAO::createKey(FDSSorting* info)
{
  return FDSSortingKey(info->userApplType(),
                       info->userAppl(),
                       info->pseudoCityType(),
                       info->pseudoCity(),
                       info->ssgGroupNo());
}

std::vector<FDSSorting*>*
FDSSortingDAO::create(FDSSortingKey key)
{
  std::vector<FDSSorting*>* ret = new std::vector<FDSSorting*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFDSSorting srt(dbAdapter->getAdapter());
    srt.findFDSSorting(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSSortingDAO::create");
    throw;
  }

  return ret;
}

void
FDSSortingDAO::destroy(FDSSortingKey key, std::vector<FDSSorting*>* recs)
{
  destroyContainer(recs);
}

void
FDSSortingDAO::load()
{
  StartupLoader<QueryGetAllFDSSorting, FDSSorting, FDSSortingDAO>();
}

std::string
FDSSortingDAO::_name("FDSSorting");
std::string
FDSSortingDAO::_cacheClass("FareDisplay");

DAOHelper<FDSSortingDAO>
FDSSortingDAO::_helper(_name);

FDSSortingDAO* FDSSortingDAO::_instance = nullptr;

} // namespace tse
