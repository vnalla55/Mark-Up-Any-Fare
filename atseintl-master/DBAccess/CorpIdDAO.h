//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
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
class CorpId;
class DeleteList;

typedef HashKey<std::string> CorpIdKey;

class CorpIdDAO : public DataAccessObject<CorpIdKey, std::vector<CorpId*> >
{
public:
  static CorpIdDAO& instance();
  const std::vector<CorpId*>& get(DeleteList& del,
                                  const std::string& corpId,
                                  const CarrierCode& carrier,
                                  const DateTime& tvlDate,
                                  const DateTime& ticketDate);

  bool corpIdExists(const std::string& corpId, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CorpIdKey& key) const override
  {
    return key.initialized = objectKey.getValue("CORPID", key._a);
  }

  CorpIdKey createKey(CorpId* info);

  void translateKey(const CorpIdKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CORPID", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CorpId, CorpIdDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CorpIdDAO>;
  static DAOHelper<CorpIdDAO> _helper;
  CorpIdDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CorpIdKey, std::vector<CorpId*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<CorpId*>* create(CorpIdKey key) override;
  void destroy(CorpIdKey key, std::vector<CorpId*>* t) override;

private:
  struct BadCxr;
  static CorpIdDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: CorpIdHistoricalDAO
// --------------------------------------------------
typedef HashKey<std::string, DateTime, DateTime> CorpIdHistoricalKey;

class CorpIdHistoricalDAO
    : public HistoricalDataAccessObject<CorpIdHistoricalKey, std::vector<CorpId*> >
{
public:
  static CorpIdHistoricalDAO& instance();
  const std::vector<CorpId*>& get(DeleteList& del,
                                  const std::string& corpId,
                                  const CarrierCode& carrier,
                                  const DateTime& tvlDate,
                                  const DateTime& ticketDate);

  bool corpIdExists(const std::string& corpId, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CorpIdHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("CORPID", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CorpIdHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CORPID", key._a);
  }

  void setKeyDateRange(CorpIdHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CorpIdHistoricalDAO>;
  static DAOHelper<CorpIdHistoricalDAO> _helper;
  CorpIdHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CorpIdHistoricalKey, std::vector<CorpId*> >(cacheSize, cacheType)
  {
  }
  std::vector<CorpId*>* create(CorpIdHistoricalKey key) override;
  void destroy(CorpIdHistoricalKey key, std::vector<CorpId*>* t) override;

private:
  struct BadCxr;
  static CorpIdHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
