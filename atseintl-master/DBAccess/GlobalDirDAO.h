//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

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
class GlobalDir;
class GlobalDirSeg;
class DeleteList;

class GlobalDirDAO : public DataAccessObject<IntKey, std::vector<GlobalDir*>, false>
{
public:
  static GlobalDirDAO& instance();
  const GlobalDir* get(DeleteList& del,
                       const GlobalDirection& key,
                       const DateTime& date,
                       const DateTime& ticketDate);
  const std::vector<GlobalDirSeg*>&
  getAll(DeleteList& del, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    key = IntKey(0);
    return key.initialized;
  }

  IntKey createKey(GlobalDir* info);

  void translateKey(const IntKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DUMMY", 0);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GlobalDirDAO>;
  static DAOHelper<GlobalDirDAO> _helper;
  GlobalDirDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, std::vector<GlobalDir*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;
  std::vector<GlobalDir*>* create(IntKey key) override;
  void destroy(IntKey key, std::vector<GlobalDir*>* t) override;

private:
  static GlobalDirDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: GlobalDirHistoricalDAO
// --------------------------------------------------
class GlobalDirHistoricalDAO
    : public HistoricalDataAccessObject<int, std::vector<GlobalDir*>, false>
{
public:
  static GlobalDirHistoricalDAO& instance();
  const GlobalDir* get(DeleteList& del,
                       const GlobalDirection& key,
                       const DateTime& date,
                       const DateTime& ticketDate);
  const std::vector<GlobalDirSeg*>&
  getAll(DeleteList& del, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, int& key) const override
  {
    key = dummy;
    return true;
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GlobalDirHistoricalDAO>;
  static DAOHelper<GlobalDirHistoricalDAO> _helper;
  GlobalDirHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<int, std::vector<GlobalDir*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<GlobalDir*>* create(int key) override;
  void destroy(int key, std::vector<GlobalDir*>* t) override;

  virtual void load() override;

private:
  struct groupByKey;
  static const int dummy = 0;
  static GlobalDirHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
