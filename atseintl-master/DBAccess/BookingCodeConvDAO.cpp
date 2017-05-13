//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/BookingCodeConvDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BookingCodeConv.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBkgCodeConv.h"

namespace tse
{
log4cxx::LoggerPtr
BookingCodeConvDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BookingCodeConvDAO"));

BookingCodeConvDAO&
BookingCodeConvDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BookingCodeConv*>&
getBookingCodeConvData(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& ruleTariff,
                       const RuleNumber& rule,
                       Indicator conventionNo,
                       const DateTime& date,
                       bool& isRuleZero,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    BookingCodeConvHistoricalDAO& dao = BookingCodeConvHistoricalDAO::instance();
    return dao.get(
        deleteList, vendor, carrier, ruleTariff, rule, conventionNo, isRuleZero, ticketDate, date);
  }
  else
  {
    BookingCodeConvDAO& dao = BookingCodeConvDAO::instance();
    return dao.get(
        deleteList, vendor, carrier, ruleTariff, rule, conventionNo, isRuleZero, ticketDate, date);
  }
}

const std::vector<BookingCodeConv*>&
BookingCodeConvDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        const CarrierCode& carrier,
                        const TariffNumber& ruleTariff,
                        const RuleNumber& rule,
                        Indicator conventionNo,
                        bool& isRuleZero,
                        const DateTime& ticketDate,
                        const DateTime& date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TariffNumber tariffNumber;
  RuleNumber ruleNumber;
  if (conventionNo == '1')
  {
    tariffNumber = 0;
    ruleNumber = "0000";
  }
  else
  {
    tariffNumber = ruleTariff;
    ruleNumber = rule;
  }
  const std::vector<BookingCodeConv*>* ret;

  BookingCodeConvKey key(vendor, carrier, tariffNumber, ruleNumber, conventionNo);
  DAOCache::pointer_type ptr = cache().get(key);
  ret = applyFilter(del, ptr, IsNotEffectiveG<BookingCodeConv>(date, ticketDate));

  if (ret->empty() && conventionNo == '2')
  {
    BookingCodeConvKey key2(vendor, carrier, tariffNumber, "0000", conventionNo);
    ptr = cache().get(key2);
    ret = applyFilter(del, ptr, IsNotEffectiveG<BookingCodeConv>(date, ticketDate));
    isRuleZero = true;
  }

  return *ret;
}

BookingCodeConvKey
BookingCodeConvDAO::createKey(BookingCodeConv* info)
{
  return BookingCodeConvKey(
      info->vendor(), info->carrier(), info->ruleTariff(), info->rule(), info->conventionNo());
}

void
BookingCodeConvDAO::load()
{
  StartupLoader<QueryGetAllBkgCodeConv, BookingCodeConv, BookingCodeConvDAO>();
}

std::vector<BookingCodeConv*>*
BookingCodeConvDAO::create(BookingCodeConvKey key)
{
  std::vector<BookingCodeConv*>* ret = new std::vector<BookingCodeConv*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBkgCodeConv bcc(dbAdapter->getAdapter());
    bcc.findBookingCodeConv(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BookingCodeConvDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
BookingCodeConvDAO::destroy(BookingCodeConvKey key, std::vector<BookingCodeConv*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
BookingCodeConvDAO::compress(const std::vector<BookingCodeConv*>* vect) const
{
  return compressVector(vect);
}

std::vector<BookingCodeConv*>*
BookingCodeConvDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<BookingCodeConv>(compressed);
}

std::string
BookingCodeConvDAO::_name("BookingCodeConv");
std::string
BookingCodeConvDAO::_cacheClass("BookingCode");
DAOHelper<BookingCodeConvDAO>
BookingCodeConvDAO::_helper(_name);
BookingCodeConvDAO* BookingCodeConvDAO::_instance = nullptr;

// Historical Stuff ////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
BookingCodeConvHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BookingCodeConvHistoricalDAO"));

BookingCodeConvHistoricalDAO&
BookingCodeConvHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BookingCodeConv*>&
BookingCodeConvHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  Indicator conventionNo,
                                  bool& isRuleZero,
                                  const DateTime& ticketDate,
                                  const DateTime& date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TariffNumber tariffNumber;
  RuleNumber ruleNumber;
  if (conventionNo == '1')
  {
    tariffNumber = 0;
    ruleNumber = "0000";
  }
  else
  {
    tariffNumber = ruleTariff;
    ruleNumber = rule;
  }
  std::vector<BookingCodeConv*>* ret = new std::vector<BookingCodeConv*>;
  del.adopt(ret);

  BookingCodeConvHistoricalKey key(vendor, carrier, ruleTariff, rule, conventionNo);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<BookingCodeConv>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<BookingCodeConv>), ret->end());

  if (ret->empty() && conventionNo == '2')
  {
    BookingCodeConvHistoricalKey key2(vendor, carrier, tariffNumber, "0000", conventionNo);
    DAOUtils::getDateRange(ticketDate, key2._f, key2._g, _cacheBy);
    ptr = cache().get(key2);

    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveH<BookingCodeConv>(date, ticketDate));
    ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<BookingCodeConv>), ret->end());
    isRuleZero = true;
  }
  del.copy(ptr);

  return *ret;
}

std::vector<BookingCodeConv*>*
BookingCodeConvHistoricalDAO::create(BookingCodeConvHistoricalKey key)
{
  std::vector<BookingCodeConv*>* ret = new std::vector<BookingCodeConv*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetBkgCodeConvHistorical bcc(dbAdapter->getAdapter());
    bcc.findBookingCodeConv(*ret, key._a, key._b, key._c, key._d, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BookingCodeConvHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BookingCodeConvHistoricalDAO::destroy(BookingCodeConvHistoricalKey key,
                                      std::vector<BookingCodeConv*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
BookingCodeConvHistoricalDAO::compress(const std::vector<BookingCodeConv*>* vect) const
{
  return compressVector(vect);
}

std::vector<BookingCodeConv*>*
BookingCodeConvHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<BookingCodeConv>(compressed);
}

std::string
BookingCodeConvHistoricalDAO::_name("BookingCodeConvHistorical");
std::string
BookingCodeConvHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<BookingCodeConvHistoricalDAO>
BookingCodeConvHistoricalDAO::_helper(_name);
BookingCodeConvHistoricalDAO* BookingCodeConvHistoricalDAO::_instance = nullptr;

} // namespace tse
