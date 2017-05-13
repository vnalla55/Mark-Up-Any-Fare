//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TicketEndorsementsInfo;
class DeleteList;

typedef HashKey<VendorCode, int> TicketEndorsementsKey;

class TicketEndorsementsDAO
    : public DataAccessObject<TicketEndorsementsKey, std::vector<TicketEndorsementsInfo*> >
{
public:
  static TicketEndorsementsDAO& instance();
  const TicketEndorsementsInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TicketEndorsementsKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<TicketEndorsementsInfo*>* vect) const override;

  virtual std::vector<TicketEndorsementsInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TicketEndorsementsDAO>;
  static DAOHelper<TicketEndorsementsDAO> _helper;
  TicketEndorsementsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TicketEndorsementsKey, std::vector<TicketEndorsementsInfo*> >(cacheSize,
                                                                                     cacheType)
  {
  }
  std::vector<TicketEndorsementsInfo*>* create(TicketEndorsementsKey key) override;
  void destroy(TicketEndorsementsKey key, std::vector<TicketEndorsementsInfo*>* t) override;

private:
  static TicketEndorsementsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TicketEndorsementsHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> TicketEndorsementsHistoricalKey;

class TicketEndorsementsHistoricalDAO
    : public HistoricalDataAccessObject<TicketEndorsementsHistoricalKey,
                                        std::vector<TicketEndorsementsInfo*> >
{
public:
  static TicketEndorsementsHistoricalDAO& instance();
  const TicketEndorsementsInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TicketEndorsementsHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TicketEndorsementsHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(TicketEndorsementsHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  TicketEndorsementsHistoricalKey createKey(const TicketEndorsementsInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const TicketEndorsementsHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<TicketEndorsementsInfo,
                                            TicketEndorsementsHistoricalDAO>(flatKey, objectKey)
        .success();
  }

  virtual sfc::CompressedData*
  compress(const std::vector<TicketEndorsementsInfo*>* vect) const override;

  virtual std::vector<TicketEndorsementsInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TicketEndorsementsHistoricalDAO>;
  static DAOHelper<TicketEndorsementsHistoricalDAO> _helper;
  TicketEndorsementsHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TicketEndorsementsHistoricalKey,
                                 std::vector<TicketEndorsementsInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<TicketEndorsementsInfo*>* create(TicketEndorsementsHistoricalKey key) override;
  void
  destroy(TicketEndorsementsHistoricalKey key, std::vector<TicketEndorsementsInfo*>* t) override;
  void load() override;

private:
  static TicketEndorsementsHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
