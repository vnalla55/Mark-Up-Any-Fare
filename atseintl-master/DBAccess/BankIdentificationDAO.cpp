// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "DBAccess/BankIdentificationDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BankIdentificationInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetBankIdentification.h"

namespace tse
{
log4cxx::LoggerPtr
BankIdentificationDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BankIdentificationDAO"));

BankIdentificationDAO&
BankIdentificationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const BankIdentificationInfo*
getBankIdentificationData(const FopBinNumber& binNumber,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    BankIdentificationHistoricalDAO& dao = BankIdentificationHistoricalDAO::instance();
    return dao.get(deleteList, binNumber, date, ticketDate);
  }
  else
  {
    BankIdentificationDAO& dao = BankIdentificationDAO::instance();
    return dao.get(deleteList, binNumber, date, ticketDate);
  }
}

const BankIdentificationInfo*
BankIdentificationDAO::get(DeleteList& del,
                           const FopBinNumber& binNumber,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(BinKey(binNumber));
  del.copy(ptr);
  BankIdentificationInfo* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<BankIdentificationInfo>(date, ticketDate));

  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<BankIdentificationInfo*>*
BankIdentificationDAO::create(BinKey key)
{
  std::vector<BankIdentificationInfo*>* ret = new std::vector<BankIdentificationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBankIdentification fnc(dbAdapter->getAdapter());
    fnc.findBankIdentificationInfo(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BankIdentificationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BankIdentificationDAO::destroy(BinKey key, std::vector<BankIdentificationInfo*>* recs)
{
  destroyContainer(recs);
}

BinKey
BankIdentificationDAO::createKey(BankIdentificationInfo* info)
{
  return BinKey(info->binNumber());
}

void
BankIdentificationDAO::load()
{
  // not pre_loading

  StartupLoaderNoDB<BankIdentificationInfo, BankIdentificationDAO>();
}

std::string
BankIdentificationDAO::_name("BankIdentification");
std::string
BankIdentificationDAO::_cacheClass("Rules");
DAOHelper<BankIdentificationDAO>
BankIdentificationDAO::_helper(_name);
BankIdentificationDAO* BankIdentificationDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
BankIdentificationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BankIdentificationHistDAO"));
BankIdentificationHistoricalDAO&
BankIdentificationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const BankIdentificationInfo*
BankIdentificationHistoricalDAO::get(DeleteList& del,
                                     const FopBinNumber& binNumber,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BinHistoricalKey key(binNumber);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  BankIdentificationInfo* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveH<BankIdentificationInfo>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<BankIdentificationInfo*>*
BankIdentificationHistoricalDAO::create(BinHistoricalKey key)
{
  std::vector<BankIdentificationInfo*>* ret = new std::vector<BankIdentificationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBankIdentificationHistorical fnc(dbAdapter->getAdapter());
    fnc.findBankIdentificationInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BankIdentificationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BankIdentificationHistoricalDAO::destroy(BinHistoricalKey key,
                                         std::vector<BankIdentificationInfo*>* recs)
{
  std::vector<BankIdentificationInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
BankIdentificationHistoricalDAO::_name("BankIdentificationHistorical");
std::string
BankIdentificationHistoricalDAO::_cacheClass("Rules");
DAOHelper<BankIdentificationHistoricalDAO>
BankIdentificationHistoricalDAO::_helper(_name);
BankIdentificationHistoricalDAO* BankIdentificationHistoricalDAO::_instance = nullptr;
}
