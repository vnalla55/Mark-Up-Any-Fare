//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/AddonZoneSITADAO.h"

#include "Common/Logger.h"
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryCheckLocsAndZones.h"

namespace tse
{
log4cxx::LoggerPtr
AddonZoneSITADAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.AddonZoneSITADAO"));

AddonZoneSITADAO&
AddonZoneSITADAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonZoneInfo*>&
getAddonZoneSITAData(const LocCode& loc,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    AddonZoneSITAHistoricalDAO& dao = AddonZoneSITAHistoricalDAO::instance();
    return dao.get(deleteList, loc, vendor, carrier, ticketDate);
  }
  else
  {
    AddonZoneSITADAO& dao = AddonZoneSITADAO::instance();
    return dao.get(deleteList, loc, vendor, carrier, ticketDate);
  }
}

const std::vector<AddonZoneInfo*>&
AddonZoneSITADAO::get(DeleteList& del,
                      const LocCode& loc,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddonZoneSITAKey key(loc, vendor, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<AddonZoneInfo*>*
AddonZoneSITADAO::create(AddonZoneSITAKey key)
{
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSITAAddonScore sas(dbAdapter->getAdapter());
    sas.evaluateAddonZoneSITA(key._a, key._b, key._c, *ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddonZoneSITADAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddonZoneSITADAO::destroy(AddonZoneSITAKey key, std::vector<AddonZoneInfo*>* recs)
{
  std::vector<AddonZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
AddonZoneSITADAO::_name("AddonZoneSITA");
std::string
AddonZoneSITADAO::_cacheClass("Fares");
DAOHelper<AddonZoneSITADAO>
AddonZoneSITADAO::_helper(_name);
AddonZoneSITADAO* AddonZoneSITADAO::_instance = nullptr;

AddonZoneSITAKey
AddonZoneSITADAO::createKey(const AddonZoneInfo* info)
{
  return AddonZoneSITAKey(info->market().loc(), info->vendor(), info->carrier());
}

void
AddonZoneSITADAO::load()
{
  StartupLoaderNoDB<AddonZoneInfo, AddonZoneSITADAO>();
}

sfc::CompressedData*
AddonZoneSITADAO::compress(const std::vector<AddonZoneInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AddonZoneInfo*>*
AddonZoneSITADAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AddonZoneInfo>(compressed);
}

// Historical Stuff /////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
AddonZoneSITAHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddonZoneSITAHistoricalDAO"));

AddonZoneSITAHistoricalDAO&
AddonZoneSITAHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonZoneInfo*>&
AddonZoneSITAHistoricalDAO::get(DeleteList& del,
                                const LocCode& loc,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddonZoneSITAHistoricalKey key(loc, vendor, carrier);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<AddonZoneInfo>(ticketDate));
  return *ret;
}

std::vector<AddonZoneInfo*>*
AddonZoneSITAHistoricalDAO::create(AddonZoneSITAHistoricalKey key)
{
  std::vector<AddonZoneInfo*>* ret = new std::vector<AddonZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetSITAAddonScoreHistorical sas(dbAdapter->getAdapter());
    sas.evaluateAddonZoneSITA(key._a, key._b, key._c, *ret, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddonZoneSITAHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddonZoneSITAHistoricalDAO::destroy(AddonZoneSITAHistoricalKey key,
                                    std::vector<AddonZoneInfo*>* recs)
{
  std::vector<AddonZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
AddonZoneSITAHistoricalDAO::compress(const std::vector<AddonZoneInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AddonZoneInfo*>*
AddonZoneSITAHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AddonZoneInfo>(compressed);
}

std::string
AddonZoneSITAHistoricalDAO::_name("AddonZoneSITAHistorical");
std::string
AddonZoneSITAHistoricalDAO::_cacheClass("Fares");
DAOHelper<AddonZoneSITAHistoricalDAO>
AddonZoneSITAHistoricalDAO::_helper(_name);
AddonZoneSITAHistoricalDAO* AddonZoneSITAHistoricalDAO::_instance = nullptr;

} // namespace tse
