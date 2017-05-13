//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/AddOnZoneDAO.h"

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
AddOnZoneDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnZoneDAO"));

AddOnZoneDAO&
AddOnZoneDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct AddOnZoneDAO::isEffective : public std::unary_function<const AddonZoneInfo*, bool>
{
  const DateTime _date;
  const bool _anyDate;

  isEffective() : _date(DateTime::localTime()), _anyDate(true) {}

  isEffective(const DateTime& date) : _date(date), _anyDate(false) {}

  bool operator()(const AddonZoneInfo* rec) const
  {
    return (_date <= rec->expireDate()) && (_anyDate || rec->effDate() <= _date);
  }
};

const std::vector<AddonZoneInfo*>&
getAddOnZoneData(const VendorCode& vendor,
                 const CarrierCode& carrier,
                 const LocCode& market,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    AddOnZoneHistoricalDAO& dao = AddOnZoneHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, market, date, ticketDate);
  }
  else
  {
    AddOnZoneDAO& dao = AddOnZoneDAO::instance();
    return dao.get(deleteList, vendor, carrier, market, date, ticketDate);
  }
}

const std::vector<AddonZoneInfo*>&
AddOnZoneDAO::get(DeleteList& del,
                  const VendorCode& vendor,
                  const CarrierCode& carrier,
                  const LocCode& market,
                  const DateTime& date,
                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnZoneKey key(vendor, carrier, market);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(date.isEmptyDate()))
  {
    return *(applyFilter(del, ptr, not1(isEffective())));
  }
  else
  {
    return *(applyFilter(del, ptr, not1(isEffective(date))));
  }
}

AddOnZoneKey
AddOnZoneDAO::createKey(AddonZoneInfo* info)
{
  return AddOnZoneKey(info->vendor(), info->carrier(), info->market().loc());
}

void
AddOnZoneDAO::load()
{
  StartupLoader<QueryGetAllAddonZones, AddonZoneInfo, AddOnZoneDAO>();
}

std::vector<AddonZoneInfo*>*
AddOnZoneDAO::create(AddOnZoneKey key)
{
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAddonZones aoz(dbAdapter->getAdapter());
    aoz.findAddonZoneInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnZoneDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnZoneDAO::destroy(AddOnZoneKey key, std::vector<AddonZoneInfo*>* recs)
{
  std::vector<AddonZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
AddOnZoneDAO::compress(const std::vector<AddonZoneInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AddonZoneInfo*>*
AddOnZoneDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AddonZoneInfo>(compressed);
}

std::string
AddOnZoneDAO::_name("AddOnZone");
std::string
AddOnZoneDAO::_cacheClass("Fares");

DAOHelper<AddOnZoneDAO>
AddOnZoneDAO::_helper(_name);

AddOnZoneDAO* AddOnZoneDAO::_instance = nullptr;

// Historical Stuff ///////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
AddOnZoneHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnZoneHistoricalDAO"));

AddOnZoneHistoricalDAO&
AddOnZoneHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonZoneInfo*>&
AddOnZoneHistoricalDAO::get(DeleteList& del,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const LocCode& market,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnZoneHistoricalKey key(vendor, carrier, market);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
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
AddOnZoneHistoricalDAO::create(AddOnZoneHistoricalKey key)
{
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetAddonZonesHistorical aoz(dbAdapter->getAdapter());
    aoz.findAddonZoneInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnZoneHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnZoneHistoricalDAO::destroy(AddOnZoneHistoricalKey key, std::vector<AddonZoneInfo*>* recs)
{
  std::vector<AddonZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
AddOnZoneHistoricalDAO::compress(const std::vector<AddonZoneInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AddonZoneInfo*>*
AddOnZoneHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AddonZoneInfo>(compressed);
}

std::string
AddOnZoneHistoricalDAO::_name("AddOnZoneHistorical");
std::string
AddOnZoneHistoricalDAO::_cacheClass("Fares");
DAOHelper<AddOnZoneHistoricalDAO>
AddOnZoneHistoricalDAO::_helper(_name);
AddOnZoneHistoricalDAO* AddOnZoneHistoricalDAO::_instance = nullptr;

} // namespace tse
