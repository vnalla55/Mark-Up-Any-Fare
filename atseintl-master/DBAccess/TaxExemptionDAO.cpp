// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DBAccess/TaxExemptionDAO.h"
#include "DBAccess/TaxExemption.h"
#include "DBAccess/Queries/QueryGetTaxExemption.h"

namespace tse
{

log4cxx::LoggerPtr
TaxExemptionDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxExemptionDAO"));

log4cxx::LoggerPtr
TaxExemptionHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxExemptionHistoricalDAO"));

std::string
TaxExemptionDAO::_name("TaxExemption");
std::string
TaxExemptionDAO::_cacheClass("Taxes");
DAOHelper<TaxExemptionDAO>
TaxExemptionDAO::_helper(_name);
TaxExemptionDAO* TaxExemptionDAO::_instance = nullptr;

std::string
TaxExemptionHistoricalDAO::_name("TaxExemptionHistorical");
std::string
TaxExemptionHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxExemptionHistoricalDAO>
TaxExemptionHistoricalDAO::_helper(_name);
TaxExemptionHistoricalDAO* TaxExemptionHistoricalDAO::_instance = nullptr;

const std::vector<TaxExemption*>&
getTaxExemptionData(const TaxCode& taxCode,
                    const PseudoCityCode& channelId,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  TaxExemptionKey key(taxCode, channelId);

  if (isHistorical)
    return TaxExemptionHistoricalDAO::instance().get(deleteList, key, date, ticketDate);
  else
    return TaxExemptionDAO::instance().get(deleteList, key, date, ticketDate);
}

TaxExemptionDAO&
TaxExemptionDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

TaxExemptionHistoricalDAO&
TaxExemptionHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const std::vector<TaxExemption*>&
TaxExemptionDAO::get(DeleteList& del,
                   const TaxExemptionKey& key,
                   const DateTime& date,
                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, IsNotEffectiveG<TaxExemption>(date, ticketDate)));
}

const std::vector<TaxExemption*>&
TaxExemptionHistoricalDAO::get(DeleteList& del,
                             const TaxExemptionKey& key,
                             const DateTime& date,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TaxExemption*>* ret = new std::vector<TaxExemption*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<TaxExemption>(date, ticketDate));
  return *ret;
}

void
TaxExemptionDAO::destroy(TaxExemptionKey key, std::vector<TaxExemption*>* recs)
{
  destroyContainer (recs);
}

void
TaxExemptionHistoricalDAO::destroy(TaxExemptionKey key, std::vector<TaxExemption*>* recs)
{
  destroyContainer (recs);
}

sfc::CompressedData*
TaxExemptionDAO::compress(const std::vector<TaxExemption*>* vect) const
{
  return compressVector(vect);
}

sfc::CompressedData*
TaxExemptionHistoricalDAO::compress(const std::vector<TaxExemption*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxExemption*>*
TaxExemptionDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxExemption>(compressed);
}

std::vector<TaxExemption*>*
TaxExemptionHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxExemption>(compressed);
}

std::vector<TaxExemption*>*
TaxExemptionDAO::create(TaxExemptionKey key)
{
  std::vector<TaxExemption*>* ret = new std::vector<TaxExemption*>;

  DBAdapterPool::pointer_type dbAdapter = DBAdapterPool::instance().get(this->cacheClass());
  try
  {
    QueryGetTaxExemption tc(dbAdapter->getAdapter());
    tc.findTaxExemption(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxExemptionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

std::vector<TaxExemption*>*
TaxExemptionHistoricalDAO::create(TaxExemptionKey key)
{
  std::vector<TaxExemption*>* ret = new std::vector<TaxExemption*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxExemptionHistorical tc(dbAdapter->getAdapter());
    tc.findTaxExemption(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxExemptionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

}
