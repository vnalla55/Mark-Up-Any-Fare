//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "DBAccess/MultiAirportCityDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/Queries/QueryGetAllMultiAirportCities.h"

namespace tse
{
log4cxx::LoggerPtr
MultiAirportCityDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MultiAirportCityDAO"));

MultiAirportCityDAO&
MultiAirportCityDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MultiAirportCity*>&
getMultiAirportCityData(const LocCode& city,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  MultiAirportCityDAO& dao = MultiAirportCityDAO::instance();
  return dao.get(deleteList, city, ticketDate);
}

const std::vector<MultiAirportCity*>&
MultiAirportCityDAO::get(DeleteList& del, const LocCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  del.copy(ptr);
  return *ptr;
}

void
MultiAirportCityDAO::load()
{
  StartupLoader<QueryGetAllMultiAirportCities, MultiAirportCity, MultiAirportCityDAO>();
}

LocCodeKey
MultiAirportCityDAO::createKey(MultiAirportCity* info)
{
  return LocCodeKey(info->airportCode());
}

std::vector<MultiAirportCity*>*
MultiAirportCityDAO::create(LocCodeKey key)
{
  std::vector<MultiAirportCity*>* ret = new std::vector<MultiAirportCity*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMultiAirportCity mac(dbAdapter->getAdapter());
    mac.findMultiAirportCity(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MultiAirportCityDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MultiAirportCityDAO::destroy(LocCodeKey key, std::vector<MultiAirportCity*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MultiAirportCityDAO::compress(const std::vector<MultiAirportCity*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiAirportCity*>*
MultiAirportCityDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiAirportCity>(compressed);
}

std::string
MultiAirportCityDAO::_name("MultiAirportCity");
std::string
MultiAirportCityDAO::_cacheClass("Common");

DAOHelper<MultiAirportCityDAO>
MultiAirportCityDAO::_helper(_name);

MultiAirportCityDAO* MultiAirportCityDAO::_instance = nullptr;

} // namespace tse
