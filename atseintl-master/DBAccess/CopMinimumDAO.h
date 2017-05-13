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
class CopMinimum;
class DeleteList;

class CopMinimumDAO : public DataAccessObject<NationKey, std::vector<CopMinimum*>, false>
{
public:
  static CopMinimumDAO& instance();
  const std::vector<CopMinimum*>&
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NationKey& key) const override
  {
    return key.initialized = objectKey.getValue("NATIONCODE", key._a);
  }

  NationKey createKey(CopMinimum* info);

  void translateKey(const NationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("NATIONCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CopMinimum, CopMinimumDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CopMinimumDAO>;
  static DAOHelper<CopMinimumDAO> _helper;
  CopMinimumDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NationKey, std::vector<CopMinimum*>, false>(cacheSize, cacheType, 2)
  {
  }
  virtual void load() override;
  std::vector<CopMinimum*>* create(NationKey key) override;
  void destroy(NationKey key, std::vector<CopMinimum*>* t) override;

private:
  struct isEffective;
  struct groupByKey;
  static CopMinimumDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: CopMinimumHistoricalDAO
// --------------------------------------------------
class CopMinimumHistoricalDAO
    : public HistoricalDataAccessObject<NationCode, std::vector<CopMinimum*>, false>
{
public:
  static CopMinimumHistoricalDAO& instance();
  const std::vector<CopMinimum*>&
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NationCode& key) const override
  {
    return objectKey.getValue("NATIONCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CopMinimumHistoricalDAO>;
  static DAOHelper<CopMinimumHistoricalDAO> _helper;
  CopMinimumHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NationCode, std::vector<CopMinimum*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<CopMinimum*>* create(NationCode key) override;
  void destroy(NationCode key, std::vector<CopMinimum*>* t) override;

private:
  struct isEffective;
  struct groupByKey;
  static CopMinimumHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
