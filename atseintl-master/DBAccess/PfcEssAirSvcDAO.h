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
class PfcEssAirSvc;
class DeleteList;

typedef HashKey<LocCode, LocCode> PfcEssAirSvcKey;

class PfcEssAirSvcDAO : public DataAccessObject<PfcEssAirSvcKey, std::vector<PfcEssAirSvc*>, false>
{
public:
  static PfcEssAirSvcDAO& instance();
  const std::vector<PfcEssAirSvc*>& get(DeleteList& del,
                                        const LocCode& easHubArpt,
                                        const LocCode& easArpt,
                                        const DateTime& date,
                                        const DateTime& ticketDate);

  const std::vector<PfcEssAirSvc*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, PfcEssAirSvcKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("EASHUBARPT", key._a) && objectKey.getValue("EASARPT", key._b);
  }

  PfcEssAirSvcKey createKey(PfcEssAirSvc* info);

  void translateKey(const PfcEssAirSvcKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("EASHUBARPT", key._a);
    objectKey.setValue("EASARPT", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PfcEssAirSvc, PfcEssAirSvcDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<PfcEssAirSvc*>* vect) const override;

  virtual std::vector<PfcEssAirSvc*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcEssAirSvcDAO>;
  virtual size_t clear() override;
  virtual size_t invalidate(const ObjectKey& objectKey) override;
  static DAOHelper<PfcEssAirSvcDAO> _helper;
  PfcEssAirSvcDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PfcEssAirSvcKey, std::vector<PfcEssAirSvc*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;

  std::vector<PfcEssAirSvc*>* create(PfcEssAirSvcKey key) override;
  void destroy(PfcEssAirSvcKey key, std::vector<PfcEssAirSvc*>* t) override;

private:
  struct isEffective;
  static PfcEssAirSvcDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: PfcEssAirSvcHistoricalDAO
// --------------------------------------------------
typedef HashKey<LocCode, LocCode, DateTime, DateTime> PfcEssAirSvcHistoricalKey;

class PfcEssAirSvcHistoricalDAO : public HistoricalDataAccessObject<PfcEssAirSvcHistoricalKey,
                                                                    std::vector<PfcEssAirSvc*>,
                                                                    false>
{
public:
  static PfcEssAirSvcHistoricalDAO& instance();
  const std::vector<PfcEssAirSvc*>& get(DeleteList& del,
                                        const LocCode& easHubArpt,
                                        const LocCode& easArpt,
                                        const DateTime& date,
                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcEssAirSvcHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("EASHUBARPT", key._a) && objectKey.getValue("EASARPT", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PfcEssAirSvcHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("EASHUBARPT", key._a) && objectKey.getValue("EASARPT", key._b);
  }

  void setKeyDateRange(PfcEssAirSvcHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcEssAirSvcHistoricalDAO>;
  static DAOHelper<PfcEssAirSvcHistoricalDAO> _helper;
  PfcEssAirSvcHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PfcEssAirSvcHistoricalKey, std::vector<PfcEssAirSvc*>, false>(
          cacheSize, cacheType)
  {
  }
  std::vector<PfcEssAirSvc*>* create(PfcEssAirSvcHistoricalKey key) override;
  void destroy(PfcEssAirSvcHistoricalKey key, std::vector<PfcEssAirSvc*>* t) override;

  virtual sfc::CompressedData* compress(const std::vector<PfcEssAirSvc*>* vect) const override;

  virtual std::vector<PfcEssAirSvc*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  struct isEffective;
  static PfcEssAirSvcHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
