//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
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
class FreeBaggageInfo;
class DeleteList;

class FreeBaggageDAO : public DataAccessObject<CarrierKey, std::vector<FreeBaggageInfo*> >
{
public:
  static FreeBaggageDAO& instance();
  const std::vector<FreeBaggageInfo*>& get(DeleteList& del, const CarrierCode& key);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(const FreeBaggageInfo* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FreeBaggageInfo, FreeBaggageDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FreeBaggageDAO>;
  static DAOHelper<FreeBaggageDAO> _helper;
  FreeBaggageDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<FreeBaggageInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<FreeBaggageInfo*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<FreeBaggageInfo*>* recs) override;
  void load() override;

private:
  static FreeBaggageDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FreeBaggageHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode, DateTime, DateTime> FreeBaggageHistoricalKey;

class FreeBaggageHistoricalDAO
    : public HistoricalDataAccessObject<FreeBaggageHistoricalKey, std::vector<FreeBaggageInfo*> >
{
public:
  static FreeBaggageHistoricalDAO& instance();
  const std::vector<FreeBaggageInfo*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FreeBaggageHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FreeBaggageHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void setKeyDateRange(FreeBaggageHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FreeBaggageHistoricalDAO>;
  static DAOHelper<FreeBaggageHistoricalDAO> _helper;
  FreeBaggageHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FreeBaggageHistoricalKey, std::vector<FreeBaggageInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<FreeBaggageInfo*>* create(FreeBaggageHistoricalKey key) override;
  void destroy(FreeBaggageHistoricalKey key, std::vector<FreeBaggageInfo*>* t) override;

private:
  struct isEffective;
  static FreeBaggageHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}
