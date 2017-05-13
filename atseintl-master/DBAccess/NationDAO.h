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
class Nation;
class DeleteList;

class NationDAO : public DataAccessObject<NationKey, std::vector<Nation*>, false>
{
public:
  static NationDAO& instance();
  const Nation*
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<Nation*>& getAll(DeleteList& del, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NationKey& key) const override
  {
    return key.initialized = objectKey.getValue("NATIONCODE", key._a);
  }

  NationKey createKey(Nation* info);

  void translateKey(const NationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("NATIONCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Nation, NationDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationDAO>;
  static DAOHelper<NationDAO> _helper;
  NationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NationKey, std::vector<Nation*>, false>(cacheSize, cacheType)
  {
  }

  // separately-maintained sorted list
  std::vector<Nation*> loadList;
  TSEReadWriteLock _loadListMutex;

  virtual void load() override;
  virtual size_t clear() override;
  virtual size_t invalidate(const ObjectKey& objectKey) override;
  std::vector<Nation*>* create(NationKey key) override;
  void destroy(NationKey key, std::vector<Nation*>* recs) override;

private:
  struct isEffective;
  static NationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: NationHistoricalDAO
// --------------------------------------------------
class NationHistoricalDAO
    : public HistoricalDataAccessObject<NationCode, std::vector<Nation*>, false>
{
public:
  static NationHistoricalDAO& instance();
  const Nation*
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);
  const std::vector<Nation*>& getAll(DeleteList& del, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, NationCode& key) const override
  {
    return objectKey.getValue("NATIONCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationHistoricalDAO>;
  static DAOHelper<NationHistoricalDAO> _helper;
  NationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NationCode, std::vector<Nation*>, false>(cacheSize, cacheType)
  {
  }
  std::vector<Nation*> loadList;
  TSEReadWriteLock _loadListMutex;

  virtual void load() override;
  virtual size_t clear() override;
  virtual size_t invalidate(const ObjectKey& objectKey) override;
  std::vector<Nation*>* create(NationCode key) override;
  void destroy(NationCode key, std::vector<Nation*>* recs) override;
  void loadNationList();

private:
  struct groupByKey;
  static NationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
