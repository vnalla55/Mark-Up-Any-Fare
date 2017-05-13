//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/AddOnFareDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetAddonFares.h"

namespace tse
{
log4cxx::LoggerPtr
AddOnFareDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnFareDAO"));

AddOnFareDAO&
AddOnFareDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct AddOnFareDAO::isEffective : public std::unary_function<const AddonFareInfo*, bool>
{
  DateTime _date;
  bool _checkEffDate;
  bool _isFareDisplay;

  isEffective(const DateTime& date, bool isFareDisplay)
    : _date(date), _checkEffDate(true), _isFareDisplay(isFareDisplay)
  {
    if (LIKELY(date.isEmptyDate()))
    {
      _date = DateTime::localTime();
      _checkEffDate = false;
    }
  }

  bool operator()(const AddonFareInfo* rec) const
  {
    if (_isFareDisplay)
    {
      if (InhibitForFD<AddonFareInfo>(rec))
        return false;
    }
    else
    {
      if (Inhibit<AddonFareInfo>(rec))
        return false;
    }

    return (_date <= rec->expireDate()) && (!_checkEffDate || rec->effDate() <= _date);
  }
};

const std::vector<AddonFareInfo*>&
getAddOnFareData(const LocCode& interiorMarket,
                 const CarrierCode& carrier,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical,
                 bool isFareDisplay)
{
  if (UNLIKELY(isHistorical))
  {
    AddOnFareHistoricalDAO& dao = AddOnFareHistoricalDAO::instance();
    return dao.get(deleteList, interiorMarket, carrier, date, ticketDate, isFareDisplay);
  }
  else
  {
    AddOnFareDAO& dao = AddOnFareDAO::instance();
    return dao.get(deleteList, interiorMarket, carrier, date, ticketDate, isFareDisplay);
  }
}

const std::vector<AddonFareInfo*>&
AddOnFareDAO::get(DeleteList& del,
                  const LocCode& interiorMarket,
                  const CarrierCode& carrier,
                  const DateTime& date,
                  const DateTime& ticketDate,
                  bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnFareKey key(interiorMarket, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(isEffective(date,
  isFareDisplay)));
  return *ret;
  */
  return *(applyFilter(del, ptr, not1(isEffective(date, isFareDisplay))));
}

std::vector<AddonFareInfo*>*
AddOnFareDAO::create(AddOnFareKey key)
{
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;
  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAddonFares aof(dbAdapter->getAdapter());
    aof.findAddonFareInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnFareDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnFareDAO::destroy(AddOnFareKey key, std::vector<AddonFareInfo*>* recs)
{
  std::vector<AddonFareInfo*>::const_iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

AddOnFareKey
AddOnFareDAO::createKey(AddonFareInfo* info)
{
  return AddOnFareKey(info->interiorMarket(), info->carrier());
}

void
AddOnFareDAO::load()
{
  StartupLoaderNoDB<AddonFareInfo, AddOnFareDAO>();
}

sfc::CompressedData*
AddOnFareDAO::compress(const std::vector<AddonFareInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AddonFareInfo*>*
AddOnFareDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AddonFareInfo>(compressed);
}

std::string
AddOnFareDAO::_name("AddOnFare");
std::string
AddOnFareDAO::_cacheClass("Fares");
DAOHelper<AddOnFareDAO>
AddOnFareDAO::_helper(_name);
AddOnFareDAO* AddOnFareDAO::_instance = nullptr;

// Historical Stuff //////////////////////////////////////////////////////////
log4cxx::LoggerPtr
AddOnFareHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnFareHistoricalDAO"));

AddOnFareHistoricalDAO&
AddOnFareHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonFareInfo*>&
AddOnFareHistoricalDAO::get(DeleteList& del,
                            const LocCode& interiorMarket,
                            const CarrierCode& carrier,
                            const DateTime& date,
                            const DateTime& ticketDate,
                            bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnFareHistoricalKey key(interiorMarket, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;
  del.adopt(ret);

  if (date.isEmptyDate())
    remove_copy_if(
        ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<AddonFareInfo>(ticketDate));
  else
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveHist<AddonFareInfo>(date, ticketDate));

  if (isFareDisplay)
    ret->erase(std::remove_if(ret->begin(), ret->end(), InhibitForFD<AddonFareInfo>), ret->end());
  else
    ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<AddonFareInfo>), ret->end());

  return *ret;
}

std::vector<AddonFareInfo*>*
AddOnFareHistoricalDAO::create(AddOnFareHistoricalKey key)
{
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetAddonFaresHistorical afh(dbAdapter->getAdapter());
    afh.findAddonFareInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnFareHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnFareHistoricalDAO::destroy(AddOnFareHistoricalKey key, std::vector<AddonFareInfo*>* recs)
{
  std::vector<AddonFareInfo*>::const_iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
AddOnFareHistoricalDAO::compress(const std::vector<AddonFareInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AddonFareInfo*>*
AddOnFareHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AddonFareInfo>(compressed);
}

std::string
AddOnFareHistoricalDAO::_name("AddOnFareHistorical");
std::string
AddOnFareHistoricalDAO::_cacheClass("Fares");
DAOHelper<AddOnFareHistoricalDAO>
AddOnFareHistoricalDAO::_helper(_name);
AddOnFareHistoricalDAO* AddOnFareHistoricalDAO::_instance = nullptr;

} // namespace tse
