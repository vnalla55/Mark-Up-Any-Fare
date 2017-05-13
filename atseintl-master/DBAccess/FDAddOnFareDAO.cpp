//----------------------------------------------------------------------------
//  File:   FDAddOnFareDAO.cpp
//  Created: May 23, 2005
//  Authors: Partha Kumar Chakraborti
//
//  Description:
//
//  Change History:
//    date - initials - description
//
//  Copyright Sabre 2005
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//----------------------------------------------------------------------------
#include "DBAccess/FDAddOnFareDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetAddonFares.h"
#include "DBAccess/TariffCrossRefDAO.h"
#include "DBAccess/TariffCrossRefInfo.h"

namespace tse
{
log4cxx::LoggerPtr
FDAddOnFareDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDAddOnFareDAO"));

FDAddOnFareDAO&
FDAddOnFareDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonFareInfo*>&
getAddOnFareData(const LocCode& gatewayMarket,
                 const LocCode& interiorMarket,
                 const CarrierCode& carrier,
                 const RecordScope& crossRefType,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (isHistorical)
  {
    FDAddOnFareHistoricalDAO& dao = FDAddOnFareHistoricalDAO::instance();
    return dao.get(
        deleteList, gatewayMarket, interiorMarket, carrier, crossRefType, date, ticketDate);
  }
  else
  {
    FDAddOnFareDAO& dao = FDAddOnFareDAO::instance();
    return dao.get(
        deleteList, gatewayMarket, interiorMarket, carrier, crossRefType, date, ticketDate);
  }
}

const std::vector<AddonFareInfo*>&
FDAddOnFareDAO::get(DeleteList& del,
                    const LocCode& gatewayMarket,
                    const LocCode& interiorMarket,
                    const CarrierCode& carrier,
                    const RecordScope& crossRefType,
                    const DateTime& date,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDAddOnFareKey key(gatewayMarket, interiorMarket, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;
  del.adopt(ret);

  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<AddonFareInfo>(date, ticketDate));
  while (i != ptr->end())
  {
    if (NotInhibitForFD<AddonFareInfo>(*i))
    {
      ret->push_back(*i);
    }
    i = find_if(++i, ptr->end(), IsEffectiveG<AddonFareInfo>(date, ticketDate));
  }

  getGlobalDirFrmTariffXRef(del, *ret, crossRefType, ticketDate);
  return *ret;
}

void
FDAddOnFareDAO::getGlobalDirFrmTariffXRef(DeleteList& del,
                                          std::vector<AddonFareInfo*>& addOnFareInfoList,
                                          const RecordScope& crossRefType,
                                          const DateTime& ticketDate)
{
  std::vector<AddonFareInfo*>::iterator iB = addOnFareInfoList.begin();
  std::vector<AddonFareInfo*>::iterator iE = addOnFareInfoList.end();

  for (; iB != iE; iB++)
  {
    if ((*iB)->vendor().equalToConst("SITA"))
      continue;

    const std::vector<TariffCrossRefInfo*>& tariffCrossRefInfoList =
        getTariffXRefData((*iB)->vendor(), (*iB)->carrier(), INTERNATIONAL, del, ticketDate, false);

    std::vector<TariffCrossRefInfo*>::const_iterator iTB = tariffCrossRefInfoList.begin();
    std::vector<TariffCrossRefInfo*>::const_iterator iTE = tariffCrossRefInfoList.end();

    TariffNumber& addOnTariff = (*iB)->addonTariff();

    for (; iTB != iTE; iTB++)
    {
      const TariffNumber& tarXRef1 = (*iTB)->addonTariff1();
      const TariffNumber& tarXRef2 = (*iTB)->addonTariff2();

      if (tarXRef1 != -1 && tarXRef1 == addOnTariff)
      {
        (*iB)->globalDir() = (*iTB)->globalDirection();
        break;
      }
      else if (tarXRef2 != -1 && tarXRef2 == addOnTariff)
      {
        (*iB)->globalDir() = (*iTB)->globalDirection();
        break;
      }
    }
  }
}

std::vector<AddonFareInfo*>*
FDAddOnFareDAO::create(FDAddOnFareKey key)
{
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;
  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAddonFaresGW aof(dbAdapter->getAdapter());
    aof.findAddonFareInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDAddOnFareDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FDAddOnFareDAO::destroy(FDAddOnFareKey key, std::vector<AddonFareInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FDAddOnFareDAO::_name("FDAddOnFare");
std::string
FDAddOnFareDAO::_cacheClass("FareDisplay");
DAOHelper<FDAddOnFareDAO>
FDAddOnFareDAO::_helper(_name);
FDAddOnFareDAO* FDAddOnFareDAO::_instance = nullptr;

FDAddOnFareKey
FDAddOnFareDAO::createKey(const AddonFareInfo* info)
{
  return FDAddOnFareKey(info->gatewayMarket(), info->interiorMarket(), info->carrier());
}

void
FDAddOnFareDAO::load()
{
  StartupLoaderNoDB<AddonFareInfo, FDAddOnFareDAO>();
}

///////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////
log4cxx::LoggerPtr
FDAddOnFareHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FDAddOnFareHistoricalDAO"));

FDAddOnFareHistoricalDAO&
FDAddOnFareHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonFareInfo*>&
FDAddOnFareHistoricalDAO::get(DeleteList& del,
                              const LocCode& gatewayMarket,
                              const LocCode& interiorMarket,
                              const CarrierCode& carrier,
                              const RecordScope& crossRefType,
                              const DateTime& date,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDAddOnFareHistoricalKey key(gatewayMarket, interiorMarket, carrier);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;
  del.adopt(ret);

  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<AddonFareInfo>(date, ticketDate));
  while (i != ptr->end())
  {
    if (NotInhibitForFD<AddonFareInfo>(*i))
    {
      ret->push_back(*i);
    }
    i = find_if(++i, ptr->end(), IsEffectiveHist<AddonFareInfo>(date, ticketDate));
  }

  getGlobalDirFrmTariffXRef(del, *ret, crossRefType, ticketDate);
  return *ret;
}

void
FDAddOnFareHistoricalDAO::getGlobalDirFrmTariffXRef(DeleteList& del,
                                                    std::vector<AddonFareInfo*>& addOnFareInfoList,
                                                    const RecordScope& crossRefType,
                                                    const DateTime& ticketDate)
{
  std::vector<AddonFareInfo*>::iterator iB = addOnFareInfoList.begin();
  std::vector<AddonFareInfo*>::iterator iE = addOnFareInfoList.end();

  for (; iB != iE; iB++)
  {
    const std::vector<TariffCrossRefInfo*>& tariffCrossRefInfoList =
        getTariffXRefData((*iB)->vendor(), (*iB)->carrier(), INTERNATIONAL, del, ticketDate, true);

    std::vector<TariffCrossRefInfo*>::const_iterator iTB = tariffCrossRefInfoList.begin();
    std::vector<TariffCrossRefInfo*>::const_iterator iTE = tariffCrossRefInfoList.end();

    TariffNumber& addOnTariff = (*iB)->addonTariff();

    for (; iTB != iTE; iTB++)
    {
      const TariffNumber& tarXRef1 = (*iTB)->addonTariff1();
      const TariffNumber& tarXRef2 = (*iTB)->addonTariff2();

      if (tarXRef1 != -1 && tarXRef1 == addOnTariff)
        (*iB)->globalDir() = (*iTB)->globalDirection();

      else if (tarXRef2 != -1 && tarXRef2 == addOnTariff)
        (*iB)->globalDir() = (*iTB)->globalDirection();
    }
  }
}

std::vector<AddonFareInfo*>*
FDAddOnFareHistoricalDAO::create(FDAddOnFareHistoricalKey key)
{
  std::vector<AddonFareInfo*>* ret = new std::vector<AddonFareInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetAddonFaresGWHistorical afg(dbAdapter->getAdapter());
    afg.findAddonFareInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDAddOnFareHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FDAddOnFareHistoricalDAO::destroy(FDAddOnFareHistoricalKey key, std::vector<AddonFareInfo*>* recs)
{
  std::vector<AddonFareInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
FDAddOnFareHistoricalDAO::_name("FDAddOnFareHistorical");
std::string
FDAddOnFareHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FDAddOnFareHistoricalDAO>
FDAddOnFareHistoricalDAO::_helper(_name);
FDAddOnFareHistoricalDAO* FDAddOnFareHistoricalDAO::_instance = nullptr;

} // namespace tse
