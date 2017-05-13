//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
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
class PrintOption;

typedef HashKey<VendorCode, std::string> PrintOptionKey;
typedef HashKey<VendorCode, std::string, DateTime, DateTime> PrintOptionHistoricalKey;

class PrintOptionDAO : public DataAccessObject<PrintOptionKey, std::vector<PrintOption*> >
{
public:
  friend class DAOHelper<PrintOptionDAO>;

  static PrintOptionDAO& instance();

  const PrintOption* get(DeleteList& del,
                         const VendorCode& vendor,
                         const std::string& fareSource,
                         const DateTime& date,
                         const DateTime& ticketDate);

  PrintOptionKey createKey(PrintOption* info);

  bool translateKey(const ObjectKey& objectKey, PrintOptionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("FARESOURCE", key._b);
  }

  void translateKey(const PrintOptionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("FARESOURCE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PrintOption, PrintOptionDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  PrintOptionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PrintOptionKey, std::vector<PrintOption*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<PrintOption*>* create(PrintOptionKey key) override;
  void destroy(PrintOptionKey key, std::vector<PrintOption*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<PrintOptionDAO> _helper;
  static PrintOptionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class PrintOptionHistoricalDAO
    : public HistoricalDataAccessObject<PrintOptionHistoricalKey, std::vector<PrintOption*> >
{
public:
  friend class DAOHelper<PrintOptionHistoricalDAO>;

  static PrintOptionHistoricalDAO& instance();

  const PrintOption* get(DeleteList& del,
                         const VendorCode& vendor,
                         const std::string& fareSource,
                         const DateTime& date,
                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PrintOptionHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("FARESOURCE", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PrintOptionHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);

    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("FARESOURCE", key._b);
  }

  void setKeyDateRange(PrintOptionHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  PrintOptionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PrintOptionHistoricalKey, std::vector<PrintOption*> >(
          cacheSize, cacheType, 1)
  {
  }
  std::vector<PrintOption*>* create(PrintOptionHistoricalKey key) override;
  void destroy(PrintOptionHistoricalKey key, std::vector<PrintOption*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<PrintOptionHistoricalDAO> _helper;
  static PrintOptionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

