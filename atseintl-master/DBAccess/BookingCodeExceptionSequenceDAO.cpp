//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/BookingCodeExceptionSequenceDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BookingCodeConv.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBkgException.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
BookingCodeExceptionSequenceDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BookingCodeExceptionSequenceDAO"));

BookingCodeExceptionSequenceDAO&
BookingCodeExceptionSequenceDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const BookingCodeExceptionSequenceList&
getBookingCodeExceptionSequenceData(const VendorCode& vendor,
                                    const int itemNo,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical,
                                    bool filterExpireDate)
{
  if (UNLIKELY(isHistorical))
  {
    BookingCodeExceptionSequenceHistoricalDAO& dao =
        BookingCodeExceptionSequenceHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate).setKey(vendor);
  }
  else
  {
    BookingCodeExceptionSequenceDAO& dao = BookingCodeExceptionSequenceDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate, filterExpireDate).setKey(vendor);
  }
}

const BookingCodeExceptionSequenceList&
getBookingCodeExceptionSequenceData(const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& ruleTariff,
                                    const RuleNumber& rule,
                                    Indicator conventionNo,
                                    const DateTime& date,
                                    bool& isRuleZero,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical,
                                    bool filterExpireDate)
{
  const std::vector<BookingCodeConv*>& convs = getBookingCodeConvData(vendor,
                                                                      carrier,
                                                                      ruleTariff,
                                                                      rule,
                                                                      conventionNo,
                                                                      date,
                                                                      isRuleZero,
                                                                      deleteList,
                                                                      ticketDate,
                                                                      isHistorical);
  if (convs.empty())
  {
    BookingCodeExceptionSequenceList* ret = new BookingCodeExceptionSequenceList();
    ret->setKey(vendor); // just a marker tbl. no
    deleteList.adopt(ret);
    return *ret;
  }

  if (LIKELY(convs.size() == 1)) // This should be the only possibility
  {
    return getBookingCodeExceptionSequenceData(convs.front()->vendor(),
                                               convs.front()->bookingCodetblItemNo(),
                                               deleteList,
                                               ticketDate,
                                               isHistorical,
                                               filterExpireDate);
  }

  BookingCodeExceptionSequenceList* ret = new BookingCodeExceptionSequenceList();
  ret->setKey(vendor); // just a marker tbl. no
  deleteList.adopt(ret);
  std::vector<BookingCodeConv*>::const_iterator i = convs.begin();

  for (; i != convs.end(); ++i)
  {
    BookingCodeConv& conv = **i;
    const BookingCodeExceptionSequenceList& excpt =
        getBookingCodeExceptionSequenceData(conv.vendor(),
                                            conv.bookingCodetblItemNo(),
                                            deleteList,
                                            ticketDate,
                                            isHistorical,
                                            filterExpireDate);
    copy(excpt.begin(), excpt.end(), back_inserter(ret->getSequences()));
  }

  return *ret;
}

const BookingCodeExceptionSequenceList&
BookingCodeExceptionSequenceDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate,
                                     bool filterExpireDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  BookingCodeExceptionSequenceKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(filterExpireDate))
    return *(applyFilter(del, ptr, IsNotCurrentG<BookingCodeExceptionSequence>(ticketDate)));
  else
  {
    del.copy(ptr);
    return *(ptr.get());
  }
}

BookingCodeExceptionSequenceList*
BookingCodeExceptionSequenceDAO::create(BookingCodeExceptionSequenceKey key)
{
  BookingCodeExceptionSequenceList* ret = new BookingCodeExceptionSequenceList();

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBkgException be(dbAdapter->getAdapter());
    be.findBkgExcpt(ret->getSequences(), key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BookingCodeExceptionSequenceDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BookingCodeExceptionSequenceDAO::destroy(BookingCodeExceptionSequenceKey key,
                                         BookingCodeExceptionSequenceList* recs)
{
  std::vector<BookingCodeExceptionSequence*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

BookingCodeExceptionSequenceKey
BookingCodeExceptionSequenceDAO::createKey(BookingCodeExceptionSequence* info)
{
  return BookingCodeExceptionSequenceKey(info->vendor(), info->itemNo());
}
BookingCodeExceptionSequenceKey
BookingCodeExceptionSequenceDAO::createKey(BookingCodeExceptionSequenceList* info)
{
  if (info->empty())
    return BookingCodeExceptionSequenceKey("", 0);
  return createKey(info->getSequences().front());
}

void
BookingCodeExceptionSequenceDAO::load()
{
  StartupLoaderNoDB<BookingCodeExceptionSequence, BookingCodeExceptionSequenceDAO>();
}

sfc::CompressedData*
BookingCodeExceptionSequenceDAO::compress(const BookingCodeExceptionSequenceList* entry) const
{
  return compressEntry(entry);
}

BookingCodeExceptionSequenceList*
BookingCodeExceptionSequenceDAO::uncompress(const sfc::CompressedData& compressed) const
{
  const BookingCodeExceptionSequenceList* dummy(nullptr);
  return uncompressEntry(compressed, dummy);
}

std::string
BookingCodeExceptionSequenceDAO::_name("BookingCodeExceptionSequence");
std::string
BookingCodeExceptionSequenceDAO::_cacheClass("BookingCode");
DAOHelper<BookingCodeExceptionSequenceDAO>
BookingCodeExceptionSequenceDAO::_helper(_name);
BookingCodeExceptionSequenceDAO* BookingCodeExceptionSequenceDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: BookingCodeExceptionSequenceHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
BookingCodeExceptionSequenceHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BookingCodeExceptionSequenceHistoricalDAO"));
BookingCodeExceptionSequenceHistoricalDAO&
BookingCodeExceptionSequenceHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const BookingCodeExceptionSequenceList&
BookingCodeExceptionSequenceHistoricalDAO::get(DeleteList& del,
                                               const VendorCode& vendor,
                                               int itemNo,
                                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  BookingCodeExceptionSequenceHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  BookingCodeExceptionSequenceList* ret = new BookingCodeExceptionSequenceList();
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(ret->getSequences()),
                 IsNotCurrentH<BookingCodeExceptionSequence>(ticketDate));

  return *ret;
}

std::vector<BookingCodeExceptionSequence*>*
BookingCodeExceptionSequenceHistoricalDAO::create(BookingCodeExceptionSequenceHistoricalKey key)
{
  std::vector<BookingCodeExceptionSequence*>* ret = new std::vector<BookingCodeExceptionSequence*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);

  try
  {
    QueryGetBkgExceptionHistorical be(dbAdapter->getAdapter());
    be.findBkgExcpt(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BookingCodeExceptionSequenceHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BookingCodeExceptionSequenceHistoricalDAO::destroy(BookingCodeExceptionSequenceHistoricalKey key,
                                                   std::vector<BookingCodeExceptionSequence*>* recs)
{
  std::vector<BookingCodeExceptionSequence*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
BookingCodeExceptionSequenceHistoricalDAO::compress(
    const std::vector<BookingCodeExceptionSequence*>* vect) const
{
  return compressVector(vect);
}

std::vector<BookingCodeExceptionSequence*>*
BookingCodeExceptionSequenceHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<BookingCodeExceptionSequence>(compressed);
}

std::string
BookingCodeExceptionSequenceHistoricalDAO::_name("BookingCodeExceptionSequenceHistorical");
std::string
BookingCodeExceptionSequenceHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<BookingCodeExceptionSequenceHistoricalDAO>
BookingCodeExceptionSequenceHistoricalDAO::_helper(_name);
BookingCodeExceptionSequenceHistoricalDAO* BookingCodeExceptionSequenceHistoricalDAO::_instance = nullptr;

} // namespace tse
