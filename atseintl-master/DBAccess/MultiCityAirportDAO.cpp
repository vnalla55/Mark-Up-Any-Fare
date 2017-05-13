//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "DBAccess/MultiCityAirportDAO.h"

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
MultiCityAirportDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MultiCityAirportDAO"));

MultiCityAirportDAO&
MultiCityAirportDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MultiAirportCity*>&
getMultiCityAirportData(const LocCode& locCode,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  MultiCityAirportDAO& dao = MultiCityAirportDAO::instance();
  return dao.get(deleteList, locCode, ticketDate);
}

const std::vector<MultiAirportCity*>&
MultiCityAirportDAO::get(DeleteList& del, const LocCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  del.copy(ptr);
  return *ptr;
}

void
MultiCityAirportDAO::load()
{
  StartupLoader<QueryGetAllMultiAirportCities, MultiAirportCity, MultiCityAirportDAO>();
}

LocCodeKey
MultiCityAirportDAO::createKey(MultiAirportCity* info)
{
  return LocCodeKey(info->city());
}

std::vector<MultiAirportCity*>*
MultiCityAirportDAO::create(LocCodeKey key)
{
  return new std::vector<MultiAirportCity*>;
}

void
MultiCityAirportDAO::destroy(LocCodeKey key, std::vector<MultiAirportCity*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MultiCityAirportDAO::compress(const std::vector<MultiAirportCity*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiAirportCity*>*
MultiCityAirportDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiAirportCity>(compressed);
}

std::string
MultiCityAirportDAO::_name("MultiCityAirport");
std::string
MultiCityAirportDAO::_cacheClass("Common");

DAOHelper<MultiCityAirportDAO>
MultiCityAirportDAO::_helper(_name);

MultiCityAirportDAO* MultiCityAirportDAO::_instance = nullptr;

} // namespace tse
