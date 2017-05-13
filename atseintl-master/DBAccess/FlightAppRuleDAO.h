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
class FlightAppRule;
class DeleteList;

typedef HashKey<VendorCode, int> FlightAppRuleKey;

class FlightAppRuleDAO : public DataAccessObject<FlightAppRuleKey, std::vector<FlightAppRule*> >
{
public:
  static FlightAppRuleDAO& instance();
  const FlightAppRule*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FlightAppRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  FlightAppRuleKey createKey(FlightAppRule* info);

  void translateKey(const FlightAppRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FlightAppRule, FlightAppRuleDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<FlightAppRule*>* vect) const override;

  virtual std::vector<FlightAppRule*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FlightAppRuleDAO>;
  static DAOHelper<FlightAppRuleDAO> _helper;
  FlightAppRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FlightAppRuleKey, std::vector<FlightAppRule*> >(cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<FlightAppRule*>* create(FlightAppRuleKey key) override;
  void destroy(FlightAppRuleKey key, std::vector<FlightAppRule*>* t) override;

private:
  static FlightAppRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class FlightAppRuleDAO

// --------------------------------------------------
// Historical DAO: FlightAppRuleHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> FlightAppRuleHistoricalKey;

class FlightAppRuleHistoricalDAO
    : public HistoricalDataAccessObject<FlightAppRuleHistoricalKey, std::vector<FlightAppRule*> >
{
public:
  static FlightAppRuleHistoricalDAO& instance();
  const FlightAppRule*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FlightAppRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FlightAppRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  FlightAppRuleHistoricalKey createKey(const FlightAppRule* info,
                                       const DateTime& startDate = DateTime::emptyDate(),
                                       const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FlightAppRuleHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FlightAppRule, FlightAppRuleHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(FlightAppRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<FlightAppRule*>* vect) const override;

  virtual std::vector<FlightAppRule*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FlightAppRuleHistoricalDAO>;
  static DAOHelper<FlightAppRuleHistoricalDAO> _helper;
  FlightAppRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FlightAppRuleHistoricalKey, std::vector<FlightAppRule*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<FlightAppRule*>* create(FlightAppRuleHistoricalKey key) override;
  void destroy(FlightAppRuleHistoricalKey key, std::vector<FlightAppRule*>* t) override;
  void load() override;

private:
  static FlightAppRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class FlightAppRuleHistoricalDAO
} // namespace tse
