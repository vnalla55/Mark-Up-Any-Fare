//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CorpIdDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CorpId.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCorpId.h"

namespace tse
{
log4cxx::LoggerPtr
CorpIdDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CorpIdDAO"));
CorpIdDAO&
CorpIdDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct CorpIdDAO::BadCxr
{
  const CarrierCode _cxr;
  IsNotEffectiveG<CorpId> isNotEffective;

  BadCxr(const CarrierCode& cxr, const DateTime& date, const DateTime& ticketDate)
    : _cxr(cxr), isNotEffective(date, ticketDate)
  {
  }

  bool operator()(const CorpId* ci) const
  {
    if (!ci->carrier().empty() && ci->carrier() != _cxr)
      return true;
    return isNotEffective(ci);
  }
};

const std::vector<tse::CorpId*>&
getCorpIdData(const std::string& corpId,
              const CarrierCode& carrier,
              const DateTime& tvlDate,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical)
{
  if (isHistorical)
  {
    CorpIdHistoricalDAO& dao = CorpIdHistoricalDAO::instance();
    return dao.get(deleteList, corpId, carrier, tvlDate, ticketDate);
  }
  else
  {
    CorpIdDAO& dao = CorpIdDAO::instance();
    return dao.get(deleteList, corpId, carrier, tvlDate, ticketDate);
  }
}

const std::vector<CorpId*>&
CorpIdDAO::get(DeleteList& del,
               const std::string& corpId,
               const CarrierCode& carrier,
               const DateTime& tvlDate,
               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CorpIdKey key(corpId);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, BadCxr(carrier, tvlDate, ticketDate)));
}

bool
corpIdExistsData(const std::string& corpId,
                 const DateTime& tvlDate,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (isHistorical)
  {
    CorpIdHistoricalDAO& dao = CorpIdHistoricalDAO::instance();
    return dao.corpIdExists(corpId, tvlDate, ticketDate);
  }
  else
  {
    CorpIdDAO& dao = CorpIdDAO::instance();
    return dao.corpIdExists(corpId, tvlDate, ticketDate);
  }
}

bool
CorpIdDAO::corpIdExists(const std::string& corpId, const DateTime& date, const DateTime& ticketDate)
{
  CorpIdKey key(corpId);
  DAOCache::pointer_type ptr = cache().get(key);
  std::vector<CorpId*> chkVect;
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(chkVect), IsNotEffectiveG<CorpId>(date, ticketDate));
  return !chkVect.empty();
}

std::vector<CorpId*>*
CorpIdDAO::create(CorpIdKey key)
{
  std::vector<CorpId*>* ret = new std::vector<CorpId*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCorpId cid(dbAdapter->getAdapter());
    cid.findCorpId(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CorpIdDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CorpIdDAO::destroy(CorpIdKey key, std::vector<CorpId*>* recs)
{
  std::vector<CorpId*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

CorpIdKey
CorpIdDAO::createKey(CorpId* info)
{
  return CorpIdKey(info->corpId());
}

void
CorpIdDAO::load()
{
  StartupLoaderNoDB<CorpId, CorpIdDAO>();
}

std::string
CorpIdDAO::_name("CorpId");
std::string
CorpIdDAO::_cacheClass("Fares");
DAOHelper<CorpIdDAO>
CorpIdDAO::_helper(_name);
CorpIdDAO* CorpIdDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CorpIdHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CorpIdHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CorpIdHistoricalDAO"));
CorpIdHistoricalDAO&
CorpIdHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct CorpIdHistoricalDAO::BadCxr
{
  const CarrierCode _cxr;

  BadCxr(const CarrierCode& cxr) : _cxr(cxr) {}

  bool operator()(const CorpId* ci) { return !ci->carrier().empty() && ci->carrier() != _cxr; }
};

const std::vector<CorpId*>&
CorpIdHistoricalDAO::get(DeleteList& del,
                         const std::string& corpId,
                         const CarrierCode& carrier,
                         const DateTime& tvlDate,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CorpIdHistoricalKey key(corpId);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<CorpId*>* ret = new std::vector<CorpId*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<CorpId>(tvlDate, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), BadCxr(carrier)), ret->end());
  return *ret;
}

bool
CorpIdHistoricalDAO::corpIdExists(const std::string& corpId,
                                  const DateTime& date,
                                  const DateTime& ticketDate)
{
  CorpIdHistoricalKey key(corpId);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  std::vector<CorpId*> chkVect;
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(chkVect),
                 IsNotEffectiveHist<CorpId>(date, ticketDate));
  return !chkVect.empty();
}

std::vector<CorpId*>*
CorpIdHistoricalDAO::create(CorpIdHistoricalKey key)
{
  std::vector<CorpId*>* ret = new std::vector<CorpId*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCorpIdHistorical cid(dbAdapter->getAdapter());
    cid.findCorpId(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CorpIdHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CorpIdHistoricalDAO::destroy(CorpIdHistoricalKey key, std::vector<CorpId*>* recs)
{
  std::vector<CorpId*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CorpIdHistoricalDAO::_name("CorpIdHistorical");
std::string
CorpIdHistoricalDAO::_cacheClass("Fares");
DAOHelper<CorpIdHistoricalDAO>
CorpIdHistoricalDAO::_helper(_name);
CorpIdHistoricalDAO* CorpIdHistoricalDAO::_instance = nullptr;

} // namespace tse
