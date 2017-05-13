//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/SeatCabinCharacteristicDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSeatCabinCharacteristic.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"

#include <algorithm>

namespace tse
{

log4cxx::LoggerPtr
SeatCabinCharacteristicDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SeatCabinCharacteristicDAO"));
SeatCabinCharacteristicDAO&
SeatCabinCharacteristicDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SeatCabinCharacteristicInfo*>&
getSeatCabinCharacteristicData(const CarrierCode& carrier,
                               const Indicator& codeType,
                               const DateTime& travelDate,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical)
{
  if (isHistorical)
  {
    SeatCabinCharacteristicHistoricalDAO& dao = SeatCabinCharacteristicHistoricalDAO::instance();
    const std::vector<SeatCabinCharacteristicInfo*>& ret =
        dao.get(deleteList, carrier, codeType, travelDate, ticketDate);
    return ret;
  }
  else
  {
    SeatCabinCharacteristicDAO& dao = SeatCabinCharacteristicDAO::instance();
    const std::vector<SeatCabinCharacteristicInfo*>& ret =
        dao.get(deleteList, carrier, codeType, travelDate, ticketDate);
    return ret;
  }
}

struct SeatCabinCharacteristicDAO::isTravelDate
    : public std::unary_function<SeatCabinCharacteristicInfo*, bool>
{
  const DateTime _date;

  isTravelDate(const DateTime& travelDate) : _date(travelDate) {}

  bool operator()(const SeatCabinCharacteristicInfo* rec) const
  {
    return (rec->createDate() <= _date) && (_date <= rec->expireDate());
  }
};

const std::vector<SeatCabinCharacteristicInfo*>&
SeatCabinCharacteristicDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const Indicator& codeType,
                                const DateTime& travelDate,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SeatCabinCharacteristicKey key(carrier, codeType);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, not1(isTravelDate(travelDate))));
}

std::vector<SeatCabinCharacteristicInfo*>*
SeatCabinCharacteristicDAO::create(SeatCabinCharacteristicKey key)
{
  std::vector<SeatCabinCharacteristicInfo*>* ret = new std::vector<SeatCabinCharacteristicInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSeatCabinCharacteristic scc(dbAdapter->getAdapter());
    scc.findSeatCabinCharacteristic(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SeatCabinCharacteristicDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

SeatCabinCharacteristicKey
SeatCabinCharacteristicDAO::createKey(const SeatCabinCharacteristicInfo* info)
{
  return SeatCabinCharacteristicKey(info->carrier(), info->codeType());
}

bool
SeatCabinCharacteristicDAO::translateKey(const ObjectKey& objectKey,
                                         SeatCabinCharacteristicKey& key) const
{
  return key.initialized =
             objectKey.getValue("CARRIER", key._a) && objectKey.getValue("CODETYPE", key._b);
}

void
SeatCabinCharacteristicDAO::translateKey(const SeatCabinCharacteristicKey& key,
                                         ObjectKey& objectKey) const
{
  objectKey.setValue("CARRIER", key._a);
  objectKey.setValue("CODETYPE", key._b);
}

bool
SeatCabinCharacteristicDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<SeatCabinCharacteristicInfo, SeatCabinCharacteristicDAO>(
             flatKey, objectKey).success();
}

void
SeatCabinCharacteristicDAO::destroy(SeatCabinCharacteristicKey key,
                                    std::vector<SeatCabinCharacteristicInfo*>* sccInfoList)
{
  std::vector<SeatCabinCharacteristicInfo*>::iterator i;
  for (i = sccInfoList->begin(); i != sccInfoList->end(); ++i)
  {
    delete *i;
  }

  delete sccInfoList;
}

void
SeatCabinCharacteristicDAO::load()
{
  StartupLoaderNoDB<SeatCabinCharacteristicInfo, SeatCabinCharacteristicDAO>();
}

size_t
SeatCabinCharacteristicDAO::clear()
{
  size_t result(cache().clear());
  LOG4CXX_ERROR(_logger, "SeatCabinCharacteristic cache cleared");
  return result;
}

sfc::CompressedData*
SeatCabinCharacteristicDAO::compress(const std::vector<SeatCabinCharacteristicInfo*>* sccInfoList)
    const
{
  return compressVector(sccInfoList);
}

std::vector<SeatCabinCharacteristicInfo*>*
SeatCabinCharacteristicDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SeatCabinCharacteristicInfo>(compressed);
}

std::string
SeatCabinCharacteristicDAO::_name("SeatCabinCharacteristic");
std::string
SeatCabinCharacteristicDAO::_cacheClass("Fares");
DAOHelper<SeatCabinCharacteristicDAO>
SeatCabinCharacteristicDAO::_helper(_name);
SeatCabinCharacteristicDAO* SeatCabinCharacteristicDAO::_instance = nullptr;

////////////////////////////////////////////////////////
// Historical DAO Object
////////////////////////////////////////////////////////
log4cxx::LoggerPtr
SeatCabinCharacteristicHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SeatCabinCharacteristicHistoricalDAO"));
SeatCabinCharacteristicHistoricalDAO&
SeatCabinCharacteristicHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct SeatCabinCharacteristicHistoricalDAO::isTravelDate
    : public std::unary_function<SeatCabinCharacteristicInfo*, bool>
{
  const DateTime _date;

  isTravelDate(const DateTime& travelDate) : _date(travelDate) {}

  bool operator()(const SeatCabinCharacteristicInfo* rec) const
  {
    return (rec->createDate() <= _date) && (_date <= rec->expireDate());
  }
};

const std::vector<SeatCabinCharacteristicInfo*>&
SeatCabinCharacteristicHistoricalDAO::get(DeleteList& del,
                                          const CarrierCode& carrier,
                                          const Indicator& codeType,
                                          const DateTime& travelDate,
                                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SeatCabinCharacteristicHistoricalKey key(carrier, codeType);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  std::vector<SeatCabinCharacteristicInfo*>* ret = new std::vector<SeatCabinCharacteristicInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(isTravelDate(travelDate)));
  return *ret;
}

std::vector<SeatCabinCharacteristicInfo*>*
SeatCabinCharacteristicHistoricalDAO::create(SeatCabinCharacteristicHistoricalKey key)
{
  std::vector<SeatCabinCharacteristicInfo*>* ret = new std::vector<SeatCabinCharacteristicInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetSeatCabinCharacteristicHistorical scc(dbAdapter->getAdapter());
    scc.findSeatCabinCharacteristic(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SeatCabinCharacteristicHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

bool
SeatCabinCharacteristicHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                   SeatCabinCharacteristicHistoricalKey& key) const
{
  return key.initialized =
             objectKey.getValue("CARRIER", key._a) && objectKey.getValue("CODETYPE", key._b);
}

bool
SeatCabinCharacteristicHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                   SeatCabinCharacteristicHistoricalKey& key,
                                                   const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  return key.initialized =
             objectKey.getValue("CARRIER", key._a) && objectKey.getValue("CODETYPE", key._b);
}

void
SeatCabinCharacteristicHistoricalDAO::translateKey(const SeatCabinCharacteristicHistoricalKey& key,
                                                   ObjectKey& objectKey) const
{
  objectKey.setValue("CARRIER", key._a);
  objectKey.setValue("CODETYPE", key._b);
}

void
SeatCabinCharacteristicHistoricalDAO::setKeyDateRange(SeatCabinCharacteristicHistoricalKey& key,
                                                      const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
}

SeatCabinCharacteristicHistoricalKey
SeatCabinCharacteristicHistoricalDAO::createKey(const SeatCabinCharacteristicInfo* info,
                                                const DateTime& startDate,
                                                const DateTime& endDate)
{
  return SeatCabinCharacteristicHistoricalKey(
      info->carrier(), info->codeType(), startDate, endDate);
}

bool
SeatCabinCharacteristicHistoricalDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserterWithDateRange<SeatCabinCharacteristicInfo,
                                          SeatCabinCharacteristicHistoricalDAO>(flatKey, objectKey)
      .success();
}

void
SeatCabinCharacteristicHistoricalDAO::destroy(
    SeatCabinCharacteristicHistoricalKey key,
    std::vector<SeatCabinCharacteristicInfo*>* sccInfoList)
{
  std::vector<SeatCabinCharacteristicInfo*>::iterator i;
  for (i = sccInfoList->begin(); i != sccInfoList->end(); ++i)
  {
    delete *i;
  }

  delete sccInfoList;
}

void
SeatCabinCharacteristicHistoricalDAO::load()
{
  StartupLoaderNoDB<SeatCabinCharacteristicInfo, SeatCabinCharacteristicHistoricalDAO>();
}

sfc::CompressedData*
SeatCabinCharacteristicHistoricalDAO::compress(
    const std::vector<SeatCabinCharacteristicInfo*>* sccInfoList) const
{
  return compressVector(sccInfoList);
}

std::vector<SeatCabinCharacteristicInfo*>*
SeatCabinCharacteristicHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SeatCabinCharacteristicInfo>(compressed);
}

std::string
SeatCabinCharacteristicHistoricalDAO::_name("SeatCabinCharacteristicHistorical");
std::string
SeatCabinCharacteristicHistoricalDAO::_cacheClass("Fares");
DAOHelper<SeatCabinCharacteristicHistoricalDAO>
SeatCabinCharacteristicHistoricalDAO::_helper(_name);
SeatCabinCharacteristicHistoricalDAO* SeatCabinCharacteristicHistoricalDAO::_instance = nullptr;

} // namespace tse
