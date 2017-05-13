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
class FareFocusBookingCodeInfo;
class DeleteList;

typedef HashKey<uint64_t> FareFocusBookingCodeKey;

class FareFocusBookingCodeDAO
    : public DataAccessObject<FareFocusBookingCodeKey, std::vector<FareFocusBookingCodeInfo*> >
{
public:
  static FareFocusBookingCodeDAO& instance();

  const std::vector<FareFocusBookingCodeInfo*>&
  get(DeleteList& del,
      uint64_t bookingCodeItemNo,
      DateTime adjustedTicketDate,
      DateTime ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareFocusBookingCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("BOOKINGCDITEMNO", key._a);
  }

  FareFocusBookingCodeKey createKey(const FareFocusBookingCodeInfo* info);

  void translateKey(const FareFocusBookingCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("BOOKINGCDITEMNO", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusBookingCodeDAO>;
  static DAOHelper<FareFocusBookingCodeDAO> _helper;
  FareFocusBookingCodeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareFocusBookingCodeKey, std::vector<FareFocusBookingCodeInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<FareFocusBookingCodeInfo*>* create(FareFocusBookingCodeKey key) override;
  void destroy(FareFocusBookingCodeKey key, std::vector<FareFocusBookingCodeInfo*>* recs) override;

private:
  static FareFocusBookingCodeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<uint64_t, DateTime, DateTime> FareFocusBookingCodeHistoricalKey;

class FareFocusBookingCodeHistoricalDAO
    : public HistoricalDataAccessObject<FareFocusBookingCodeHistoricalKey,
                                        std::vector<FareFocusBookingCodeInfo*> >
{
public:
  static FareFocusBookingCodeHistoricalDAO& instance();

  const std::vector<FareFocusBookingCodeInfo*>&
  get(DeleteList& del, uint64_t bookingCodeItemNo, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, FareFocusBookingCodeHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._b);
    objectKey.getValue("ENDDATE", key._c);
    return key.initialized = objectKey.getValue("BOOKINGCDITEMNO", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareFocusBookingCodeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("BOOKINGCDITEMNO", key._a);
  }

  void
  translateKey(const FareFocusBookingCodeHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("BOOKINGCDITEMNO", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(FareFocusBookingCodeHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareFocusBookingCodeHistoricalKey createKey(const FareFocusBookingCodeInfo* info,
                                              const DateTime& startDate = DateTime::emptyDate(),
                                              const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareFocusBookingCodeHistoricalDAO>;
  static DAOHelper<FareFocusBookingCodeHistoricalDAO> _helper;

  FareFocusBookingCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareFocusBookingCodeHistoricalKey,
                                 std::vector<FareFocusBookingCodeInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<FareFocusBookingCodeInfo*>* create(FareFocusBookingCodeHistoricalKey key) override;
  void destroy(FareFocusBookingCodeHistoricalKey key,
               std::vector<FareFocusBookingCodeInfo*>* recs) override;

private:
  static FareFocusBookingCodeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
