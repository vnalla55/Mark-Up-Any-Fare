//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SvcFeesTicketDesignatorDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetSvcFeesTicketDesignator.h"
#include "DBAccess/SvcFeesTktDesignatorInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesTicketDesignatorDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesTicketDesignatorDAO"));

SvcFeesTicketDesignatorDAO&
SvcFeesTicketDesignatorDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesTktDesignatorInfo*>&
getSvcFeesTktDesignatorData(const VendorCode& vendor,
                            int itemNo,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical)
{
  if (isHistorical)
  {
    SvcFeesTicketDesignatorHistoricalDAO& dao = SvcFeesTicketDesignatorHistoricalDAO::instance();

    const std::vector<SvcFeesTktDesignatorInfo*>& ret =
        dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
  else
  {
    SvcFeesTicketDesignatorDAO& dao = SvcFeesTicketDesignatorDAO::instance();

    const std::vector<SvcFeesTktDesignatorInfo*>& ret =
        dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
}

const std::vector<SvcFeesTktDesignatorInfo*>&
SvcFeesTicketDesignatorDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                int itemNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesTicketDesignatorKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesTktDesignatorInfo*>* ret = new std::vector<SvcFeesTktDesignatorInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentG<SvcFeesTktDesignatorInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesTktDesignatorInfo*>*
SvcFeesTicketDesignatorDAO::create(SvcFeesTicketDesignatorKey key)
{
  std::vector<SvcFeesTktDesignatorInfo*>* ret = new std::vector<SvcFeesTktDesignatorInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesTicketDesignator ftd(dbAdapter->getAdapter());
    ftd.findSvcFeesTktDesignatorInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesTicketDesignatorDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesTicketDesignatorDAO::destroy(SvcFeesTicketDesignatorKey key,
                                    std::vector<SvcFeesTktDesignatorInfo*>* recs)
{
  destroyContainer(recs);
}

SvcFeesTicketDesignatorKey
SvcFeesTicketDesignatorDAO::createKey(SvcFeesTktDesignatorInfo* info)
{
  return SvcFeesTicketDesignatorKey(info->vendor(), info->itemNo());
}

void
SvcFeesTicketDesignatorDAO::load()
{
  // not pre_loading

  StartupLoaderNoDB<SvcFeesTktDesignatorInfo, SvcFeesTicketDesignatorDAO>();
}

std::string
SvcFeesTicketDesignatorDAO::_name("SvcFeesTicketDesignator");
std::string
SvcFeesTicketDesignatorDAO::_cacheClass("Rules");
DAOHelper<SvcFeesTicketDesignatorDAO>
SvcFeesTicketDesignatorDAO::_helper(_name);
SvcFeesTicketDesignatorDAO* SvcFeesTicketDesignatorDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
SvcFeesTicketDesignatorHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesTicketDesignatorHistDAO"));
SvcFeesTicketDesignatorHistoricalDAO&
SvcFeesTicketDesignatorHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesTktDesignatorInfo*>&
SvcFeesTicketDesignatorHistoricalDAO::get(DeleteList& del,
                                          const VendorCode& vendor,
                                          int itemNo,
                                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesTicketDesignatorHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesTktDesignatorInfo*>* ret = new std::vector<SvcFeesTktDesignatorInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<SvcFeesTktDesignatorInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesTktDesignatorInfo*>*
SvcFeesTicketDesignatorHistoricalDAO::create(SvcFeesTicketDesignatorHistoricalKey key)
{
  std::vector<SvcFeesTktDesignatorInfo*>* ret = new std::vector<SvcFeesTktDesignatorInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesTicketDesignatorHistorical ftd(dbAdapter->getAdapter());
    ftd.findSvcFeesTktDesignatorInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesTicketDesignatorHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesTicketDesignatorHistoricalDAO::destroy(SvcFeesTicketDesignatorHistoricalKey key,
                                              std::vector<SvcFeesTktDesignatorInfo*>* recs)
{
  std::vector<SvcFeesTktDesignatorInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SvcFeesTicketDesignatorHistoricalDAO::_name("SvcFeesTicketDesignatorHistorical");
std::string
SvcFeesTicketDesignatorHistoricalDAO::_cacheClass("Rules");
DAOHelper<SvcFeesTicketDesignatorHistoricalDAO>
SvcFeesTicketDesignatorHistoricalDAO::_helper(_name);
SvcFeesTicketDesignatorHistoricalDAO* SvcFeesTicketDesignatorHistoricalDAO::_instance = nullptr;

} // namespace tse
