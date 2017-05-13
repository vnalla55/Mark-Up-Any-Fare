//----------------------------------------------------------------------------
//  File: FDHeaderMsgTextDAO.cpp
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "DBAccess/FDHeaderMsgTextDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetHeaderMsgText.h"

namespace tse
{
log4cxx::LoggerPtr
FDHeaderMsgTextDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDHeaderMsgTextDAO"));

std::string
FDHeaderMsgTextDAO::_name("FDHeaderMsgText");
std::string
FDHeaderMsgTextDAO::_cacheClass("FareDisplay");

DAOHelper<FDHeaderMsgTextDAO>
FDHeaderMsgTextDAO::_helper(_name);

FDHeaderMsgTextDAO* FDHeaderMsgTextDAO::_instance = nullptr;

FDHeaderMsgTextDAO&
FDHeaderMsgTextDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<const std::string*>&
FDHeaderMsgTextDAO::get(DeleteList& del, const uint64_t& itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDHeaderMsgTextKey key(itemNo);

  DAOCache::pointer_type ptr = cache().get(key);

  del.copy(ptr);

  return *ptr;
}

std::vector<const std::string*>*
FDHeaderMsgTextDAO::create(FDHeaderMsgTextKey key)
{
  std::vector<const std::string*>* ret = new std::vector<const std::string*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetHeaderMsgText hmt(dbAdapter->getAdapter());
    hmt.findHeaderMsgText(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDHeaderMsgTextDAO::create()");
    throw;
  }

  return ret;
}

void
FDHeaderMsgTextDAO::destroy(FDHeaderMsgTextKey key, std::vector<const std::string*>* t)
{
  try
  {
    std::vector<const std::string*>::iterator i;
    for (i = t->begin(); i != t->end(); i++)
      delete *i; // lint !e605
    delete t;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDHeaderMsgTextDAO::destroy()");
    throw;
  }

  return;
}
// --------------------------------------------------
// Historical DAO: FDHeaderMsgTextHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FDHeaderMsgTextHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FDHeaderMsgTextHistoricalDAO"));
std::string
FDHeaderMsgTextHistoricalDAO::_name("FDHeaderMsgTextHistorical");
std::string
FDHeaderMsgTextHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FDHeaderMsgTextHistoricalDAO>
FDHeaderMsgTextHistoricalDAO::_helper(_name);

FDHeaderMsgTextHistoricalDAO* FDHeaderMsgTextHistoricalDAO::_instance = nullptr;

FDHeaderMsgTextHistoricalDAO&
FDHeaderMsgTextHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<const std::string*>&
FDHeaderMsgTextHistoricalDAO::get(DeleteList& del,
                                  const uint64_t& itemNo,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDHeaderMsgTextKey key(itemNo);

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<const std::string*>*
FDHeaderMsgTextHistoricalDAO::create(FDHeaderMsgTextKey key)
{
  std::vector<const std::string*>* ret = new std::vector<const std::string*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetHeaderMsgText hmt(dbAdapter->getAdapter());
    hmt.findHeaderMsgText(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDHeaderMsgTextHistoricalDAO::create()");
    throw;
  }

  return ret;
}

void
FDHeaderMsgTextHistoricalDAO::destroy(FDHeaderMsgTextKey key, std::vector<const std::string*>* t)
{
  try
  {
    std::vector<const std::string*>::iterator i;
    for (i = t->begin(); i != t->end(); i++)
      delete *i; // lint !e605
    delete t;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDHeaderMsgTextHistoricalDAO::destroy()");
    throw;
  }

  return;
}
}
