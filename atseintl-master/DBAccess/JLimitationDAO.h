//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
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
class LimitationJrny;
class DeleteList;

class JLimitationDAO : public DataAccessObject<UserApplKey, std::vector<LimitationJrny*>, false>
{
public:
  static const char* _dummyCacheKey;
  static JLimitationDAO& instance();
  const std::vector<LimitationJrny*>&
  get(DeleteList& del, const UserApplCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, UserApplKey& key) const override
  {
    key = UserApplKey(_dummyCacheKey);
    return key.initialized;
  }

  UserApplKey createKey(LimitationJrny* info);

  void translateKey(const UserApplKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DUMMY", _dummyCacheKey);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<JLimitationDAO>;
  static DAOHelper<JLimitationDAO> _helper;
  JLimitationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<UserApplKey, std::vector<LimitationJrny*>, false>(cacheSize, cacheType, 2)
  {
  }
  std::vector<LimitationJrny*>* create(UserApplKey key) override;
  void destroy(UserApplKey key, std::vector<LimitationJrny*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  static JLimitationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: JLimitationHistoricalDAO
// --------------------------------------------------
class JLimitationHistoricalDAO
    : public HistoricalDataAccessObject<UserApplCode, std::vector<LimitationJrny*>, false>
{
public:
  static JLimitationHistoricalDAO& instance();
  const std::vector<LimitationJrny*>&
  get(DeleteList& del, const UserApplCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, UserApplCode& key) const override
  {
    key = "";
    return true;
    // return objectKey.getValue("USERAPPLCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<JLimitationHistoricalDAO>;
  static DAOHelper<JLimitationHistoricalDAO> _helper;
  JLimitationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<UserApplCode, std::vector<LimitationJrny*>, false>(cacheSize,
                                                                                    cacheType)
  {
  }
  std::vector<LimitationJrny*>* create(UserApplCode key) override;
  void destroy(UserApplCode key, std::vector<LimitationJrny*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static JLimitationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
