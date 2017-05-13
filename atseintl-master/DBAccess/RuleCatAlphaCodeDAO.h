//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class RuleCatAlphaCode;
class DeleteList;

class RuleCatAlphaCodeDAO : public DataAccessObject<AlphaKey, std::vector<RuleCatAlphaCode*> >
{
public:
  static RuleCatAlphaCodeDAO& instance();
  const std::vector<RuleCatAlphaCode*>&
  get(DeleteList& del, const AlphaCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AlphaKey& key) const override
  {
    return key.initialized = objectKey.getValue("ALPHACODE", key._a);
  }

  AlphaKey createKey(RuleCatAlphaCode* info);

  void translateKey(const AlphaKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ALPHACODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<RuleCatAlphaCode, RuleCatAlphaCodeDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RuleCatAlphaCodeDAO>;
  static DAOHelper<RuleCatAlphaCodeDAO> _helper;
  RuleCatAlphaCodeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AlphaKey, std::vector<RuleCatAlphaCode*> >(cacheSize, cacheType)
  {
  }
  std::vector<RuleCatAlphaCode*>* create(AlphaKey key) override;
  void destroy(AlphaKey key, std::vector<RuleCatAlphaCode*>* recs) override;
  virtual void load() override;

private:
  static RuleCatAlphaCodeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: RuleCatAlphaCodeHistoricalDAO
// --------------------------------------------------
class RuleCatAlphaCodeHistoricalDAO
    : public HistoricalDataAccessObject<AlphaCode, std::vector<RuleCatAlphaCode*> >
{
public:
  static RuleCatAlphaCodeHistoricalDAO& instance();
  const std::vector<RuleCatAlphaCode*>&
  get(DeleteList& del, const AlphaCode& alphaCode, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AlphaCode& key) const override
  {
    return objectKey.getValue("ALPHACODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RuleCatAlphaCodeHistoricalDAO>;
  static DAOHelper<RuleCatAlphaCodeHistoricalDAO> _helper;
  RuleCatAlphaCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AlphaCode, std::vector<RuleCatAlphaCode*> >(cacheSize, cacheType)
  {
  }
  std::vector<RuleCatAlphaCode*>* create(AlphaCode key) override;
  void destroy(AlphaCode key, std::vector<RuleCatAlphaCode*>* recs) override;
  virtual void load() override;

private:
  struct groupByKey;
  static RuleCatAlphaCodeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
