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
class FareFocusLocationPairInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusLocationPairKey;

class FareFocusLocationPairDAO
    : public DataAccessObject<FareFocusLocationPairKey, std::vector<FareFocusLocationPairInfo*> >
{
public:
  static FareFocusLocationPairDAO& instance();

  const FareFocusLocationPairInfo*
  get(DeleteList& del,
      uint64_t locationPairItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusLocationPairKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCATIONPAIRITEMNO", key._a);
  }

  FareFocusLocationPairKey createKey(const FareFocusLocationPairInfo* info);

  void translateKey(const FareFocusLocationPairKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOCATIONPAIRITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusLocationPairDAO>;
  static DAOHelper<FareFocusLocationPairDAO> _helper;
  FareFocusLocationPairDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusLocationPairKey, std::vector<FareFocusLocationPairInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<FareFocusLocationPairInfo*>* create(FareFocusLocationPairKey key) override;
  void destroy(FareFocusLocationPairKey key, std::vector<FareFocusLocationPairInfo*>* recs) override;

private:
  static FareFocusLocationPairDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusLocationPairHistoricalKey;

class FareFocusLocationPairHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusLocationPairHistoricalKey,
                                        std::vector<FareFocusLocationPairInfo*> >
{
public:
  static FareFocusLocationPairHistoricalDAO& instance();

  const FareFocusLocationPairInfo*
  get(DeleteList& del, uint64_t locationPairItemNo, DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusLocationPairHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("LOCATIONPAIRITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
      FareFocusLocationPairHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("LOCATIONPAIRITEMNO", key._a);
  }

  void translateKey(const FareFocusLocationPairHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOCATIONPAIRITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusLocationPairHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusLocationPairHistoricalKey createKey(const FareFocusLocationPairInfo* info,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusLocationPairHistoricalDAO>;
  static DAOHelper<FareFocusLocationPairHistoricalDAO> _helper;

  FareFocusLocationPairHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusLocationPairHistoricalKey,
                                 std::vector<FareFocusLocationPairInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<FareFocusLocationPairInfo*>* create(FareFocusLocationPairHistoricalKey key) override;
  void
  destroy(FareFocusLocationPairHistoricalKey key, std::vector<FareFocusLocationPairInfo*>* recs) override;

private:
  static FareFocusLocationPairHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
