//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MileageDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/Queries/QueryGetMileage.h"

namespace tse
{
log4cxx::LoggerPtr
MileageDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MileageDAO"));
MileageDAO&
MileageDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Mileage*>&
getMileageData(const LocCode& origin,
               const LocCode& destination,
               Indicator mileageType,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical)
{
  //  3/31/08 -- Airports MLH and BSL are the same physical airport in city EAP -- but EAP has no
  // mileage in the DB.
  //  MLH is in France and BSL in Switzerland.
  //  So if the orig or dest is EAP -- use BSL

  const LocCode& originToUse = (origin == "EAP") ? "BSL" : origin;
  const LocCode& destinationToUse = (destination == "EAP") ? "BSL" : destination;

  if (UNLIKELY(isHistorical))
  {
    MileageHistoricalDAO& dao = MileageHistoricalDAO::instance();

    if (originToUse < destinationToUse)
    {
      return dao.get(deleteList, originToUse, destinationToUse, mileageType, ticketDate);
    }
    else
    {
      return dao.get(deleteList, destinationToUse, originToUse, mileageType, ticketDate);
    }
  }
  else
  {
    MileageDAO& dao = MileageDAO::instance();

    if (originToUse < destinationToUse)
    {
      return dao.get(deleteList, originToUse, destinationToUse, mileageType, ticketDate);
    }
    else
    {
      return dao.get(deleteList, destinationToUse, originToUse, mileageType, ticketDate);
    }
  }
}

const std::vector<Mileage*>&
MileageDAO::get(DeleteList& del,
                const LocCode& orig,
                const LocCode& dest,
                Indicator mileageType,
                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MileageKey key(orig, dest, mileageType);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<Mileage*>* ret = new std::vector<Mileage*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<Mileage>(ticketDate,
  ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<Mileage>(ticketDate, ticketDate)));
}

std::vector<Mileage*>*
MileageDAO::create(MileageKey key)
{
  std::vector<Mileage*>* ret = new std::vector<Mileage*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMileage mlg(dbAdapter->getAdapter());
    mlg.findMileage(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MileageDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
MileageDAO::compress(const std::vector<Mileage*>* vect) const
{
  return compressVector(vect);
}

std::vector<Mileage*>*
MileageDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Mileage>(compressed);
}

void
MileageDAO::destroy(MileageKey key, std::vector<Mileage*>* recs)
{
  std::vector<Mileage*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MileageKey
MileageDAO::createKey(Mileage* info)
{
  return MileageKey(info->orig(), info->dest(), info->mileageType());
}

void
MileageDAO::load()
{
  StartupLoaderNoDB<Mileage, MileageDAO>();
}

std::string
MileageDAO::_name("Mileage");
std::string
MileageDAO::_cacheClass("Routing");
DAOHelper<MileageDAO>
MileageDAO::_helper(_name);
MileageDAO* MileageDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MileageHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
MileageHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MileageHistoricalDAO"));
MileageHistoricalDAO&
MileageHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Mileage*>&
MileageHistoricalDAO::get(DeleteList& del,
                          const LocCode& orig,
                          const LocCode& dest,
                          Indicator mileageType,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MileageHistoricalKey key(orig, dest, mileageType);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<Mileage*>* ret = new std::vector<Mileage*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<Mileage>(ticketDate, ticketDate));
  return *ret;
}

std::vector<Mileage*>*
MileageHistoricalDAO::create(MileageHistoricalKey key)
{
  std::vector<Mileage*>* ret = new std::vector<Mileage*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMileageHistorical mlg(dbAdapter->getAdapter());
    mlg.findMileage(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MileageHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MileageHistoricalDAO::destroy(MileageHistoricalKey key, std::vector<Mileage*>* recs)
{
  std::vector<Mileage*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MileageHistoricalKey
MileageHistoricalDAO::createKey(Mileage* info, const DateTime& startDate, const DateTime& endDate)
{
  return MileageHistoricalKey(info->orig(), info->dest(), info->mileageType(), startDate, endDate);
}

sfc::CompressedData*
MileageHistoricalDAO::compress(const std::vector<Mileage*>* vect) const
{
  return compressVector(vect);
}

std::vector<Mileage*>*
MileageHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Mileage>(compressed);
}

void
MileageHistoricalDAO::load()
{
  StartupLoaderNoDB<Mileage, MileageHistoricalDAO>();
}

std::string
MileageHistoricalDAO::_name("MileageHistorical");
std::string
MileageHistoricalDAO::_cacheClass("Routing");
DAOHelper<MileageHistoricalDAO>
MileageHistoricalDAO::_helper(_name);
MileageHistoricalDAO* MileageHistoricalDAO::_instance = nullptr;

} // namespace tse
