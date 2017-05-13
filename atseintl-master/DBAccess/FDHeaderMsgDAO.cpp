//----------------------------------------------------------------------------
//  File: FDHeaderMsgDAO.cpp
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
#include "DBAccess/FDHeaderMsgDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetHeaderMsg.h"

namespace tse
{
log4cxx::LoggerPtr
FDHeaderMsgDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDHeaderMsgDAO"));

std::string
FDHeaderMsgDAO::_name("FDHeaderMsg");
std::string
FDHeaderMsgDAO::_cacheClass("FareDisplay");

DAOHelper<FDHeaderMsgDAO>
FDHeaderMsgDAO::_helper(_name);

FDHeaderMsgDAO* FDHeaderMsgDAO::_instance = nullptr;

FDHeaderMsgDAO&
FDHeaderMsgDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<const FDHeaderMsg*>&
getHeaderMsgDataListData(const PseudoCityCode& pseudoCityCode,
                         const PseudoCityType& pseudoCityType,
                         const Indicator& userApplType,
                         const std::string& userAppl,
                         const TJRGroup& tjrGroup,
                         const DateTime& date,
                         DeleteList& deleteList)
{
  FDHeaderMsgDAO& dao = FDHeaderMsgDAO::instance();
  return dao.get(
      deleteList, pseudoCityCode, pseudoCityType, userApplType, userAppl, tjrGroup, date);
}

std::vector<const FDHeaderMsg*>&
FDHeaderMsgDAO::get(DeleteList& del,
                    const PseudoCityCode& pseudoCityCode,
                    const PseudoCityType& pseudoCityType,
                    const Indicator& userApplType,
                    const std::string& userAppl,
                    const TJRGroup& tjrGroup,
                    const DateTime& date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDHeaderMsgKey key(pseudoCityCode, pseudoCityType, userApplType, userAppl, tjrGroup);

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<const FDHeaderMsg*>* ret = new std::vector<const FDHeaderMsg*>;
  DAOCache::value_type::iterator i = ptr->begin();

  while (ptr->end() != (i = find_if(i, ptr->end(), IsEffective<FDHeaderMsg>(date))))
  {
    ret->push_back(*i++);
  }
  del.adopt(ret);
  return *ret;
}

std::vector<const FDHeaderMsg*>*
FDHeaderMsgDAO::create(FDHeaderMsgKey key)
{
  std::vector<const FDHeaderMsg*>* ret = new std::vector<const FDHeaderMsg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetHeaderMsg hm(dbAdapter->getAdapter());
    hm.findHeaderMsgs(*ret, key._a, key._b, key._c, key._d, key._e, "");
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDHeaderMsgDAO::create()");
    throw;
  }

  return ret;
}

void
FDHeaderMsgDAO::destroy(FDHeaderMsgKey key, std::vector<const FDHeaderMsg*>* t)
{
  LOG4CXX_INFO(_logger, "Enterning destroy");

  try
  {
    std::vector<const FDHeaderMsg*>::iterator i;
    for (i = t->begin(); i != t->end(); i++)
      delete *i; // lint !e605
    delete t;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDHeaderMsgDAO::destroy");
    throw;
  }

  LOG4CXX_INFO(_logger, "Leaving destroy");
  return;
}

FDHeaderMsgKey
FDHeaderMsgDAO::createKey(const FDHeaderMsg* info)
{
  return FDHeaderMsgKey(info->pseudoCityCode(),
                        info->pseudoCityType(),
                        info->userApplType(),
                        info->userAppl(),
                        info->ssgGroupNo());
}

void
FDHeaderMsgDAO::load()
{
  StartupLoaderNoDB<FDHeaderMsg, FDHeaderMsgDAO>();
}
}
