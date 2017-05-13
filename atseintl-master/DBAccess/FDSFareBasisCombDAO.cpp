//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FDSFareBasisCombDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FDSFareBasisComb.h"
#include "DBAccess/Queries/QueryGetFDSFareBasisComb.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FDSFareBasisCombDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDSFareBasisCombDAO"));

FDSFareBasisCombDAO&
FDSFareBasisCombDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FDSFareBasisComb*>&
getFDSFareBasisCombData(const Indicator& userApplType,
                        const UserApplCode& userAppl,
                        const Indicator& pseudoCityType,
                        const PseudoCityCode& pseudoCity,
                        const TJRGroup& tjrGroup,
                        const Indicator& fareDisplayType,
                        const Indicator& domIntlAppl,
                        const uint64_t& seqno,
                        DeleteList& deleteList)
{
  FDSFareBasisCombDAO& dao = FDSFareBasisCombDAO::instance();
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

const std::vector<FDSFareBasisComb*>&
FDSFareBasisCombDAO::get(DeleteList& del,
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

  FDSFareBasisCombKey key(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FDSFareBasisComb*>* ret = new std::vector<FDSFareBasisComb*>;
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

FDSFareBasisCombKey
FDSFareBasisCombDAO::createKey(FDSFareBasisComb* info)
{
  return FDSFareBasisCombKey(info->userApplType(),
                             info->userAppl(),
                             info->pseudoCityType(),
                             info->pseudoCity(),
                             info->ssgGroupNo());
}

std::vector<FDSFareBasisComb*>*
FDSFareBasisCombDAO::create(FDSFareBasisCombKey key)
{
  std::vector<FDSFareBasisComb*>* ret = new std::vector<FDSFareBasisComb*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFDSFareBasisComb fbc(dbAdapter->getAdapter());
    fbc.findFDSFareBasisComb(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSFareBasisCombDAO::create");
    throw;
  }

  return ret;
}

void
FDSFareBasisCombDAO::destroy(FDSFareBasisCombKey key, std::vector<FDSFareBasisComb*>* recs)
{
  destroyContainer(recs);
}

void
FDSFareBasisCombDAO::load()
{
  StartupLoader<QueryGetAllFDSFareBasisComb, FDSFareBasisComb, FDSFareBasisCombDAO>();
}

std::string
FDSFareBasisCombDAO::_name("FDSFareBasisComb");
std::string
FDSFareBasisCombDAO::_cacheClass("FareDisplay");

DAOHelper<FDSFareBasisCombDAO>
FDSFareBasisCombDAO::_helper(_name);

FDSFareBasisCombDAO* FDSFareBasisCombDAO::_instance = nullptr;

} // namespace tse
