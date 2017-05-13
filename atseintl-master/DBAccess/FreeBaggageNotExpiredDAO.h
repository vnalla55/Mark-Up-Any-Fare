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

typedef HashKey<CarrierCode, DateTime> CarrierWithDateKey;

class FreeBaggageNotExpiredDAO
    : public DataAccessObject<CarrierWithDateKey, std::vector<FreeBaggageInfo*> >
{
public:
  static FreeBaggageNotExpiredDAO& instance();
  const std::vector<FreeBaggageInfo*>&
  get(DeleteList& del, const CarrierCode& carrier, DateTime& reqDate);

  /*bool translateKey(const ObjectKey& objectKey, CarrierWithDate& key) const
  {
    return objectKey.getValue("CARRIER", key);
  }*/

  CarrierWithDateKey createKey(const FreeBaggageInfo* info);

  void translateKey(const CarrierWithDateKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FreeBaggageInfo, FreeBaggageNotExpiredDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FreeBaggageNotExpiredDAO>;
  static DAOHelper<FreeBaggageNotExpiredDAO> _helper;
  FreeBaggageNotExpiredDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierWithDateKey, std::vector<FreeBaggageInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FreeBaggageInfo*>* create(CarrierWithDateKey key) override;
  void destroy(CarrierWithDateKey key, std::vector<FreeBaggageInfo*>* recs) override;
  void load() override;

private:
  static FreeBaggageNotExpiredDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FreeBaggageNotExpiredHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode, DateTime, DateTime> FreeBaggageNotExpiredHistoricalKey;

class FreeBaggageNotExpiredHistoricalDAO
    : public HistoricalDataAccessObject<FreeBaggageNotExpiredHistoricalKey,
                                        std::vector<FreeBaggageInfo*> >
{
public:
  static FreeBaggageNotExpiredHistoricalDAO& instance();
  const std::vector<FreeBaggageInfo*>&
  get(DeleteList& del, const CarrierCode& carrier, DateTime& reqDate);

  bool
  translateKey(const ObjectKey& objectKey, FreeBaggageNotExpiredHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FreeBaggageNotExpiredHistoricalKey& key,
                    const DateTime reqDate) const override
  {
    DAOUtils::getDateRange(reqDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void
  setKeyDateRange(FreeBaggageNotExpiredHistoricalKey& key, const DateTime reqDate) const override
  {
    DAOUtils::getDateRange(reqDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FreeBaggageNotExpiredHistoricalDAO>;
  static DAOHelper<FreeBaggageNotExpiredHistoricalDAO> _helper;
  FreeBaggageNotExpiredHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FreeBaggageNotExpiredHistoricalKey,
                                 std::vector<FreeBaggageInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FreeBaggageInfo*>* create(FreeBaggageNotExpiredHistoricalKey key) override;
  void destroy(FreeBaggageNotExpiredHistoricalKey key, std::vector<FreeBaggageInfo*>* t) override;

private:
  struct isEffective;
  static FreeBaggageNotExpiredHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}
