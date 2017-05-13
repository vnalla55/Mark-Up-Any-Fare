//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
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
class ServiceGroupInfo;
class DeleteList;

typedef HashKey<ServiceGroup> ServiceGroupKey;

class ServiceGroupDAO : public DataAccessObject<ServiceGroupKey, std::vector<ServiceGroupInfo*> >
{
public:
  static ServiceGroupDAO& instance();

  const std::vector<ServiceGroupInfo*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, ServiceGroupKey& key) const override
  {
    key = ServiceGroupKey("");
    return key.initialized;
  }

  ServiceGroupKey createKey(ServiceGroupInfo* info);

  void translateKey(const ServiceGroupKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DUMMY", "");
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ServiceGroupDAO>;
  static DAOHelper<ServiceGroupDAO> _helper;
  ServiceGroupDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ServiceGroupKey, std::vector<ServiceGroupInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<ServiceGroupInfo*>* create(ServiceGroupKey key) override;
  void destroy(ServiceGroupKey key, std::vector<ServiceGroupInfo*>* recs) override;

  void load() override;

private:
  static ServiceGroupDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<ServiceGroup> ServiceGroupHistoricalKey;

class ServiceGroupHistoricalDAO
    : public HistoricalDataAccessObject<ServiceGroupHistoricalKey, std::vector<ServiceGroupInfo*> >
{
public:
  static ServiceGroupHistoricalDAO& instance();

  const std::vector<ServiceGroupInfo*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, ServiceGroupHistoricalKey& key) const override
  {
    key = ServiceGroupHistoricalKey("");
    return key.initialized;
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ServiceGroupHistoricalDAO>;
  static DAOHelper<ServiceGroupHistoricalDAO> _helper;
  ServiceGroupHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ServiceGroupHistoricalKey, std::vector<ServiceGroupInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<ServiceGroupInfo*>* create(ServiceGroupHistoricalKey key) override;
  void destroy(ServiceGroupHistoricalKey key, std::vector<ServiceGroupInfo*>* t) override;

private:
  static ServiceGroupHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
