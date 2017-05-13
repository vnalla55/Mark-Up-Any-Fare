//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FDSGlobalDirDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FDSGlobalDir.h"
#include "DBAccess/Queries/QueryGetFDSGlobalDir.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FDSGlobalDirDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDSGlobalDirDAO"));

FDSGlobalDirDAO&
FDSGlobalDirDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FDSGlobalDir*>&
getFDSGlobalDirData(const Indicator& userApplType,
                    const UserApplCode& userAppl,
                    const Indicator& pseudoCityType,
                    const PseudoCityCode& pseudoCity,
                    const TJRGroup& tjrGroup,
                    const Indicator& fareDisplayType,
                    const Indicator& domIntlAppl,
                    const DateTime& versionDate,
                    const uint64_t& seqno,
                    const DateTime& createDate,
                    DeleteList& deleteList)
{
  FDSGlobalDirDAO& dao = FDSGlobalDirDAO::instance();
  return dao.get(deleteList,
                 userApplType,
                 userAppl,
                 pseudoCityType,
                 pseudoCity,
                 tjrGroup,
                 fareDisplayType,
                 domIntlAppl,
                 versionDate,
                 seqno,
                 createDate);
}

const std::vector<FDSGlobalDir*>&
FDSGlobalDirDAO::get(DeleteList& del,
                     const Indicator& userApplType,
                     const UserApplCode& userAppl,
                     const Indicator& pseudoCityType,
                     const PseudoCityCode& pseudoCity,
                     const TJRGroup& tjrGroup,
                     const Indicator& fareDisplayType,
                     const Indicator& domIntlAppl,
                     const DateTime& versionDate,
                     const uint64_t& seqno,
                     const DateTime& createDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDSGlobalDirKey key(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FDSGlobalDir*>* ret = new std::vector<FDSGlobalDir*>;
  del.adopt(ret);
  DAOCache::value_type::iterator i = ptr->begin();
  for (; i != ptr->end(); ++i)
  {
    if ((*i)->fareDisplayType() == fareDisplayType && (*i)->domIntlAppl() == domIntlAppl &&
        (*i)->versionDate() == versionDate && (*i)->seqno() == seqno &&
        (*i)->createDate() == createDate)
    {
      ret->push_back(*i);
    }
  }

  return *ret;
}

FDSGlobalDirKey
FDSGlobalDirDAO::createKey(FDSGlobalDir* info)
{
  return FDSGlobalDirKey(info->userApplType(),
                         info->userAppl(),
                         info->pseudoCityType(),
                         info->pseudoCity(),
                         info->ssgGroupNo());
}

std::vector<FDSGlobalDir*>*
FDSGlobalDirDAO::create(FDSGlobalDirKey key)
{
  std::vector<FDSGlobalDir*>* ret = new std::vector<FDSGlobalDir*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFDSGlobalDir gd(dbAdapter->getAdapter());
    gd.findFDSGlobalDir(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSGlobalDirDAO::create");
    throw;
  }

  return ret;
}

void
FDSGlobalDirDAO::destroy(FDSGlobalDirKey key, std::vector<FDSGlobalDir*>* recs)
{
  destroyContainer(recs);
}

void
FDSGlobalDirDAO::load()
{
  StartupLoader<QueryGetAllFDSGlobalDir, FDSGlobalDir, FDSGlobalDirDAO>();
}

std::string
FDSGlobalDirDAO::_name("FDSGlobalDir");
std::string
FDSGlobalDirDAO::_cacheClass("FareDisplay");

DAOHelper<FDSGlobalDirDAO>
FDSGlobalDirDAO::_helper(_name);

FDSGlobalDirDAO* FDSGlobalDirDAO::_instance = nullptr;

} // namespace tse
