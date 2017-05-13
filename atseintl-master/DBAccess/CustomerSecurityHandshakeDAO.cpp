#include "DBAccess/CustomerSecurityHandshakeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCustomerSecurityHandshake.h"

namespace tse
{
log4cxx::LoggerPtr
CustomerSecurityHandshakeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CustomerSecurityHandshakeDAO"));

CustomerSecurityHandshakeDAO&
CustomerSecurityHandshakeDAO::instance()
{
  if (UNLIKELY(nullptr == _instance))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CustomerSecurityHandshakeInfo*>&
getCustomerSecurityHandshakeData(const PseudoCityCode& pcc,
                                 const Code<8>& productCD,
                                 DeleteList& deleteList,
                                 const DateTime&,
                                 const DateTime& dateTime,
                                 bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CustomerSecurityHandshakeHistoricalDAO& dao(CustomerSecurityHandshakeHistoricalDAO::instance());

    const std::vector<CustomerSecurityHandshakeInfo*>& ret(dao.get(deleteList, pcc, productCD, dateTime));

    return ret;
  }
  else
  {
    CustomerSecurityHandshakeDAO& dao(CustomerSecurityHandshakeDAO::instance());

    const std::vector<CustomerSecurityHandshakeInfo*>& ret(dao.get(deleteList, pcc, productCD, dateTime));

    return ret;
  }
}

const std::vector<CustomerSecurityHandshakeInfo*>&
CustomerSecurityHandshakeDAO::get(DeleteList& del,
                                  const PseudoCityCode& pcc,
                                  const Code<8>& productCD,
                                  const DateTime& dateTime)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  CustomerSecurityHandshakeKey key(pcc, productCD);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotEffectiveT<CustomerSecurityHandshakeInfo> isNotEffective(dateTime);
  return *(applyFilter(del, ptr, isNotEffective));
}

std::vector<CustomerSecurityHandshakeInfo*>*
CustomerSecurityHandshakeDAO::create(CustomerSecurityHandshakeKey key)
{
  std::vector<CustomerSecurityHandshakeInfo*>* ret(new std::vector<CustomerSecurityHandshakeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetCustomerSecurityHandshake q(dbAdapter->getAdapter());
    q.findCustomerSecurityHandshake(*ret, key._a, key._b);
    /*
    std::cerr << "  !!! QueryGetCustomerSecurityHandshake: " << ' ' << ret->size() << " !!" << std::endl;
    std::vector<CustomerSecurityHandshakeInfo*> *all(new std::vector<CustomerSecurityHandshakeInfo*>);// test
    QueryGetAllCustomerSecurityHandshake qall(dbAdapter->getAdapter());// test
    qall.findAllCustomerSecurityHandshake(*all);// test
    std::cerr << "  !! QueryGetAllCustomerSecurityHandshake: " << ' ' << all->size() << " !! << std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CustomerSecurityHandshakeDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CustomerSecurityHandshakeKey
CustomerSecurityHandshakeDAO::createKey(const CustomerSecurityHandshakeInfo* info)
{
  return CustomerSecurityHandshakeKey(info->securitySourcePCC(), info->productCode());
}

void
CustomerSecurityHandshakeDAO::destroy(CustomerSecurityHandshakeKey, std::vector<CustomerSecurityHandshakeInfo*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
CustomerSecurityHandshakeDAO::compress(const std::vector<CustomerSecurityHandshakeInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<CustomerSecurityHandshakeInfo*>*
CustomerSecurityHandshakeDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CustomerSecurityHandshakeInfo>(compressed);
}

std::string
CustomerSecurityHandshakeDAO::_name("CustomerSecurityHandshake");
std::string
CustomerSecurityHandshakeDAO::_cacheClass("Rules");
DAOHelper<CustomerSecurityHandshakeDAO>
CustomerSecurityHandshakeDAO::_helper(_name);
CustomerSecurityHandshakeDAO*
CustomerSecurityHandshakeDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
CustomerSecurityHandshakeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CustomerSecurityHandshakeHistoricalDAO"));

CustomerSecurityHandshakeHistoricalDAO&
CustomerSecurityHandshakeHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CustomerSecurityHandshakeInfo*>&
CustomerSecurityHandshakeHistoricalDAO::get(DeleteList& del,
                                            const PseudoCityCode& pcc,
                                            const Code<8>& productCD,
                                            const DateTime& dateTime)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  CustomerSecurityHandshakeHistoricalKey key(pcc, productCD);
  DAOUtils::getDateRange(dateTime, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<CustomerSecurityHandshakeInfo*>* ret(new std::vector<CustomerSecurityHandshakeInfo*>);
  del.adopt(ret);
  IsNotEffectiveH<CustomerSecurityHandshakeInfo> isNotEffective(key._c, key._d, dateTime);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotEffective);
  return *ret;
}

std::vector<CustomerSecurityHandshakeInfo*>*
CustomerSecurityHandshakeHistoricalDAO::create(CustomerSecurityHandshakeHistoricalKey key)
{
  std::vector<CustomerSecurityHandshakeInfo*>* ret(new std::vector<CustomerSecurityHandshakeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetCustomerSecurityHandshakeHistorical q(dbAdapter->getAdapter());
    q.findCustomerSecurityHandshake(*ret, key._a, key._b, key._c, key._d);
    //std::cerr << "!! QueryGetCustomerSecurityHandshakeHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CustomerSecurityHandshakeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CustomerSecurityHandshakeHistoricalKey
CustomerSecurityHandshakeHistoricalDAO::createKey(const CustomerSecurityHandshakeInfo* info,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  return CustomerSecurityHandshakeHistoricalKey(info->securitySourcePCC(), info->productCode(), startDate, endDate);
}

void
CustomerSecurityHandshakeHistoricalDAO::destroy(CustomerSecurityHandshakeHistoricalKey,
                                    std::vector<CustomerSecurityHandshakeInfo*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
CustomerSecurityHandshakeHistoricalDAO::compress(const std::vector<CustomerSecurityHandshakeInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<CustomerSecurityHandshakeInfo*>*
CustomerSecurityHandshakeHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CustomerSecurityHandshakeInfo>(compressed);
}

std::string
CustomerSecurityHandshakeHistoricalDAO::_name("CustomerSecurityHandshakeHistorical");
std::string
CustomerSecurityHandshakeHistoricalDAO::_cacheClass("Rules");
DAOHelper<CustomerSecurityHandshakeHistoricalDAO>
CustomerSecurityHandshakeHistoricalDAO::_helper(_name);
CustomerSecurityHandshakeHistoricalDAO*
CustomerSecurityHandshakeHistoricalDAO::_instance(nullptr);

} // namespace tse
