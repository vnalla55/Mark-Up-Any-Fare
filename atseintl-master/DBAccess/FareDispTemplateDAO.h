//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//
#pragma once

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
class FareDispTemplate;
class DeleteList;

typedef HashKey<int, Indicator> FareDispTemplateKey;

class FareDispTemplateDAO
    : public DataAccessObject<FareDispTemplateKey, std::vector<FareDispTemplate*>, false>
{
public:
  static FareDispTemplateDAO& instance();

  const std::vector<FareDispTemplate*>& get(DeleteList& del,
                                            const int& templateID,
                                            const Indicator& templateType,
                                            const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispTemplateKey& key) const override
  {
    return key.initialized = objectKey.getValue("TEMPLATEID", key._a) &&
                             objectKey.getValue("TEMPLATETYPE", key._b);
  }

  FareDispTemplateKey createKey(FareDispTemplate* info);

  void translateKey(const FareDispTemplateKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("TEMPLATEID", key._a);
    objectKey.setValue("TEMPLATETYPE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispTemplate, FareDispTemplateDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispTemplateDAO>;

  static DAOHelper<FareDispTemplateDAO> _helper;

  FareDispTemplateDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispTemplateKey, std::vector<FareDispTemplate*>, false>(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<FareDispTemplate*>* create(FareDispTemplateKey key) override;
  void destroy(FareDispTemplateKey key, std::vector<FareDispTemplate*>* recs) override;

  virtual void load() override;

private:
  static FareDispTemplateDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispTemplateHistoricalDAO
// --------------------------------------------------
class FareDispTemplateHistoricalDAO
    : public HistoricalDataAccessObject<FareDispTemplateKey, std::vector<FareDispTemplate*>, false>
{
public:
  static FareDispTemplateHistoricalDAO& instance();

  const std::vector<FareDispTemplate*>& get(DeleteList& del,
                                            const int& templateID,
                                            const Indicator& templateType,
                                            const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispTemplateKey& key) const override
  {
    return key.initialized = objectKey.getValue("TEMPLATEID", key._a) &&
                             objectKey.getValue("TEMPLATETYPE", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispTemplateHistoricalDAO>;

  static DAOHelper<FareDispTemplateHistoricalDAO> _helper;

  FareDispTemplateHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispTemplateKey, std::vector<FareDispTemplate*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispTemplate*>* create(FareDispTemplateKey key) override;
  void destroy(FareDispTemplateKey key, std::vector<FareDispTemplate*>* recs) override;

private:
  static FareDispTemplateHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

