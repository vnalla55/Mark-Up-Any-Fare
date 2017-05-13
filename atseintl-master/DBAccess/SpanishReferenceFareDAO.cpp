#include "DBAccess/SpanishReferenceFareDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/SpanishReferenceFareInfo.h"
#include "DBAccess/Queries/QueryGetSpanishReferenceFare.h"

namespace tse
{
namespace
{
  Logger logger("atseintl.DBAccess.SpanishReferenceFareDAO");
  Logger loggerH("atseintl.DBAccess.SpanishReferenceFareHistoricalDAO");
}

SpanishReferenceFareDAO&
SpanishReferenceFareDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SpanishReferenceFareInfo*>&
getSpanishReferenceFareData(CarrierCode tktCarrier, const CarrierCode& fareCarrier,
                            const LocCode& sourceLoc, const LocCode& destLoc,
                            DeleteList& deleteList, const DateTime& date,
                            const DateTime& ticketDate, bool isHistorical)
{
  if (isHistorical)
  {
    SpanishReferenceFareHistoricalDAO& dao = SpanishReferenceFareHistoricalDAO::instance();
    return dao.get(deleteList, tktCarrier, fareCarrier, sourceLoc, destLoc, date, ticketDate);
  }
  else
  {
    SpanishReferenceFareDAO& dao(SpanishReferenceFareDAO::instance());
    return dao.get(deleteList, tktCarrier, fareCarrier, sourceLoc, destLoc, date, ticketDate);
  }
}

const std::vector<SpanishReferenceFareInfo*>&
SpanishReferenceFareDAO::get (DeleteList& del, const CarrierCode& tktCarrier,
                              const CarrierCode& fareCarrier,
                              const LocCode& sourceLoc,
                              const LocCode& destLoc, const DateTime& date, const DateTime& ticketDate)
{
  ++_codeCoverageGetCallCount;

  SpanishReferenceFareKey key(tktCarrier, fareCarrier, sourceLoc, destLoc);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  return *(applyFilter(del, ptr, IsNotEffectiveG<SpanishReferenceFareInfo>(date, ticketDate)));
}

std::vector<SpanishReferenceFareInfo*>*
SpanishReferenceFareDAO::create(SpanishReferenceFareKey key)
{
  std::vector<SpanishReferenceFareInfo*>* ret(new std::vector<SpanishReferenceFareInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetSpanishReferenceFare q(dbAdapter->getAdapter());
    q.findSpanishReferenceFare(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(logger, "DB exception in SpanishReferenceFareDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

SpanishReferenceFareKey
SpanishReferenceFareDAO::createKey(const SpanishReferenceFareInfo* info)
{
  using namespace tag;
  return SpanishReferenceFareKey (info->get<TktCarrier>(), info->get<FareCarrier>(),
                                  info->get<OriginAirport>(),
                                  info->get<DestinationAirport>());
}

void
SpanishReferenceFareDAO::destroy(SpanishReferenceFareKey, std::vector<SpanishReferenceFareInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
SpanishReferenceFareDAO::_name("SpanishReferenceFare");
std::string
SpanishReferenceFareDAO::_cacheClass("Rules");
DAOHelper<SpanishReferenceFareDAO>
SpanishReferenceFareDAO::_helper(_name);
SpanishReferenceFareDAO*
SpanishReferenceFareDAO::_instance(nullptr);

SpanishReferenceFareHistoricalDAO&
SpanishReferenceFareHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SpanishReferenceFareInfo*>&
SpanishReferenceFareHistoricalDAO::get(DeleteList& del, const CarrierCode& tktCarrier,
                                       const CarrierCode& fareCarrier,
                                       const LocCode& sourceLoc,
                                       const LocCode& destLoc,
                                       const DateTime& date,
                                       const DateTime& ticketDate)
{
  ++_codeCoverageGetCallCount;

  SpanishReferenceFareHistoricalKey key(tktCarrier, fareCarrier, sourceLoc, destLoc);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<SpanishReferenceFareInfo*>* ret = new std::vector<SpanishReferenceFareInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotEffectiveH<SpanishReferenceFareInfo>(date, ticketDate));
  return *ret;
}

std::vector<SpanishReferenceFareInfo*>*
SpanishReferenceFareHistoricalDAO::create(SpanishReferenceFareHistoricalKey key)
{
  std::vector<SpanishReferenceFareInfo*>* ret = new std::vector<SpanishReferenceFareInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSpanishReferenceFareHistorical q(dbAdapter->getAdapter());
    q.findSpanishReferenceFare(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(loggerH, "DB exception in SpanishReferenceFareHistoricalDAO::create");;
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SpanishReferenceFareHistoricalDAO::destroy(SpanishReferenceFareHistoricalKey,
                                           std::vector<SpanishReferenceFareInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
SpanishReferenceFareHistoricalDAO::_name("SpanishReferenceFareHistorical");
std::string
SpanishReferenceFareHistoricalDAO::_cacheClass("Rules");
DAOHelper<SpanishReferenceFareHistoricalDAO>
SpanishReferenceFareHistoricalDAO::_helper(_name);
SpanishReferenceFareHistoricalDAO* SpanishReferenceFareHistoricalDAO::_instance = nullptr;
} // namespace tse
