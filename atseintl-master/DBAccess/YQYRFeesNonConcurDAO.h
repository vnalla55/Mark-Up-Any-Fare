//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class YQYRFeesNonConcur;
class DeleteList;

class YQYRFeesNonConcurDAO : public DataAccessObject<CarrierKey, std::vector<YQYRFeesNonConcur*> >
{
public:
  static YQYRFeesNonConcurDAO& instance();
  static log4cxx::LoggerPtr _logger;

  const std::vector<YQYRFeesNonConcur*>& get(DeleteList& del,
                                             const CarrierCode& carrier,
                                             const DateTime& date,
                                             const DateTime& ticketDate);

  const std::string& cacheClass() override { return _cacheClass; }

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(YQYRFeesNonConcur* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<YQYRFeesNonConcur, YQYRFeesNonConcurDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<YQYRFeesNonConcurDAO>;

  static DAOHelper<YQYRFeesNonConcurDAO> _helper;

  YQYRFeesNonConcurDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<YQYRFeesNonConcur*> >(cacheSize, cacheType, 2)
  {
  }

  virtual void load() override;

  std::vector<YQYRFeesNonConcur*>* create(CarrierKey key) override;

  void destroy(CarrierKey key, std::vector<YQYRFeesNonConcur*>* t) override;

private:
  struct isEffective;

  static YQYRFeesNonConcurDAO* _instance;
};

// --------------------------------------------------
// Historical DAO: YQYRFeesNonConcurHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode, DateTime, DateTime> YQYRFeesNonConcurHistoricalKey;

class YQYRFeesNonConcurHistoricalDAO
    : public HistoricalDataAccessObject<YQYRFeesNonConcurHistoricalKey,
                                        std::vector<YQYRFeesNonConcur*> >
{
public:
  static YQYRFeesNonConcurHistoricalDAO& instance();
  const std::vector<YQYRFeesNonConcur*>& get(DeleteList& del,
                                             const CarrierCode& carrier,
                                             const DateTime& date,
                                             const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, YQYRFeesNonConcurHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    YQYRFeesNonConcurHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void setKeyDateRange(YQYRFeesNonConcurHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<YQYRFeesNonConcurHistoricalDAO>;
  static DAOHelper<YQYRFeesNonConcurHistoricalDAO> _helper;
  YQYRFeesNonConcurHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<YQYRFeesNonConcurHistoricalKey, std::vector<YQYRFeesNonConcur*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<YQYRFeesNonConcur*>* create(YQYRFeesNonConcurHistoricalKey key) override;
  void destroy(YQYRFeesNonConcurHistoricalKey key, std::vector<YQYRFeesNonConcur*>* t) override;

private:
  struct isEffective;
  static YQYRFeesNonConcurHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

