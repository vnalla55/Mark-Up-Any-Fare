//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SvcFeesCxrResultingFCLDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetSvcFeesCxrResultingFCL.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesCxrResultingFCLDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesCxrResultingFCLDAO"));

SvcFeesCxrResultingFCLDAO&
SvcFeesCxrResultingFCLDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesCxrResultingFCLInfo*>&
getSvcFeesCxrResultingFCLData(const VendorCode& vendor,
                              int itemNo,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SvcFeesCxrResultingFCLHistoricalDAO& dao = SvcFeesCxrResultingFCLHistoricalDAO::instance();

    const std::vector<SvcFeesCxrResultingFCLInfo*>& ret =
        dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
  else
  {
    SvcFeesCxrResultingFCLDAO& dao = SvcFeesCxrResultingFCLDAO::instance();

    const std::vector<SvcFeesCxrResultingFCLInfo*>& ret =
        dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
}

const std::vector<SvcFeesCxrResultingFCLInfo*>&
SvcFeesCxrResultingFCLDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesCxrResultingFCLKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesCxrResultingFCLInfo*>* ret = new std::vector<SvcFeesCxrResultingFCLInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentG<SvcFeesCxrResultingFCLInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesCxrResultingFCLInfo*>*
SvcFeesCxrResultingFCLDAO::create(SvcFeesCxrResultingFCLKey key)
{
  std::vector<SvcFeesCxrResultingFCLInfo*>* ret = new std::vector<SvcFeesCxrResultingFCLInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesCxrResultingFCL qsfac(dbAdapter->getAdapter());
    qsfac.findSvcFeesCxrResultingFCLInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesCxrResultingFCLDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesCxrResultingFCLDAO::destroy(SvcFeesCxrResultingFCLKey key,
                                   std::vector<SvcFeesCxrResultingFCLInfo*>* recs)
{
  destroyContainer(recs);
}

SvcFeesCxrResultingFCLKey
SvcFeesCxrResultingFCLDAO::createKey(SvcFeesCxrResultingFCLInfo* info)
{
  return SvcFeesCxrResultingFCLKey(info->vendor(), info->itemNo());
}

void
SvcFeesCxrResultingFCLDAO::load()
{
  // StartupLoader<QueryGetSvcFeesCxrResultingFCL,SvcFeesCxrResultingFCLInfo,SvcFeesCxrResultingFCLDAO>()
  // ;
  // Not pre_loading
  StartupLoaderNoDB<SvcFeesCxrResultingFCLInfo, SvcFeesCxrResultingFCLDAO>();
}

sfc::CompressedData*
SvcFeesCxrResultingFCLDAO::compress(const std::vector<SvcFeesCxrResultingFCLInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SvcFeesCxrResultingFCLInfo*>*
SvcFeesCxrResultingFCLDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SvcFeesCxrResultingFCLInfo>(compressed);
}

std::string
SvcFeesCxrResultingFCLDAO::_name("SvcFeesCxrResultingFCL");
std::string
SvcFeesCxrResultingFCLDAO::_cacheClass("Rules");
DAOHelper<SvcFeesCxrResultingFCLDAO>
SvcFeesCxrResultingFCLDAO::_helper(_name);
SvcFeesCxrResultingFCLDAO* SvcFeesCxrResultingFCLDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
SvcFeesCxrResultingFCLHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesCxrResultingFCLHistDAO"));
SvcFeesCxrResultingFCLHistoricalDAO&
SvcFeesCxrResultingFCLHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesCxrResultingFCLInfo*>&
SvcFeesCxrResultingFCLHistoricalDAO::get(DeleteList& del,
                                         const VendorCode& vendor,
                                         int itemNo,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesCxrResultingFCLHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesCxrResultingFCLInfo*>* ret = new std::vector<SvcFeesCxrResultingFCLInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<SvcFeesCxrResultingFCLInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesCxrResultingFCLInfo*>*
SvcFeesCxrResultingFCLHistoricalDAO::create(SvcFeesCxrResultingFCLHistoricalKey key)
{
  std::vector<SvcFeesCxrResultingFCLInfo*>* ret = new std::vector<SvcFeesCxrResultingFCLInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesCxrResultingFCLHistorical fnc(dbAdapter->getAdapter());
    fnc.findSvcFeesCxrResultingFCLInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesCxrResultingFCLHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesCxrResultingFCLHistoricalDAO::destroy(SvcFeesCxrResultingFCLHistoricalKey key,
                                             std::vector<SvcFeesCxrResultingFCLInfo*>* recs)
{
  std::vector<SvcFeesCxrResultingFCLInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SvcFeesCxrResultingFCLHistoricalDAO::compress(const std::vector<SvcFeesCxrResultingFCLInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SvcFeesCxrResultingFCLInfo*>*
SvcFeesCxrResultingFCLHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SvcFeesCxrResultingFCLInfo>(compressed);
}

std::string
SvcFeesCxrResultingFCLHistoricalDAO::_name("SvcFeesCxrResultingFCLHistorical");
std::string
SvcFeesCxrResultingFCLHistoricalDAO::_cacheClass("Rules");
DAOHelper<SvcFeesCxrResultingFCLHistoricalDAO>
SvcFeesCxrResultingFCLHistoricalDAO::_helper(_name);
SvcFeesCxrResultingFCLHistoricalDAO* SvcFeesCxrResultingFCLHistoricalDAO::_instance = nullptr;

} // namespace tse
