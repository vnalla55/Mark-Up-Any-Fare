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
class FareFocusRuleInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusRuleKey;

class FareFocusRuleDAO
    : public DataAccessObject<FareFocusRuleKey, std::vector<FareFocusRuleInfo*> >
{
public:
  static FareFocusRuleDAO& instance();

  const FareFocusRuleInfo*
  get(DeleteList& del,
      uint64_t fareFocusRuleId,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusRuleKey& key) const override
  {
    return key.initialized = objectKey.getValue("FAREFOCUSRULEID", key._a);
  }

  FareFocusRuleKey createKey(const FareFocusRuleInfo* info);

  void translateKey(const FareFocusRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FAREFOCUSRULEID", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
    compress(const std::vector<FareFocusRuleInfo*>* vect) const override;

  virtual std::vector<FareFocusRuleInfo*>*
    uncompress(const sfc::CompressedData& compressed) const override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusRuleDAO>;
  static DAOHelper<FareFocusRuleDAO> _helper;
  FareFocusRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusRuleKey, std::vector<FareFocusRuleInfo*> >(
          cacheSize, cacheType, 4)
  {
  }
  virtual std::vector<FareFocusRuleInfo*>* create(FareFocusRuleKey key) override;
  void destroy(FareFocusRuleKey key, std::vector<FareFocusRuleInfo*>* recs) override;

private:
  static FareFocusRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusRuleHistoricalKey;

class FareFocusRuleHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusRuleHistoricalKey,
                                        std::vector<FareFocusRuleInfo*> >
{
public:
  static FareFocusRuleHistoricalDAO& instance();

  const FareFocusRuleInfo*
  get(DeleteList& del, uint64_t fareFocusRuleId, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("FAREFOCUSRULEID", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("FAREFOCUSRULEID", key._a);
  }

  void translateKey(const FareFocusRuleHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("FAREFOCUSRULEID", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void setKeyDateRange(FareFocusRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusRuleHistoricalKey createKey(const FareFocusRuleInfo* info,
                                       const DateTime& startDate = DateTime::emptyDate(),
                                       const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
    compress(const std::vector<FareFocusRuleInfo*>* vect) const override;

  virtual std::vector<FareFocusRuleInfo*>*
    uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusRuleHistoricalDAO>;
  static DAOHelper<FareFocusRuleHistoricalDAO> _helper;

  FareFocusRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusRuleHistoricalKey,
                                 std::vector<FareFocusRuleInfo*> >(cacheSize, cacheType, 4)
  {
  }
  std::vector<FareFocusRuleInfo*>* create(FareFocusRuleHistoricalKey key) override;
  void destroy(FareFocusRuleHistoricalKey key, std::vector<FareFocusRuleInfo*>* recs) override;

private:
  static FareFocusRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
