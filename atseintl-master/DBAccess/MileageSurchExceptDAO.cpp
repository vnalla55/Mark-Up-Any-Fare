//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MileageSurchExceptDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MileageSurchExcept.h"
#include "DBAccess/Queries/QueryGetMileageSurchExcept.h"

namespace tse
{
log4cxx::LoggerPtr
MileageSurchExceptDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MileageSurchExceptDAO"));

MileageSurchExceptDAO&
MileageSurchExceptDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MileageSurchExcept*>&
getMileageSurchExceptData(const VendorCode& vendor,
                          int textTblItemNo,
                          const CarrierCode& governingCarrier,
                          const TariffNumber& ruleTariff,
                          const RuleNumber& rule,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MileageSurchExceptHistoricalDAO& dao = MileageSurchExceptHistoricalDAO::instance();
    return dao.get(
        deleteList, vendor, textTblItemNo, governingCarrier, ruleTariff, rule, date, ticketDate);
  }
  else
  {
    MileageSurchExceptDAO& dao = MileageSurchExceptDAO::instance();
    return dao.get(
        deleteList, vendor, textTblItemNo, governingCarrier, ruleTariff, rule, date, ticketDate);
  }
}

const std::vector<MileageSurchExcept*>&
MileageSurchExceptDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int textTblItemNo,
                           const CarrierCode& governingCarrier,
                           const TariffNumber& ruleTariff,
                           const RuleNumber& rule,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MileageSurchExceptKey key(vendor, textTblItemNo, governingCarrier, ruleTariff, rule);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<MileageSurchExcept*>* ret = new std::vector<MileageSurchExcept*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<MileageSurchExcept>(date, ticketDate));
  if (!governingCarrier.empty())
  {
    MileageSurchExceptKey key2(vendor, textTblItemNo, "", -1, "");
    ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveG<MileageSurchExcept>(date, ticketDate));
  }
  if (textTblItemNo != 0)
  {
    MileageSurchExceptKey key3(vendor, 0, governingCarrier, ruleTariff, rule);
    ptr = cache().get(key3);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveG<MileageSurchExcept>(date, ticketDate));
  }

  return *ret;
}

MileageSurchExceptKey
MileageSurchExceptDAO::createKey(MileageSurchExcept* info)
{
  return MileageSurchExceptKey(info->vendor(),
                               info->textTblItemNo(),
                               info->governingCarrier(),
                               info->ruleTariff(),
                               info->rule());
}

void
MileageSurchExceptDAO::load()
{
  StartupLoader<QueryGetAllMileageSurchExceptBase, MileageSurchExcept, MileageSurchExceptDAO>();
}

std::vector<MileageSurchExcept*>*
MileageSurchExceptDAO::create(MileageSurchExceptKey key)
{
  std::vector<MileageSurchExcept*>* ret = new std::vector<MileageSurchExcept*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneMileageSurchExceptBase mseb(dbAdapter->getAdapter());
    mseb.findMileageSurchExcept(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MileageSurchExceptDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
MileageSurchExceptDAO::destroy(MileageSurchExceptKey key, std::vector<MileageSurchExcept*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MileageSurchExceptDAO::compress(const std::vector<MileageSurchExcept*>* vect) const
{
  return compressVector(vect);
}

std::vector<MileageSurchExcept*>*
MileageSurchExceptDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MileageSurchExcept>(compressed);
}

std::string
MileageSurchExceptDAO::_name("MileageSurchExcept");
std::string
MileageSurchExceptDAO::_cacheClass("BookingCode");
DAOHelper<MileageSurchExceptDAO>
MileageSurchExceptDAO::_helper(_name);
MileageSurchExceptDAO* MileageSurchExceptDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MileageSurchExceptHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
MileageSurchExceptHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MileageSurchExceptHistoricalDAO"));
MileageSurchExceptHistoricalDAO&
MileageSurchExceptHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MileageSurchExcept*>&
MileageSurchExceptHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int textTblItemNo,
                                     const CarrierCode& governingCarrier,
                                     const TariffNumber& ruleTariff,
                                     const RuleNumber& rule,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MileageSurchExceptHistoricalKey key(vendor, textTblItemNo, governingCarrier, ruleTariff, rule);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<MileageSurchExcept*>* ret = new std::vector<MileageSurchExcept*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<MileageSurchExcept>(date, ticketDate));
  if (!governingCarrier.empty())
  {
    MileageSurchExceptHistoricalKey key2(vendor, textTblItemNo, "", -1, "");
    DAOUtils::getDateRange(ticketDate, key2._f, key2._g, _cacheBy);
    ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveHist<MileageSurchExcept>(date, ticketDate));
  }
  if (textTblItemNo != 0)
  {
    MileageSurchExceptHistoricalKey key3(vendor, 0, governingCarrier, ruleTariff, rule);
    DAOUtils::getDateRange(ticketDate, key3._f, key3._g, _cacheBy);
    ptr = cache().get(key3);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveHist<MileageSurchExcept>(date, ticketDate));
  }
  return *ret;
}

std::vector<MileageSurchExcept*>*
MileageSurchExceptHistoricalDAO::create(MileageSurchExceptHistoricalKey key)
{
  std::vector<MileageSurchExcept*>* ret = new std::vector<MileageSurchExcept*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneMileageSurchExceptBaseHistorical mseb(dbAdapter->getAdapter());
    mseb.findMileageSurchExcept(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MileageSurchExceptHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MileageSurchExceptHistoricalDAO::destroy(MileageSurchExceptHistoricalKey key,
                                         std::vector<MileageSurchExcept*>* recs)
{
  std::vector<MileageSurchExcept*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MileageSurchExceptHistoricalKey
MileageSurchExceptHistoricalDAO::createKey(const MileageSurchExcept* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return MileageSurchExceptHistoricalKey(info->vendor(),
                                         info->textTblItemNo(),
                                         info->governingCarrier(),
                                         info->ruleTariff(),
                                         info->rule(),
                                         startDate,
                                         endDate);
}

void
MileageSurchExceptHistoricalDAO::load()
{
  StartupLoaderNoDB<MileageSurchExcept, MileageSurchExceptHistoricalDAO>();
}

sfc::CompressedData*
MileageSurchExceptHistoricalDAO::compress(const std::vector<MileageSurchExcept*>* vect) const
{
  return compressVector(vect);
}

std::vector<MileageSurchExcept*>*
MileageSurchExceptHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MileageSurchExcept>(compressed);
}

std::string
MileageSurchExceptHistoricalDAO::_name("MileageSurchExceptHistorical");
std::string
MileageSurchExceptHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<MileageSurchExceptHistoricalDAO>
MileageSurchExceptHistoricalDAO::_helper(_name);
MileageSurchExceptHistoricalDAO* MileageSurchExceptHistoricalDAO::_instance = nullptr;

} // namespace tse
