//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
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
class TpdPsr;
class DeleteList;

typedef HashKey<Indicator, CarrierCode, Indicator, Indicator> TpdPsrKey;

class TpdPsrDAO : public DataAccessObject<TpdPsrKey, std::vector<TpdPsr*>, false>
{
public:
  static TpdPsrDAO& instance();
  const std::vector<TpdPsr*>& get(DeleteList& del,
                                  Indicator applInd,
                                  const CarrierCode& carrier,
                                  Indicator area1,
                                  Indicator area2,
                                  const DateTime& date,
                                  const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TpdPsrKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("APPLIND", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("AREA1", key._c) && objectKey.getValue("AREA2", key._d);
  }

  TpdPsrKey createKey(TpdPsr* info);

  void translateKey(const TpdPsrKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("APPLIND", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("AREA1", key._c);
    objectKey.setValue("AREA2", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TpdPsr, TpdPsrDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TpdPsr*>* vect) const override;

  virtual std::vector<TpdPsr*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TpdPsrDAO>;
  static DAOHelper<TpdPsrDAO> _helper;
  TpdPsrDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TpdPsrKey, std::vector<TpdPsr*>, false>(cacheSize, cacheType, 3)
  {
  }

  std::vector<TpdPsr*>* create(TpdPsrKey key) override;
  void destroy(TpdPsrKey key, std::vector<TpdPsr*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  static TpdPsrDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TpdPsrHistoricalDAO
// --------------------------------------------------
typedef HashKey<Indicator, CarrierCode, Indicator, Indicator, DateTime, DateTime>
TpdPsrHistoricalKey;

class TpdPsrHistoricalDAO
    : public HistoricalDataAccessObject<TpdPsrHistoricalKey, std::vector<TpdPsr*>, false>
{
public:
  static TpdPsrHistoricalDAO& instance();
  const std::vector<TpdPsr*>& get(DeleteList& del,
                                  Indicator applInd,
                                  const CarrierCode& carrier,
                                  Indicator area1,
                                  Indicator area2,
                                  const DateTime& date,
                                  const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TpdPsrHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("APPLIND", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("AREA1", key._c) && objectKey.getValue("AREA2", key._d) &&
               objectKey.getValue("STARTDATE", key._e) && objectKey.getValue("ENDDATE", key._f);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TpdPsrHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized =
               objectKey.getValue("APPLIND", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("AREA1", key._c) && objectKey.getValue("AREA2", key._d);
  }

  void setKeyDateRange(TpdPsrHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TpdPsr*>* vect) const override;

  virtual std::vector<TpdPsr*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TpdPsrHistoricalDAO>;
  static DAOHelper<TpdPsrHistoricalDAO> _helper;
  TpdPsrHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TpdPsrHistoricalKey, std::vector<TpdPsr*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<TpdPsr*>* create(TpdPsrHistoricalKey key) override;
  void destroy(TpdPsrHistoricalKey key, std::vector<TpdPsr*>* recs) override;

private:
  struct isEffective;
  static TpdPsrHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
