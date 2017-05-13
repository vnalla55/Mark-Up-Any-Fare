#include "DBAccess/CommissionContractDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetCommissionContract.h"

namespace tse
{
namespace
{
struct IsNotEffectiveContractG
{
  IsNotEffectiveContractG(DateTime ticketDate)
    : _ticketDate(ticketDate.isEmptyDate() ? DateTime::localTime() : ticketDate)
  {
  }

  bool operator()(const CommissionContractInfo* rec) const
  {
    if (rec->effDateTime() > _ticketDate
        || rec->expireDate() < _ticketDate)
    {
      return true;
    }
    return false;
  }

  DateTime _ticketDate;
};

struct IsNotEffectiveContractH
{
  IsNotEffectiveContractH(DateTime startDate,
                          DateTime endDate,
                          DateTime ticketDate)
    : _startDate(startDate.date())
    , _endDate(endDate.date())
    , _ticketDate(ticketDate)
    , _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  bool operator()(const CommissionContractInfo* rec) const
  {
    if (rec->effDateTime() > _endDate)
      return true;

    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return true;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return true;
    }
    return false;
  }
  const boost::gregorian::date _startDate;
  const DateTime _endDate;
  const DateTime _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;

  void init(DateTime ticketDate)
  {
    if (ticketDate.isEmptyDate())
    {
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
    }
  }
};

}// namespace

log4cxx::LoggerPtr CommissionContractDAO::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionContractDAO"));

CommissionContractDAO& CommissionContractDAO::instance()
{
  if (0 == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionContractInfo*>& getCommissionContractData(const VendorCode& vendor,
                                                                      const CarrierCode& carrier,
                                                                      const PseudoCityCode& pcc,
                                                                      DeleteList& deleteList,
                                                                      DateTime ticketDate,
                                                                      bool isHistorical)
{
  if (isHistorical)
  {
    CommissionContractHistoricalDAO& dao(CommissionContractHistoricalDAO::instance());

    return dao.get(deleteList, vendor, carrier, pcc, ticketDate);
  }
  else
  {
    CommissionContractDAO& dao(CommissionContractDAO::instance());

    return dao.get(deleteList, vendor, carrier, pcc, ticketDate);
  }
}

const std::vector<CommissionContractInfo*>& CommissionContractDAO::get(DeleteList& del,
                                                                       const VendorCode& vendor,
                                                                       const CarrierCode& carrier,
                                                                       const PseudoCityCode& pcc,
                                                                       DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;
  CommissionContractKey key(vendor, carrier, pcc);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotEffectiveContractG isNotEffective(ticketDate);
  return *applyFilter(del, ptr, isNotEffective);
}

std::vector<CommissionContractInfo*>* CommissionContractDAO::create(CommissionContractKey key)
{
  std::vector<CommissionContractInfo*>* ret(new std::vector<CommissionContractInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetCommissionContract q(dbAdapter->getAdapter());
    q.findCommissionContract(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionContractDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CommissionContractKey CommissionContractDAO::createKey(const CommissionContractInfo* info)
{
  return CommissionContractKey(info->vendor(), info->carrier(), info->pcc());
}

void CommissionContractDAO::destroy(CommissionContractKey,
                                    std::vector<CommissionContractInfo*>* recs)
{
  destroyContainer(recs);
}

std::string CommissionContractDAO::_name("CommissionContract");
std::string CommissionContractDAO::_cacheClass("Rules");
DAOHelper<CommissionContractDAO> CommissionContractDAO::_helper(_name);
CommissionContractDAO* CommissionContractDAO::_instance(0);

// Historical DAO Object
log4cxx::LoggerPtr CommissionContractHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionContractHistoricalDAO"));

CommissionContractHistoricalDAO& CommissionContractHistoricalDAO::instance()
{
  if (0 == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionContractInfo*>& CommissionContractHistoricalDAO::get(DeleteList& del,
                                                                                 const VendorCode& vendor,
                                                                                 const CarrierCode& carrier,
                                                                                 const PseudoCityCode& pcc,
                                                                                 DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  CommissionContractHistoricalKey key(vendor, carrier, pcc);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<CommissionContractInfo*>* ret(new std::vector<CommissionContractInfo*>);
  del.adopt(ret);
  IsNotEffectiveContractH isNotEffective(key._d, key._e, ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotEffective);
  return *ret;
}

std::vector<CommissionContractInfo*>* CommissionContractHistoricalDAO::create(CommissionContractHistoricalKey key)
{
  std::vector<CommissionContractInfo*>* ret(new std::vector<CommissionContractInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetCommissionContractHistorical q(dbAdapter->getAdapter());
    q.findCommissionContract(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionContractHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CommissionContractHistoricalKey CommissionContractHistoricalDAO::createKey(const CommissionContractInfo* info,
                                                                           const DateTime& startDate,
                                                                           const DateTime& endDate)
{
  return CommissionContractHistoricalKey(info->vendor(), info->carrier(), info->pcc(), startDate, endDate);
}

void CommissionContractHistoricalDAO::destroy(CommissionContractHistoricalKey,
                                              std::vector<CommissionContractInfo*>* recs)
{
  destroyContainer(recs);
}

std::string CommissionContractHistoricalDAO::_name("CommissionContractHistorical");
std::string CommissionContractHistoricalDAO::_cacheClass("Rules");
DAOHelper<CommissionContractHistoricalDAO> CommissionContractHistoricalDAO::_helper(_name);
CommissionContractHistoricalDAO* CommissionContractHistoricalDAO::_instance(0);

}// tse
