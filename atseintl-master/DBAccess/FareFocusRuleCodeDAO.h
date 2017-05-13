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
class FareFocusRuleCodeInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusRuleCodeKey;

class FareFocusRuleCodeDAO
    : public DataAccessObject<FareFocusRuleCodeKey, std::vector<FareFocusRuleCodeInfo*> >
{
public:
  static FareFocusRuleCodeDAO& instance();

  const std::vector<FareFocusRuleCodeInfo*>&
  get(DeleteList& del,
      uint64_t ruleCdItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusRuleCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("RULECDITEMNO", key._a);
  }

  FareFocusRuleCodeKey createKey(const FareFocusRuleCodeInfo* info);

  void translateKey(const FareFocusRuleCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("RULECDITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusRuleCodeDAO>;
  static DAOHelper<FareFocusRuleCodeDAO> _helper;
  FareFocusRuleCodeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusRuleCodeKey, std::vector<FareFocusRuleCodeInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusRuleCodeInfo*>* create(FareFocusRuleCodeKey key) override;
  void destroy(FareFocusRuleCodeKey key, std::vector<FareFocusRuleCodeInfo*>* recs) override;

private:
  static FareFocusRuleCodeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusRuleCodeHistoricalKey;

class FareFocusRuleCodeHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusRuleCodeHistoricalKey,
                                        std::vector<FareFocusRuleCodeInfo*> >
{
public:
  static FareFocusRuleCodeHistoricalDAO& instance();

  const std::vector<FareFocusRuleCodeInfo*>&
  get(DeleteList& del, uint64_t ruleCdItemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusRuleCodeHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("RULECDITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusRuleCodeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("RULECDITEMNO", key._a);
  }

  void translateKey(const FareFocusRuleCodeHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("RULECDITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusRuleCodeHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusRuleCodeHistoricalKey createKey(const FareFocusRuleCodeInfo* info,
                                           const DateTime& startDate = DateTime::emptyDate(),
                                           const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusRuleCodeHistoricalDAO>;
  static DAOHelper<FareFocusRuleCodeHistoricalDAO> _helper;

  FareFocusRuleCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusRuleCodeHistoricalKey,
                                 std::vector<FareFocusRuleCodeInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusRuleCodeInfo*>* create(FareFocusRuleCodeHistoricalKey key) override;
  void
  destroy(FareFocusRuleCodeHistoricalKey key, std::vector<FareFocusRuleCodeInfo*>* recs) override;

private:
  static FareFocusRuleCodeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse



