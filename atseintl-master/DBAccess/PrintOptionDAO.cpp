//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/PrintOptionDAO.h"

#include "DBAccess/PrintOption.h"
#include "DBAccess/Queries/QueryGetPrintOption.h"

namespace tse
{
log4cxx::LoggerPtr
PrintOptionDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PrintOptionDAO"));

std::string
PrintOptionDAO::_name("PrintOption");
std::string
PrintOptionDAO::_cacheClass("Rules");
DAOHelper<PrintOptionDAO>
PrintOptionDAO::_helper(_name);
PrintOptionDAO* PrintOptionDAO::_instance = nullptr;

PrintOptionDAO&
PrintOptionDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PrintOption*
getPrintOptionData(const VendorCode& vendor,
                   const std::string& fareSource,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    PrintOptionHistoricalDAO& dao = PrintOptionHistoricalDAO::instance();
    return dao.get(deleteList, vendor, fareSource, date, ticketDate);
  }
  else
  {
    PrintOptionDAO& dao = PrintOptionDAO::instance();
    return dao.get(deleteList, vendor, fareSource, date, ticketDate);
  }
}

const PrintOption*
PrintOptionDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    const std::string& fareSource,
                    const DateTime& date,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PrintOptionKey key(vendor, fareSource);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;
  del.copy(ptr);

  IsEffectiveG<PrintOption> isEff(date, ticketDate);
  for (const auto elem : *ptr)
  {
    if (isEff(elem))
      return elem;
  }
  return nullptr;
}

std::vector<PrintOption*>*
PrintOptionDAO::create(PrintOptionKey key)
{
  std::vector<PrintOption*>* ret = new std::vector<PrintOption*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPrintOption fnc(dbAdapter->getAdapter());
    fnc.findPrintOption(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PrintOptionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PrintOptionDAO::destroy(PrintOptionKey key, std::vector<PrintOption*>* recs)
{
  destroyContainer(recs);
}

PrintOptionKey
PrintOptionDAO::createKey(PrintOption* info)
{
  return PrintOptionKey(info->vendor(), info->fareSource());
}

void
PrintOptionDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<PrintOption, PrintOptionDAO>();
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
PrintOptionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PrintOptionHistoricalDAO"));

std::string
PrintOptionHistoricalDAO::_name("PrintOptionHistorical");
std::string
PrintOptionHistoricalDAO::_cacheClass("Rules");
DAOHelper<PrintOptionHistoricalDAO>
PrintOptionHistoricalDAO::_helper(_name);
PrintOptionHistoricalDAO* PrintOptionHistoricalDAO::_instance = nullptr;

PrintOptionHistoricalDAO&
PrintOptionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PrintOption*
PrintOptionHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              const std::string& fareSource,
                              const DateTime& date,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PrintOptionHistoricalKey key(vendor, fareSource);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;
  del.copy(ptr);

  IsEffectiveH<PrintOption> isEff(date, ticketDate);
  for (const auto elem : *ptr)
  {
    if (isEff(elem))
      return elem;
  }
  return nullptr;
}

std::vector<PrintOption*>*
PrintOptionHistoricalDAO::create(PrintOptionHistoricalKey key)
{
  std::vector<PrintOption*>* ret = new std::vector<PrintOption*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPrintOptionHistorical fnc(dbAdapter->getAdapter());
    fnc.findPrintOption(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PrintOptionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PrintOptionHistoricalDAO::destroy(PrintOptionHistoricalKey key, std::vector<PrintOption*>* recs)
{
  std::vector<PrintOption*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}
} // tse
