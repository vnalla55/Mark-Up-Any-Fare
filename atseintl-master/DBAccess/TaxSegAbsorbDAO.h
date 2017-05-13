//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
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
class TaxSegAbsorb;
class DeleteList;

class TaxSegAbsorbDAO : public DataAccessObject<CarrierKey, std::vector<TaxSegAbsorb*>, false>
{
public:
  static TaxSegAbsorbDAO& instance();
  const std::vector<TaxSegAbsorb*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(TaxSegAbsorb* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxSegAbsorb, TaxSegAbsorbDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxSegAbsorbDAO>;
  static DAOHelper<TaxSegAbsorbDAO> _helper;
  TaxSegAbsorbDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<TaxSegAbsorb*>, false>(cacheSize, cacheType, 2)
  {
  }

  std::vector<TaxSegAbsorb*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<TaxSegAbsorb*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  static TaxSegAbsorbDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TaxSegAbsorbHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode, DateTime, DateTime> TaxSegAbsorbHistoricalKey;

class TaxSegAbsorbHistoricalDAO : public HistoricalDataAccessObject<TaxSegAbsorbHistoricalKey,
                                                                    std::vector<TaxSegAbsorb*>,
                                                                    false>
{
public:
  static TaxSegAbsorbHistoricalDAO& instance();
  const std::vector<TaxSegAbsorb*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxSegAbsorbHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxSegAbsorbHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void setKeyDateRange(TaxSegAbsorbHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxSegAbsorbHistoricalDAO>;
  static DAOHelper<TaxSegAbsorbHistoricalDAO> _helper;
  TaxSegAbsorbHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxSegAbsorbHistoricalKey, std::vector<TaxSegAbsorb*>, false>(
          cacheSize, cacheType, 2)
  {
  }

  std::vector<TaxSegAbsorb*>* create(TaxSegAbsorbHistoricalKey key) override;
  void destroy(TaxSegAbsorbHistoricalKey key, std::vector<TaxSegAbsorb*>* recs) override;

private:
  struct isEffective;
  static TaxSegAbsorbHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
