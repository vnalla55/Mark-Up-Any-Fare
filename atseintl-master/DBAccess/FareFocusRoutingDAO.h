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
class FareFocusRoutingInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusRoutingKey;

class FareFocusRoutingDAO
    : public DataAccessObject<FareFocusRoutingKey, std::vector<FareFocusRoutingInfo*> >
{
public:
  static FareFocusRoutingDAO& instance();

  const FareFocusRoutingInfo*
  get(DeleteList& del,
      uint64_t routingItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusRoutingKey& key) const override
  {
    return key.initialized = objectKey.getValue("ROUTINGITEMNO", key._a);
  }

  FareFocusRoutingKey createKey(const FareFocusRoutingInfo* info);

  void translateKey(const FareFocusRoutingKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ROUTINGITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusRoutingDAO>;
  static DAOHelper<FareFocusRoutingDAO> _helper;
  FareFocusRoutingDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusRoutingKey, std::vector<FareFocusRoutingInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusRoutingInfo*>* create(FareFocusRoutingKey key) override;
  void destroy(FareFocusRoutingKey key, std::vector<FareFocusRoutingInfo*>* recs) override;

private:
  static FareFocusRoutingDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusRoutingHistoricalKey;

class FareFocusRoutingHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusRoutingHistoricalKey,
                                        std::vector<FareFocusRoutingInfo*> >
{
public:
  static FareFocusRoutingHistoricalDAO& instance();

  const FareFocusRoutingInfo*
  get(DeleteList& del, uint64_t routingItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusRoutingHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("ROUTINGITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusRoutingHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("ROUTINGITEMNO", key._a);
  }

  void translateKey(const FareFocusRoutingHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ROUTINGITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusRoutingHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusRoutingHistoricalKey createKey(const FareFocusRoutingInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusRoutingHistoricalDAO>;
  static DAOHelper<FareFocusRoutingHistoricalDAO> _helper;

  FareFocusRoutingHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusRoutingHistoricalKey,
                                 std::vector<FareFocusRoutingInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusRoutingInfo*>* create(FareFocusRoutingHistoricalKey key) override;
  void
  destroy(FareFocusRoutingHistoricalKey key, std::vector<FareFocusRoutingInfo*>* recs) override;

private:
  static FareFocusRoutingHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
