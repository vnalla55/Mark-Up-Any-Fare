//-------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DBAccess/MaxPermittedAmountFareDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MaxPermittedAmountFareInfo.h"

namespace tse
{
namespace
{
Logger
logger("atseintl.DBAccess.MaxPermittedAmountFareDAO");
Logger
loggerH("atseintl.DBAccess.MaxPermittedAmountFareHistoricalDAO");
}

MaxPermittedAmountFareDAO&
MaxPermittedAmountFareDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MaxPermittedAmountFareInfo*>&
getMaxPermittedAmountFareData(const Loc& origin,
                              const Loc& dest,
                              DeleteList& deleteList,
                              const DateTime& date,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    MaxPermittedAmountFareHistoricalDAO& dao(MaxPermittedAmountFareHistoricalDAO::instance());
    return dao.get(deleteList, origin, dest, date, ticketDate);
  }
  else
  {
    MaxPermittedAmountFareDAO& dao(MaxPermittedAmountFareDAO::instance());
    return dao.get(deleteList, origin, dest, date, ticketDate);
  }
}

const std::vector<MaxPermittedAmountFareInfo*>&
MaxPermittedAmountFareDAO::get(DeleteList& del,
                               const Loc& origin,
                               const Loc& dest,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  ++_codeCoverageGetCallCount;

  const LocCode& originCity = origin.city().empty() ? origin.loc() : origin.city();
  const LocCode& destCity = dest.city().empty() ? dest.loc() : dest.city();

  MaxPermittedAmountFareKey key(
      origin.loc(), originCity, origin.nation(), dest.loc(), destCity, dest.nation());
  DAOCache::pointer_type ptr = cache().get(key);

  return *(applyFilter(del, ptr, IsNotEffectiveG<MaxPermittedAmountFareInfo>(date, ticketDate)));
}

std::vector<MaxPermittedAmountFareInfo*>*
MaxPermittedAmountFareDAO::create(MaxPermittedAmountFareKey key)
{
  std::vector<MaxPermittedAmountFareInfo*>* ret(new std::vector<MaxPermittedAmountFareInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetMaxPermittedAmountFare q(dbAdapter->getAdapter());
    q.findMaxPermittedAmountFare(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(logger, "DB exception in MaxPermittedAmountFareDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

MaxPermittedAmountFareKey
MaxPermittedAmountFareDAO::createKey(const MaxPermittedAmountFareInfo* info)
{
  using namespace tag;
  return MaxPermittedAmountFareKey(info->get<OriginAirport>(),
                                   info->get<OriginCity>(),
                                   info->get<OriginNation>(),
                                   info->get<DestAirport>(),
                                   info->get<DestCity>(),
                                   info->get<DestNation>());
}

void
MaxPermittedAmountFareDAO::destroy(MaxPermittedAmountFareKey,
                                   std::vector<MaxPermittedAmountFareInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
MaxPermittedAmountFareDAO::_name("MaxPermittedAmountFare");
std::string
MaxPermittedAmountFareDAO::_cacheClass("Rules");
DAOHelper<MaxPermittedAmountFareDAO>
MaxPermittedAmountFareDAO::_helper(_name);
MaxPermittedAmountFareDAO*
MaxPermittedAmountFareDAO::_instance(nullptr);

MaxPermittedAmountFareHistoricalDAO&
MaxPermittedAmountFareHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MaxPermittedAmountFareInfo*>&
MaxPermittedAmountFareHistoricalDAO::get(DeleteList& del,
                                         const Loc& origin,
                                         const Loc& dest,
                                         const DateTime& date,
                                         const DateTime& ticketDate)
{
  ++_codeCoverageGetCallCount;

  const LocCode& originCity = origin.city().empty() ? origin.loc() : origin.city();
  const LocCode& destCity = dest.city().empty() ? dest.loc() : dest.city();

  MaxPermittedAmountFareHistoricalKey key(
      origin.loc(), originCity, origin.nation(), dest.loc(), destCity, dest.nation());
  DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<MaxPermittedAmountFareInfo*>* ret = new std::vector<MaxPermittedAmountFareInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<MaxPermittedAmountFareInfo>(date, ticketDate));
  return *ret;
}

std::vector<MaxPermittedAmountFareInfo*>*
MaxPermittedAmountFareHistoricalDAO::create(MaxPermittedAmountFareHistoricalKey key)
{
  std::vector<MaxPermittedAmountFareInfo*>* ret = new std::vector<MaxPermittedAmountFareInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMaxPermittedAmountFareHistorical q(dbAdapter->getAdapter());
    q.findMaxPermittedAmountFare(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(loggerH, "DB exception in MaxPermittedAmountFareHistoricalDAO::create");
    ;
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MaxPermittedAmountFareHistoricalDAO::destroy(MaxPermittedAmountFareHistoricalKey,
                                             std::vector<MaxPermittedAmountFareInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
MaxPermittedAmountFareHistoricalDAO::_name("MaxPermittedAmountFareHistorical");
std::string
MaxPermittedAmountFareHistoricalDAO::_cacheClass("Rules");
DAOHelper<MaxPermittedAmountFareHistoricalDAO>
MaxPermittedAmountFareHistoricalDAO::_helper(_name);
MaxPermittedAmountFareHistoricalDAO* MaxPermittedAmountFareHistoricalDAO::_instance = nullptr;
} // namespace tse
