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
class LimitationCmn;
class DeleteList;

class PULimitationDAO : public DataAccessObject<UserApplKey, std::vector<LimitationCmn*>, false>
{
public:
  static const char* _dummyCacheKey;
  static PULimitationDAO& instance();
  const std::vector<LimitationCmn*>&
  get(DeleteList& del, const UserApplCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, UserApplKey& key) const override
  {
    key = UserApplKey(_dummyCacheKey);
    return key.initialized;
  }

  UserApplKey createKey(LimitationCmn* info);

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
  friend class DAOHelper<PULimitationDAO>;
  static DAOHelper<PULimitationDAO> _helper;
  PULimitationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<UserApplKey, std::vector<LimitationCmn*>, false>(cacheSize, cacheType, 2)
  {
  }
  std::vector<LimitationCmn*>* create(UserApplKey key) override;
  void destroy(UserApplKey key, std::vector<LimitationCmn*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  static PULimitationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: PULimitationHistoricalDAO
// --------------------------------------------------
class PULimitationHistoricalDAO
    : public HistoricalDataAccessObject<UserApplCode, std::vector<LimitationCmn*>, false>
{
public:
  static PULimitationHistoricalDAO& instance();
  const std::vector<LimitationCmn*>&
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
  friend class DAOHelper<PULimitationHistoricalDAO>;
  static DAOHelper<PULimitationHistoricalDAO> _helper;
  PULimitationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<UserApplCode, std::vector<LimitationCmn*>, false>(cacheSize,
                                                                                   cacheType)
  {
  }
  std::vector<LimitationCmn*>* create(UserApplCode key) override;
  void destroy(UserApplCode key, std::vector<LimitationCmn*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static PULimitationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
