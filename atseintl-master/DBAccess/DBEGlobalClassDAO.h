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
class DBEGlobalClass;
class DeleteList;

class DBEGlobalClassDAO : public DataAccessObject<DBEClassKey, std::vector<DBEGlobalClass*>, false>
{
public:
  static DBEGlobalClassDAO& instance();

  const std::vector<DBEGlobalClass*>&
  get(DeleteList& del, const DBEClass& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DBEClassKey& key) const override
  {
    return key.initialized = objectKey.getValue("DBEGLOBALCLASS", key._a);
  }

  DBEClassKey createKey(DBEGlobalClass* info);

  void translateKey(const DBEClassKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DBEGLOBALCLASS", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<DBEGlobalClass, DBEGlobalClassDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<DBEGlobalClassDAO>;

  static DAOHelper<DBEGlobalClassDAO> _helper;

  DBEGlobalClassDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<DBEClassKey, std::vector<DBEGlobalClass*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<DBEGlobalClass*>* create(DBEClassKey key) override;
  void destroy(DBEClassKey key, std::vector<DBEGlobalClass*>* recs) override;

  virtual void load() override;

private:
  static DBEGlobalClassDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: DBEGlobalClassHistoricalDAO
// --------------------------------------------------
class DBEGlobalClassHistoricalDAO
    : public HistoricalDataAccessObject<DBEClass, std::vector<DBEGlobalClass*>, false>
{
public:
  static DBEGlobalClassHistoricalDAO& instance();

  const std::vector<DBEGlobalClass*>&
  get(DeleteList& del, const DBEClass& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DBEClass& key) const override
  {
    return objectKey.getValue("DBEGLOBALCLASS", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<DBEGlobalClassHistoricalDAO>;

  static DAOHelper<DBEGlobalClassHistoricalDAO> _helper;

  DBEGlobalClassHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<DBEClass, std::vector<DBEGlobalClass*>, false>(cacheSize,
                                                                                cacheType)
  {
  }

  std::vector<DBEGlobalClass*>* create(DBEClass key) override;
  void destroy(DBEClass key, std::vector<DBEGlobalClass*>* recs) override;

private:
  static DBEGlobalClassHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

