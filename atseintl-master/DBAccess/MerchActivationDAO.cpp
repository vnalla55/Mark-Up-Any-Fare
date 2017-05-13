//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MerchActivationDAO.h"

#include "DBAccess/MerchActivationInfo.h"
#include "DBAccess/Queries/QueryGetMerchActivation.h"

namespace tse
{
log4cxx::LoggerPtr
MerchActivationDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MerchActivationDAO"));

std::string
MerchActivationDAO::_name("MerchActivation");
std::string
MerchActivationDAO::_cacheClass("Rules");
DAOHelper<MerchActivationDAO>
MerchActivationDAO::_helper(_name);
MerchActivationDAO* MerchActivationDAO::_instance = nullptr;

MerchActivationDAO&
MerchActivationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MerchActivationInfo*>&
getMerchActivationData(uint64_t productId,
                       const CarrierCode& carrier,
                       const PseudoCityCode& pseudoCity,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  PseudoCityCode pcc(pseudoCity);
  if (carrier != ANY_CARRIER)
    pcc = EMPTY_STRING();
  const DateTime& dt = (date == DateTime::emptyDate()) ? ticketDate : date;

  if (isHistorical)
  {
    MerchActivationHistoricalDAO& dao = MerchActivationHistoricalDAO::instance();
    return dao.get(deleteList, productId, carrier, pcc, dt, ticketDate);
  }
  else
  {
    MerchActivationDAO& dao = MerchActivationDAO::instance();
    return dao.get(deleteList, productId, carrier, pcc, dt, ticketDate);
  }
}

const std::vector<MerchActivationInfo*>&
MerchActivationDAO::get(DeleteList& del,
                        uint64_t productId,
                        const CarrierCode& carrier,
                        const PseudoCityCode& pseudoCity,
                        const DateTime& date,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MerchActivationKey key(productId, carrier, pseudoCity);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<MerchActivationInfo*>* ret = new std::vector<MerchActivationInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<MerchActivationInfo>(date, ticketDate));

  return *ret;
}

std::vector<MerchActivationInfo*>*
MerchActivationDAO::create(MerchActivationKey key)
{
  std::vector<MerchActivationInfo*>* ret = new std::vector<MerchActivationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMerchActivation fnc(dbAdapter->getAdapter());
    fnc.findMerchActivationInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MerchActivationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MerchActivationDAO::destroy(MerchActivationKey key, std::vector<MerchActivationInfo*>* recs)
{
  destroyContainer(recs);
}

MerchActivationKey
MerchActivationDAO::createKey(MerchActivationInfo* info)
{
  return MerchActivationKey(info->productId(), info->carrier(), info->pseudoCity());
}

void
MerchActivationDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<MerchActivationInfo, MerchActivationDAO>();
}

sfc::CompressedData*
MerchActivationDAO::compress(const std::vector<MerchActivationInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<MerchActivationInfo*>*
MerchActivationDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MerchActivationInfo>(compressed);
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
MerchActivationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MerchActivationHistoricalDAO"));

std::string
MerchActivationHistoricalDAO::_name("MerchActivationHistorical");
std::string
MerchActivationHistoricalDAO::_cacheClass("Rules");
DAOHelper<MerchActivationHistoricalDAO>
MerchActivationHistoricalDAO::_helper(_name);
MerchActivationHistoricalDAO* MerchActivationHistoricalDAO::_instance = nullptr;

MerchActivationHistoricalDAO&
MerchActivationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MerchActivationInfo*>&
MerchActivationHistoricalDAO::get(DeleteList& del,
                                  uint64_t productId,
                                  const CarrierCode& carrier,
                                  const PseudoCityCode& pseudoCity,
                                  const DateTime& date,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MerchActivationHistoricalKey key(productId, carrier, pseudoCity);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<MerchActivationInfo*>* ret = new std::vector<MerchActivationInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<MerchActivationInfo>(date, ticketDate));

  return *ret;
}

std::vector<MerchActivationInfo*>*
MerchActivationHistoricalDAO::create(MerchActivationHistoricalKey key)
{
  std::vector<MerchActivationInfo*>* ret = new std::vector<MerchActivationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMerchActivationHistorical fnc(dbAdapter->getAdapter());
    fnc.findMerchActivationInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MerchActivationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MerchActivationHistoricalDAO::destroy(MerchActivationHistoricalKey key,
                                      std::vector<MerchActivationInfo*>* recs)
{
  std::vector<MerchActivationInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MerchActivationHistoricalDAO::compress(const std::vector<MerchActivationInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<MerchActivationInfo*>*
MerchActivationHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MerchActivationInfo>(compressed);
}
} // tse
