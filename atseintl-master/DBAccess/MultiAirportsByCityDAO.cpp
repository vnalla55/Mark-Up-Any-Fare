//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "DBAccess/MultiAirportsByCityDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MultiAirportsByCity.h"
#include "DBAccess/Queries/QueryGetAllMultiAirportCities.h"

namespace tse
{
log4cxx::LoggerPtr
MultiAirportsByCityDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MultiAirportsByCityDAO"));

MultiAirportsByCityDAO&
MultiAirportsByCityDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MultiAirportsByCity*>&
MultiAirportsByCityDAO::get(DeleteList& del, const LocCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  del.copy(ptr);
  return *ptr;
}

void
MultiAirportsByCityDAO::load()
{
  StartupLoader<QueryGetAllMultiAirportsByCity, MultiAirportsByCity, MultiAirportsByCityDAO>();
}

LocCodeKey
MultiAirportsByCityDAO::createKey(MultiAirportsByCity* info)
{
  return LocCodeKey(info->city());
}

std::vector<MultiAirportsByCity*>*
MultiAirportsByCityDAO::create(LocCodeKey key)
{
  return new std::vector<MultiAirportsByCity*>;
}

void
MultiAirportsByCityDAO::destroy(LocCodeKey key, std::vector<MultiAirportsByCity*>* recs)
{
  destroyContainer(recs);
}

std::string
MultiAirportsByCityDAO::_name("MultiAirportsByCity");
std::string
MultiAirportsByCityDAO::_cacheClass("Common");

DAOHelper<MultiAirportsByCityDAO>
MultiAirportsByCityDAO::_helper(_name);

MultiAirportsByCityDAO* MultiAirportsByCityDAO::_instance = nullptr;

} // namespace tse
