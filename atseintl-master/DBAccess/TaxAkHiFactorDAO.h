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
class TaxAkHiFactor;
class DeleteList;

class TaxAkHiFactorDAO : public DataAccessObject<LocCodeKey, std::vector<TaxAkHiFactor*>, false>
{
public:
  static TaxAkHiFactorDAO& instance();
  const std::vector<TaxAkHiFactor*>&
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCCODE", key._a);
  }

  LocCodeKey createKey(TaxAkHiFactor* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOCCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxAkHiFactor, TaxAkHiFactorDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxAkHiFactorDAO>;
  static DAOHelper<TaxAkHiFactorDAO> _helper;
  TaxAkHiFactorDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<TaxAkHiFactor*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<TaxAkHiFactor*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<TaxAkHiFactor*>* t) override;

private:
  struct isEffective;
  static TaxAkHiFactorDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TaxAkHiFactorHistoricalDAO
// --------------------------------------------------

typedef HashKey<LocCode, DateTime, DateTime> TaxAkHiFactorHistoricalKey;

class TaxAkHiFactorHistoricalDAO : public HistoricalDataAccessObject<TaxAkHiFactorHistoricalKey,
                                                                     std::vector<TaxAkHiFactor*>,
                                                                     false>
{
public:
  static TaxAkHiFactorHistoricalDAO& instance();

  const std::vector<TaxAkHiFactor*>&
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxAkHiFactorHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCCODE", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxAkHiFactorHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("LOCCODE", key._a);
  }

  void setKeyDateRange(TaxAkHiFactorHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxAkHiFactorHistoricalDAO>;
  static DAOHelper<TaxAkHiFactorHistoricalDAO> _helper;
  TaxAkHiFactorHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxAkHiFactorHistoricalKey, std::vector<TaxAkHiFactor*>, false>(
          cacheSize, cacheType)
  {
  }
  std::vector<TaxAkHiFactor*>* create(TaxAkHiFactorHistoricalKey key) override;
  void destroy(TaxAkHiFactorHistoricalKey key, std::vector<TaxAkHiFactor*>* t) override;

private:
  struct isEffective;
  static TaxAkHiFactorHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
