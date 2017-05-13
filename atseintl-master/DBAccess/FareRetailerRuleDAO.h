#pragma once

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
class FareRetailerRuleInfo;
class DeleteList;

typedef HashKey<uint64_t> FareRetailerRuleKey;

class FareRetailerRuleDAO
    : public DataAccessObject<FareRetailerRuleKey, std::vector<FareRetailerRuleInfo*> >
{
public:
  static FareRetailerRuleDAO& instance();

  const FareRetailerRuleInfo*   get(DeleteList& del,
                                    uint64_t fareRetailerRuleId,
                                    DateTime adjustedTicketDate,
                                    DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerRuleKey& key) const override
  {
    return key.initialized = objectKey.getValue("FARERETAILERRULEID", key._a);
  }

  FareRetailerRuleKey createKey(const FareRetailerRuleInfo* info);

  void translateKey(const FareRetailerRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FARERETAILERRULEID", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerRuleDAO>;
  static DAOHelper<FareRetailerRuleDAO> _helper;
  FareRetailerRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareRetailerRuleKey, std::vector<FareRetailerRuleInfo*> >(
          cacheSize, cacheType, 4)
  {
  }
  virtual std::vector<FareRetailerRuleInfo*>* create(FareRetailerRuleKey key) override;
  void destroy(FareRetailerRuleKey key, std::vector<FareRetailerRuleInfo*>* recs) override;

private:
  static FareRetailerRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareRetailerRuleHistoricalKey;

class FareRetailerRuleHistoricalDAO
    : public HistoricalDataAccessObject<FareRetailerRuleHistoricalKey,
                                        std::vector<FareRetailerRuleInfo*> >
{
public:
  static FareRetailerRuleHistoricalDAO& instance();

  const FareRetailerRuleInfo*
  get(DeleteList& del, uint64_t fareRetailerRuleId, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareRetailerRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("FARERETAILERRULEID", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareRetailerRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("FARERETAILERRULEID", key._a);
  }

  void translateKey(const FareRetailerRuleHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FARERETAILERRULEID", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void setKeyDateRange(FareRetailerRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareRetailerRuleHistoricalKey createKey(const FareRetailerRuleInfo* info,
                                          const DateTime& startDate = DateTime::emptyDate(),
                                          const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareRetailerRuleHistoricalDAO>;
  static DAOHelper<FareRetailerRuleHistoricalDAO> _helper;

  FareRetailerRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareRetailerRuleHistoricalKey,
                                 std::vector<FareRetailerRuleInfo*> >(cacheSize, cacheType, 4)
  {
  }
  std::vector<FareRetailerRuleInfo*>* create(FareRetailerRuleHistoricalKey key) override;
  void destroy(FareRetailerRuleHistoricalKey key, std::vector<FareRetailerRuleInfo*>* recs) override;

private:
  static FareRetailerRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse


