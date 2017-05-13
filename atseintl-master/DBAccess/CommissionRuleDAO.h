#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CommissionRuleInfo;
class DeleteList;

typedef HashKey<VendorCode, uint64_t> CommissionRuleKey;

class CommissionRuleDAO
    : public DataAccessObject<CommissionRuleKey, std::vector<CommissionRuleInfo*> >
{
public:
  static CommissionRuleDAO& instance();

  const std::vector<CommissionRuleInfo*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              uint64_t programId,
                                              DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    CommissionRuleKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("PROGRAMID", key._b);
  }

  CommissionRuleKey createKey(const CommissionRuleInfo* info);

  void translateKey(const CommissionRuleKey& key,
                    ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("PROGRAMID", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CommissionRuleDAO>;
  static DAOHelper<CommissionRuleDAO> _helper;
  CommissionRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CommissionRuleKey, std::vector<CommissionRuleInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<CommissionRuleInfo*>* create(CommissionRuleKey key) override;
  void destroy(CommissionRuleKey key, std::vector<CommissionRuleInfo*>* recs) override;

private:
  static CommissionRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<VendorCode, uint64_t, DateTime, DateTime> CommissionRuleHistoricalKey;

class CommissionRuleHistoricalDAO
    : public HistoricalDataAccessObject<CommissionRuleHistoricalKey,
                                        std::vector<CommissionRuleInfo*> >
{
public:
  static CommissionRuleHistoricalDAO& instance();

  const std::vector<CommissionRuleInfo*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              uint64_t programId,
                                              DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    CommissionRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("PROGRAMID", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CommissionRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("PROGRAMID", key._b);
  }

  void translateKey(const CommissionRuleHistoricalKey& key,
                    ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("PROGRAMID", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  void setKeyDateRange(CommissionRuleHistoricalKey& key,
                       const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  CommissionRuleHistoricalKey createKey(const CommissionRuleInfo* info,
                                        const DateTime& startDate = DateTime::emptyDate(),
                                        const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CommissionRuleHistoricalDAO>;
  static DAOHelper<CommissionRuleHistoricalDAO> _helper;

  CommissionRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CommissionRuleHistoricalKey,
                                 std::vector<CommissionRuleInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<CommissionRuleInfo*>* create(CommissionRuleHistoricalKey key) override;
  void destroy(CommissionRuleHistoricalKey key,
               std::vector<CommissionRuleInfo*>* recs) override;

private:
  static CommissionRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}// tse
