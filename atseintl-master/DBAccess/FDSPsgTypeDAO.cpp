//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FDSPsgTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FDSPsgType.h"
#include "DBAccess/Queries/QueryGetFDSPsgType.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FDSPsgTypeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDSPsgTypeDAO"));

FDSPsgTypeDAO&
FDSPsgTypeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FDSPsgType*>&
getFDSPsgTypeData(const Indicator& userApplType,
                  const UserApplCode& userAppl,
                  const Indicator& pseudoCityType,
                  const PseudoCityCode& pseudoCity,
                  const TJRGroup& tjrGroup,
                  const Indicator& fareDisplayType,
                  const Indicator& domIntlAppl,
                  const uint64_t& seqno,
                  DeleteList& deleteList)
{
  FDSPsgTypeDAO& dao = FDSPsgTypeDAO::instance();
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

const std::vector<FDSPsgType*>&
FDSPsgTypeDAO::get(DeleteList& del,
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

  FDSPsgTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FDSPsgType*>* ret = new std::vector<FDSPsgType*>;
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

FDSPsgTypeKey
FDSPsgTypeDAO::createKey(FDSPsgType* info)
{
  return FDSPsgTypeKey(info->userApplType(),
                       info->userAppl(),
                       info->pseudoCityType(),
                       info->pseudoCity(),
                       info->ssgGroupNo());
}

std::vector<FDSPsgType*>*
FDSPsgTypeDAO::create(FDSPsgTypeKey key)
{
  std::vector<FDSPsgType*>* ret = new std::vector<FDSPsgType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFDSPsgType pt(dbAdapter->getAdapter());
    pt.findFDSPsgType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSPsgTypeDAO::create");
    throw;
  }

  return ret;
}

void
FDSPsgTypeDAO::destroy(FDSPsgTypeKey key, std::vector<FDSPsgType*>* recs)
{
  destroyContainer(recs);
}

void
FDSPsgTypeDAO::load()
{
  StartupLoader<QueryGetAllFDSPsgType, FDSPsgType, FDSPsgTypeDAO>();
}

std::string
FDSPsgTypeDAO::_name("FDSPsgType");
std::string
FDSPsgTypeDAO::_cacheClass("FareDisplay");

DAOHelper<FDSPsgTypeDAO>
FDSPsgTypeDAO::_helper(_name);

FDSPsgTypeDAO* FDSPsgTypeDAO::_instance = nullptr;

} // namespace tse
