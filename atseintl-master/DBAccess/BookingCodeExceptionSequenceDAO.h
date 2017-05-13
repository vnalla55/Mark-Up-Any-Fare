//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class BookingCodeExceptionSequence;
class DeleteList;

typedef HashKey<VendorCode, int> BookingCodeExceptionSequenceKey;

class BookingCodeExceptionSequenceDAO
    : public DataAccessObject<BookingCodeExceptionSequenceKey, BookingCodeExceptionSequenceList>
{
public:
  static BookingCodeExceptionSequenceDAO& instance();

  const BookingCodeExceptionSequenceList& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              int itemNo,
                                              const DateTime& ticketDate,
                                              bool filterExpireDate);

  bool translateKey(const ObjectKey& objectKey, BookingCodeExceptionSequenceKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  BookingCodeExceptionSequenceKey createKey(BookingCodeExceptionSequenceList* info);
  BookingCodeExceptionSequenceKey createKey(BookingCodeExceptionSequence* info);

  void translateKey(const BookingCodeExceptionSequenceKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BookingCodeExceptionSequenceList, BookingCodeExceptionSequenceDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const BookingCodeExceptionSequenceList* entry) const override;

  virtual BookingCodeExceptionSequenceList*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BookingCodeExceptionSequenceDAO>;
  static DAOHelper<BookingCodeExceptionSequenceDAO> _helper;
  BookingCodeExceptionSequenceDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BookingCodeExceptionSequenceKey, BookingCodeExceptionSequenceList>(
          cacheSize, cacheType, 2)
  {
  }
  void load() override;
  BookingCodeExceptionSequenceList* create(BookingCodeExceptionSequenceKey key) override;
  void destroy(BookingCodeExceptionSequenceKey key, BookingCodeExceptionSequenceList* t) override;

private:
  static BookingCodeExceptionSequenceDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class BookingCodeExceptionSequenceDAO

// --------------------------------------------------
// Historical DAO: BookingCodeExceptionSequenceHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> BookingCodeExceptionSequenceHistoricalKey;

class BookingCodeExceptionSequenceHistoricalDAO
    : public HistoricalDataAccessObject<BookingCodeExceptionSequenceHistoricalKey,
                                        std::vector<BookingCodeExceptionSequence*> >
{
public:
  static BookingCodeExceptionSequenceHistoricalDAO& instance();

  const BookingCodeExceptionSequenceList&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    BookingCodeExceptionSequenceHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    BookingCodeExceptionSequenceHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(BookingCodeExceptionSequenceHistoricalKey& key,
                       const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<BookingCodeExceptionSequence*>* vect) const override;

  virtual std::vector<BookingCodeExceptionSequence*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BookingCodeExceptionSequenceHistoricalDAO>;
  static DAOHelper<BookingCodeExceptionSequenceHistoricalDAO> _helper;
  BookingCodeExceptionSequenceHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BookingCodeExceptionSequenceHistoricalKey,
                                 std::vector<BookingCodeExceptionSequence*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<BookingCodeExceptionSequence*>*
  create(BookingCodeExceptionSequenceHistoricalKey key) override;
  void destroy(BookingCodeExceptionSequenceHistoricalKey key,
               std::vector<BookingCodeExceptionSequence*>* t) override;

private:
  static BookingCodeExceptionSequenceHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class BookingCodeExceptionSequenceHistoricalDAO
} // namespace tse

