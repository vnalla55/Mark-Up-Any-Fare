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
class ValueCodeAlgorithm;

typedef HashKey<VendorCode, CarrierCode, std::string> ValueCodeAlgorithmKey;
typedef HashKey<VendorCode, CarrierCode, std::string, DateTime, DateTime>
ValueCodeAlgorithmHistoricalKey;

class ValueCodeAlgorithmDAO
    : public DataAccessObject<ValueCodeAlgorithmKey, std::vector<ValueCodeAlgorithm*> >
{
public:
  friend class DAOHelper<ValueCodeAlgorithmDAO>;

  static ValueCodeAlgorithmDAO& instance();

  const ValueCodeAlgorithm* get(DeleteList& del,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const std::string& name,
                                const DateTime& date,
                                const DateTime& ticketDate);

  ValueCodeAlgorithmKey createKey(ValueCodeAlgorithm* info);

  bool translateKey(const ObjectKey& objectKey, ValueCodeAlgorithmKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("ALGORITHMNAME", key._c);
  }

  void translateKey(const ValueCodeAlgorithmKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("ALGORITHMNAME", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<ValueCodeAlgorithm, ValueCodeAlgorithmDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  ValueCodeAlgorithmDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ValueCodeAlgorithmKey, std::vector<ValueCodeAlgorithm*> >(
          cacheSize, cacheType, 1)
  {
  }

  std::vector<ValueCodeAlgorithm*>* create(ValueCodeAlgorithmKey key) override;
  void destroy(ValueCodeAlgorithmKey key, std::vector<ValueCodeAlgorithm*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<ValueCodeAlgorithmDAO> _helper;
  static ValueCodeAlgorithmDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class ValueCodeAlgorithmHistoricalDAO
    : public HistoricalDataAccessObject<ValueCodeAlgorithmHistoricalKey,
                                        std::vector<ValueCodeAlgorithm*> >
{
public:
  friend class DAOHelper<ValueCodeAlgorithmHistoricalDAO>;

  static ValueCodeAlgorithmHistoricalDAO& instance();

  const ValueCodeAlgorithm* get(DeleteList& del,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const std::string& name,
                                const DateTime& date,
                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ValueCodeAlgorithmHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("ALGORITHMNAME", key._c) &&
               objectKey.getValue("STARTDATE", key._d) && objectKey.getValue("ENDDATE", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    ValueCodeAlgorithmHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);

    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("ALGORITHMNAME", key._c);
  }

  void
  setKeyDateRange(ValueCodeAlgorithmHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  ValueCodeAlgorithmHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ValueCodeAlgorithmHistoricalKey,
                                 std::vector<ValueCodeAlgorithm*> >(cacheSize, cacheType, 1)
  {
  }
  std::vector<ValueCodeAlgorithm*>* create(ValueCodeAlgorithmHistoricalKey key) override;
  void destroy(ValueCodeAlgorithmHistoricalKey key, std::vector<ValueCodeAlgorithm*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<ValueCodeAlgorithmHistoricalDAO> _helper;
  static ValueCodeAlgorithmHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

