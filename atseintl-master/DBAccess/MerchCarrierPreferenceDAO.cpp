//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MerchCarrierPreferenceDAO.h"

#include "DBAccess/MerchCarrierPreferenceInfo.h"
#include "DBAccess/Queries/QueryGetMerchCarrierPreference.h"

namespace tse
{
log4cxx::LoggerPtr
MerchCarrierPreferenceDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MerchCarrierPreferenceDAO"));

std::string
MerchCarrierPreferenceDAO::_name("MerchCarrierPreference");
std::string
MerchCarrierPreferenceDAO::_cacheClass("Rules");
DAOHelper<MerchCarrierPreferenceDAO>
MerchCarrierPreferenceDAO::_helper(_name);
MerchCarrierPreferenceDAO* MerchCarrierPreferenceDAO::_instance = nullptr;

MerchCarrierPreferenceDAO&
MerchCarrierPreferenceDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MerchCarrierPreferenceInfo*
getMerchCarrierPreferenceData(const CarrierCode& carrier,
                              const ServiceGroup& groupCode,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  ServiceGroup sg(groupCode);

  if (isHistorical)
  {
    MerchCarrierPreferenceHistoricalDAO& dao = MerchCarrierPreferenceHistoricalDAO::instance();
    return dao.get(deleteList, carrier, sg, ticketDate);
  }
  else
  {
    MerchCarrierPreferenceDAO& dao = MerchCarrierPreferenceDAO::instance();
    return dao.get(deleteList, carrier, sg, ticketDate);
  }
}

const MerchCarrierPreferenceInfo*
MerchCarrierPreferenceDAO::get(DeleteList& del,
                               const CarrierCode& carrier,
                               const ServiceGroup& groupCode,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MerchCarrierPreferenceKey key(carrier, groupCode);
  DAOCache::pointer_type ptr = cache().get(key);

  if (ptr->empty())
    return nullptr;

  del.copy(ptr);
  IsEffectiveG<MerchCarrierPreferenceInfo> isEffective(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isEffective(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<MerchCarrierPreferenceInfo*>*
MerchCarrierPreferenceDAO::create(MerchCarrierPreferenceKey key)
{
  std::vector<MerchCarrierPreferenceInfo*>* ret = new std::vector<MerchCarrierPreferenceInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMerchCarrierPreference fnc(dbAdapter->getAdapter());
    fnc.findMerchCarrierPreferenceInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MerchCarrierPreferenceDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MerchCarrierPreferenceDAO::destroy(MerchCarrierPreferenceKey key,
                                   std::vector<MerchCarrierPreferenceInfo*>* recs)
{
  destroyContainer(recs);
}

MerchCarrierPreferenceKey
MerchCarrierPreferenceDAO::createKey(MerchCarrierPreferenceInfo* info)
{
  return MerchCarrierPreferenceKey(info->carrier(), info->groupCode());
}

void
MerchCarrierPreferenceDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<MerchCarrierPreferenceInfo, MerchCarrierPreferenceDAO>();
}

sfc::CompressedData*
MerchCarrierPreferenceDAO::compress(const std::vector<MerchCarrierPreferenceInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<MerchCarrierPreferenceInfo*>*
MerchCarrierPreferenceDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MerchCarrierPreferenceInfo>(compressed);
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
MerchCarrierPreferenceHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MerchCarrierPreferenceHistoricalDAO"));

std::string
MerchCarrierPreferenceHistoricalDAO::_name("MerchCarrierPreferenceHistorical");
std::string
MerchCarrierPreferenceHistoricalDAO::_cacheClass("Rules");
DAOHelper<MerchCarrierPreferenceHistoricalDAO>
MerchCarrierPreferenceHistoricalDAO::_helper(_name);
MerchCarrierPreferenceHistoricalDAO* MerchCarrierPreferenceHistoricalDAO::_instance = nullptr;

MerchCarrierPreferenceHistoricalDAO&
MerchCarrierPreferenceHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MerchCarrierPreferenceInfo*
MerchCarrierPreferenceHistoricalDAO::get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const ServiceGroup& groupCode,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MerchCarrierPreferenceHistoricalKey key(carrier, groupCode);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  if (ptr->empty())
    return nullptr;

  del.copy(ptr);
  IsEffectiveH<MerchCarrierPreferenceInfo> isEffective(ticketDate, ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isEffective(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<MerchCarrierPreferenceInfo*>*
MerchCarrierPreferenceHistoricalDAO::create(MerchCarrierPreferenceHistoricalKey key)
{
  std::vector<MerchCarrierPreferenceInfo*>* ret = new std::vector<MerchCarrierPreferenceInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMerchCarrierPreferenceHistorical fnc(dbAdapter->getAdapter());
    fnc.findMerchCarrierPreferenceInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MerchCarrierPreferenceHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MerchCarrierPreferenceHistoricalDAO::destroy(MerchCarrierPreferenceHistoricalKey key,
                                             std::vector<MerchCarrierPreferenceInfo*>* recs)
{
  std::vector<MerchCarrierPreferenceInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MerchCarrierPreferenceHistoricalDAO::compress(const std::vector<MerchCarrierPreferenceInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<MerchCarrierPreferenceInfo*>*
MerchCarrierPreferenceHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MerchCarrierPreferenceInfo>(compressed);
}

} // tse
