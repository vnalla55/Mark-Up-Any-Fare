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
class LimitationFare;
class DeleteList;

class FCLimitationDAO : public DataAccessObject<UserApplKey, std::vector<LimitationFare*>, false>
{
public:
  static const char* _dummyCacheKey;
  static FCLimitationDAO& instance();

  const std::vector<LimitationFare*>&
  get(DeleteList& del, const UserApplCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, UserApplKey& key) const override
  {
    key = UserApplKey(_dummyCacheKey);
    return key.initialized;
  }

  UserApplKey createKey(LimitationFare* info);

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
  friend class DAOHelper<FCLimitationDAO>;
  static DAOHelper<FCLimitationDAO> _helper;
  FCLimitationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<UserApplKey, std::vector<LimitationFare*>, false>(cacheSize, cacheType, 2)
  {
  }
  std::vector<LimitationFare*>* create(UserApplKey key) override;
  void destroy(UserApplKey key, std::vector<LimitationFare*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  static FCLimitationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FCLimitationHistoricalDAO
// --------------------------------------------------
class FCLimitationHistoricalDAO
    : public HistoricalDataAccessObject<UserApplCode, std::vector<LimitationFare*>, false>
{
public:
  static FCLimitationHistoricalDAO& instance();

  const std::vector<LimitationFare*>&
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
  friend class DAOHelper<FCLimitationHistoricalDAO>;
  static DAOHelper<FCLimitationHistoricalDAO> _helper;
  FCLimitationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<UserApplCode, std::vector<LimitationFare*>, false>(cacheSize,
                                                                                    cacheType)
  {
  }
  std::vector<LimitationFare*>* create(UserApplCode key) override;
  void destroy(UserApplCode key, std::vector<LimitationFare*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static FCLimitationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

