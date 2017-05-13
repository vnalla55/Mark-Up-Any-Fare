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
class PfcMultiAirport;
class DeleteList;

class PfcMultiAirportDAO : public DataAccessObject<LocCodeKey, std::vector<PfcMultiAirport*>, false>
{
public:
  static PfcMultiAirportDAO& instance();
  const PfcMultiAirport*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<PfcMultiAirport*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOC", key._a);
  }

  LocCodeKey createKey(PfcMultiAirport* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOC", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PfcMultiAirport, PfcMultiAirportDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcMultiAirportDAO>;
  virtual size_t clear() override;
  virtual size_t invalidate(const ObjectKey& objectKey) override;
  static DAOHelper<PfcMultiAirportDAO> _helper;
  PfcMultiAirportDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<PfcMultiAirport*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<PfcMultiAirport*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<PfcMultiAirport*>* t) override;

private:
  struct isEffective;
  static PfcMultiAirportDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: PfcMultiAirportHistoricalDAO
// --------------------------------------------------
typedef HashKey<LocCode, DateTime, DateTime> PfcMultiAirportHistoricalKey;

class PfcMultiAirportHistoricalDAO
    : public HistoricalDataAccessObject<PfcMultiAirportHistoricalKey,
                                        std::vector<PfcMultiAirport*>,
                                        false>
{
public:
  static PfcMultiAirportHistoricalDAO& instance();
  const PfcMultiAirport*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcMultiAirportHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOC", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PfcMultiAirportHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("LOC", key._a);
  }

  void setKeyDateRange(PfcMultiAirportHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcMultiAirportHistoricalDAO>;
  static DAOHelper<PfcMultiAirportHistoricalDAO> _helper;
  PfcMultiAirportHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PfcMultiAirportHistoricalKey,
                                 std::vector<PfcMultiAirport*>,
                                 false>(cacheSize, cacheType)
  {
  }
  std::vector<PfcMultiAirport*>* create(PfcMultiAirportHistoricalKey key) override;
  void destroy(PfcMultiAirportHistoricalKey key, std::vector<PfcMultiAirport*>* t) override;

private:
  struct isEffective;
  static PfcMultiAirportHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
