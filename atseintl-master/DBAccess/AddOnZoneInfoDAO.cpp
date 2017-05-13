//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/AddOnZoneInfoDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetAddonZones.h"

namespace tse
{
log4cxx::LoggerPtr
AddOnZoneInfoDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnZoneInfoDAO"));

AddOnZoneInfoDAO&
AddOnZoneInfoDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonZoneInfo*>&
getAddOnZoneData(const VendorCode& vendor,
                 const CarrierCode& carrier,
                 const TariffNumber& fareTariff,
                 const AddonZone& zone,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (isHistorical)
  {
    AddOnZoneInfoHistoricalDAO& dao = AddOnZoneInfoHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, fareTariff, zone, date, ticketDate);
  }
  else
  {
    AddOnZoneInfoDAO& dao = AddOnZoneInfoDAO::instance();
    return dao.get(deleteList, vendor, carrier, fareTariff, zone, date, ticketDate);
  }
}

const std::vector<AddonZoneInfo*>&
AddOnZoneInfoDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& fareTariff,
                      const AddonZone& zone,
                      const DateTime& date,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnZoneInfoKey key(vendor, carrier, fareTariff, zone);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<AddonZoneInfo>(date,
  ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<AddonZoneInfo>(date, ticketDate)));
}

std::vector<AddonZoneInfo*>*
AddOnZoneInfoDAO::create(AddOnZoneInfoKey key)
{
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAddonZoneInfo aoz(dbAdapter->getAdapter());
    aoz.findAddonZoneInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnZoneInfoDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnZoneInfoDAO::destroy(AddOnZoneInfoKey key, std::vector<AddonZoneInfo*>* recs)
{
  std::vector<AddonZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
AddOnZoneInfoDAO::_name("AddOnZoneInfo");
std::string
AddOnZoneInfoDAO::_cacheClass("Fares");
DAOHelper<AddOnZoneInfoDAO>
AddOnZoneInfoDAO::_helper(_name);
AddOnZoneInfoDAO* AddOnZoneInfoDAO::_instance = nullptr;

AddOnZoneInfoKey
AddOnZoneInfoDAO::createKey(const AddonZoneInfo* info)
{
  return AddOnZoneInfoKey(info->vendor(), info->carrier(), info->fareTariff(), info->zone());
}

void
AddOnZoneInfoDAO::load()
{
  StartupLoaderNoDB<AddonZoneInfo, AddOnZoneInfoDAO>();
}

// --------------------------------------------------
// Historical DAO: AddOnZoneInfoHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
AddOnZoneInfoHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnZoneInfoHistoricalDAO"));

AddOnZoneInfoHistoricalDAO&
AddOnZoneInfoHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonZoneInfo*>&
AddOnZoneInfoHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const TariffNumber& fareTariff,
                                const AddonZone& zone,
                                const DateTime& date,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnZoneInfoHistoricalKey key(vendor, carrier, fareTariff, zone);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<AddonZoneInfo>(date, ticketDate));
  return *ret;
}

std::vector<AddonZoneInfo*>*
AddOnZoneInfoHistoricalDAO::create(AddOnZoneInfoHistoricalKey key)
{
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAddonZoneInfoHistorical aoz(dbAdapter->getAdapter());
    aoz.findAddonZoneInfo(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnZoneInfoHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnZoneInfoHistoricalDAO::destroy(AddOnZoneInfoHistoricalKey key,
                                    std::vector<AddonZoneInfo*>* recs)
{
  std::vector<AddonZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
AddOnZoneInfoHistoricalDAO::_name("AddOnZoneInfoHistorical");
std::string
AddOnZoneInfoHistoricalDAO::_cacheClass("Fares");
DAOHelper<AddOnZoneInfoHistoricalDAO>
AddOnZoneInfoHistoricalDAO::_helper(_name);
AddOnZoneInfoHistoricalDAO* AddOnZoneInfoHistoricalDAO::_instance = nullptr;

} // namespace tse
